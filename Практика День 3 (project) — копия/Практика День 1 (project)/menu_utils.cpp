#include "menu_utils.h"
#include <iostream>
#include <limits>

using namespace std;

// Введення цілого числа з перевіркою
int readInt(const string& prompt, int min, int max) {
    int value;

    while (true) {
        cout << prompt;

        if (cin >> value && value >= min && value <= max) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }

        cout << "Помилка введення! Спробуйте ще раз.\n";

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// Введення дробового числа з перевіркою
double readDouble(const string& prompt, double min, double max) {
    double value;

    while (true) {
        cout << prompt;

        if (cin >> value && value >= min && value <= max) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }

        cout << "Помилка введення! Спробуйте ще раз.\n";

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}