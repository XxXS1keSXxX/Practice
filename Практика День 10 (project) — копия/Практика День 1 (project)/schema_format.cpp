#include "schema_format.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <limits>
#include <sstream>
#include <iomanip>
#include <cctype>

namespace {

    std::string trimText(const std::string& text) {
        std::size_t first = text.find_first_not_of(" \t\r\n");

        if (first == std::string::npos)
            return "";

        std::size_t last = text.find_last_not_of(" \t\r\n");

        return text.substr(first, last - first + 1);
    }

    std::string lowerText(std::string text) {
        for (char& c : text) {
            c = static_cast<char>(
                std::tolower(static_cast<unsigned char>(c))
                );
        }

        return text;
    }

    template<typename T>
    T parseInteger(const std::string& text, const std::string& field) {
        T value{};

        auto result = std::from_chars(
            text.data(),
            text.data() + text.size(),
            value
        );

        if (result.ec != std::errc{} ||
            result.ptr != text.data() + text.size()) {
            throw std::runtime_error(
                "Поле " + field + " має неправильне ціле значення: " + text
            );
        }

        return value;
    }

    std::uint16_t readU16(std::istream& in) {
        std::uint16_t value = readU8(in);
        value |= static_cast<std::uint16_t>(readU8(in)) << 8;
        return value;
    }

    void writeU16(std::ostream& out, std::uint16_t value) {
        writeU8(out, static_cast<std::uint8_t>(value));
        writeU8(out, static_cast<std::uint8_t>(value >> 8));
    }

    bool validDate(const DateValue& date) {
        if (date.year < 1 || date.month < 1 || date.month > 12)
            return false;

        static const int daysInMonth[] = {
            31, 28, 31, 30, 31, 30,
            31, 31, 30, 31, 30, 31
        };

        int days = daysInMonth[date.month - 1];

        bool leap =
            date.year % 400 == 0 ||
            (date.year % 4 == 0 && date.year % 100 != 0);

        if (date.month == 2 && leap)
            days = 29;

        return date.day >= 1 && date.day <= days;
    }

}

Column::Column(std::string name)
    : name_(std::move(name)) {}

const std::string& Column::name() const {
    return name_;
}

ColumnType IntColumn::type() const {
    return ColumnType::Int32;
}

CellValue IntColumn::parse(const std::string& text) const {
    return parseInteger<std::int32_t>(text, name_);
}

void IntColumn::write(
    std::ostream& out,
    const CellValue& value
) const {
    writeU32(
        out,
        static_cast<std::uint32_t>(
            std::get<std::int32_t>(value)
            )
    );
}

CellValue IntColumn::read(std::istream& in) const {
    return static_cast<std::int32_t>(readU32(in));
}

std::string IntColumn::toText(const CellValue& value) const {
    return std::to_string(std::get<std::int32_t>(value));
}

ColumnType Int64Column::type() const {
    return ColumnType::Int64;
}

CellValue Int64Column::parse(const std::string& text) const {
    return parseInteger<std::int64_t>(text, name_);
}

void Int64Column::write(
    std::ostream& out,
    const CellValue& value
) const {
    writeU64(
        out,
        static_cast<std::uint64_t>(
            std::get<std::int64_t>(value)
            )
    );
}

CellValue Int64Column::read(std::istream& in) const {
    return static_cast<std::int64_t>(readU64(in));
}

std::string Int64Column::toText(const CellValue& value) const {
    return std::to_string(std::get<std::int64_t>(value));
}

ColumnType DoubleColumn::type() const {
    return ColumnType::Double;
}

CellValue DoubleColumn::parse(const std::string& text) const {
    try {
        std::size_t used = 0;
        double value = std::stod(text, &used);

        if (used != text.size() || !std::isfinite(value))
            throw std::runtime_error("");

        return value;
    }
    catch (...) {
        throw std::runtime_error(
            "Поле " + name_ + " має неправильне значення double: " + text
        );
    }
}

void DoubleColumn::write(
    std::ostream& out,
    const CellValue& value
) const {
    writeDouble(out, std::get<double>(value));
}

CellValue DoubleColumn::read(std::istream& in) const {
    return readDouble(in);
}

std::string DoubleColumn::toText(const CellValue& value) const {
    std::ostringstream out;
    out << std::setprecision(15) << std::get<double>(value);
    return out.str();
}

ColumnType BoolColumn::type() const {
    return ColumnType::Bool;
}

CellValue BoolColumn::parse(const std::string& text) const {
    std::string value = lowerText(trimText(text));

    if (value == "true" || value == "1")
        return true;

    if (value == "false" || value == "0")
        return false;

    throw std::runtime_error(
        "Поле " + name_ + " має бути true або false"
    );
}

void BoolColumn::write(
    std::ostream& out,
    const CellValue& value
) const {
    writeU8(out, std::get<bool>(value) ? 1 : 0);
}

CellValue BoolColumn::read(std::istream& in) const {
    std::uint8_t value = readU8(in);

    if (value > 1)
        throw std::runtime_error("Некоректне значення bool");

    return value != 0;
}

