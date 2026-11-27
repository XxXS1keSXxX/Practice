#include "tests8.h"
#include "schema_store.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <vector>

using namespace std;
using namespace std::chrono;

namespace {

    bool testPassed(const string& name, bool result) {
        cout << (result ? "[PASS] " : "[FAIL] ")
            << name << "\n";

        return result;
    }

    DataRow makeTestRow(
        const Schema& schema,
        const string& id,
        const string& name,
        const string& grade
    ) {
        DataRow row;

        row.push_back(schema.column(0).parse(id));
        row.push_back(schema.column(1).parse(name));
        row.push_back(schema.column(2).parse(grade));

        return row;
    }

}

void runTests8() {
    cout << "\n===== АВТОМАТИЧНІ ТЕСТИ ДНЯ 8 =====\n";

    int passed = 0;
    int total = 0;

    auto check = [&](const string& name, bool result) {
        ++total;

        if (testPassed(name, result))
            ++passed;
        };

    const string crcText = "123456789";

    std::uint32_t crc = crc32(
        reinterpret_cast<const unsigned char*>(crcText.data()),
        crcText.size()
    );

    check("CRC-32 контрольного рядка", crc == 0xCBF43926u);

    Schema schema("id:int32,name:string,grade:double");

    DataTable rows;

    rows.push_back(makeTestRow(schema, "1", "Оксана", "95.5"));
    rows.push_back(makeTestRow(schema, "2", "Іван", "80"));

    const filesystem::path binPath = "day8_test.bin";
    const filesystem::path csvPath = "day8_test.csv";
    const filesystem::path badPath = "day8_corrupted.bin";

    try {
        SchemaStore store;

        store.create(binPath, schema, rows);

        check("Створення BIN-файлу", filesystem::exists(binPath));

        store.close();

        store.open(binPath);

        check("Повторне відкриття BIN", store.rowCount() == 2);

        DataRow row = store.readRow(0);

        check(
            "Перевірка значення після читання",
            get<std::string>(row[1]) == "Оксана"
        );

        store.exportCsv(csvPath);

        check("Експорт BIN у CSV", filesystem::exists(csvPath));

        store.close();

        SchemaStore reopened;
        reopened.open(binPath);

        check("Читання після закриття та повторного запуску",
            reopened.rowCount() == 2);

        DataRow selected = reopened.readSelected(0, { "name" });

        check(
            "Вибір лише одного стовпця",
            selected.size() == 1 &&
            get<std::string>(selected[0]) == "Оксана"
        );

        reopened.close();

        bool invalidValueDetected = false;

        try {
            schema.column(0).parse("abc");
        }
        catch (...) {
            invalidValueDetected = true;
        }

        check("Помилкове int32 значення abc", invalidValueDetected);

        filesystem::copy_file(
            binPath,
            badPath,
            filesystem::copy_options::overwrite_existing
        );

        std::uint64_t size = filesystem::file_size(badPath);

        if (size > 20) {
            fstream file(
                badPath,
                ios::in | ios::out | ios::binary
            );

            std::uint64_t offset = size / 2;

            file.seekg(static_cast<streamoff>(offset));

            char byte = 0;
            file.read(&byte, 1);

            byte ^= 0x01;

            file.seekp(static_cast<streamoff>(offset));
            file.write(&byte, 1);
            file.close();
        }

        bool corruptionDetected = false;

        try {
            SchemaStore corrupted;
            corrupted.open(badPath);
        }
        catch (...) {
            corruptionDetected = true;
        }

        check("Виявлення пошкодженого байта через CRC", corruptionDetected);

        filesystem::remove(binPath);
        filesystem::remove(csvPath);
        filesystem::remove(badPath);
    }
    catch (const exception& e) {
        cout << "Помилка тесту: " << e.what() << "\n";
    }

    cout << "\nРезультат: " << passed << " / " << total
        << " тестів пройдено.\n";
}

void runLoadTest8() {
    cout << "\n===== НАВАНТАЖУВАЛЬНИЙ ТЕСТ =====\n";

    try {
        Schema schema("id:int32,name:string,grade:double");

        DataTable rows;
        rows.reserve(10000);

        for (int i = 1; i <= 10000; ++i) {
            DataRow row;

            row.push_back(static_cast<std::int32_t>(i));
            row.push_back("Student_" + to_string(i));
            row.push_back(static_cast<double>(50 + i % 50));

            rows.push_back(std::move(row));
        }

        const filesystem::path path = "day8_load_test.bin";

        auto startWrite = high_resolution_clock::now();

        SchemaStore store;
        store.create(path, schema, rows);

        auto endWrite = high_resolution_clock::now();

        store.close();

        auto startRead = high_resolution_clock::now();

        SchemaStore reopened;
        reopened.open(path);

        DataTable loaded = reopened.readRange(0, 10000, {});

        auto endRead = high_resolution_clock::now();

        double writeMs = duration<double, milli>(
            endWrite - startWrite
        ).count();

        double readMs = duration<double, milli>(
            endRead - startRead
        ).count();

        std::uintmax_t fileSize = filesystem::file_size(path);

        cout << "Кількість записів: " << loaded.size() << "\n";
        cout << "Час запису: " << writeMs << " мс\n";
        cout << "Час читання: " << readMs << " мс\n";
        cout << "Розмір файлу: " << fileSize << " байт\n";

        cout << (loaded.size() == 10000
            ? "[PASS] 10 000 записів\n"
            : "[FAIL] Перевірка кількості записів\n");

        reopened.close();
        filesystem::remove(path);
    }
    catch (const exception& e) {
        cout << "Помилка навантажувального тесту: "
            << e.what() << "\n";
    }
}