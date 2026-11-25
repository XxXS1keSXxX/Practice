#include <iostream>
#include "menu_utils.h"
#include "strings_task.h"
#include "types_task.h"
#include "recursion_task.h"
#include "files_task.h"
#include "oop_task.h"
#include "generic_task.h"
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
        cout << "6. Шаблони та власні структури даних\n";
        cout << "0. Вихід\n";

        choice = readInt("Ваш вибір: ", 0, 6);

        switch (choice) {
        case 1:
            stringsMenu();
            break;

        case 2:
            typesTask();
            break;

        case 3:
            recursionTask();
            break;

        case 4:
            filesTask();
            break;

        case 5:
            oopMenu();
            break;
        
        case 6:
            genericMenu();
            break;

        case 0:
            cout << "Дякую за роботу! До побачення!\n";
            break;
        }

    } while (choice != 0);

    return 0;
}
