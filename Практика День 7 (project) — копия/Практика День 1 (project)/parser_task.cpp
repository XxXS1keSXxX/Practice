#include "parser_task.h"
#include "tests7.h"
#include "types_task.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cctype>
#include <utility>

using namespace std;

namespace {

    string numberToString(double value) {
        ostringstream out;
        out << setprecision(10) << value;
        return out.str();
    }

    string dateToString(const Date& date) {
        return to_string(date.day) + "." +
            to_string(date.month) + "." +
            to_string(date.year);
    }

    string studentField(
        const Student& student,
        const string& field
    ) {
        if (field == "id")
            return to_string(student.id);

        if (field == "name")
            return student.name;

        if (field == "birthdate")
            return dateToString(student.birthDate);

        if (field == "faculty") {
            if (student.faculty == Faculty::IT)
                return "ІТ";

            return toString(student.faculty);
        }

        if (field == "avggrade")
            return numberToString(student.avgGrade);

        if (field == "birthyear")
            return to_string(student.birthDate.year);

        return "";
    }

    bool compareStudents(
        const Student& a,
        const Student& b,
        const string& field
    ) {
        if (field == "id")
            return a.id < b.id;

        if (field == "avggrade")
            return a.avgGrade < b.avgGrade;

        if (field == "birthyear")
            return a.birthDate.year < b.birthDate.year;

        if (field == "name")
            return a.name < b.name;

        if (field == "faculty")
            return studentField(a, field) <
            studentField(b, field);

        if (field == "birthdate")
            return dateToString(a.birthDate) <
            dateToString(b.birthDate);

        return false;
    }

    string csvEscape(const string& value) {
        string result = "\"";

        for (char c : value) {
            if (c == '"')
                result += "\"\"";
            else
                result += c;
        }

        result += "\"";
        return result;
    }

    void saveHistory(const string& query) {
        vector<string> history;
        ifstream input("sql_history.txt");

        string line;

        while (getline(input, line)) {
            if (!line.empty())
                history.push_back(line);
        }

        input.close();

        history.push_back(query);

        if (history.size() > 10) {
            history.erase(
                history.begin(),
                history.begin() + history.size() - 10
            );
        }

        ofstream output("sql_history.txt");

        for (const string& item : history)
            output << item << '\n';
    }

    void showHistory() {
        ifstream input("sql_history.txt");

        if (!input) {
            cout << "Історія запитів поки порожня.\n";
            return;
        }

        cout << "\nОСТАННІ ЗАПИТИ:\n";

        string line;
        int number = 1;

        while (getline(input, line)) {
            cout << number++ << ". " << line << '\n';
        }
    }

    void saveCsv(const QueryResult& result) {
        ofstream file("query_result.csv");

        if (!file) {
            cout << "Не вдалося створити CSV-файл.\n";
            return;
        }

        for (size_t i = 0; i < result.columns.size(); ++i) {
            if (i > 0)
                file << ",";

            file << csvEscape(result.columns[i]);
        }

        file << '\n';

        for (const auto& row : result.rows) {
            for (size_t i = 0; i < row.size(); ++i) {
                if (i > 0)
                    file << ",";

                file << csvEscape(row[i]);
            }

            file << '\n';
        }

        cout << "Результат збережено у query_result.csv\n";
    }

    void showAst(const SQLQuery& query) {
        if (!query.where) {
            cout << "У запиті немає умови WHERE.\n";
            return;
        }

        cout << "\nДЕРЕВО AST:\n";
        query.where->print(cout);

        NodeCounter counter;
        query.where->accept(counter);

        cout << "Кількість вузлів дерева: "
            << counter.count() << '\n';
    }

    string readQuery() {
        string query;

        cout << "Введіть SQL-запит:\n";

        getline(cin, query);

        return query;
    }

    void executeAndShow(const string& sql) {
        try {
            Parser parser(sql);
            SQLQuery query = parser.parse();

            showAst(query);

            QueryResult result = executeQuery(query);

            printQueryResult(result);

            saveCsv(result);
            saveHistory(sql);
        }
        catch (const ParseError& error) {
            SourcePosition position = error.position();

            cout << "Помилка синтаксису: "
                << error.what() << '\n';

            cout << "Рядок: " << position.line
                << ", позиція: " << position.column << '\n';
        }
        catch (const RuntimeError& error) {
            SourcePosition position = error.position();

            cout << "Помилка виконання: "
                << error.what() << '\n';

            cout << "Рядок: " << position.line
                << ", позиція: " << position.column << '\n';
        }
        catch (const exception& error) {
            cout << "Помилка: " << error.what() << '\n';
        }
    }

