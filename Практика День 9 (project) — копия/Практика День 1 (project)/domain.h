#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <cmath>

class Shape;

/**
 * @brief Структура прямокутної області, що обмежує геометричну фігуру.
 */
struct Rect {
    double xMin;
    double yMin;
    double xMax;
    double yMax;
};

/**
 * @brief Інтерфейс для фігур, які підтримують експорт у SVG.
 */
class ISvgExportable {
public:
    /// @brief Віртуальний деструктор інтерфейсу.
    virtual ~ISvgExportable() = default;

    /**
     * @brief Створює SVG-представлення фігури.
     * @return Рядок із SVG-кодом.
     */
    virtual std::string toSvg() const = 0;
};

/**
 * @brief Клас двовимірної точки.
 *
 * Зберігає координати X та Y і дозволяє
 * виконувати операції з точками.
 */
class Point {
private:
    double x_;
    double y_;

public:
    /**
     * @brief Створює точку з заданими координатами.
     * @param x Координата X.
     * @param y Координата Y.
     */
    Point(double x = 0, double y = 0);

    /// @brief Повертає координату X.
    double getX() const;

    /// @brief Повертає координату Y.
    double getY() const;

    /**
     * @brief Змінює координату X.
     * @param x Нове значення координати.
     */
    void setX(double x);

    /**
     * @brief Змінює координату Y.
     * @param y Нове значення координати.
     */
    void setY(double y);

    /**
     * @brief Обчислює відстань до іншої точки.
     * @param other Інша точка.
     * @return Відстань між точками.
     */
    double distanceTo(const Point& other) const;

    /**
     * @brief Виводить координати точки.
     * @param os Потік виведення.
     */
    void print(std::ostream& os) const;
};

/**
 * @brief Інтерфейс для об'єктів, які можуть перевіряти перетин.
 */
class IIntersectable {
public:
    /// @brief Віртуальний деструктор інтерфейсу.
    virtual ~IIntersectable() = default;

    /**
     * @brief Перевіряє перетин з іншою геометричною фігурою.
     * @param other Інша геометрична фігура.
     * @return true, якщо фігури перетинаються, інакше false.
     */
    virtual bool intersects(const Shape& other) const = 0;
};

/**
 * @brief Абстрактний базовий клас геометричної фігури.
 *
 * Зберігає унікальний ідентифікатор і колір фігури.
 * Визначає спільний інтерфейс для всіх геометричних фігур.
 */
class Shape {
private:
    int id_;
    static int counter_;

protected:
    std::string color_;

    /// @brief Конструктор копіювання зі створенням нового ідентифікатора.
    Shape(const Shape& other);

public:
    /**
     * @brief Створює геометричну фігуру.
     * @param color Колір фігури.
     */
    Shape(const std::string& color);

    /// @brief Віртуальний деструктор.
    virtual ~Shape() = default;

    /// @brief Повертає унікальний ідентифікатор фігури.
    int getId() const;

    /// @brief Повертає колір фігури.
    std::string getColor() const;

    /**
     * @brief Змінює колір фігури.
     * @param color Новий колір.
     */
    void setColor(const std::string& color);

    /// @brief Повертає назву типу фігури.
    virtual std::string typeName() const = 0;

    /// @brief Обчислює площу фігури.
    virtual double area() const = 0;

    /// @brief Обчислює периметр фігури.
    virtual double perimeter() const = 0;

    /**
     * @brief Переміщує фігуру на задані координати.
     * @param dx Зміщення по осі X.
     * @param dy Зміщення по осі Y.
     */
    virtual void move(double dx, double dy) = 0;

    /**
     * @brief Масштабує фігуру відносно її центра.
     * @param k Коефіцієнт масштабування, більший за нуль.
     */
    virtual void scale(double k) = 0;

    /**
     * @brief Повертає прямокутну область, що обмежує фігуру.
     * @return Межі фігури.
     */
    virtual Rect boundingBox() const = 0;

