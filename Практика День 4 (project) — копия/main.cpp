#include <iostream>
#include "menu_utils.h"
#include "strings_task.h"
#include <windows.h>

using namespace std;

int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    int choice;

    do {
        cout << "\n================================\n";
        cout << "      НАВЧАЛЬНИЙ КОМПЛЕКС\n";
        cout << "================================\n";

        cout << "1. Робота з рядками\n";
        cout << "2. Користувацькі типи даних\n";
        cout << "3. Рекурсивні функції\n";
        cout << "4. Робота з файлами\n";
        cout << "5. ООП\n";
        cout << "0. Вихід\n";

        choice = readInt("Ваш вибір: ", 0, 5);

        switch (choice) {
        case 1:
            stringsMenu();
            break;

        case 2:
            cout << "Модуль буде реалізовано у День 2.\n";
            break;

        case 3:
            cout << "Модуль буде реалізовано у День 3.\n";
            break;

        case 4:
            cout << "Модуль буде реалізовано у День 4.\n";
            break;

        case 5:
            cout << "Модуль буде реалізовано у День 5.\n";
            break;

        case 0:
            cout << "Дякую за роботу! До побачення!\n";
            break;
        }

    } while (choice != 0);

    return 0;
}
