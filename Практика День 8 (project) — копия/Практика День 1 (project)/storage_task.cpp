#include "storage_task.h"
#include "schema_store.h"
#include "tests8.h"

#include <iostream>
#include <string>
#include <limits>

using namespace std;

namespace {

    string readLine(const string& prompt) {
        cout << prompt;

        string value;
        getline(cin, value);

        return value;
    }

    int readChoice(int minValue, int maxValue) {
        while (true) {
            string input = readLine("Ваш вибір: ");

            try {
                size_t used = 0;
                int value = stoi(input, &used);

                if (used == input.size() &&
                    value >= minValue &&
                    value <= maxValue) {
                    return value;
                }
            }
            catch (...) {}

            cout << "Некоректний вибір. Спробуйте ще раз.\n";
        }
    }

    void printSchema(const Schema& schema) {
        cout << "\nСхема: " << schema.text() << "\n";
        cout << "Кількість полів: " << schema.size() << "\n";
    }

    void printRows(
        SchemaStore& store,
        std::uint64_t first,
        std::uint64_t count,
        const vector<string>& selected
    ) {
        DataTable rows = store.readRange(first, count, selected);

        vector<size_t> indexes;

        if (selected.empty()) {
            for (size_t i = 0; i < store.schema().size(); ++i)
                indexes.push_back(i);
        }
        else {
            for (const string& name : selected) {
                int index = store.schema().findColumn(name);

                if (index < 0)
                    throw runtime_error("Невідомий стовпець: " + name);

                indexes.push_back(static_cast<size_t>(index));
            }
        }

        for (size_t i = 0; i < indexes.size(); ++i) {
            if (i > 0)
                cout << "\t";

            cout << store.schema().info(indexes[i]).name;
        }

        cout << "\n";

        for (const DataRow& row : rows) {
            for (size_t c = 0; c < row.size(); ++c) {
                if (c > 0)
                    cout << "\t";

                cout << store.schema().column(indexes[c]).toText(row[c]);
            }

            cout << "\n";
        }

        cout << "Кількість показаних рядків: " << rows.size() << "\n";
    }

    vector<string> readColumnList() {
        string input = readLine(
            "Назви стовпців через кому (Enter — усі): "
        );

        vector<string> result;

        if (input.empty())
            return result;

        size_t start = 0;

        while (start < input.size()) {
            size_t comma = input.find(',', start);

            if (comma == string::npos)
                comma = input.size();

            string name = input.substr(start, comma - start);

            size_t first = name.find_first_not_of(" \t");
            size_t last = name.find_last_not_of(" \t");

            if (first != string::npos)
                result.push_back(name.substr(first, last - first + 1));

            start = comma + 1;
        }

        return result;
    }

}

void storageMenu() {
    SchemaStore store;

    string currentPath;

    while (true) {
        cout << "\n====================================\n";
        cout << "       ДЕНЬ 8. ФАЙЛОВЕ СХОВИЩЕ\n";
        cout << "====================================\n";
        cout << "1. Створити BIN із CSV\n";
        cout << "2. Відкрити BIN-файл\n";
        cout << "3. Переглянути записи\n";
        cout << "4. Експортувати BIN у CSV\n";
        cout << "5. Переглянути HEX-файл\n";
        cout << "6. Перевірити CRC-32\n";
        cout << "7. Запустити автоматичні тести\n";
        cout << "8. Навантажувальний тест 10 000 рядків\n";
        cout << "0. Назад\n";

        int choice = readChoice(0, 8);

        try {
            if (choice == 0) {
                store.close();
                return;
            }

            if (choice == 1) {
                string schemaText = readLine(
                    "Введіть схему (наприклад id:int32,name:string,grade:double): "
                );

                Schema schema(schemaText);

                string csvPath = readLine("CSV-файл: ");
                string binPath = readLine("Новий BIN-файл: ");

                store.importCsv(csvPath, binPath, schema);

                currentPath = binPath;

                cout << "CSV успішно перетворено у BIN.\n";
                printSchema(store.schema());
            }
            else if (choice == 2) {
                string path = readLine("Шлях до BIN-файлу: ");

                store.open(path);
                currentPath = path;

                cout << "Файл відкрито та перевірено.\n";
                cout << "Кількість рядків: " << store.rowCount() << "\n";

                printSchema(store.schema());
            }
            else if (choice == 3) {
                if (!store.isOpen()) {
                    cout << "Спочатку відкрийте BIN-файл.\n";
                    continue;
                }

                string firstText = readLine("Перший рядок (від 1): ");
                string countText = readLine("Кількість рядків: ");

                std::uint64_t first = stoull(firstText);
                std::uint64_t count = stoull(countText);

                if (first == 0)
                    throw runtime_error("Номер рядка починається з 1");

                vector<string> columns = readColumnList();

                printRows(store, first - 1, count, columns);
            }
            else if (choice == 4) {
                if (!store.isOpen()) {
                    cout << "Спочатку відкрийте BIN-файл.\n";
                    continue;
                }

                string csvPath = readLine("Назва CSV-файлу для експорту: ");

                vector<string> columns = readColumnList();

                store.exportCsv(
                    csvPath,
                    0,
                    UINT64_MAX,
                    columns
                );

                cout << "Експорт завершено: " << csvPath << "\n";
            }
            else if (choice == 5) {
                string path = currentPath;

                if (path.empty())
                    path = readLine("Шлях до файлу: ");

                string countText = readLine("Скільки перших байтів показати: ");

                size_t count = stoull(countText);

                hexdump(path, count);
            }
            else if (choice == 6) {
                if (!store.isOpen()) {
                    cout << "Спочатку відкрийте BIN-файл.\n";
                    continue;
                }

                store.verify();

                cout << "CRC-32 правильна. Файл не пошкоджений.\n";
            }
            else if (choice == 7) {
                runTests8();
            }
            else if (choice == 8) {
                runLoadTest8();
            }
        }
        catch (const exception& e) {
            cout << "Помилка: " << e.what() << "\n";
        }
    }
}