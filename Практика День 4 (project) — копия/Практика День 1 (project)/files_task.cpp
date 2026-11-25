
#include "files_task.h"
#include "menu_utils.h"
#include "types_task.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <cstring>
#include <cctype>
#include <limits>

using namespace std;
namespace fs = std::filesystem;

// Робочий текстовий файл
fs::path workingFile = "data/students.txt";

// Бінарна структура запису
struct StudentRec {
    int id;
    char name[50];
    int day, month, year;
    int faculty;
    double avgGrade;
};

// Шлях до бінарного файлу
fs::path binaryFilePath() {
    fs::path path = workingFile;
    path.replace_extension(".bin");
    return path;
}

// Створення каталогів перед записом
bool createParentDirectories(const fs::path& path) {
    try {
        if (!path.parent_path().empty())
            fs::create_directories(path.parent_path());

        return true;
    }
    catch (const fs::filesystem_error& e) {
        cout << "Помилка створення каталогу: " << e.what() << endl;
        return false;
    }
}

// Перевірка існування файлу
bool fileExists(const fs::path& path) {
    if (!fs::exists(path)) {
        cout << "Файл не знайдено: " << path.string() << endl;
        return false;
    }

    if (!fs::is_regular_file(path)) {
        cout << "Вказаний шлях не є файлом.\n";
        return false;
    }

    return true;
}

// Зміна робочого файлу
void changeWorkingFile() {
    string path;

    cout << "\nПоточний файл: " << workingFile.string() << endl;
    cout << "Введіть новий шлях або Enter для стандартного: ";

    getline(cin, path);

    if (path.empty())
        workingFile = "data/students.txt";
    else
        workingFile = path;

    cout << "Робочий файл змінено: "
        << workingFile.string() << endl;
}

