#include "schema_store.h"

#include <cstdio>
#include <limits>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <utility>

namespace {

    constexpr std::uint32_t FORMAT_VERSION = 1;
    constexpr char FILE_MAGIC[4] = { 'S', 'C', 'B', '8' };

    std::uint64_t getFileSize(const std::filesystem::path& path) {
        return std::filesystem::file_size(path);
    }

    std::string csvEscape(const std::string& value) {
        bool quote = value.find_first_of(",\"\r\n") != std::string::npos;

        if (!quote)
            return value;

        std::string result = "\"";

        for (char c : value) {
            if (c == '"')
                result += "\"\"";
            else
                result += c;
        }

        result += '"';
        return result;
    }

    std::vector<std::string> parseCsvLine(const std::string& line) {
        std::vector<std::string> fields;
        std::string value;
        bool quoted = false;

        for (std::size_t i = 0; i < line.size(); ++i) {
            char c = line[i];

            if (quoted) {
                if (c == '"') {
                    if (i + 1 < line.size() && line[i + 1] == '"') {
                        value += '"';
                        ++i;
                    }
                    else {
                        quoted = false;
                    }
                }
                else {
                    value += c;
                }
            }
            else {
                if (c == ',') {
                    fields.push_back(value);
                    value.clear();
                }
                else if (c == '"' && value.empty()) {
                    quoted = true;
                }
                else {
                    value += c;
                }
            }
        }

        if (quoted)
            throw std::runtime_error("Незакриті лапки у CSV");

        fields.push_back(value);
        return fields;
    }

    std::vector<std::size_t> allColumns(const Schema& schema) {
        std::vector<std::size_t> result;

        for (std::size_t i = 0; i < schema.size(); ++i)
            result.push_back(i);

        return result;
    }

}

SchemaStore::~SchemaStore() {
    close();
}

void SchemaStore::create(
    const std::filesystem::path& path,
    const Schema& schema,
    const DataTable& rows
) {
    if (schema.size() == 0)
        throw std::runtime_error("Неможливо створити файл без схеми");

    if (rows.size() > UINT32_MAX)
        throw std::runtime_error("Забагато рядків");

    std::filesystem::path tempPath = path;
    tempPath += ".tmp";

    try {
        std::ofstream out(
            tempPath,
            std::ios::binary | std::ios::trunc
        );

        if (!out)
            throw std::runtime_error("Не вдалося створити BIN-файл");

        out.write(FILE_MAGIC, 4);
        writeU32(out, FORMAT_VERSION);
        writeString(out, schema.text());
        writeU64(out, static_cast<std::uint64_t>(rows.size()));

        for (std::size_t r = 0; r < rows.size(); ++r) {
            if (rows[r].size() != schema.size()) {
                throw std::runtime_error(
                    "Кількість полів у рядку " +
                    std::to_string(r + 1) + " не відповідає схемі"
                );
            }

            for (std::size_t c = 0; c < schema.size(); ++c) {
                try {
                    schema.column(c).write(out, rows[r][c]);
                }
                catch (const std::exception& e) {
                    throw std::runtime_error(
                        "Рядок " + std::to_string(r + 1) +
                        ", поле " + schema.info(c).name +
                        ": " + e.what()
                    );
                }
            }
        }

        out.flush();

        if (!out)
            throw std::runtime_error("Помилка запису BIN-файлу");

        out.close();

        std::uint64_t sizeBeforeCrc = getFileSize(tempPath);
        std::uint32_t checksum = crc32File(tempPath, sizeBeforeCrc);

        std::ofstream append(
            tempPath,
            std::ios::binary | std::ios::app
        );

        writeU32(append, checksum);
        append.flush();

        if (!append)
            throw std::runtime_error("Помилка запису CRC-32");

        append.close();

        close();

        std::error_code ec;
        std::filesystem::rename(tempPath, path, ec);

        if (ec) {
            std::filesystem::remove(path, ec);
            ec.clear();
            std::filesystem::rename(tempPath, path, ec);
        }

        if (ec)
            throw std::runtime_error(
                "Не вдалося замінити файл: " + ec.message()
            );

        open(path);
    }
    catch (...) {
        std::error_code ec;
        std::filesystem::remove(tempPath, ec);
        throw;
    }
}

