
#include "strings_task.h"
#include "menu_utils.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <cctype>

using namespace std;

// Змінна для зберігання введеного рядка
string text;

// 1. Введення рядка
void inputText() {
    cout << "Введіть рядок: ";
    getline(cin, text);

    if (text.empty()) {
        cout << "Рядок не повинен бути порожнім!\n";
    }
    else {
        cout << "Рядок збережено.\n";
    }
}

// 2. Статистика рядка
void showStatistics() {
    if (text.empty()) {
        cout << "Спочатку введіть рядок!\n";
        return;
    }

    int words = 0;
    int vowels = 0;
    int consonants = 0;
    int digits = 0;
    int punctuation = 0;

    bool inWord = false;

    for (char c : text) {
        unsigned char ch = static_cast<unsigned char>(c);

        if (isspace(ch)) {
            inWord = false;
        }
        else if (!inWord) {
            words++;
            inWord = true;
        }

        if (isalpha(ch)) {
            string vowelLetters = "aeiouyAEIOUY";

            if (vowelLetters.find(c) != string::npos)
                vowels++;
            else
                consonants++;
        }

        if (isdigit(ch))
            digits++;

        if (ispunct(ch))
            punctuation++;
    }

    cout << "Кількість символів: " << text.length() << endl;
    cout << "Кількість слів: " << words << endl;
    cout << "Голосних (латиниця): " << vowels << endl;
    cout << "Приголосних (латиниця): " << consonants << endl;
    cout << "Цифр: " << digits << endl;
    cout << "Розділових знаків: " << punctuation << endl;
}

// 3. Найдовше та найкоротше слово
void findWords() {
    stringstream ss(text);
    string word;
    string shortest = "";
    string longest = "";

    while (ss >> word) {
        if (shortest.empty() || word.length() < shortest.length())
            shortest = word;

        if (word.length() > longest.length())
            longest = word;
    }

    if (shortest.empty()) {
        cout << "У рядку немає слів.\n";
    }
    else {
        cout << "Найкоротше слово: " << shortest << endl;
        cout << "Найдовше слово: " << longest << endl;
    }
}

// 4. Перевернути рядок та порядок слів
void reverseText() {
    string reversed = text;

    reverse(reversed.begin(), reversed.end());

    cout << "Рядок навпаки: " << reversed << endl;

    stringstream ss(text);
    vector<string> words;
    string word;

    while (ss >> word) {
        words.push_back(word);
    }

    cout << "Порядок слів навпаки: ";

    for (int i = static_cast<int>(words.size()) - 1; i >= 0; i--) {
        cout << words[i] << " ";
    }

    cout << endl;
}

// 5. Перевірка на паліндром
void checkPalindrome() {
    string s = "";

    for (char c : text) {
        if (!isspace(static_cast<unsigned char>(c))) {
            s += static_cast<char>(tolower(static_cast<unsigned char>(c)));
        }
    }

    string reversed = s;
    reverse(reversed.begin(), reversed.end());

    if (s == reversed)
        cout << "Рядок є паліндромом.\n";
    else
        cout << "Рядок не є паліндромом.\n";
}

// 6. Пошук підрядка
void searchSubstring() {
    string part;

    cout << "Введіть підрядок для пошуку: ";
    getline(cin, part);

    if (part.empty()) {
        cout << "Підрядок не повинен бути порожнім!\n";
        return;
    }

    size_t position = 0;
    int count = 0;

    while ((position = text.find(part, position)) != string::npos) {
        cout << "Позиція: " << position << endl;
        count++;
        position++;
    }

    cout << "Кількість входжень: " << count << endl;
}

// 7. Заміна підрядка
void replaceSubstring() {
    string oldText;
    string newText;

    cout << "Введіть текст для заміни: ";
    getline(cin, oldText);

    if (oldText.empty()) {
        cout << "Текст для заміни не повинен бути порожнім!\n";
        return;
    }

    cout << "Введіть новий текст: ";
    getline(cin, newText);

    size_t position = 0;
    int count = 0;

    while ((position = text.find(oldText, position)) != string::npos) {
        text.replace(position, oldText.length(), newText);
        position += newText.length();
        count++;
    }

    cout << "Кількість замін: " << count << endl;
    cout << "Новий рядок: " << text << endl;
}

// 8. Видалення зайвих пробілів
void removeSpaces() {
    stringstream ss(text);
    string word;
    string result = "";

    while (ss >> word) {
        if (!result.empty())
            result += " ";

        result += word;
    }

    text = result;

    cout << "Результат: " << text << endl;
}

// 9. Шифр Цезаря
void caesarCipher() {
    int choice = readInt(
        "1. Шифрувати\n2. Розшифрувати\nВаш вибір: ", 1, 2);

    int key = readInt("Введіть ключ (1-25): ", 1, 25);

    if (choice == 2)
        key = 26 - key;

    for (char& c : text) {
        if (c >= 'a' && c <= 'z')
            c = 'a' + (c - 'a' + key) % 26;

        else if (c >= 'A' && c <= 'Z')
            c = 'A' + (c - 'A' + key) % 26;
    }

    cout << "Результат: " << text << endl;
}

// Меню текстового процесора
void stringsMenu() {
    int choice;

    do {
        cout << "\n--- ТЕКСТОВИЙ ПРОЦЕСОР ---\n";
        cout << "1. Ввести рядок\n";
        cout << "2. Статистика рядка\n";
        cout << "3. Знайти найдовше та найкоротше слово\n";
        cout << "4. Перевернути рядок та порядок слів\n";
        cout << "5. Перевірка на паліндром\n";
        cout << "6. Пошук підрядка\n";
        cout << "7. Заміна підрядка\n";
        cout << "8. Видалення зайвих пробілів\n";
        cout << "9. Шифр Цезаря\n";
        cout << "0. Назад\n";

        choice = readInt("Ваш вибір: ", 0, 9);

        switch (choice) {
        case 1: inputText(); break;
        case 2: showStatistics(); break;
        case 3: findWords(); break;
        case 4: reverseText(); break;
        case 5: checkPalindrome(); break;
        case 6: searchSubstring(); break;
        case 7: replaceSubstring(); break;
        case 8: removeSpaces(); break;
        case 9: caesarCipher(); break;
        case 0: break;
        }

    } while (choice != 0);
}
