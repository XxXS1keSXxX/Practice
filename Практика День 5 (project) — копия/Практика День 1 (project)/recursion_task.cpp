
#include "recursion_task.h"
#include "menu_utils.h"

#include <iostream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <limits>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

// 1. Факторіал рекурсивно
unsigned long long factorialRecursive(int n) {
    // Базовий випадок
    if (n <= 1)
        return 1;

    // Рекурсивний крок
    return n * factorialRecursive(n - 1);
}

// Факторіал ітеративно
unsigned long long factorialIterative(int n) {
    unsigned long long result = 1;

    for (int i = 2; i <= n; i++)
        result *= i;

    return result;
}

void factorialTask() {
    int n = readInt("Введіть число від 0 до 20: ", 0, 20);

    cout << "Рекурсивний результат: "
        << factorialRecursive(n) << endl;

    cout << "Ітеративний результат: "
        << factorialIterative(n) << endl;
}

// 2. Числа Фібоначчі рекурсивно
unsigned long long fibonacciRecursive(int n, unsigned long long& calls) {
    calls++;

    // Базовий випадок
    if (n <= 1)
        return n;

    // Рекурсивний крок
    return fibonacciRecursive(n - 1, calls) +
        fibonacciRecursive(n - 2, calls);
}

// Ітеративне обчислення Фібоначчі
unsigned long long fibonacciIterative(int n) {
    if (n <= 1)
        return n;

    unsigned long long a = 0, b = 1;

    for (int i = 2; i <= n; i++) {
        unsigned long long temp = a + b;
        a = b;
        b = temp;
    }

    return b;
}

// Рекурсивний вивід перших n чисел Фібоначчі
void printFibonacci(int n, int index,
    unsigned long long a, unsigned long long b) {
    // Базовий випадок
    if (index >= n)
        return;

    cout << a << " ";

    // Рекурсивний крок
    printFibonacci(n, index + 1, b, a + b);
}

void fibonacciTask() {
    int n = readInt("Введіть кількість чисел Фібоначчі (1-94): ", 1, 94);

    cout << "Перші " << n << " чисел Фібоначчі:\n";
    printFibonacci(n, 0, 0, 1);
    cout << endl;

    int number = readInt("Введіть номер числа для порівняння (35-40): ", 35, 40);

    unsigned long long calls = 0;

    auto start = chrono::high_resolution_clock::now();
    unsigned long long recursiveResult = fibonacciRecursive(number, calls);
    auto end = chrono::high_resolution_clock::now();

    auto recursiveTime =
        chrono::duration<double, milli>(end - start).count();

    start = chrono::high_resolution_clock::now();
    unsigned long long iterativeResult = fibonacciIterative(number);
    end = chrono::high_resolution_clock::now();

    auto iterativeTime =
        chrono::duration<double, milli>(end - start).count();

    cout << "\nРезультати:\n";
    cout << "Рекурсивне число: " << recursiveResult << endl;
    cout << "Ітеративне число: " << iterativeResult << endl;
    cout << "Кількість рекурсивних викликів: " << calls << endl;

    cout << fixed << setprecision(3);
    cout << "Час рекурсії: " << recursiveTime << " мс\n";
    cout << "Час ітерації: " << iterativeTime << " мс\n";
}

// 3. Сума цифр числа
int sumDigits(int n) {
    // Базовий випадок
    if (n == 0)
        return 0;

    // Рекурсивний крок
    return n % 10 + sumDigits(n / 10);
}

void sumDigitsTask() {
    int n = readInt("Введіть число (-1000000000 - 1000000000): ",
        -1000000000, 1000000000);

    cout << "Сума цифр: " << sumDigits(abs(n)) << endl;
}

// 4. Піднесення до степеня
double powerRecursive(double x, int n) {
    // Базовий випадок
    if (n == 0)
        return 1;

    // Рекурсивний крок
    if (n < 0)
        return 1.0 / powerRecursive(x, -n);

    return x * powerRecursive(x, n - 1);
}

void powerTask() {
    int x = readInt("Введіть основу числа (-100 - 100): ", -100, 100);
    int n = readInt("Введіть степінь (-20 - 20): ", -20, 20);

    if (x == 0 && n < 0) {
        cout << "Помилка: нуль не можна підносити до від'ємного степеня!\n";
        return;
    }

    cout << "Результат: " << powerRecursive(x, n) << endl;
}

// 5. Найбільший спільний дільник
int gcdRecursive(int a, int b) {
    // Базовий випадок
    if (b == 0)
        return a;

    // Рекурсивний крок
    return gcdRecursive(b, a % b);
}

void gcdTask() {
    int a = readInt("Введіть перше число (0-1000000000): ",
        0, 1000000000);
    int b = readInt("Введіть друге число (0-1000000000): ",
        0, 1000000000);

    if (a == 0 && b == 0) {
        cout << "НСД для двох нулів не визначений!\n";
        return;
    }

    cout << "Найбільший спільний дільник: "
        << gcdRecursive(a, b) << endl;
}