void SchemaStore::open(const std::filesystem::path& path) {
    close();

    path_ = path;

    std::uint64_t fileSize = getFileSize(path);

    if (fileSize < 20)
        throw std::runtime_error("Файл занадто малий або пошкоджений");

    file_.open(path, std::ios::binary);

    if (!file_)
        throw std::runtime_error("Не вдалося відкрити BIN-файл");

    char magic[4]{};
    file_.read(magic, 4);

    if (!file_ || std::string(magic, 4) != std::string(FILE_MAGIC, 4)) {
        close();
        throw std::runtime_error(
            "Невірна сигнатура файлу. Це не формат SCB8"
        );
    }

    std::uint32_t version = readU32(file_);

    if (version != FORMAT_VERSION) {
        close();
        throw std::runtime_error(
            "Непідтримувана версія BIN-файлу"
        );
    }

    std::string schemaText = readString(file_);
    schema_.parse(schemaText);

    rowCount_ = readU64(file_);

    dataStart_ = static_cast<std::uint64_t>(file_.tellg());

    crcOffset_ = fileSize - 4;

    if (dataStart_ > crcOffset_) {
        close();
        throw std::runtime_error("Пошкоджений заголовок BIN-файлу");
    }

    verify();
    buildIndex();
}

void SchemaStore::close() {
    if (file_.is_open())
        file_.close();

    rowOffsets_.clear();
    rowCount_ = 0;
    dataStart_ = 0;
    crcOffset_ = 0;
}

bool SchemaStore::isOpen() const {
    return file_.is_open();
}

const Schema& SchemaStore::schema() const {
    return schema_;
}

std::uint64_t SchemaStore::rowCount() const {
    return rowCount_;
}

void SchemaStore::verify() const {
    if (!isOpen())
        throw std::runtime_error("Файл не відкритий");

    std::uint32_t actual = crc32File(path_, crcOffset_);

    file_.clear();
    file_.seekg(static_cast<std::streamoff>(crcOffset_));

    std::uint32_t expected = readU32(file_);

    if (actual != expected)
        throw std::runtime_error(
            "CRC-32 не збігається: файл пошкоджено"
        );

    file_.clear();
}

void SchemaStore::skipValue(
    std::istream& in,
    const Column& column
) const {
    column.read(in);
}

void SchemaStore::buildIndex() {
    rowOffsets_.clear();

    file_.clear();
    file_.seekg(static_cast<std::streamoff>(dataStart_));

    for (std::uint64_t row = 0; row < rowCount_; ++row) {
        std::uint64_t offset =
            static_cast<std::uint64_t>(file_.tellg());

        rowOffsets_.push_back(offset);

        for (std::size_t col = 0; col < schema_.size(); ++col)
            skipValue(file_, schema_.column(col));

        if (!file_)
            throw std::runtime_error(
                "Пошкоджені дані у рядку " +
                std::to_string(row + 1)
            );

        if (static_cast<std::uint64_t>(file_.tellg()) > crcOffset_)
            throw std::runtime_error("Дані виходять за межі файлу");
    }

    if (static_cast<std::uint64_t>(file_.tellg()) != crcOffset_)
        throw std::runtime_error(
            "Розмір даних не відповідає кількості рядків"
        );

    file_.clear();
}

std::vector<std::size_t> SchemaStore::resolveColumns(
    const std::vector<std::string>& names
) const {
    if (names.empty())
        return allColumns(schema_);

    std::vector<std::size_t> result;

    for (const std::string& name : names) {
        int index = schema_.findColumn(name);

        if (index < 0)
            throw std::runtime_error("Невідомий стовпець: " + name);

        result.push_back(static_cast<std::size_t>(index));
    }

    return result;
}

