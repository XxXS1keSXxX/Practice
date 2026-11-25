
#include "generic_task.h"
#include "hash_map.h"
#include <iostream>
#include <string>
#include <limits>
#include <algorithm>
#include <unordered_map>
#include <random>
#include <fstream>
#include <cctype>
#include <vector>
#include <iomanip>
#include <chrono>

using namespace std;

struct Student {
    int id;
    string name;
    int age;
    double averageGrade;
};

void testHashMapAgainstSTL() {
    HashMap<int, int> customMap;
    unordered_map<int, int> standardMap;

    mt19937 generator(42);
    uniform_int_distribution<int> operation(0, 3);
    uniform_int_distribution<int> keyDistribution(-1000, 1000);
    uniform_int_distribution<int> valueDistribution(-10000, 10000);

    bool success = true;

    for (int i = 0; i < 10000; ++i) {
        int op = operation(generator);
        int key = keyDistribution(generator);
        int value = valueDistribution(generator);

        if (op == 0) {
            bool customResult = customMap.insert(key, value);
            bool standardResult = standardMap.insert({ key, value }).second;

            if (customResult != standardResult) {
                success = false;
                break;
            }
        }
        else if (op == 1) {
            bool customResult = customMap.erase(key);
            bool standardResult = standardMap.erase(key) > 0;

            if (customResult != standardResult) {
                success = false;
                break;
            }
        }
        else if (op == 2) {
            customMap[key] = value;
            standardMap[key] = value;
        }
        else {
            bool customFound = customMap.contains(key);
            bool standardFound = standardMap.find(key) != standardMap.end();

            if (customFound != standardFound) {
                success = false;
                break;
            }
        }

        if (customMap.size() != standardMap.size()) {
            success = false;
            break;
        }

        for (const auto& item : standardMap) {
            if (!customMap.contains(item.first) ||
                customMap.at(item.first) != item.second) {
                success = false;
                break;
            }
        }

        if (!success) {
            break;
        }
    }

    cout << "\n===== АВТОМАТИЧНЕ ТЕСТУВАННЯ =====\n";
    cout << "Кількість операцій: 10000\n";
    cout << "Результат: "
        << (success ? "PASS" : "FAIL") << '\n';
}

void countWordFrequencies() {
    string fileName;

    cout << "Введіть назву текстового файлу: ";
    cin >> fileName;

    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Помилка введення назви файлу.\n";
        return;
    }

    ifstream file(fileName);

    if (!file.is_open()) {
        cout << "Не вдалося відкрити файл.\n";
        return;
    }

    HashMap<string, int> wordMap;
    string word;

    while (file >> word) {
        for (char& c : word) {
            c = static_cast<char>(
                tolower(static_cast<unsigned char>(c))
                );
        }

        word.erase(
            remove_if(word.begin(), word.end(),
                [](unsigned char c) {
                    return ispunct(c);
                }),
            word.end()
        );

        if (!word.empty()) {
            wordMap[word]++;
        }
    }

    file.close();

    vector<pair<string, int>> words;

    for (const auto& item : wordMap) {
        words.push_back({ item.first, item.second });
    }

    sort(words.begin(), words.end(),
        [](const auto& a, const auto& b) {
            if (a.second != b.second) {
                return a.second > b.second;
            }

            return a.first < b.first;
        });

    cout << "\n===== ТОП-10 НАЙЧАСТІШИХ СЛІВ =====\n";

    for (size_t i = 0; i < words.size() && i < 10; ++i) {
        cout << i + 1 << ". "
            << words[i].first << " — "
            << words[i].second << '\n';
    }

    cout << "Усього різних слів: " << wordMap.size() << '\n';
}

