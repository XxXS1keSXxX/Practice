
#include "types_task.h"
#include "menu_utils.h"

#include <iostream>
#include <string>
#include <iomanip>
#include <algorithm>

using namespace std;

StudentList students;

// Перетворення факультету в текст
string toString(Faculty faculty) {
    switch (faculty) {
    case Faculty::IT: return "Інформаційні технології";
    case Faculty::Economics: return "Економіка";
    case Faculty::Law: return "Право";
    case Faculty::Other: return "Інший";
    }

    return "Невідомий";
}

// Перевірка високосного року
bool isLeapYear(int year) {
    return (year % 400 == 0) ||
        (year % 4 == 0 && year % 100 != 0);
}

// Перевірка коректності дати
bool isValidDate(Date date) {
    if (date.year < 1900 || date.year > 2026)
        return false;

    if (date.month < 1 || date.month > 12)
        return false;

    int daysInMonth[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (isLeapYear(date.year))
        daysInMonth[1] = 29;

    return date.day >= 1 &&
        date.day <= daysInMonth[date.month - 1];
}

// Введення дати
Date inputDate() {
    Date date;

    while (true) {
        cout << "\nВведіть дату народження:\n";

        date.day = readInt("День: ", 1, 31);
        date.month = readInt("Місяць: ", 1, 12);
        date.year = readInt("Рік: ", 1900, 2026);

        if (isValidDate(date))
            return date;

        cout << "Некоректна дата! Спробуйте ще раз.\n";
    }
}

// Введення середнього бала
double inputGrade(string message) {
    return readDouble(message, 0, 100);
}

// Вибір факультету
Faculty inputFaculty() {
    cout << "\nФакультети:\n";
    cout << "1. Інформаційні технології\n";
    cout << "2. Економіка\n";
    cout << "3. Право\n";
    cout << "4. Інший\n";

    int choice = readInt("Ваш вибір: ", 1, 4);

    switch (choice) {
    case 1: return Faculty::IT;
    case 2: return Faculty::Economics;
    case 3: return Faculty::Law;
    default: return Faculty::Other;
    }
}

// Перевірка унікальності ID
bool isUniqueId(int id) {
    for (const Student& student : students) {
        if (student.id == id)
            return false;
    }

    return true;
}

// Отримання прізвища
string getSurname(string name) {
    size_t pos = name.find_last_of(' ');

    if (pos == string::npos)
        return name;

    return name.substr(pos + 1);
}

// 1. Додавання студента
void addStudent() {
    Student student{};

    cout << "\n--- ДОДАВАННЯ СТУДЕНТА ---\n";

    while (true) {
        student.id = readInt("Введіть ID: ", 1, 1000000000);

        if (isUniqueId(student.id))
            break;

        cout << "Такий ID вже існує!\n";
    }

    do {
        cout << "Введіть ім'я та прізвище: ";
        getline(cin, student.name);

        if (student.name.empty())
            cout << "Ім'я не може бути порожнім!\n";

    } while (student.name.empty());

    student.birthDate = inputDate();
    student.faculty = inputFaculty();
    student.avgGrade = inputGrade("Введіть середній бал (0-100): ");

    students.push_back(student);

    cout << "\nСтудента успішно додано!\n";
}

// 2. Виведення таблиці студентів
void showStudents() {
    if (students.empty()) {
        cout << "\nСписок студентів порожній.\n";
        return;
    }

    cout << "\n";

    cout << left
        << setw(8) << "ID"
        << setw(25) << "Ім'я"
        << setw(15) << "Дата"
        << setw(30) << "Факультет"
        << setw(12) << "Сер. бал"
        << endl;

    cout << string(90, '-') << endl;

    for (const Student& student : students) {
        string date = to_string(student.birthDate.day) + "." +
            to_string(student.birthDate.month) + "." +
            to_string(student.birthDate.year);

        cout << left
            << setw(8) << student.id
            << setw(25) << student.name
            << setw(15) << date
            << setw(30) << toString(student.faculty)
            << fixed << setprecision(2)
            << setw(12) << student.avgGrade
            << endl;
    }
}

// 3. Пошук студента за прізвищем
void searchStudent() {
    string surname;

    cout << "Введіть прізвище для пошуку: ";
    getline(cin, surname);

    if (surname.empty()) {
        cout << "Пошуковий запит порожній.\n";
        return;
    }

    bool found = false;

    for (const Student& student : students) {
        if (student.name.find(surname) != string::npos) {
            cout << "\nЗнайдено студента:\n";
            cout << "ID: " << student.id << endl;
            cout << "Ім'я: " << student.name << endl;
            cout << "Дата народження: "
                << student.birthDate.day << "."
                << student.birthDate.month << "."
                << student.birthDate.year << endl;
            cout << "Факультет: "
                << toString(student.faculty) << endl;
            cout << "Середній бал: "
                << student.avgGrade << endl;

            found = true;
        }
    }

    if (!found)
        cout << "Студента не знайдено.\n";
}

// 4. Сортування студентів
void sortStudents() {
    if (students.empty()) {
        cout << "Список студентів порожній.\n";
        return;
    }

    cout << "\n--- СОРТУВАННЯ ---\n";
    cout << "1. За прізвищем\n";
    cout << "2. За середнім балом\n";
    cout << "3. За датою народження\n";

    int choice = readInt("Ваш вибір: ", 1, 3);

    switch (choice) {
    case 1:
        sort(students.begin(), students.end(),
            [](const Student& a, const Student& b) {
                return getSurname(a.name) < getSurname(b.name);
            });
        break;

    case 2:
        sort(students.begin(), students.end(),
            [](const Student& a, const Student& b) {
                return a.avgGrade > b.avgGrade;
            });
        break;

    case 3:
        sort(students.begin(), students.end(),
            [](const Student& a, const Student& b) {
                if (a.birthDate.year != b.birthDate.year)
                    return a.birthDate.year < b.birthDate.year;

                if (a.birthDate.month != b.birthDate.month)
                    return a.birthDate.month < b.birthDate.month;

                return a.birthDate.day < b.birthDate.day;
            });
        break;
    }

    cout << "Сортування виконано!\n";
    showStudents();
}

// 5. Видалення студента
void deleteStudent() {
    int id = readInt("Введіть ID студента: ", 1, 1000000000);

    auto it = find_if(students.begin(), students.end(),
        [id](const Student& student) {
            return student.id == id;
        });

    if (it != students.end()) {
        students.erase(it);
        cout << "Студента видалено!\n";
    }
    else {
        cout << "Студента з таким ID не знайдено.\n";
    }
}

// 6. Статистика факультетів
void facultyStatistics() {
    int counts[4] = { 0, 0, 0, 0 };
    double sums[4] = { 0, 0, 0, 0 };

    for (const Student& student : students) {
        int index = static_cast<int>(student.faculty);

        counts[index]++;
        sums[index] += student.avgGrade;
    }

    cout << "\n--- СТАТИСТИКА ФАКУЛЬТЕТІВ ---\n";

    Faculty faculties[] = {
        Faculty::IT,
        Faculty::Economics,
        Faculty::Law,
        Faculty::Other
    };

    for (int i = 0; i < 4; i++) {
        cout << "\nФакультет: " << toString(faculties[i]);
        cout << "\nКількість студентів: " << counts[i];

        if (counts[i] > 0) {
            cout << "\nСередній бал: "
                << fixed << setprecision(2)
                << sums[i] / counts[i];
        }
        else {
            cout << "\nСередній бал: немає даних";
        }

        cout << "\n";
    }
}

// 7. Редагування студента
void editStudent() {
    int id = readInt("Введіть ID студента: ", 1, 1000000000);

    auto it = find_if(students.begin(), students.end(),
        [id](const Student& student) {
            return student.id == id;
        });

    if (it == students.end()) {
        cout << "Студента не знайдено.\n";
        return;
    }

    cout << "\nЩо потрібно змінити?\n";
    cout << "1. Ім'я та прізвище\n";
    cout << "2. Дату народження\n";
    cout << "3. Факультет\n";
    cout << "4. Середній бал\n";

    int choice = readInt("Ваш вибір: ", 1, 4);

    switch (choice) {
    case 1:
        do {
            cout << "Нове ім'я та прізвище: ";
            getline(cin, it->name);

            if (it->name.empty())
                cout << "Ім'я не може бути порожнім!\n";

        } while (it->name.empty());
        break;

    case 2:
        it->birthDate = inputDate();
        break;

    case 3:
        it->faculty = inputFaculty();
        break;

    case 4:
        it->avgGrade = inputGrade("Новий середній бал (0-100): ");
        break;
    }

    cout << "Дані студента оновлено!\n";
}

// Головне меню другого дня
void typesTask() {
    int choice;

    do {
        cout << "\n=== ОБЛІК СТУДЕНТІВ ===\n";
        cout << "1. Додати студента\n";
        cout << "2. Показати всіх студентів\n";
        cout << "3. Пошук за прізвищем\n";
        cout << "4. Сортування\n";
        cout << "5. Видалити студента за ID\n";
        cout << "6. Статистика по факультетах\n";
        cout << "7. Редагувати запис\n";
        cout << "8. Назад\n";

        choice = readInt("Ваш вибір: ", 1, 8);

        switch (choice) {
        case 1: addStudent(); break;
        case 2: showStudents(); break;
        case 3: searchStudent(); break;
        case 4: sortStudents(); break;
        case 5: deleteStudent(); break;
        case 6: facultyStatistics(); break;
        case 7: editStudent(); break;
        case 8: break;
        }

    } while (choice != 8);
}

// Отримання доступу до списку студентів
StudentList& getStudents() {
    return students;
}