std::string BoolColumn::toText(const CellValue& value) const {
    return std::get<bool>(value) ? "true" : "false";
}

ColumnType StringColumn::type() const {
    return ColumnType::String;
}

CellValue StringColumn::parse(const std::string& text) const {
    return text;
}

void StringColumn::write(
    std::ostream& out,
    const CellValue& value
) const {
    writeString(out, std::get<std::string>(value));
}

CellValue StringColumn::read(std::istream& in) const {
    return readString(in);
}

std::string StringColumn::toText(const CellValue& value) const {
    return std::get<std::string>(value);
}

ColumnType DateColumn::type() const {
    return ColumnType::Date;
}

CellValue DateColumn::parse(const std::string& text) const {
    DateValue date;

    char first = 0;
    char second = 0;

    std::istringstream in(text);

    int year = 0;
    int month = 0;
    int day = 0;

    in >> year >> first >> month >> second >> day;

    if (!in || first != '-' || second != '-' || in.peek() != EOF)
        throw std::runtime_error(
            "Поле " + name_ + " має формат YYYY-MM-DD"
        );

    if (year < 1 || year > 65535 ||
        month < 1 || month > 12 ||
        day < 1 || day > 31) {
        throw std::runtime_error("Некоректна дата");
    }

    date.year = static_cast<std::uint16_t>(year);
    date.month = static_cast<std::uint8_t>(month);
    date.day = static_cast<std::uint8_t>(day);

    if (!validDate(date))
        throw std::runtime_error("Такої календарної дати не існує");

    return date;
}

void DateColumn::write(
    std::ostream& out,
    const CellValue& value
) const {
    const DateValue& date = std::get<DateValue>(value);

    writeU16(out, date.year);
    writeU8(out, date.month);
    writeU8(out, date.day);
}

CellValue DateColumn::read(std::istream& in) const {
    DateValue date;

    date.year = readU16(in);
    date.month = readU8(in);
    date.day = readU8(in);

    if (!validDate(date))
        throw std::runtime_error("Пошкоджена дата у BIN-файлі");

    return date;
}

std::string DateColumn::toText(const CellValue& value) const {
    const DateValue& date = std::get<DateValue>(value);

    std::ostringstream out;

    out << std::setfill('0')
        << std::setw(4) << date.year << "-"
        << std::setw(2) << static_cast<int>(date.month) << "-"
        << std::setw(2) << static_cast<int>(date.day);

    return out.str();
}

Schema::Schema(const std::string& text) {
    parse(text);
}

void Schema::parse(const std::string& text) {
    text_ = trimText(text);
    info_.clear();
    columns_.clear();

    if (text_.empty())
        throw std::runtime_error("Схема не може бути порожньою");

    std::istringstream input(text_);
    std::string item;

    while (std::getline(input, item, ',')) {
        item = trimText(item);

        std::size_t colon = item.find(':');

        if (colon == std::string::npos)
            throw std::runtime_error("Неправильний опис поля: " + item);

        std::string name = trimText(item.substr(0, colon));
        std::string typeName = lowerText(
            trimText(item.substr(colon + 1))
        );

        if (name.empty() || typeName.empty())
            throw std::runtime_error("Порожня назва поля або тип");

        for (const auto& field : info_) {
            if (lowerText(field.name) == lowerText(name))
                throw std::runtime_error("Повторне поле: " + name);
        }

        std::unique_ptr<Column> column;

        if (typeName == "int32") {
            column = std::make_unique<IntColumn>(name);
        }
        else if (typeName == "int64") {
            column = std::make_unique<Int64Column>(name);
        }
        else if (typeName == "double") {
            column = std::make_unique<DoubleColumn>(name);
        }
        else if (typeName == "bool") {
            column = std::make_unique<BoolColumn>(name);
        }
        else if (typeName == "string") {
            column = std::make_unique<StringColumn>(name);
        }
        else if (typeName == "date") {
            column = std::make_unique<DateColumn>(name);
        }
        else {
            throw std::runtime_error("Невідомий тип поля: " + typeName);
        }

        info_.push_back({ name, column->type() });
        columns_.push_back(std::move(column));
    }

    if (columns_.empty())
        throw std::runtime_error("Схема не містить полів");
}

const std::string& Schema::text() const {
    return text_;
}

std::size_t Schema::size() const {
    return columns_.size();
}

const Column& Schema::column(std::size_t index) const {
    return *columns_.at(index);
}

const ColumnInfo& Schema::info(std::size_t index) const {
    return info_.at(index);
}

int Schema::findColumn(const std::string& name) const {
    std::string target = lowerText(name);

    for (std::size_t i = 0; i < info_.size(); ++i) {
        if (lowerText(info_[i].name) == target)
            return static_cast<int>(i);
    }

    return -1;
}

std::vector<std::string> Schema::columnNames() const {
    std::vector<std::string> names;

    for (const auto& field : info_)
        names.push_back(field.name);

    return names;
}

std::string cellToText(
    const Column& column,
    const CellValue& value
) {
    return column.toText(value);
}