bool saveIntMapBinary(const HashMap<int, int>& map,
    const string& fileName) {

    ofstream file(fileName, ios::binary);

    if (!file.is_open()) {
        return false;
    }

    size_t count = map.size();

    file.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (const auto& item : map) {
        int key = item.first;
        int value = item.second;

        file.write(reinterpret_cast<const char*>(&key), sizeof(key));
        file.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    return file.good();
}

bool loadIntMapBinary(HashMap<int, int>& map,
    const string& fileName) {

    ifstream file(fileName, ios::binary);

    if (!file.is_open()) {
        return false;
    }

    size_t count = 0;

    file.read(reinterpret_cast<char*>(&count), sizeof(count));

    if (!file.good() || count > 1000000) {
        return false;
    }

    HashMap<int, int> loaded;

    for (size_t i = 0; i < count; ++i) {
        int key;
        int value;

        file.read(reinterpret_cast<char*>(&key), sizeof(key));
        file.read(reinterpret_cast<char*>(&value), sizeof(value));

        if (!file.good()) {
            return false;
        }

        loaded.insert(key, value);
    }

    map = std::move(loaded);

    return true;
}

bool saveStringMapText(const HashMap<string, int>& map,
    const string& fileName) {

    ofstream file(fileName);

    if (!file.is_open()) {
        return false;
    }

    file << map.size() << '\n';

    for (const auto& item : map) {
        file << quoted(item.first) << ' '
            << item.second << '\n';
    }

    return file.good();
}

bool loadStringMapText(HashMap<string, int>& map,
    const string& fileName) {

    ifstream file(fileName);

    if (!file.is_open()) {
        return false;
    }

    size_t count = 0;

    file >> count;

    if (file.fail() || count > 1000000) {
        return false;
    }

    HashMap<string, int> loaded;

    for (size_t i = 0; i < count; ++i) {
        string key;
        int value;

        file >> quoted(key) >> value;

        if (file.fail()) {
            return false;
        }

        loaded.insert(key, value);
    }

    map = std::move(loaded);

    return true;
}

bool saveStudentMapText(const HashMap<int, Student>& map,
    const string& fileName) {

    ofstream file(fileName);

    if (!file.is_open()) {
        return false;
    }

    file << map.size() << '\n';

    for (const auto& item : map) {
        const Student& student = item.second;

        file << item.first << ' '
            << student.id << ' '
            << quoted(student.name) << ' '
            << student.age << ' '
            << student.averageGrade << '\n';
    }

    return file.good();
}

bool loadStudentMapText(HashMap<int, Student>& map,
    const string& fileName) {

    ifstream file(fileName);

    if (!file.is_open()) {
        return false;
    }

    size_t count = 0;

    file >> count;

    if (file.fail() || count > 1000000) {
        return false;
    }

    HashMap<int, Student> loaded;

    for (size_t i = 0; i < count; ++i) {
        int key;
        Student student{};

        file >> key
            >> student.id
            >> quoted(student.name)
            >> student.age
            >> student.averageGrade;

        if (file.fail()) {
            return false;
        }

        loaded.insert(key, student);
    }

    map = std::move(loaded);

    return true;
}

void demoStringMap() {
    HashMap<string, int> ages;

    ages.insert("Олександр", 18);
    ages.insert("Андрій", 19);
    ages.insert("Максим", 17);

    cout << "\nСписок студентів:\n";

    for (auto it = ages.begin(); it != ages.end(); ++it) {
        cout << it->first << " — " << it->second << " років\n";
    }

    cout << "\nПошук Олександра: ";

    int* age = ages.find("Олександр");

    if (age) {
        cout << *age << " років\n";
    }
    else {
        cout << "Студента не знайдено.\n";
    }

    ages["Ярослав"] = 18;

    cout << "Кількість записів: " << ages.size() << '\n';

    HashMap<string, int> saveMap;

    saveMap.insert("Олександр", 20);
    saveMap.insert("Ярослав", 18);
    saveMap.insert("Іван Петренко", 21);

    cout << "\nПеревірка текстового збереження:\n";

    if (saveStringMapText(saveMap, "hashmap.txt")) {
        cout << "Дані успішно збережено.\n";
    }
    else {
        cout << "Помилка збереження.\n";
    }

    HashMap<string, int> loadedMap;

    if (loadStringMapText(loadedMap, "hashmap.txt")) {
        cout << "Дані успішно завантажено.\n";
        cout << "Кількість елементів: " << loadedMap.size() << '\n';
        cout << "Значення Ярослав: " << loadedMap.at("Ярослав") << '\n';
        cout << "Значення Іван Петренко: "
            << loadedMap.at("Іван Петренко") << '\n';
    }
    else {
        cout << "Помилка завантаження.\n";
    }
}

void demoStudentMap() {
    HashMap<int, Student> students;

    students.insert(1, { 1, "Олександр", 18, 4.5 });
    students.insert(2, { 2, "Андрій", 19, 4.8 });
    students.insert(3, { 3, "Максим", 17, 4.2 });

    cout << "\nСписок студентів:\n";

    for (auto it = students.begin(); it != students.end(); ++it) {
        cout << "ID: " << it->first
            << ", ім'я: " << it->second.name
            << ", вік: " << it->second.age
            << ", середній бал: " << it->second.averageGrade
            << '\n';
    }

    cout << "\nПошук студента з ID 2:\n";

    Student* student = students.find(2);

    if (student) {
        cout << "Ім'я: " << student->name << '\n';
        cout << "Вік: " << student->age << '\n';
        cout << "Середній бал: " << student->averageGrade << '\n';
    }
    else {
        cout << "Студента не знайдено.\n";
    }

    cout << "\nКількість студентів: " << students.size() << '\n';

    cout << "\nПеревірка текстового збереження студентів:\n";

    if (saveStudentMapText(students, "students.txt")) {
        cout << "Дані студентів успішно збережено.\n";
    }
    else {
        cout << "Помилка збереження студентів.\n";
    }

    HashMap<int, Student> loadedStudents;

    if (loadStudentMapText(loadedStudents, "students.txt")) {
        cout << "Дані студентів успішно завантажено.\n";

        for (const auto& item : loadedStudents) {
            cout << item.second.name
                << ", вік: " << item.second.age
                << ", середній бал: " << item.second.averageGrade
                << '\n';
        }
    }
    else {
        cout << "Помилка завантаження студентів.\n";
    }
}

void benchmarkHashMap() {
    vector<int> sizes = { 1000, 10000, 50000 };

    cout << "\n===== ПОРІВНЯННЯ ШВИДКОДІЇ =====\n";
    cout << fixed << setprecision(3);

    for (int count : sizes) {
        HashMap<int, int> customMap;
        unordered_map<int, int> standardMap;

        auto customStart = chrono::high_resolution_clock::now();

        for (int i = 0; i < count; ++i) {
            customMap.insert(i, i * 10);
        }

        for (int i = 0; i < count; ++i) {
            customMap.find(i);
        }

        auto customEnd = chrono::high_resolution_clock::now();

        auto standardStart = chrono::high_resolution_clock::now();

        for (int i = 0; i < count; ++i) {
            standardMap.insert({ i, i * 10 });
        }

        for (int i = 0; i < count; ++i) {
            standardMap.find(i);
        }

        auto standardEnd = chrono::high_resolution_clock::now();

        double customTime =
            chrono::duration<double, milli>(
                customEnd - customStart
            ).count();

        double standardTime =
            chrono::duration<double, milli>(
                standardEnd - standardStart
            ).count();

        cout << "\nКількість елементів: " << count << '\n';
        cout << "Власна HashMap: " << customTime << " мс\n";
        cout << "std::unordered_map: " << standardTime << " мс\n";
    }
}

void testHashMapErrors() {
    cout << "\n===== ПЕРЕВІРКА ПОМИЛКОВИХ СИТУАЦІЙ =====\n";

    HashMap<int, int> map;

    try {
        map.at(100);
        cout << "Помилка: at() не виявив відсутній ключ.\n";
    }
    catch (const out_of_range&) {
        cout << "PASS: at() правильно обробляє відсутній ключ.\n";
    }

    try {
        HashMap<int, int> invalidMap(0);
        cout << "Помилка: дозволено створення таблиці з 0 кошиків.\n";
    }
    catch (const invalid_argument&) {
        cout << "PASS: перевірка кількості кошиків працює.\n";
    }

    cout << "Видалення відсутнього ключа: "
        << (map.erase(100) ? "помилка" : "PASS") << '\n';

    cout << "Пошук відсутнього ключа: "
        << (map.find(100) == nullptr ? "PASS" : "помилка") << '\n';
}

void demoIntMap() {
    HashMap<int, int> numbers;

    numbers.insert(1, 100);
    numbers.insert(2, 200);
    numbers.insert(3, 300);

    cout << "\nЕлементи HashMap:\n";

    for (auto it = numbers.begin(); it != numbers.end(); ++it) {
        cout << "Ключ: " << it->first
            << ", значення: " << it->second << '\n';
    }

    cout << "\nПошук ключа 2: ";

    int* value = numbers.find(2);

    if (value) {
        cout << *value << '\n';
    }
    else {
        cout << "Ключ не знайдено.\n";
    }

    numbers[4] = 400;

    cout << "Кількість елементів: " << numbers.size() << '\n';
    cout << "Коефіцієнт заповнення: " << numbers.loadFactor() << '\n';

    numbers.erase(1);

    cout << "Після видалення ключа 1: "
        << numbers.size() << " елементів\n";

    HashMap<int, int> testMap;

    for (int i = 0; i < 100; ++i) {
        testMap.insert(i, i * 10);
    }

    cout << "\nПеревірка перехешування:\n";
    cout << "Кількість елементів: " << testMap.size() << '\n';
    cout << "Кількість кошиків: " << testMap.bucketCount() << '\n';
    cout << "Найдовший ланцюжок: " << testMap.maxChainLength() << '\n';
    cout << "Коефіцієнт заповнення: " << testMap.loadFactor() << '\n';

    bool allFound = true;

    for (int i = 0; i < 100; ++i) {
        if (!testMap.contains(i)) {
            allFound = false;
            break;
        }
    }

    cout << "Перевірка ключів: "
        << (allFound ? "усі 100 знайдено" : "є помилки") << '\n';

    testMap.clear();

    cout << "\nПеревірка очищення:\n";
    cout << "Кількість елементів після очищення: "
        << testMap.size() << '\n';
    cout << "Таблиця порожня: "
        << (testMap.empty() ? "так" : "ні") << '\n';

    HashMap<int, int> original;
    original.insert(1, 100);

    HashMap<int, int> copied(original);

    original[1] = 999;

    cout << "\nПеревірка копіювання:\n";
    cout << "Оригінал: " << original.at(1) << '\n';
    cout << "Копія: " << copied.at(1) << '\n';

    HashMap<int, int> moved(std::move(copied));

    cout << "\nПеревірка переміщення:\n";
    cout << "Переміщена таблиця: " << moved.at(1) << '\n';
    cout << "Розмір переміщеної таблиці: " << moved.size() << '\n';

    cout << "\nПовторне використання початкової таблиці:\n";
    cout << "Розмір після переміщення: " << copied.size() << '\n';

    copied.insert(50, 500);

    cout << "Новий елемент: " << copied.at(50) << '\n';

    HashMap<int, int> source;
    source.insert(5, 500);

    HashMap<int, int> destination;
    destination.insert(10, 1000);

    destination = std::move(source);

    cout << "\nПеревірка переміщувального присвоєння:\n";
    cout << "Значення: " << destination.at(5) << '\n';
    cout << "Кількість елементів: " << destination.size() << '\n';

    cout << "\nПовторне використання початкової таблиці:\n";
    cout << "Розмір після переміщення: " << source.size() << '\n';

    source.insert(60, 600);

    cout << "Новий елемент: " << source.at(60) << '\n';

    HashMap<int, int> assigned;
    assigned.insert(20, 2000);

    assigned = original;

    original[1] = 777;

    cout << "\nПеревірка копіювального присвоєння:\n";
    cout << "Оригінал: " << original.at(1) << '\n';
    cout << "Копія: " << assigned.at(1) << '\n';

    auto it = std::find(
        moved.begin(),
        moved.end(),
        std::pair<const int, int>{1, 100}
    );

    cout << "\nПеревірка std::find():\n";

    if (it != moved.end()) {
        cout << "Елемент знайдено: "
            << it->first << " -> " << it->second << '\n';
    }
    else {
        cout << "Елемент не знайдено.\n";
    }

    cout << "\nПеревірка std::distance():\n";
    cout << "Кількість елементів у таблиці: "
        << std::distance(moved.begin(), moved.end()) << '\n';

    HashMap<int, int> firstMap;
    HashMap<int, int> secondMap;

    firstMap.insert(1, 100);
    firstMap.insert(2, 200);

    secondMap.insert(2, 200);
    secondMap.insert(1, 100);

    cout << "\nПеревірка оператора ==:\n";
    cout << "Таблиці однакові: "
        << (firstMap == secondMap ? "так" : "ні") << '\n';

    secondMap[2] = 999;

    cout << "Після зміни значення: "
        << (firstMap == secondMap ? "так" : "ні") << '\n';

    cout << "Перевірка оператора !=: "
        << (firstMap != secondMap ? "так" : "ні") << '\n';

    cout << "\nПеревірка оператора <<:\n";
    cout << firstMap << '\n';

    testHashMapAgainstSTL();
    testHashMapErrors();

    HashMap<int, int> saveMap;

    saveMap.insert(1, 100);
    saveMap.insert(2, 200);
    saveMap.insert(3, 300);

    cout << "\nПеревірка бінарного збереження:\n";

    if (saveIntMapBinary(saveMap, "hashmap.bin")) {
        cout << "Дані успішно збережено.\n";
    }
    else {
        cout << "Помилка збереження.\n";
    }

    HashMap<int, int> loadedMap;

    if (loadIntMapBinary(loadedMap, "hashmap.bin")) {
        cout << "Дані успішно завантажено.\n";
        cout << "Кількість елементів: " << loadedMap.size() << '\n';
        cout << "Значення ключа 2: " << loadedMap.at(2) << '\n';
    }
    else {
        cout << "Помилка завантаження.\n";
    }

    benchmarkHashMap();
}

void genericMenu() {
    int choice = -1;

    do {
        cout << "\n===== ШАБЛОНИ ТА ВЛАСНІ СТРУКТУРИ ДАНИХ =====\n";
        cout << "1. Демонстрація HashMap<int, int>\n";
        cout << "2. Демонстрація HashMap<string, int>\n";
        cout << "3. Демонстрація HashMap<int, Student>\n";
        cout << "4. Підрахунок частоти слів із файлу\n";
        cout << "0. Назад\n";
        cout << "Ваш вибір: ";

        cin >> choice;

        if (cin.eof()) {
            break;
        }

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Помилка введення. Спробуйте ще раз.\n";
            choice = -1;
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
        case 1:
            demoIntMap();
            break;

        case 2:
            demoStringMap();
            break;

        case 3:
            demoStudentMap();
            break;

        case 4:
            countWordFrequencies();
            break;

        case 0:
            cout << "Повернення до головного меню.\n";
            break;

        default:
            cout << "Невірний пункт меню.\n";
        }

    } while (choice != 0);
}