DataRow SchemaStore::readRow(std::uint64_t rowIndex) const {
    return readSelected(rowIndex, {});
}

DataRow SchemaStore::readSelected(
    std::uint64_t rowIndex,
    const std::vector<std::string>& selectedColumns
) const {
    if (!isOpen())
        throw std::runtime_error("Файл не відкритий");

    if (rowIndex >= rowCount_)
        throw std::runtime_error("Номер рядка поза межами таблиці");

    std::vector<std::size_t> selected =
        resolveColumns(selectedColumns);

    DataRow result;
    result.reserve(selected.size());

    file_.clear();
    file_.seekg(
        static_cast<std::streamoff>(rowOffsets_[rowIndex])
    );

    for (std::size_t c = 0; c < schema_.size(); ++c) {
        CellValue value = schema_.column(c).read(file_);

        auto it = std::find(selected.begin(), selected.end(), c);

        if (it != selected.end()) {
            result.push_back(std::move(value));
        }
    }

    if (!file_)
        throw std::runtime_error("Помилка читання рядка");

    if (selected.size() != result.size())
        throw std::runtime_error("Не вдалося прочитати вибрані поля");

    return result;
} 

DataTable SchemaStore::readRange(
    std::uint64_t firstRow,
    std::uint64_t count,
    const std::vector<std::string>& selectedColumns
) const {
    if (firstRow > rowCount_)
        throw std::runtime_error("Початковий рядок поза межами таблиці");

    std::uint64_t end = std::min(
        rowCount_,
        firstRow + std::min(count, rowCount_ - firstRow)
    );

    DataTable result;

    for (std::uint64_t i = firstRow; i < end; ++i)
        result.push_back(readSelected(i, selectedColumns));

    return result;
}

void SchemaStore::exportCsv(
    const std::filesystem::path& csvPath,
    std::uint64_t firstRow,
    std::uint64_t count,
    const std::vector<std::string>& selectedColumns
) const {
    std::vector<std::size_t> selected =
        resolveColumns(selectedColumns);

    std::ofstream out(csvPath, std::ios::binary | std::ios::trunc);

    if (!out)
        throw std::runtime_error("Не вдалося створити CSV-файл");

    std::uint64_t end = std::min(
        rowCount_,
        firstRow + std::min(count, rowCount_ - firstRow)
    );

    for (std::uint64_t r = firstRow; r < end; ++r) {
        DataRow row = readRow(r);

        for (std::size_t i = 0; i < selected.size(); ++i) {
            if (i > 0)
                out << ',';

            std::size_t columnIndex = selected[i];

            out << csvEscape(
                schema_.column(columnIndex).toText(row[columnIndex])
            );
        }

        out << "\n";
    }

    if (!out)
        throw std::runtime_error("Помилка експорту CSV");
}

void SchemaStore::importCsv(
    const std::filesystem::path& csvPath,
    const std::filesystem::path& binPath,
    const Schema& schema
) {
    std::ifstream in(csvPath, std::ios::binary);

    if (!in)
        throw std::runtime_error("Не вдалося відкрити CSV-файл");

    DataTable rows;
    std::string line;
    std::size_t lineNumber = 0;

    while (std::getline(in, line)) {
        ++lineNumber;

        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.empty())
            continue;

        try {
            std::vector<std::string> fields = parseCsvLine(line);

            if (fields.size() != schema.size()) {
                throw std::runtime_error(
                    "Кількість полів не відповідає схемі"
                );
            }

            DataRow row;

            for (std::size_t c = 0; c < schema.size(); ++c) {
                row.push_back(schema.column(c).parse(fields[c]));
            }

            rows.push_back(std::move(row));
        }
        catch (const std::exception& e) {
            throw std::runtime_error(
                "Помилка CSV у рядку " +
                std::to_string(lineNumber) + ": " + e.what()
            );
        }
    }

    create(binPath, schema, rows);
}