    /**
     * @brief Виводить інформацію про фігуру.
     * @param os Потік виведення.
     */
    virtual void print(std::ostream& os) const = 0;

    /// @brief Створює SVG-представлення фігури.
    virtual std::string toSVG() const = 0;

    /// @brief Перетворює фігуру на рядок для збереження у файл.
    virtual std::string toFileLine() const = 0;

    /// @brief Створює незалежну копію фігури.
    virtual std::unique_ptr<Shape> clone() const = 0;
};

/**
 * @brief Виводить інформацію про фігуру через потік.
 * @param os Потік виведення.
 * @param shape Геометрична фігура.
 * @return Потік виведення.
 */
std::ostream& operator<<(std::ostream& os, const Shape& shape);

/**
 * @brief Порівнює фігури за площею.
 */
bool operator<(const Shape& first, const Shape& second);

/**
 * @brief Перевіряє рівність фігур за їхніми властивостями.
 */
bool operator==(const Shape& first, const Shape& second);

/**
 * @brief Проміжний базовий клас відкритих геометричних фігур.
 */
class OpenShape : public Shape {
public:
    /// @brief Створює відкриту геометричну фігуру.
    OpenShape(const std::string& color);
};

/**
 * @brief Проміжний базовий клас замкнутих геометричних фігур.
 */
class ClosedShape : public Shape, public IIntersectable {
public:
    /// @brief Створює замкнуту геометричну фігуру.
    ClosedShape(const std::string& color);

    /**
     * @brief Перевіряє перетин із іншою фігурою.
     * @param other Інша геометрична фігура.
     * @return Результат перевірки перетину.
     */
    bool intersects(const Shape& other) const override;
};

/**
 * @brief Клас геометричної лінії.
 *
 * Зберігає початкову та кінцеву точки,
 * обчислює довжину і перевіряє перетини.
 */
class Line : public OpenShape, public IIntersectable, public ISvgExportable {
private:
    Point start_;
    Point end_;

public:
    /**
     * @brief Створює лінію між двома точками.
     * @param start Початкова точка.
     * @param end Кінцева точка.
     * @param color Колір лінії.
     */
    Line(const Point& start, const Point& end, const std::string& color);

    /// @brief Повертає назву типу фігури.
    std::string typeName() const override;

    /// @brief Повертає площу лінії.
    double area() const override;

    /// @brief Обчислює довжину лінії.
    double perimeter() const override;

    void move(double dx, double dy) override;
    void scale(double k) override;
    Rect boundingBox() const override;

    /**
     * @brief Виводить інформацію про лінію.
     * @param os Потік виведення.
     */
    void print(std::ostream& os) const override;

    /// @brief Створює SVG-представлення лінії.
    std::string toSVG() const override;

    /// @brief Реалізує інтерфейс експорту SVG.
    std::string toSvg() const override;

    /// @brief Створює рядок для збереження лінії у файл.
    std::string toFileLine() const override;

    /// @brief Створює копію лінії.
    std::unique_ptr<Shape> clone() const override;

    /**
     * @brief Перевіряє перетин лінії з іншою фігурою.
     * @param other Інша геометрична фігура.
     * @return Результат перевірки перетину.
     */
    bool intersects(const Shape& other) const override;
};

/**
 * @brief Клас геометричного кола.
 *
 * Зберігає координати центра та радіус кола.
 */
class Circle : public ClosedShape, public ISvgExportable {
private:
    Point center_;
    double radius_;

    friend class Circle;
    friend class Rectangle;
    friend class Triangle;
    friend class Line;
    friend class RegularPolygon;

public:
    /**
     * @brief Створює коло.
     * @param center Центр кола.
     * @param radius Радіус кола.
     * @param color Колір кола.
     */
    Circle(const Point& center, double radius, const std::string& color);

    /// @brief Повертає радіус кола.
    double getRadius() const;

    /**
     * @brief Змінює радіус кола.
     * @param radius Новий радіус.
     */
    void setRadius(double radius);

