#include "oop_task.h"
#include "menu_utils.h"
#include "registry.h"
#include "errors.h"
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <limits>

using namespace std;

Point readPoint() {
    double x = readDouble("Введіть координату X: ", -1000000, 1000000);
    double y = readDouble("Введіть координату Y: ", -1000000, 1000000);

    return Point(x, y);
}

string readColor() {
    string color;

    cout << "Введіть колір фігури: ";
    getline(cin, color);

    return color;
}

void addShape(Registry& registry) {
    cout << "\n=== ДОДАВАННЯ ФІГУРИ ===\n";
    cout << "1. Лінія\n";
    cout << "2. Коло\n";
    cout << "3. Прямокутник\n";
    cout << "4. Трикутник\n";
    cout << "5. Квадрат\n";
    cout << "6. Правильний багатокутник\n";

    int type = readInt("Оберіть тип фігури: ", 1, 6);

    try {
        unique_ptr<Shape> shape;

        if (type == 1) {
            cout << "\nПочаткова точка:\n";
            Point start = readPoint();

            cout << "Кінцева точка:\n";
            Point end = readPoint();

            string color = readColor();

            shape = make_unique<Line>(start, end, color);
        }
        else if (type == 2) {
            cout << "\nЦентр кола:\n";
            Point center = readPoint();

            double radius = readDouble("Введіть радіус: ", 0.000001, 1000000);

            string color = readColor();

            shape = make_unique<Circle>(center, radius, color);
        }
        else if (type == 3) {
            cout << "\nВерхній лівий кут:\n";
            Point topLeft = readPoint();

            double width = readDouble("Введіть ширину: ", 0.000001, 1000000);
            double height = readDouble("Введіть висоту: ", 0.000001, 1000000);

            string color = readColor();

            shape = make_unique<Rectangle>(topLeft, width, height, color);
        }
        else if (type == 4) {
            cout << "\nПерша точка:\n";
            Point a = readPoint();

            cout << "Друга точка:\n";
            Point b = readPoint();

            cout << "Третя точка:\n";
            Point c = readPoint();

            string color = readColor();

            shape = make_unique<Triangle>(a, b, c, color);
        }
        else if (type == 5) {
            cout << "\nВерхній лівий кут квадрата:\n";
            Point topLeft = readPoint();

            double side = readDouble("Введіть довжину сторони: ", 0.000001, 1000000);

            string color = readColor();

            shape = make_unique<Square>(topLeft, side, color);
        }
        else {
            cout << "\nЦентр правильного багатокутника:\n";
            Point center = readPoint();

            int sides = readInt("Введіть кількість сторін (3-20): ", 3, 20);

            double sideLength = readDouble("Введіть довжину сторони: ", 0.000001, 1000000);

            string color = readColor();

            shape = make_unique<RegularPolygon>(center, sides, sideLength, color);
        }

        registry.add(move(shape));

        cout << "Фігуру успішно додано!\n";
    }
    catch (const exception& e) {
        cout << "Помилка: " << e.what() << '\n';
    }
}

