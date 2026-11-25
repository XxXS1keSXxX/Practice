#include "tests7.h"
#include "parser.h"
#include "parser_task.h"
#include "types_task.h"

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <utility>

using namespace std;

void runTests7(ostream& out) {
    StudentList backup = getStudents();

    StudentList testStudents = {
        {4, "Ґалаган", {10, 5, 2005}, Faculty::IT, 99},
        {1, "Шевченко", {12, 3, 2005}, Faculty::IT, 98},
        {7, "Коваль", {15, 7, 2004}, Faculty::IT, 87},
        {3, "Іваненко", {20, 1, 2005}, Faculty::Economics, 86},
        {5, "Бондар", {8, 9, 2004}, Faculty::Law, 75},
        {6, "Петренко", {2, 4, 2005}, Faculty::Other, 60}
    };

    getStudents() = testStudents;

    struct Test {
        string name;
        function<bool()> run;
    };

    vector<Test> tests;

    tests.push_back({
        "SELECT з WHERE та ORDER BY",
        [] {
            Parser parser(
                "SELECT name FROM students "
                "WHERE faculty = 'ІТ' "
                "ORDER BY avgGrade DESC"
            );

            SQLQuery query = parser.parse();
            QueryResult result = executeQuery(query);

            return result.rows.size() == 3 &&
                result.rows[0][0] == "Ґалаган" &&
                result.rows[1][0] == "Шевченко" &&
                result.rows[2][0] == "Коваль";
        }
        });

    tests.push_back({
        "COUNT(*) з умовою",
        [] {
            Parser parser(
                "SELECT COUNT(*) FROM students "
                "WHERE avgGrade > 85"
            );

            SQLQuery query = parser.parse();
            QueryResult result = executeQuery(query);

            return result.rows.size() == 1 &&
                result.rows[0][0] == "4";
        }
        });

    tests.push_back({
        "LIKE з відсотком",
        [] {
            Parser parser(
                "SELECT id FROM students "
                "WHERE name LIKE 'Ко%'"
            );

            SQLQuery query = parser.parse();
            QueryResult result = executeQuery(query);

            return result.rows.size() == 1 &&
                result.rows[0][0] == "7";
        }
        });

    tests.push_back({
        "ORDER BY DESC та LIMIT",
        [] {
            Parser parser(
                "SELECT id FROM students "
                "ORDER BY avgGrade DESC LIMIT 2"
            );

            SQLQuery query = parser.parse();
            QueryResult result = executeQuery(query);

            return result.rows.size() == 2 &&
                result.rows[0][0] == "4" &&
                result.rows[1][0] == "1";
        }
        });

    tests.push_back({
        "SELECT усіх полів",
        [] {
            Parser parser(
                "SELECT * FROM students"
            );

            SQLQuery query = parser.parse();
            QueryResult result = executeQuery(query);

            return result.columns.size() == 5 &&
                result.rows.size() == 6;
        }
        });

    tests.push_back({
        "Оператор OR",
        [] {
            Parser parser(
                "SELECT id FROM students "
                "WHERE avgGrade >= 98 OR id = 7"
            );

            SQLQuery query = parser.parse();
            QueryResult result = executeQuery(query);

            return result.rows.size() == 3;
        }
        });

    tests.push_back({
        "Оператор NOT",
        [] {
            Parser parser(
                "SELECT id FROM students "
                "WHERE NOT (avgGrade > 85)"
            );

            SQLQuery query = parser.parse();
            QueryResult result = executeQuery(query);

            return result.rows.size() == 2;
        }
        });

    tests.push_back({
        "LIKE з символом підкреслення",
        [] {
            Parser parser(
                "SELECT id FROM students "
                "WHERE name LIKE 'К_валь'"
            );

            SQLQuery query = parser.parse();
            QueryResult result = executeQuery(query);

            return result.rows.size() == 1 &&
                result.rows[0][0] == "7";
        }
        });

    tests.push_back({
        "Дужки та пріоритет AND/OR",
        [] {
            Parser parser(
                "SELECT id FROM students "
                "WHERE (faculty = 'ІТ' AND avgGrade > 90) "
                "OR id = 7"
            );

            SQLQuery query = parser.parse();
            QueryResult result = executeQuery(query);

            return result.rows.size() == 3;
        }
        });

    tests.push_back({
        "Помилка невідомого поля",
        [] {
            try {
                Parser parser(
                    "SELECT age FROM students"
                );

                parser.parse();
            }
            catch (const ParseError&) {
                return true;
            }

            return false;
        }
        });

    tests.push_back({
        "Помилка несумісності типів",
        [] {
            try {
                Parser parser(
                    "SELECT id FROM students WHERE name = 5"
                );

                parser.parse();
            }
            catch (const ParseError&) {
                return true;
            }

            return false;
        }
        });

    tests.push_back({
        "Помилка невідомої таблиці",
        [] {
            try {
                Parser parser(
                    "SELECT id FROM teachers"
                );

                parser.parse();
            }
            catch (const ParseError&) {
                return true;
            }

            return false;
        }
        });

    int passed = 0;

    out << "\n========== ТЕСТУВАННЯ ДНЯ 7 ==========\n";

    for (size_t i = 0; i < tests.size(); ++i) {
        bool success = false;

        try {
            success = tests[i].run();
        }
        catch (...) {
            success = false;
        }

        out << i + 1 << ". "
            << tests[i].name << " — "
            << (success ? "PASS" : "FAIL")
            << '\n';

        if (success)
            ++passed;
    }

    out << "======================================\n";
    out << "Усього тестів: " << tests.size() << '\n';
    out << "Успішно: " << passed << '\n';
    out << "Помилок: " << tests.size() - passed << '\n';

    getStudents() = std::move(backup);
}