    /// @brief Повертає назву типу фігури.
    std::string typeName() const override;

    /// @brief Обчислює площу кола.
    double area() const override;

    /// @brief Обчислює довжину кола.
    double perimeter() const override;

    void move(double dx, double dy) override;
    void scale(double k) override;
    Rect boundingBox() const override;

    /**
     * @brief Виводить інформацію про коло.
     * @param os Потік виведення.
     */
    void print(std::ostream& os) const override;

    /// @brief Створює SVG-представлення кола.
    std::string toSVG() const override;

    /// @brief Реалізує інтерфейс експорту SVG.
    std::string toSvg() const override;

    /// @brief Створює рядок для збереження кола у файл.
    std::string toFileLine() const override;

    /// @brief Створює копію кола.
    std::unique_ptr<Shape> clone() const override;

    /**
     * @brief Перевіряє перетин кола з іншою фігурою.
     * @param other Інша геометрична фігура.
     * @return Результат перевірки перетину.
     */
    bool intersects(const Shape& other) const override;
};

/**
 * @brief Клас геометричного прямокутника.
 *
 * Зберігає верхню ліву точку, ширину та висоту.
 */
class Rectangle : public ClosedShape, public ISvgExportable {
private:
    Point topLeft_;

    friend class Circle;
    friend class Rectangle;
    friend class Triangle;
    friend class Line;
    friend class RegularPolygon;

protected:
    double width_;
    double height_;

public:
    /**
     * @brief Створює прямокутник.
     * @param topLeft Верхня ліва точка.
     * @param width Ширина прямокутника.
     * @param height Висота прямокутника.
     * @param color Колір прямокутника.
     */
    Rectangle(const Point& topLeft, double width, double height, const std::string& color);

    /**
     * @brief Змінює ширину та висоту прямокутника.
     * @param width Нова ширина.
     * @param height Нова висота.
     */
    void setDimensions(double width, double height);

    /// @brief Повертає назву типу фігури.
    std::string typeName() const override;

    /// @brief Обчислює площу прямокутника.
    double area() const override;

    /// @brief Обчислює периметр прямокутника.
    double perimeter() const override;

    void move(double dx, double dy) override;
    void scale(double k) override;
    Rect boundingBox() const override;

    /**
     * @brief Виводить інформацію про прямокутник.
     * @param os Потік виведення.
     */
    void print(std::ostream& os) const override;

    /// @brief Створює SVG-представлення прямокутника.
    std::string toSVG() const override;

    /// @brief Реалізує інтерфейс експорту SVG.
    std::string toSvg() const override;

    /// @brief Створює рядок для збереження прямокутника у файл.
    std::string toFileLine() const override;

    /// @brief Створює копію прямокутника.
    std::unique_ptr<Shape> clone() const override;

    /**
     * @brief Перевіряє перетин прямокутника з іншою фігурою.
     * @param other Інша геометрична фігура.
     * @return Результат перевірки перетину.
     */
    bool intersects(const Shape& other) const override;
};

/**
 * @brief Клас квадрата.
 *
 * Успадковує прямокутник і забезпечує однакову довжину сторін.
 */
class Square : public Rectangle {
public:
    /**
     * @brief Створює квадрат.
     * @param topLeft Верхня ліва точка.
     * @param side Довжина сторони.
     * @param color Колір квадрата.
     */
    Square(const Point& topLeft, double side, const std::string& color);

    /// @brief Повертає назву типу фігури.
    std::string typeName() const override;

    /// @brief Повертає довжину сторони квадрата.
    double getSide() const;

    /**
     * @brief Змінює довжину обох сторін квадрата.
     * @param side Нова довжина сторони.
     */
    void setSide(double side);

    /// @brief Створює SVG-представлення квадрата.
    std::string toSVG() const override;

    /// @brief Реалізує інтерфейс експорту SVG.
    std::string toSvg() const override;

    /// @brief Створює рядок для збереження квадрата у файл.
    std::string toFileLine() const override;