void runSelfTests() {
    int passed = 0;
    int failed = 0;

    const double EPSILON = 0.0001;
    const double PI = 3.141592653589793;

    auto test = [&](const string& name, bool result) {
        if (result) {
            cout << "[PASS] " << name << '\n';
            passed++;
        }
        else {
            cout << "[FAIL] " << name << '\n';
            failed++;
        }
        };

    cout << "\n===== АВТОМАТИЧНЕ ТЕСТУВАННЯ =====\n\n";

    try {
        Point p1(0, 0);
        Point p2(3, 4);

        test("Відстань між точками",
            abs(p1.distanceTo(p2) - 5.0) < EPSILON);

        Circle circle(Point(0, 0), 2, "red");

        test("Площа кола",
            abs(circle.area() - 4 * PI) < EPSILON);

        test("Довжина кола",
            abs(circle.perimeter() - 4 * PI) < EPSILON);

        Rectangle rectangle(Point(0, 0), 4, 3, "blue");

        test("Площа прямокутника",
            abs(rectangle.area() - 12.0) < EPSILON);

        test("Периметр прямокутника",
            abs(rectangle.perimeter() - 14.0) < EPSILON);

        Square square(Point(-1.5, -1.5), 3, "purple");

        test("Площа квадрата",
            abs(square.area() - 9.0) < EPSILON);

        test("Периметр квадрата",
            abs(square.perimeter() - 12.0) < EPSILON);

        Rect squareBox = square.boundingBox();

        test("Габарити квадрата",
            abs(squareBox.xMin + 1.5) < EPSILON &&
            abs(squareBox.yMin + 1.5) < EPSILON &&
            abs(squareBox.xMax - 1.5) < EPSILON &&
            abs(squareBox.yMax - 1.5) < EPSILON);

        Triangle triangle(
            Point(0, 0),
            Point(4, 0),
            Point(0, 3),
            "green"
        );

        test("Площа трикутника",
            abs(triangle.area() - 6.0) < EPSILON);

        test("Периметр трикутника",
            abs(triangle.perimeter() - 12.0) < EPSILON);

        Point center = triangle.getCenter();

        test("Геометричний центр трикутника",
            abs(center.getX() - 4.0 / 3.0) < EPSILON &&
            abs(center.getY() - 1.0) < EPSILON);

        RegularPolygon polygon(Point(0, 0), 6, 2, "orange");

        test("Кількість сторін шестикутника",
            polygon.getSides() == 6);

        test("Довжина сторони шестикутника",
            abs(polygon.getSideLength() - 2.0) < EPSILON);

        test("Площа правильного шестикутника",
            abs(polygon.area() - 6 * sqrt(3.0)) < EPSILON);

        test("Периметр правильного шестикутника",
            abs(polygon.perimeter() - 12.0) < EPSILON);

        test("Радіус описаного кола шестикутника",
            abs(polygon.boundingBox().xMax -
                polygon.boundingBox().xMin - 4.0) < EPSILON);

        Line line(Point(0, 0), Point(3, 4), "black");

        test("Довжина лінії",
            abs(line.perimeter() - 5.0) < EPSILON);

        Circle circle2(Point(3, 0), 2, "yellow");

        test("Перетин двох кіл",
            circle.intersects(circle2));

        auto copy = circle.clone();

        test("Клонування фігури",
            copy != nullptr &&
            copy->typeName() == circle.typeName() &&
            copy->getId() != circle.getId());

        test("Оператор ==",
            copy != nullptr && *copy == circle);

        test("Оператор <",
            triangle < circle);

        test("Генерація SVG",
            circle.toSVG().find("<circle") != string::npos);

        test("Експорт квадрата у SVG",
            square.toSVG().find("<rect") != string::npos);

        test("Експорт трикутника у SVG",
            triangle.toSVG().find("<polygon") != string::npos);

        test("Експорт багатокутника у SVG",
            polygon.toSVG().find("<polygon") != string::npos);
    }
    catch (const exception& e) {
        cout << "[ERROR] " << e.what() << '\n';
        failed++;
    }

    cout << "\n===== РЕЗУЛЬТАТИ =====\n";
    cout << "Успішно: " << passed << '\n';
    cout << "Помилок: " << failed << '\n';
    cout << "Усього тестів: " << passed + failed << '\n';
}