// Перевірка дати
bool validDate(Date date) {
    if (date.year < 1900 || date.year > 2026)
        return false;

    if (date.month < 1 || date.month > 12)
        return false;

    int days[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    bool leap = (date.year % 400 == 0) ||
        (date.year % 4 == 0 && date.year % 100 != 0);

    if (leap)
        days[1] = 29;

    return date.day >= 1 &&
        date.day <= days[date.month - 1];
}

// Введення дати
Date inputFileDate() {
    Date date;

    while (true) {
        cout << "\nВведіть дату народження:\n";
        date.day = readInt("День: ", 1, 31);
        date.month = readInt("Місяць: ", 1, 12);
        date.year = readInt("Рік: ", 1900, 2026);

        if (validDate(date))
            return date;

        cout << "Некоректна дата!\n";
    }
}

// Введення факультету
Faculty inputFileFaculty() {
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

// Введення нового студента
Student inputFileStudent() {
    Student student{};

    student.id = readInt("Введіть ID: ", 1, 1000000000);

    do {
        cout << "Введіть ім'я та прізвище: ";
        getline(cin, student.name);

        if (student.name.empty() ||
            student.name.find(';') != string::npos) {
            cout << "Ім'я не може бути порожнім або містити ;\n";
        }
    } while (student.name.empty() ||
        student.name.find(';') != string::npos);

    student.birthDate = inputFileDate();
    student.faculty = inputFileFaculty();
    student.avgGrade = readDouble("Середній бал (0-100): ", 0, 100);

    return student;
}

// Виведення одного студента
void printStudent(const Student& student) {
    cout << "\nID: " << student.id << endl;
    cout << "ПІБ: " << student.name << endl;
    cout << "Дата народження: "
        << student.birthDate.day << "."
        << student.birthDate.month << "."
        << student.birthDate.year << endl;
    cout << "Факультет: " << toString(student.faculty) << endl;
    cout << "Середній бал: "
        << fixed << setprecision(2)
        << student.avgGrade << endl;
}

// Перетворення текстового факультету
bool parseFaculty(const string& text, Faculty& faculty) {
    if (text == "Інформаційні технології") {
        faculty = Faculty::IT;
        return true;
    }

    if (text == "Економіка") {
        faculty = Faculty::Economics;
        return true;
    }

    if (text == "Право") {
        faculty = Faculty::Law;
        return true;
    }

    if (text == "Інший") {
        faculty = Faculty::Other;
        return true;
    }

    return false;
}

// Перетворення дати з формату дд.мм.рррр
bool parseDate(const string& text, Date& date) {
    char dot1, dot2;
    istringstream input(text);

    if (!(input >> date.day >> dot1 >> date.month
        >> dot2 >> date.year))
        return false;

    if (dot1 != '.' || dot2 != '.')
        return false;

    if (input.peek() != EOF)
        return false;

    return validDate(date);
}

// Розбір одного текстового запису
bool parseStudentLine(const string& line, Student& student) {
    string fields[5];
    stringstream input(line);

    for (int i = 0; i < 5; i++) {
        if (!getline(input, fields[i], ';'))
            return false;
    }

    string extra;
    if (getline(input, extra, ';'))
        return false;

    try {
        size_t pos = 0;
        student.id = stoi(fields[0], &pos);

        if (pos != fields[0].size() ||
            student.id <= 0)
            return false;

        student.name = fields[1];

        if (student.name.empty())
            return false;

        if (!parseDate(fields[2], student.birthDate))
            return false;

        if (!parseFaculty(fields[3], student.faculty))
            return false;

        pos = 0;
        student.avgGrade = stod(fields[4], &pos);

        if (pos != fields[4].size() ||
            student.avgGrade < 0 ||
            student.avgGrade > 100)
            return false;
    }
    catch (...) {
        return false;
    }

    return true;
}

// Перевірка унікальності ID
bool uniqueStudentId(const StudentList& students, int id) {
    for (const Student& student : students) {
        if (student.id == id)
            return false;
    }

    return true;
}

// 1. Збереження списку студентів у текстовий файл
void saveTextFile() {
    StudentList& students = getStudents();

    if (!createParentDirectories(workingFile))
        return;

    for (const Student& student : students) {
        if (student.name.find(';') != string::npos) {
            cout << "Помилка: ПІБ студента з ID "
                << student.id << " містить символ ;\n";
            return;
        }
    }

    ofstream out(workingFile);

    if (!out.is_open()) {
        cout << "Не вдалося відкрити файл для запису.\n";
        return;
    }

    for (const Student& student : students) {
        out << student.id << ";"
            << student.name << ";"
            << setfill('0') << setw(2) << student.birthDate.day << "."
            << setw(2) << student.birthDate.month << "."
            << setw(4) << student.birthDate.year
            << setfill(' ') << ";"
            << toString(student.faculty) << ";"
            << fixed << setprecision(2)
            << student.avgGrade << "\n";
    }

    out.close();

    cout << "Список студентів збережено!\n";
    cout << "Файл: " << workingFile.string() << endl;
}

// 2. Завантаження списку студентів із текстового файлу
void loadTextFile() {
    if (!fileExists(workingFile))
        return;

    ifstream in(workingFile);

    if (!in.is_open()) {
        cout << "Не вдалося відкрити файл для читання.\n";
        return;
    }

    StudentList loaded;
    string line;
    int lineNumber = 0;
    int skipped = 0;

    while (getline(in, line)) {
        lineNumber++;

        Student student{};

        if (!parseStudentLine(line, student) ||
            !uniqueStudentId(loaded, student.id)) {
            cout << "Пошкоджений запис у рядку "
                << lineNumber << " — пропущено.\n";
            skipped++;
            continue;
        }

        loaded.push_back(student);
    }

    in.close();

    getStudents() = loaded;

    cout << "\nЗавантаження завершено.\n";
    cout << "Завантажено студентів: " << loaded.size() << endl;
    cout << "Пропущено записів: " << skipped << endl;
}

// 3. Додавання запису в кінець текстового файлу
void appendTextFile() {
    StudentList& students = getStudents();

    Student student = inputFileStudent();

    if (!uniqueStudentId(students, student.id)) {
        cout << "Студент із таким ID вже існує!\n";
        return;
    }

    if (!createParentDirectories(workingFile))
        return;

    ofstream out(workingFile, ios::app);

    if (!out.is_open()) {
        cout << "Не вдалося відкрити файл для додавання.\n";
        return;
    }

    out << student.id << ";"
        << student.name << ";"
        << setfill('0') << setw(2) << student.birthDate.day << "."
        << setw(2) << student.birthDate.month << "."
        << setw(4) << student.birthDate.year
        << setfill(' ') << ";"
        << toString(student.faculty) << ";"
        << fixed << setprecision(2)
        << student.avgGrade << "\n";

    out.close();

    students.push_back(student);

    cout << "Запис додано у файл і список студентів!\n";
}

// Кількість символів UTF-8
size_t countCharacters(const string& text) {
    size_t count = 0;

    for (unsigned char ch : text) {
        if ((ch & 0xC0) != 0x80)
            count++;
    }

    return count;
}

// 4. Статистика текстового файлу
void textFileStatistics() {
    if (!fileExists(workingFile))
        return;

    ifstream in(workingFile);

    if (!in.is_open()) {
        cout << "Не вдалося відкрити файл.\n";
        return;
    }

    string line;
    size_t lines = 0;
    size_t words = 0;
    size_t characters = 0;

    while (getline(in, line)) {
        lines++;
        characters += countCharacters(line);

        bool inWord = false;

        for (unsigned char ch : line) {
            if (isspace(ch)) {
                inWord = false;
            }
            else if (!inWord) {
                words++;
                inWord = true;
            }
        }
    }

    in.close();

    cout << "\n--- СТАТИСТИКА ФАЙЛУ ---\n";
    cout << "Кількість рядків: " << lines << endl;
    cout << "Кількість слів: " << words << endl;
    cout << "Кількість символів: " << characters << endl;
}

// 5. Пошук слова у текстовому файлі
void searchWordInFile() {
    if (!fileExists(workingFile))
        return;

    string word;

    cout << "Введіть слово для пошуку: ";
    getline(cin, word);

    if (word.empty()) {
        cout << "Пошукове слово порожнє.\n";
        return;
    }

    ifstream in(workingFile);

    if (!in.is_open()) {
        cout << "Не вдалося відкрити файл.\n";
        return;
    }

    string line;
    int lineNumber = 0;
    bool found = false;

    while (getline(in, line)) {
        lineNumber++;

        if (line.find(word) != string::npos) {
            cout << "Знайдено у рядку: "
                << lineNumber << endl;
            found = true;
        }
    }

    in.close();

    if (!found)
        cout << "Слово не знайдено.\n";
}

// Перетворення студента у бінарний запис
bool makeBinaryRecord(const Student& student, StudentRec& rec) {
    if (student.name.size() >= sizeof(rec.name)) {
        cout << "ПІБ студента з ID " << student.id
            << " задовгий для бінарного формату.\n";
        return false;
    }

    rec = {};

    rec.id = student.id;
    memcpy(rec.name, student.name.c_str(),
        student.name.size() + 1);

    rec.day = student.birthDate.day;
    rec.month = student.birthDate.month;
    rec.year = student.birthDate.year;
    rec.faculty = static_cast<int>(student.faculty);
    rec.avgGrade = student.avgGrade;

    return true;
}

// Перетворення бінарного запису у студента
bool makeStudent(const StudentRec& rec, Student& student) {
    if (memchr(rec.name, '\0', sizeof(rec.name)) == nullptr)
        return false;

    if (rec.id <= 0 ||
        rec.faculty < 0 || rec.faculty > 3 ||
        rec.avgGrade < 0 || rec.avgGrade > 100)
        return false;

    student.id = rec.id;
    student.name = rec.name;
    student.birthDate = { rec.day, rec.month, rec.year };
    student.faculty = static_cast<Faculty>(rec.faculty);
    student.avgGrade = rec.avgGrade;

    return validDate(student.birthDate);
}

// Запис усіх студентів у бінарний файл
bool writeBinaryFile(const fs::path& path) {
    if (!createParentDirectories(path))
        return false;

    ofstream out(path, ios::binary | ios::trunc);

    if (!out.is_open()) {
        cout << "Не вдалося відкрити бінарний файл.\n";
        return false;
    }

    for (const Student& student : getStudents()) {
        StudentRec rec{};

        if (!makeBinaryRecord(student, rec)) {
            out.close();
            return false;
        }

        out.write(reinterpret_cast<const char*>(&rec),
            sizeof(rec));

        if (!out) {
            cout << "Помилка запису у бінарний файл.\n";
            out.close();
            return false;
        }
    }

    out.close();
    return true;
}

// Читання всіх бінарних записів
bool readAllBinary(const fs::path& path, StudentList& result) {
    if (!fileExists(path))
        return false;

    ifstream in(path, ios::binary);

    if (!in.is_open()) {
        cout << "Не вдалося відкрити бінарний файл.\n";
        return false;
    }

    result.clear();

    StudentRec rec{};

    while (in.read(reinterpret_cast<char*>(&rec), sizeof(rec))) {
        Student student{};

        if (!makeStudent(rec, student)) {
            cout << "Виявлено пошкоджений бінарний запис.\n";
            in.close();
            return false;
        }

        result.push_back(student);
    }

    bool correctEnd = in.eof() &&
        in.gcount() == 0;

    in.close();

    if (!correctEnd) {
        cout << "Бінарний файл має неправильний розмір або пошкоджений.\n";
        return false;
    }

    return true;
}

// 6. Збереження списку студентів у бінарний файл
void saveBinaryFile() {
    fs::path path = binaryFilePath();

    if (writeBinaryFile(path)) {
        cout << "Бінарний файл успішно збережено!\n";
        cout << "Файл: " << path.string() << endl;
    }
}

// 7. Читання бінарного файлу
void loadBinaryFile() {
    fs::path path = binaryFilePath();

    StudentList loaded;

    if (!readAllBinary(path, loaded))
        return;

    cout << "\n--- ВМІСТ БІНАРНОГО ФАЙЛУ ---\n";

    for (const Student& student : loaded)
        printStudent(student);

    cout << "\nКількість записів: " << loaded.size() << endl;
}

// Перевірка кількості записів у бінарному файлі
bool getBinaryRecordCount(const fs::path& path,
    size_t& count) {
    if (!fileExists(path))
        return false;

    try {
        uintmax_t size = fs::file_size(path);

        if (size % sizeof(StudentRec) != 0) {
            cout << "Неправильний розмір бінарного файлу.\n";
            return false;
        }

        count = static_cast<size_t>(size / sizeof(StudentRec));
        return true;
    }
    catch (const fs::filesystem_error& e) {
        cout << "Помилка отримання розміру файлу: "
            << e.what() << endl;
        return false;
    }
}

// 8. Читання запису за номером через seekg
void readBinaryRecordByNumber() {
    fs::path path = binaryFilePath();

    size_t count;

    if (!getBinaryRecordCount(path, count))
        return;

    if (count == 0) {
        cout << "Бінарний файл порожній.\n";
        return;
    }

    int number = readInt("Введіть номер запису: ",
        1, static_cast<int>(count));

    ifstream in(path, ios::binary);

    if (!in.is_open()) {
        cout << "Не вдалося відкрити бінарний файл.\n";
        return;
    }

    StudentRec rec{};

    streamoff position =
        static_cast<streamoff>(number - 1) *
        static_cast<streamoff>(sizeof(StudentRec));

    in.seekg(position, ios::beg);

    if (!in.read(reinterpret_cast<char*>(&rec), sizeof(rec))) {
        cout << "Помилка читання запису.\n";
        in.close();
        return;
    }

    in.close();

    Student student{};

    if (!makeStudent(rec, student)) {
        cout << "Запис пошкоджений.\n";
        return;
    }

    cout << "\nЗнайдений запис №" << number << ":\n";
    printStudent(student);
}

// 9. Зміна запису через seekp
void editBinaryRecord() {
    fs::path path = binaryFilePath();

    size_t count;

    if (!getBinaryRecordCount(path, count))
        return;

    if (count == 0) {
        cout << "Бінарний файл порожній.\n";
        return;
    }

    int number = readInt("Введіть номер запису для зміни: ",
        1, static_cast<int>(count));

    StudentList current;

    if (!readAllBinary(path, current))
        return;

    cout << "\nПоточні дані:\n";
    printStudent(current[number - 1]);

    cout << "\nВведіть нові дані студента:\n";

    Student updated = inputFileStudent();

    if (!uniqueStudentId(current, updated.id) &&
        updated.id != current[number - 1].id) {
        cout << "Такий ID вже використовується!\n";
        return;
    }

    StudentRec rec{};

    if (!makeBinaryRecord(updated, rec))
        return;

    fstream file(path, ios::binary | ios::in | ios::out);

    if (!file.is_open()) {
        cout << "Не вдалося відкрити файл для зміни.\n";
        return;
    }

    streamoff position =
        static_cast<streamoff>(number - 1) *
        static_cast<streamoff>(sizeof(StudentRec));

    file.seekp(position, ios::beg);

    if (!file) {
        cout << "Помилка позиціонування у файлі.\n";
        file.close();
        return;
    }

    file.write(reinterpret_cast<const char*>(&rec),
        sizeof(rec));

    if (!file) {
        cout << "Помилка зміни запису.\n";
        file.close();
        return;
    }

    file.close();

    cout << "Запис успішно змінено!\n";

    current[number - 1] = updated;
    getStudents() = current;
}

// 10. Порівняння розмірів текстового та бінарного файлів
void compareFileSizes() {
    fs::path binaryPath = binaryFilePath();

    cout << "Створення текстового файлу...\n";

    // Зберігаємо поточний список у текстовий файл
    StudentList& students = getStudents();

    if (!createParentDirectories(workingFile))
        return;

    for (const Student& student : students) {
        if (student.name.find(';') != string::npos) {
            cout << "ПІБ містить символ ;. Порівняння неможливе.\n";
            return;
        }
    }

    ofstream textOut(workingFile);

    if (!textOut.is_open()) {
        cout << "Не вдалося створити текстовий файл.\n";
        return;
    }

    for (const Student& student : students) {
        textOut << student.id << ";"
            << student.name << ";"
            << setfill('0') << setw(2) << student.birthDate.day << "."
            << setw(2) << student.birthDate.month << "."
            << setw(4) << student.birthDate.year
            << setfill(' ') << ";"
            << toString(student.faculty) << ";"
            << fixed << setprecision(2)
            << student.avgGrade << "\n";
    }

    textOut.close();

    cout << "Створення бінарного файлу...\n";

    if (!writeBinaryFile(binaryPath))
        return;

    try {
        uintmax_t textSize = fs::file_size(workingFile);
        uintmax_t binarySize = fs::file_size(binaryPath);

        cout << "\n--- ПОРІВНЯННЯ РОЗМІРІВ ---\n";
        cout << "Текстовий файл: "
            << textSize << " байт\n";
        cout << "Бінарний файл: "
            << binarySize << " байт\n";

        if (textSize > binarySize)
            cout << "Бінарний файл менший.\n";
        else if (textSize < binarySize)
            cout << "Текстовий файл менший.\n";
        else
            cout << "Розміри файлів однакові.\n";
    }
    catch (const fs::filesystem_error& e) {
        cout << "Помилка отримання розміру: "
            << e.what() << endl;
    }
}

// Підменю текстових файлів
void textFilesMenu() {
    int choice;

    do {
        cout << "\n=== ТЕКСТОВІ ФАЙЛИ ===\n";
        cout << "1. Зберегти список студентів\n";
        cout << "2. Завантажити список студентів\n";
        cout << "3. Додати запис у кінець файлу\n";
        cout << "4. Статистика текстового файлу\n";
        cout << "5. Пошук слова у файлі\n";
        cout << "6. Назад\n";

        choice = readInt("Ваш вибір: ", 1, 6);

        switch (choice) {
        case 1: saveTextFile(); break;
        case 2: loadTextFile(); break;
        case 3: appendTextFile(); break;
        case 4: textFileStatistics(); break;
        case 5: searchWordInFile(); break;
        case 6: break;
        }

    } while (choice != 6);
}

// Підменю бінарних файлів
void binaryFilesMenu() {
    int choice;

    do {
        cout << "\n=== БІНАРНІ ФАЙЛИ ===\n";
        cout << "6. Зберегти список студентів\n";
        cout << "7. Прочитати бінарний файл\n";
        cout << "8. Прочитати запис за номером\n";
        cout << "9. Змінити запис у файлі\n";
        cout << "10. Порівняти розміри файлів\n";
        cout << "11. Назад\n";

        choice = readInt("Ваш вибір: ", 6, 11);

        switch (choice) {
        case 6: saveBinaryFile(); break;
        case 7: loadBinaryFile(); break;
        case 8: readBinaryRecordByNumber(); break;
        case 9: editBinaryRecord(); break;
        case 10: compareFileSizes(); break;
        case 11: break;
        }

    } while (choice != 11);
}

// Головне меню модуля файлів
void filesTask() {
    int choice;

    do {
        cout << "\n=== РОБОТА З ФАЙЛАМИ ===\n";
        cout << "1. Текстові файли\n";
        cout << "2. Бінарні файли\n";
        cout << "3. Змінити робочий файл\n";
        cout << "4. Назад\n";

        choice = readInt("Ваш вибір: ", 1, 4);

        switch (choice) {
        case 1: textFilesMenu(); break;
        case 2: binaryFilesMenu(); break;
        case 3: changeWorkingFile(); break;
        case 4: break;
        }

    } while (choice != 4);
}