    /// @brief Створює копію квадрата.
    std::unique_ptr<Shape> clone() const override;

    /**
     * @brief Виводить інформацію про квадрат.
     * @param os Потік виведення.
     */
    void print(std::ostream& os) const override;
};

/**
 * @brief Клас геометричного трикутника.
 *
 * Зберігає координати трьох вершин трикутника.
 */
class Triangle : public ClosedShape, public ISvgExportable {
private:
    Point a_;
    Point b_;
    Point c_;

    friend class Circle;
    friend class Rectangle;
    friend class Triangle;
    friend class Line;
    friend class RegularPolygon;

public:
    /**
     * @brief Створює трикутник.
     * @param a Перша вершина.
     * @param b Друга вершина.
     * @param c Третя вершина.
     * @param color Колір трикутника.
     */
    Triangle(const Point& a, const Point& b, const Point& c, const std::string& color);

    /// @brief Повертає назву типу фігури.
    std::string typeName() const override;

    /// @brief Обчислює площу трикутника.
    double area() const override;

    /// @brief Обчислює периметр трикутника.
    double perimeter() const override;

    /// @brief Повертає геометричний центр трикутника.
    Point getCenter() const;

    void move(double dx, double dy) override;
    void scale(double k) override;
    Rect boundingBox() const override;

    /**
     * @brief Виводить інформацію про трикутник.
     * @param os Потік виведення.
     */
    void print(std::ostream& os) const override;

    /// @brief Створює SVG-представлення трикутника.
    std::string toSVG() const override;

    /// @brief Реалізує інтерфейс експорту SVG.
    std::string toSvg() const override;

    /// @brief Створює рядок для збереження трикутника у файл.
    std::string toFileLine() const override;

    /// @brief Створює копію трикутника.
    std::unique_ptr<Shape> clone() const override;

    /**
     * @brief Перевіряє перетин трикутника з іншою фігурою.
     * @param other Інша геометрична фігура.
     * @return Результат перевірки перетину.
     */
    bool intersects(const Shape& other) const override;
};

/**
 * @brief Клас правильного багатокутника.
 *
 * Зберігає центр, кількість сторін і довжину сторони.
 */
class RegularPolygon : public ClosedShape, public ISvgExportable {
private:
    Point center_;
    int sides_;
    double sideLength_;

    friend class Circle;
    friend class Rectangle;
    friend class Triangle;
    friend class Line;
    friend class RegularPolygon;

public:
    /**
     * @brief Створює правильний багатокутник.
     * @param center Центр багатокутника.
     * @param sides Кількість сторін від 3 до 20.
     * @param sideLength Довжина сторони.
     * @param color Колір багатокутника.
     */
    RegularPolygon(const Point& center, int sides,
        double sideLength, const std::string& color);

    /// @brief Повертає назву типу фігури.
    std::string typeName() const override;

    /// @brief Повертає кількість сторін.
    int getSides() const;

    /// @brief Повертає довжину сторони.
    double getSideLength() const;

    /// @brief Обчислює площу правильного багатокутника.
    double area() const override;

    /// @brief Обчислює периметр правильного багатокутника.
    double perimeter() const override;

    void move(double dx, double dy) override;
    void scale(double k) override;
    Rect boundingBox() const override;

    /**
     * @brief Виводить інформацію про багатокутник.
     * @param os Потік виведення.
     */
    void print(std::ostream& os) const override;

    /// @brief Створює SVG-представлення багатокутника.
    std::string toSVG() const override;

    /// @brief Реалізує інтерфейс експорту SVG.
    std::string toSvg() const override;

    /// @brief Створює рядок для збереження багатокутника у файл.
    std::string toFileLine() const override;

    /// @brief Створює копію багатокутника.
    std::unique_ptr<Shape> clone() const override;

    /**
     * @brief Перевіряє перетин багатокутника з іншою фігурою.
     * @param other Інша геометрична фігура.
     * @return Результат перевірки перетину.
     */
    bool intersects(const Shape& other) const override;
};