void oopMenu() {
    Registry registry;
    int choice;

    do {
        cout << "\n=== ДЕНЬ 5. ООП: ГРАФІЧНИЙ РЕДАКТОР ===\n";
        cout << "1. Додати фігуру\n";
        cout << "2. Показати всі фігури\n";
        cout << "3. Видалити фігуру за ID\n";
        cout << "4. Дублювати фігуру\n";
        cout << "5. Пошук фігури за ID\n";
        cout << "6. Фільтрація за типом\n";
        cout << "7. Сортування фігур\n";
        cout << "8. Перевірити перетин двох фігур\n";
        cout << "9. Зберегти фігури у файл\n";
        cout << "10. Завантажити фігури з файлу\n";
        cout << "11. Експортувати фігури у SVG\n";
        cout << "12. Автоматичне тестування\n";
        cout << "13. Знайти найбільші фігури\n";
        cout << "14. Сортувати за площею через оператор <\n";
        cout << "0. Назад\n";

        choice = readInt("Ваш вибір: ", 0, 14);

        switch (choice) {
        case 1:
            addShape(registry);
            break;

        case 2:
            registry.showAll();
            break;

        case 3: {
            int id = readInt("Введіть ID фігури: ", 1, 1000000000);

            if (registry.removeById(id)) {
                cout << "Фігуру видалено!\n";
            }
            else {
                cout << "Фігуру з таким ID не знайдено.\n";
            }

            break;
        }

        case 4: {
            int id = readInt("Введіть ID фігури для дублювання: ", 1, 1000000000);

            if (registry.duplicateById(id)) {
                cout << "Фігуру успішно дубльовано!\n";
            }
            else {
                cout << "Фігуру з таким ID не знайдено.\n";
            }

            break;
        }

        case 5: {
            int id = readInt("Введіть ID фігури: ", 1, 1000000000);

            const Shape* shape = registry.findById(id);

            if (shape != nullptr) {
                cout << *shape << '\n';
            }
            else {
                cout << "Фігуру не знайдено.\n";
            }

            break;
        }

        case 6: {
            cout << "1. Лінія\n";
            cout << "2. Коло\n";
            cout << "3. Прямокутник\n";
            cout << "4. Трикутник\n";
            cout << "5. Квадрат\n";
            cout << "6. Правильний багатокутник\n";

            int type = readInt("Оберіть тип: ", 1, 6);

            string types[] = {
                "Лінія",
                "Коло",
                "Прямокутник",
                "Трикутник",
                "Квадрат",
                "Правильний багатокутник"
            };

            auto result = registry.filterByType(types[type - 1]);

            if (result.empty()) {
                cout << "Фігур такого типу немає.\n";
            }
            else {
                for (const Shape* shape : result) {
                    cout << *shape << '\n';
                }
            }

            break;
        }

        case 7: {
            cout << "1. За ID\n";
            cout << "2. За площею\n";
            cout << "3. За периметром\n";

            int sortType = readInt("Оберіть спосіб сортування: ", 1, 3);

            if (sortType == 1) {
                registry.sortById();
            }
            else if (sortType == 2) {
                registry.sortByArea();
            }
            else {
                registry.sortByPerimeter();
            }

            cout << "Фігури успішно відсортовано!\n";
            registry.showAll();

            break;
        }
        
        case 8: {
            if (registry.size() < 2) {
                cout << "Для перевірки потрібно щонайменше дві фігури.\n";
                break;
            }

            int firstId = readInt("Введіть ID першої фігури: ", 1, 1000000000);
            int secondId = readInt("Введіть ID другої фігури: ", 1, 1000000000);

            const Shape* first = registry.findById(firstId);
            const Shape* second = registry.findById(secondId);

            if (first == nullptr || second == nullptr) {
                cout << "Одну або обидві фігури не знайдено.\n";
                break;
            }

            if (firstId == secondId) {
                cout << "Потрібно вибрати дві різні фігури.\n";
                break;
            }

            const IIntersectable* firstIntersectable =
                dynamic_cast<const IIntersectable*>(first);

            const IIntersectable* secondIntersectable =
                dynamic_cast<const IIntersectable*>(second);

            bool result = false;

            if (firstIntersectable != nullptr) {
                result = firstIntersectable->intersects(*second);
            }

            if (!result && secondIntersectable != nullptr) {
                result = secondIntersectable->intersects(*first);
            }

            cout << "\n=== РЕЗУЛЬТАТ ПЕРЕВІРКИ ===\n";

            cout << "Перша фігура: " << *first << '\n';
            cout << "Друга фігура: " << *second << '\n';

            if (result) {
                cout << "Фігури перетинаються!\n";
            }
            else {
                cout << "Фігури не перетинаються.\n";
            }

            break;
        }
        
        case 9: {
            string filename;
            cout << "Введіть назву файлу для збереження: ";
            getline(cin, filename);

            try {
                registry.saveToFile(filename);
            }
            catch (const exception& e) {
                cout << "Помилка: " << e.what() << '\n';
            }
            break;
        }

        case 10: {
            string filename;
            cout << "Введіть назву файлу для завантаження: ";
            getline(cin, filename);

            try {
                registry.loadFromFile(filename);
            }
            catch (const exception& e) {
                cout << "Помилка: " << e.what() << '\n';
            }
            break;
        }
        
        case 11: {
            string filename;
            cout << "Введіть назву SVG-файлу: ";
            getline(cin, filename);

            try {
                registry.exportToSVG(filename);
            }
            catch (const exception& e) {
                cout << "Помилка: " << e.what() << '\n';
            }
            break;
        }
        
        case 12: {
            runSelfTests();
            break;
        }

        case 13: {
            const Shape* largestArea = registry.findLargestByArea();
            const Shape* largestPerimeter = registry.findLargestByPerimeter();

            if (largestArea == nullptr) {
                cout << "Колекція фігур порожня!\n";
            }
            else {
                cout << "\nФігура з найбільшою площею:\n";
                cout << *largestArea;

                cout << "\nФігура з найбільшим периметром:\n";
                cout << *largestPerimeter;
            }

            break;
        }

        case 14: {
            registry.sortByAreaUsingOperator();
            cout << "Фігури відсортовано за площею через оператор <!\n";
            registry.showAll();
            break;
        }

        case 0:
            cout << "Повернення до головного меню...\n";
            break;
        }

    } while (choice != 0);
}