// 6. Розворот рядка
string reverseRecursive(const string& str, int index) {
    // Базовий випадок
    if (index < 0)
        return "";

    // Рекурсивний крок
    return str[index] + reverseRecursive(str, index - 1);
}

// Перевірка паліндрому
bool palindromeRecursive(const string& str, int left, int right) {
    // Базовий випадок
    if (left >= right)
        return true;

    if (str[left] != str[right])
        return false;

    // Рекурсивний крок
    return palindromeRecursive(str, left + 1, right - 1);
}

void stringTask() {
    string str;

    cout << "Введіть рядок: ";
    getline(cin, str);

    cout << "Розвернутий рядок: "
        << reverseRecursive(str, static_cast<int>(str.size()) - 1)
        << endl;

    if (palindromeRecursive(str, 0, static_cast<int>(str.size()) - 1))
        cout << "Рядок є паліндромом.\n";
    else
        cout << "Рядок не є паліндромом.\n";
}

// 7. Переведення десяткового числа у двійкову систему
string binaryRecursive(unsigned long long n) {
    // Базовий випадок
    if (n < 2)
        return to_string(n);

    // Рекурсивний крок
    return binaryRecursive(n / 2) + to_string(n % 2);
}

void binaryTask() {
    int n = readInt("Введіть десяткове число (-1000000000 - 1000000000): ",
        -1000000000, 1000000000);

    cout << "Двійкове число: ";

    if (n < 0)
        cout << "-";

    cout << binaryRecursive(n < 0 ? -static_cast<long long>(n) : n)
        << endl;
}

// 8. Ханойські вежі
void hanoi(int n, char from, char to, char auxiliary) {
    // Базовий випадок
    if (n == 0)
        return;

    // Рекурсивний крок
    hanoi(n - 1, from, auxiliary, to);

    cout << "Перемістити диск " << n
        << " зі стрижня " << from
        << " на стрижень " << to << endl;

    hanoi(n - 1, auxiliary, to, from);
}

void hanoiTask() {
    int n = readInt("Введіть кількість дисків (1-10): ", 1, 10);

    cout << "\nПослідовність переміщень:\n";

    hanoi(n, 'A', 'C', 'B');

    unsigned long long moves = (1ULL << n) - 1;

    cout << "\nЗагальна кількість ходів: " << moves << endl;
}

// 9. Рекурсивний обхід каталогу
void directoryRecursive(const fs::path& path, int depth) {
    // Базовий випадок
    if (depth > 20)
        return;

    error_code error;

    fs::directory_iterator it(path, error);

    if (error) {
        cout << string(depth * 2, ' ')
            << "[Немає доступу]\n";
        return;
    }

    // Рекурсивний крок
    for (const auto& entry : it) {
        cout << string(depth * 2, ' ')
            << entry.path().filename().string();

        error_code typeError;

        if (entry.is_directory(typeError) && !typeError) {
            cout << "/\n";
            directoryRecursive(entry.path(), depth + 1);
        }
        else {
            cout << endl;
        }
    }
}

void directoryTask() {
    string path;

    cout << "Введіть шлях до каталогу: ";
    getline(cin, path);

    if (!fs::exists(path) || !fs::is_directory(path)) {
        cout << "Каталог не знайдено!\n";
        return;
    }

    cout << "\nДерево файлів:\n";
    directoryRecursive(path, 0);
}

// Головне меню рекурсії
void recursionTask() {
    int choice;

    do {
        cout << "\n========== РЕКУРСІЯ ==========\n";
        cout << "1. Факторіал числа\n";
        cout << "2. Числа Фібоначчі\n";
        cout << "3. Сума цифр числа\n";
        cout << "4. Піднесення до степеня\n";
        cout << "5. Найбільший спільний дільник\n";
        cout << "6. Розворот рядка та перевірка паліндрома\n";
        cout << "7. Переведення числа у двійкову систему\n";
        cout << "8. Ханойські вежі\n";
        cout << "9. Обхід каталогу\n";
        cout << "10. Назад\n";

        choice = readInt("Ваш вибір: ", 1, 10);

        switch (choice) {
        case 1:
            factorialTask();
            break;
        case 2:
            fibonacciTask();
            break;
        case 3:
            sumDigitsTask();
            break;
        case 4:
            powerTask();
            break;
        case 5:
            gcdTask();
            break;
        case 6:
            stringTask();
            break;
        case 7:
            binaryTask();
            break;
        case 8:
            hanoiTask();
            break;
        case 9:
            directoryTask();
            break;
        case 10:
            cout << "Повернення до головного меню.\n";
            break;
        }

    } while (choice != 10);
}