    void keyboardMode() {
        string query = readQuery();

        if (!query.empty())
            executeAndShow(query);
    }

    void fileMode() {
        cout << "Введіть назву файлу із SQL-запитом: ";

        string filename;
        getline(cin, filename);

        ifstream file(filename);

        if (!file) {
            cout << "Не вдалося відкрити файл.\n";
            return;
        }

        stringstream buffer;
        buffer << file.rdbuf();

        executeAndShow(buffer.str());
    }

    void replMode() {
        string lastQuery;

        cout << "\nРЕЖИМ REPL\n";
        cout << "Введіть SQL-запит.\n";
        cout << "Команди: !! — повторити останній запит, "
            << "назад — повернутися.\n";

        while (true) {
            cout << "\nSQL> ";

            string query;
            getline(cin, query);

            if (query == "назад" || query == "back")
                break;

            if (query == "!!") {
                if (lastQuery.empty()) {
                    cout << "Ще немає запиту для повторення.\n";
                    continue;
                }

                query = lastQuery;
                cout << query << '\n';
            }

            if (query.empty())
                continue;

            lastQuery = query;
            executeAndShow(query);
        }
    }

}

QueryResult executeQuery(const SQLQuery& query) {
    StudentList& students = getStudents();

    vector<const Student*> selected;

    for (const Student& student : students) {
        if (
            !query.where ||
            query.where->evaluate(student)
            ) {
            selected.push_back(&student);
        }
    }

    if (!query.orderBy.empty()) {
        sort(
            selected.begin(),
            selected.end(),
            [&](const Student* a, const Student* b) {
                if (query.descending)
                    return compareStudents(*b, *a, query.orderBy);

                return compareStudents(*a, *b, query.orderBy);
            }
        );
    }

    QueryResult result;

    if (query.count) {
        result.columns = { "COUNT(*)" };
        result.rows.push_back({
            to_string(selected.size())
            });

        return result;
    }

    result.columns = query.fields;

    if (query.hasLimit && selected.size() > query.limit)
        selected.resize(query.limit);

    for (const Student* student : selected) {
        vector<string> row;

        for (const string& field : query.fields) {
            row.push_back(studentField(*student, field));
        }

        result.rows.push_back(std::move(row));
    }

    return result;
}

void printQueryResult(const QueryResult& result) {
    if (result.columns.empty()) {
        cout << "Немає стовпців для відображення.\n";
        return;
    }

    vector<size_t> widths(result.columns.size());

    for (size_t i = 0; i < result.columns.size(); ++i)
        widths[i] = result.columns[i].size();

    for (const auto& row : result.rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            widths[i] = max(widths[i], row[i].size());
        }
    }

    cout << '\n';

    for (size_t i = 0; i < result.columns.size(); ++i) {
        cout << left << setw(static_cast<int>(widths[i] + 3))
            << result.columns[i];
    }

    cout << '\n';

    for (size_t i = 0; i < result.columns.size(); ++i)
        cout << string(widths[i] + 3, '-');

    cout << '\n';

    for (const auto& row : result.rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            cout << left << setw(static_cast<int>(widths[i] + 3))
                << row[i];
        }

        cout << '\n';
    }

    cout << "\nКількість рядків: "
        << result.rows.size() << '\n';
}

void parserMenu() {
    int choice = -1;

    do {
        cout << "\n====================================\n";
        cout << "       ДЕНЬ 7. SQL-ПАРСЕР\n";
        cout << "====================================\n";
        cout << "1. Ввести SQL-запит з клавіатури\n";
        cout << "2. Завантажити запит із файлу\n";
        cout << "3. Режим REPL\n";
        cout << "4. Переглянути історію запитів\n";
        cout << "5. Запустити автоматичні тести\n";
        cout << "0. Назад\n";

        cout << "Ваш вибір: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(
                numeric_limits<streamsize>::max(),
                '\n'
            );

            cout << "Некоректне введення.\n";
            continue;
        }

        cin.ignore(
            numeric_limits<streamsize>::max(),
            '\n'
        );

        switch (choice) {
        case 1:
            keyboardMode();
            break;

        case 2:
            fileMode();
            break;

        case 3:
            replMode();
            break;

        case 4:
            showHistory();
            break;

        case 5:
            runTests7(cout);
            break;

        case 0:
            break;

        default:
            cout << "Невідомий пункт меню.\n";
        }

    } while (choice != 0);
}