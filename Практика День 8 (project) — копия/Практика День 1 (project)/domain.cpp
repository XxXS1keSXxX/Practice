
#include "domain.h"
#include "errors.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

using namespace std;

namespace {
    const double EPSILON = 1e-10;
    const double PI = 3.14159265358979323846;

    bool circlesIntersect(const Point& a, double r1,
        const Point& b, double r2) {
        return a.distanceTo(b) <= r1 + r2;
    }

    double crossProduct(const Point& a, const Point& b, const Point& c) {
        return (b.getX() - a.getX()) * (c.getY() - a.getY()) -
            (b.getY() - a.getY()) * (c.getX() - a.getX());
    }

    bool pointInPolygon(const Point& p, const vector<Point>& polygon) {
        bool inside = false;

        for (size_t i = 0, j = polygon.size() - 1;
            i < polygon.size(); j = i++) {

            const Point& a = polygon[i];
            const Point& b = polygon[j];

            double cross = crossProduct(a, b, p);

            if (abs(cross) < EPSILON &&
                p.getX() >= min(a.getX(), b.getX()) - EPSILON &&
                p.getX() <= max(a.getX(), b.getX()) + EPSILON &&
                p.getY() >= min(a.getY(), b.getY()) - EPSILON &&
                p.getY() <= max(a.getY(), b.getY()) + EPSILON) {
                return true;
            }

            if ((a.getY() > p.getY()) != (b.getY() > p.getY())) {
                double x = (b.getX() - a.getX()) *
                    (p.getY() - a.getY()) /
                    (b.getY() - a.getY()) + a.getX();

                if (p.getX() < x) {
                    inside = !inside;
                }
            }
        }

        return inside;
    }

    bool onSegment(const Point& a, const Point& b, const Point& p) {
        return abs(crossProduct(a, b, p)) < EPSILON &&
            p.getX() >= min(a.getX(), b.getX()) - EPSILON &&
            p.getX() <= max(a.getX(), b.getX()) + EPSILON &&
            p.getY() >= min(a.getY(), b.getY()) - EPSILON &&
            p.getY() <= max(a.getY(), b.getY()) + EPSILON;
    }

    bool segmentsIntersect(
        const Point& a, const Point& b,
        const Point& c, const Point& d) {

        double c1 = crossProduct(a, b, c);
        double c2 = crossProduct(a, b, d);
        double c3 = crossProduct(c, d, a);
        double c4 = crossProduct(c, d, b);

        if (((c1 > EPSILON && c2 < -EPSILON) ||
            (c1 < -EPSILON && c2 > EPSILON)) &&
            ((c3 > EPSILON && c4 < -EPSILON) ||
                (c3 < -EPSILON && c4 > EPSILON))) {
            return true;
        }

        return onSegment(a, b, c) ||
            onSegment(a, b, d) ||
            onSegment(c, d, a) ||
            onSegment(c, d, b);
    }

    double pointSegmentDistance(
        const Point& p, const Point& a, const Point& b) {

        double dx = b.getX() - a.getX();
        double dy = b.getY() - a.getY();

        double lengthSquared = dx * dx + dy * dy;

        if (lengthSquared < EPSILON) {
            return p.distanceTo(a);
        }

        double t = ((p.getX() - a.getX()) * dx +
            (p.getY() - a.getY()) * dy) / lengthSquared;

        t = max(0.0, min(1.0, t));

        Point closest(
            a.getX() + t * dx,
            a.getY() + t * dy
        );

        return p.distanceTo(closest);
    }

    bool circleTriangleIntersect(
        const Point& center, double radius,
        const vector<Point>& triangle) {

        if (pointInPolygon(center, triangle)) {
            return true;
        }

        for (size_t i = 0; i < triangle.size(); i++) {
            const Point& a = triangle[i];
            const Point& b = triangle[(i + 1) % triangle.size()];

            if (pointSegmentDistance(center, a, b) <= radius) {
                return true;
            }
        }

        return false;
    }

    bool polygonsIntersect(
        const vector<Point>& first,
        const vector<Point>& second) {

        for (size_t i = 0; i < first.size(); i++) {
            for (size_t j = 0; j < second.size(); j++) {
                if (segmentsIntersect(
                    first[i], first[(i + 1) % first.size()],
                    second[j], second[(j + 1) % second.size()])) {
                    return true;
                }
            }
        }

        return pointInPolygon(first[0], second) ||
            pointInPolygon(second[0], first);
    }

    bool rectanglesIntersect(
        const Point& a, double aw, double ah,
        const Point& b, double bw, double bh) {

        return a.getX() <= b.getX() + bw &&
            a.getX() + aw >= b.getX() &&
            a.getY() <= b.getY() + bh &&
            a.getY() + ah >= b.getY();
    }

    bool circleRectangleIntersect(
        const Point& center, double radius,
        const Point& topLeft, double width, double height) {

        double closestX = max(topLeft.getX(),
            min(center.getX(), topLeft.getX() + width));

        double closestY = max(topLeft.getY(),
            min(center.getY(), topLeft.getY() + height));

        Point closest(closestX, closestY);

        return center.distanceTo(closest) <= radius;
    }

    vector<Point> regularPolygonPoints(
        const Point& center, int sides, double sideLength) {

        vector<Point> points;

        double radius = sideLength / (2 * sin(PI / sides));

        for (int i = 0; i < sides; i++) {
            double angle = -PI / 2 + 2 * PI * i / sides;

            points.emplace_back(
                center.getX() + radius * cos(angle),
                center.getY() + radius * sin(angle)
            );
        }

        return points;
    }
}

// Point

Point::Point(double x, double y) {
    setX(x);
    setY(y);
}

double Point::getX() const {
    return x_;
}

double Point::getY() const {
    return y_;
}

void Point::setX(double x) {
    if (!isfinite(x)) {
        throw DomainError("Некоректна координата X!");
    }

    x_ = x;
}

void Point::setY(double y) {
    if (!isfinite(y)) {
        throw DomainError("Некоректна координата Y!");
    }

    y_ = y;
}

double Point::distanceTo(const Point& other) const {
    double dx = x_ - other.x_;
    double dy = y_ - other.y_;

    return sqrt(dx * dx + dy * dy);
}

void Point::print(ostream& os) const {
    os << "(" << x_ << "; " << y_ << ")";
}

// Shape

int Shape::counter_ = 0;

Shape::Shape(const string& color) {
    id_ = ++counter_;
    setColor(color);
}

Shape::Shape(const Shape& other) {
    id_ = ++counter_;
    color_ = other.color_;
}

int Shape::getId() const {
    return id_;
}

string Shape::getColor() const {
    return color_;
}

void Shape::setColor(const string& color) {
    if (color.empty()) {
        throw DomainError("Колір не може бути порожнім!");
    }

    color_ = color;
}

ostream& operator<<(ostream& os, const Shape& shape) {
    shape.print(os);
    return os;
}

bool operator<(const Shape& first, const Shape& second) {
    return first.area() < second.area();
}

bool operator==(const Shape& first, const Shape& second) {
    const double epsilon = 0.000001;

    return first.typeName() == second.typeName() &&
        abs(first.area() - second.area()) < epsilon &&
        abs(first.perimeter() - second.perimeter()) < epsilon;
}

// OpenShape and ClosedShape

OpenShape::OpenShape(const string& color)
    : Shape(color) {}

ClosedShape::ClosedShape(const string& color)
    : Shape(color) {}

bool ClosedShape::intersects(const Shape& other) const {
    return false;
}

// Line

Line::Line(const Point& start, const Point& end, const string& color)
    : OpenShape(color), start_(start), end_(end) {

    if (start_.distanceTo(end_) < EPSILON) {
        throw DomainError("Кінці лінії не можуть збігатися!");
    }
}

string Line::typeName() const {
    return "Лінія";
}

double Line::area() const {
    return 0;
}

double Line::perimeter() const {
    return start_.distanceTo(end_);
}

void Line::move(double dx, double dy) {
    start_.setX(start_.getX() + dx);
    start_.setY(start_.getY() + dy);
    end_.setX(end_.getX() + dx);
    end_.setY(end_.getY() + dy);
}

void Line::scale(double k) {
    if (!isfinite(k) || k <= 0) {
        throw DomainError("Коефіцієнт масштабування повинен бути більшим за нуль!");
    }

    Point center(
        (start_.getX() + end_.getX()) / 2,
        (start_.getY() + end_.getY()) / 2
    );

    start_.setX(center.getX() + (start_.getX() - center.getX()) * k);
    start_.setY(center.getY() + (start_.getY() - center.getY()) * k);
    end_.setX(center.getX() + (end_.getX() - center.getX()) * k);
    end_.setY(center.getY() + (end_.getY() - center.getY()) * k);
}

Rect Line::boundingBox() const {
    return {
        min(start_.getX(), end_.getX()),
        min(start_.getY(), end_.getY()),
        max(start_.getX(), end_.getX()),
        max(start_.getY(), end_.getY())
    };
}

void Line::print(ostream& os) const {
    os << "Лінія | ID: " << getId()
        << " | Колір: " << getColor()
        << " | Початок: ";

    start_.print(os);
    os << " | Кінець: ";
    end_.print(os);

    os << " | Довжина: " << perimeter();
}

string Line::toSVG() const {
    return "<line x1=\"" + to_string(start_.getX()) +
        "\" y1=\"" + to_string(start_.getY()) +
        "\" x2=\"" + to_string(end_.getX()) +
        "\" y2=\"" + to_string(end_.getY()) +
        "\" stroke=\"" + getColor() + "\" />";
}

string Line::toSvg() const {
    return toSVG();
}

string Line::toFileLine() const {
    ostringstream os;

    os << "Line " << quoted(getColor()) << " "
        << start_.getX() << " " << start_.getY() << " "
        << end_.getX() << " " << end_.getY();

    return os.str();
}

unique_ptr<Shape> Line::clone() const {
    return make_unique<Line>(*this);
}

bool Line::intersects(const Shape& other) const {
    if (const Line* line = dynamic_cast<const Line*>(&other)) {
        return segmentsIntersect(
            start_, end_,
            line->start_, line->end_
        );
    }

    if (const Circle* circle = dynamic_cast<const Circle*>(&other)) {
        return pointSegmentDistance(
            circle->center_, start_, end_
        ) <= circle->radius_;
    }

    if (const Rectangle* rectangle = dynamic_cast<const Rectangle*>(&other)) {
        vector<Point> rect = {
            rectangle->topLeft_,
            Point(rectangle->topLeft_.getX() + rectangle->width_,
                rectangle->topLeft_.getY()),
            Point(rectangle->topLeft_.getX() + rectangle->width_,
                rectangle->topLeft_.getY() + rectangle->height_),
            Point(rectangle->topLeft_.getX(),
                rectangle->topLeft_.getY() + rectangle->height_)
        };

        if (pointInPolygon(start_, rect) || pointInPolygon(end_, rect)) {
            return true;
        }

        for (size_t i = 0; i < rect.size(); i++) {
            if (segmentsIntersect(
                start_, end_,
                rect[i], rect[(i + 1) % rect.size()])) {
                return true;
            }
        }

        return false;
    }

    if (const Triangle* triangle = dynamic_cast<const Triangle*>(&other)) {
        vector<Point> points = {
            triangle->a_, triangle->b_, triangle->c_
        };

        if (pointInPolygon(start_, points) || pointInPolygon(end_, points)) {
            return true;
        }

        for (size_t i = 0; i < points.size(); i++) {
            if (segmentsIntersect(
                start_, end_,
                points[i], points[(i + 1) % points.size()])) {
                return true;
            }
        }

        return false;
    }

    if (const RegularPolygon* polygon =
        dynamic_cast<const RegularPolygon*>(&other)) {

        vector<Point> points = regularPolygonPoints(
            polygon->center_, polygon->sides_, polygon->sideLength_
        );

        if (pointInPolygon(start_, points) || pointInPolygon(end_, points)) {
            return true;
        }

        for (size_t i = 0; i < points.size(); i++) {
            if (segmentsIntersect(
                start_, end_,
                points[i], points[(i + 1) % points.size()])) {
                return true;
            }
        }
    }

    return false;
}

// Circle

Circle::Circle(const Point& center, double radius, const string& color)
    : ClosedShape(color), center_(center), radius_(radius) {
    setRadius(radius);
}

double Circle::getRadius() const {
    return radius_;
}

void Circle::setRadius(double radius) {
    if (!isfinite(radius) || radius <= 0) {
        throw DomainError("Радіус повинен бути додатним числом!");
    }

    radius_ = radius;
}

string Circle::typeName() const {
    return "Коло";
}

double Circle::area() const {
    return PI * radius_ * radius_;
}

double Circle::perimeter() const {
    return 2 * PI * radius_;
}

void Circle::move(double dx, double dy) {
    center_.setX(center_.getX() + dx);
    center_.setY(center_.getY() + dy);
}

void Circle::scale(double k) {
    if (!isfinite(k) || k <= 0) {
        throw DomainError("Коефіцієнт масштабування повинен бути більшим за нуль!");
    }

    setRadius(radius_ * k);
}

Rect Circle::boundingBox() const {
    return {
        center_.getX() - radius_,
        center_.getY() - radius_,
        center_.getX() + radius_,
        center_.getY() + radius_
    };
}

void Circle::print(ostream& os) const {
    os << "Коло | ID: " << getId()
        << " | Колір: " << getColor()
        << " | Центр: ";

    center_.print(os);

    os << " | Радіус: " << radius_
        << " | Площа: " << area();
}

string Circle::toSVG() const {
    return "<circle cx=\"" + to_string(center_.getX()) +
        "\" cy=\"" + to_string(center_.getY()) +
        "\" r=\"" + to_string(radius_) +
        "\" fill=\"none\" stroke=\"" + getColor() + "\" />";
}

string Circle::toSvg() const {
    return toSVG();
}

string Circle::toFileLine() const {
    ostringstream os;

    os << "Circle " << quoted(getColor()) << " "
        << center_.getX() << " " << center_.getY() << " "
        << radius_;

    return os.str();
}

unique_ptr<Shape> Circle::clone() const {
    return make_unique<Circle>(*this);
}

bool Circle::intersects(const Shape& other) const {
    if (const Circle* circle = dynamic_cast<const Circle*>(&other)) {
        return circlesIntersect(
            center_, radius_,
            circle->center_, circle->radius_
        );
    }

    if (const Rectangle* rectangle = dynamic_cast<const Rectangle*>(&other)) {
        return circleRectangleIntersect(
            center_, radius_,
            rectangle->topLeft_,
            rectangle->width_,
            rectangle->height_
        );
    }

    if (const Triangle* triangle = dynamic_cast<const Triangle*>(&other)) {
        return circleTriangleIntersect(
            center_, radius_,
            { triangle->a_, triangle->b_, triangle->c_ }
        );
    }

    if (const RegularPolygon* polygon =
        dynamic_cast<const RegularPolygon*>(&other)) {

        return circleTriangleIntersect(
            center_, radius_,
            regularPolygonPoints(
                polygon->center_, polygon->sides_, polygon->sideLength_
            )
        );
    }

    if (const Line* line = dynamic_cast<const Line*>(&other)) {
        return line->intersects(*this);
    }

    return false;
}

// Rectangle

Rectangle::Rectangle(
    const Point& topLeft, double width, double height,
    const string& color)
    : ClosedShape(color), topLeft_(topLeft),
    width_(width), height_(height) {

    setDimensions(width, height);
}

void Rectangle::setDimensions(double width, double height) {
    if (!isfinite(width) || !isfinite(height) ||
        width <= 0 || height <= 0) {
        throw DomainError("Розміри прямокутника повинні бути додатними!");
    }

    width_ = width;
    height_ = height;
}

string Rectangle::typeName() const {
    return "Прямокутник";
}

double Rectangle::area() const {
    return width_ * height_;
}

double Rectangle::perimeter() const {
    return 2 * (width_ + height_);
}

void Rectangle::move(double dx, double dy) {
    topLeft_.setX(topLeft_.getX() + dx);
    topLeft_.setY(topLeft_.getY() + dy);
}

void Rectangle::scale(double k) {
    if (!isfinite(k) || k <= 0) {
        throw DomainError("Коефіцієнт масштабування повинен бути більшим за нуль!");
    }

    double centerX = topLeft_.getX() + width_ / 2;
    double centerY = topLeft_.getY() + height_ / 2;

    width_ *= k;
    height_ *= k;

    topLeft_.setX(centerX - width_ / 2);
    topLeft_.setY(centerY - height_ / 2);
}

Rect Rectangle::boundingBox() const {
    return {
        topLeft_.getX(),
        topLeft_.getY(),
        topLeft_.getX() + width_,
        topLeft_.getY() + height_
    };
}

void Rectangle::print(ostream& os) const {
    os << typeName() << " | ID: " << getId()
        << " | Колір: " << getColor()
        << " | Верхній лівий кут: ";

    topLeft_.print(os);

    os << " | Ширина: " << width_
        << " | Висота: " << height_
        << " | Площа: " << area();
}

string Rectangle::toSVG() const {
    return "<rect x=\"" + to_string(topLeft_.getX()) +
        "\" y=\"" + to_string(topLeft_.getY()) +
        "\" width=\"" + to_string(width_) +
        "\" height=\"" + to_string(height_) +
        "\" fill=\"none\" stroke=\"" + getColor() + "\" />";
}

string Rectangle::toSvg() const {
    return toSVG();
}

string Rectangle::toFileLine() const {
    ostringstream os;

    os << "Rectangle " << quoted(getColor()) << " "
        << topLeft_.getX() << " " << topLeft_.getY() << " "
        << width_ << " " << height_;

    return os.str();
}

unique_ptr<Shape> Rectangle::clone() const {
    return make_unique<Rectangle>(*this);
}

bool Rectangle::intersects(const Shape& other) const {
    if (const Rectangle* rectangle = dynamic_cast<const Rectangle*>(&other)) {
        return rectanglesIntersect(
            topLeft_, width_, height_,
            rectangle->topLeft_,
            rectangle->width_,
            rectangle->height_
        );
    }

    if (const Circle* circle = dynamic_cast<const Circle*>(&other)) {
        return circleRectangleIntersect(
            circle->center_,
            circle->radius_,
            topLeft_, width_, height_
        );
    }

    if (const Triangle* triangle = dynamic_cast<const Triangle*>(&other)) {
        vector<Point> rect = {
            topLeft_,
            Point(topLeft_.getX() + width_, topLeft_.getY()),
            Point(topLeft_.getX() + width_, topLeft_.getY() + height_),
            Point(topLeft_.getX(), topLeft_.getY() + height_)
        };

        return polygonsIntersect(
            rect, { triangle->a_, triangle->b_, triangle->c_ }
        );
    }

    if (const RegularPolygon* polygon =
        dynamic_cast<const RegularPolygon*>(&other)) {

        vector<Point> rect = {
            topLeft_,
            Point(topLeft_.getX() + width_, topLeft_.getY()),
            Point(topLeft_.getX() + width_, topLeft_.getY() + height_),
            Point(topLeft_.getX(), topLeft_.getY() + height_)
        };

        return polygonsIntersect(
            rect,
            regularPolygonPoints(
                polygon->center_, polygon->sides_, polygon->sideLength_
            )
        );
    }

    if (const Line* line = dynamic_cast<const Line*>(&other)) {
        return line->intersects(*this);
    }

    return false;
}

// Square

Square::Square(const Point& topLeft, double side, const string& color)
    : Rectangle(topLeft, side, side, color) {}

string Square::typeName() const {
    return "Квадрат";
}

double Square::getSide() const {
    return width_;
}

void Square::setSide(double side) {
    setDimensions(side, side);
}

string Square::toSVG() const {
    return Rectangle::toSVG();
}

string Square::toSvg() const {
    return toSVG();
}

string Square::toFileLine() const {
    Rect box = boundingBox();

    ostringstream os;
    os << "Square " << quoted(getColor()) << " "
        << box.xMin << " " << box.yMin << " "
        << width_;

    return os.str();
}

unique_ptr<Shape> Square::clone() const {
    Rect box = boundingBox();

    return make_unique<Square>(
        Point(box.xMin, box.yMin), width_, getColor()
    );
}

void Square::print(ostream& os) const {
    os << "Квадрат | ID: " << getId()
        << " | Колір: " << getColor()
        << " | Сторона: " << width_
        << " | Площа: " << area()
        << " | Периметр: " << perimeter();
}

// Triangle

Triangle::Triangle(
    const Point& a, const Point& b, const Point& c,
    const string& color)
    : ClosedShape(color), a_(a), b_(b), c_(c) {

    double twiceArea = abs(crossProduct(a_, b_, c_));

    if (twiceArea < EPSILON) {
        throw DomainError("Точки не утворюють трикутник!");
    }
}

string Triangle::typeName() const {
    return "Трикутник";
}

double Triangle::area() const {
    return abs(crossProduct(a_, b_, c_)) / 2;
}

double Triangle::perimeter() const {
    return a_.distanceTo(b_) +
        b_.distanceTo(c_) +
        c_.distanceTo(a_);
}

Point Triangle::getCenter() const {
    return Point(
        (a_.getX() + b_.getX() + c_.getX()) / 3,
        (a_.getY() + b_.getY() + c_.getY()) / 3
    );
}

void Triangle::move(double dx, double dy) {
    a_.setX(a_.getX() + dx);
    a_.setY(a_.getY() + dy);

    b_.setX(b_.getX() + dx);
    b_.setY(b_.getY() + dy);

    c_.setX(c_.getX() + dx);
    c_.setY(c_.getY() + dy);
}

void Triangle::scale(double k) {
    if (!isfinite(k) || k <= 0) {
        throw DomainError("Коефіцієнт масштабування повинен бути більшим за нуль!");
    }

    Point center = getCenter();

    a_.setX(center.getX() + (a_.getX() - center.getX()) * k);
    a_.setY(center.getY() + (a_.getY() - center.getY()) * k);

    b_.setX(center.getX() + (b_.getX() - center.getX()) * k);
    b_.setY(center.getY() + (b_.getY() - center.getY()) * k);

    c_.setX(center.getX() + (c_.getX() - center.getX()) * k);
    c_.setY(center.getY() + (c_.getY() - center.getY()) * k);
}

Rect Triangle::boundingBox() const {
    return {
        min({ a_.getX(), b_.getX(), c_.getX() }),
        min({ a_.getY(), b_.getY(), c_.getY() }),
        max({ a_.getX(), b_.getX(), c_.getX() }),
        max({ a_.getY(), b_.getY(), c_.getY() })
    };
}

void Triangle::print(ostream& os) const {
    os << "Трикутник | ID: " << getId()
        << " | Колір: " << getColor()
        << " | Площа: " << area()
        << " | Периметр: " << perimeter()
        << " | Центр: ";

    getCenter().print(os);
}

string Triangle::toSVG() const {
    return "<polygon points=\"" +
        to_string(a_.getX()) + "," + to_string(a_.getY()) + " " +
        to_string(b_.getX()) + "," + to_string(b_.getY()) + " " +
        to_string(c_.getX()) + "," + to_string(c_.getY()) +
        "\" fill=\"none\" stroke=\"" + getColor() + "\" />";
}

string Triangle::toSvg() const {
    return toSVG();
}

string Triangle::toFileLine() const {
    ostringstream os;

    os << "Triangle " << quoted(getColor()) << " "
        << a_.getX() << " " << a_.getY() << " "
        << b_.getX() << " " << b_.getY() << " "
        << c_.getX() << " " << c_.getY();

    return os.str();
}

unique_ptr<Shape> Triangle::clone() const {
    return make_unique<Triangle>(*this);
}

bool Triangle::intersects(const Shape& other) const {
    vector<Point> triangle = { a_, b_, c_ };

    if (const Circle* circle = dynamic_cast<const Circle*>(&other)) {
        return circleTriangleIntersect(
            circle->center_, circle->radius_, triangle
        );
    }

    if (const Rectangle* rectangle = dynamic_cast<const Rectangle*>(&other)) {
        vector<Point> rect = {
            rectangle->topLeft_,
            Point(rectangle->topLeft_.getX() + rectangle->width_,
                rectangle->topLeft_.getY()),
            Point(rectangle->topLeft_.getX() + rectangle->width_,
                rectangle->topLeft_.getY() + rectangle->height_),
            Point(rectangle->topLeft_.getX(),
                rectangle->topLeft_.getY() + rectangle->height_)
        };

        return polygonsIntersect(triangle, rect);
    }

    if (const Triangle* otherTriangle = dynamic_cast<const Triangle*>(&other)) {
        vector<Point> otherPoints = {
            otherTriangle->a_, otherTriangle->b_, otherTriangle->c_
        };

        return polygonsIntersect(triangle, otherPoints);
    }

    if (const RegularPolygon* polygon =
        dynamic_cast<const RegularPolygon*>(&other)) {

        return polygonsIntersect(
            triangle,
            regularPolygonPoints(
                polygon->center_, polygon->sides_, polygon->sideLength_
            )
        );
    }

    return false;
}

// RegularPolygon

RegularPolygon::RegularPolygon(
    const Point& center, int sides,
    double sideLength, const string& color)
    : ClosedShape(color), center_(center),
    sides_(sides), sideLength_(sideLength) {

    if (sides_ < 3 || sides_ > 20) {
        throw DomainError("Кількість сторін повинна бути від 3 до 20!");
    }

    if (!isfinite(sideLength_) || sideLength_ <= 0) {
        throw DomainError("Довжина сторони повинна бути додатною!");
    }
}

string RegularPolygon::typeName() const {
    return "Правильний багатокутник";
}

int RegularPolygon::getSides() const {
    return sides_;
}

double RegularPolygon::getSideLength() const {
    return sideLength_;
}

double RegularPolygon::area() const {
    return sides_ * sideLength_ * sideLength_ /
        (4 * tan(PI / sides_));
}

double RegularPolygon::perimeter() const {
    return sides_ * sideLength_;
}

void RegularPolygon::move(double dx, double dy) {
    center_.setX(center_.getX() + dx);
    center_.setY(center_.getY() + dy);
}

void RegularPolygon::scale(double k) {
    if (!isfinite(k) || k <= 0) {
        throw DomainError("Коефіцієнт масштабування повинен бути більшим за нуль!");
    }

    sideLength_ *= k;
}

Rect RegularPolygon::boundingBox() const {
    double radius = sideLength_ / (2 * sin(PI / sides_));

    return {
        center_.getX() - radius,
        center_.getY() - radius,
        center_.getX() + radius,
        center_.getY() + radius
    };
}

void RegularPolygon::print(ostream& os) const {
    os << "Правильний багатокутник | ID: " << getId()
        << " | Колір: " << getColor()
        << " | Кількість сторін: " << sides_
        << " | Довжина сторони: " << sideLength_
        << " | Площа: " << area()
        << " | Периметр: " << perimeter();
}

string RegularPolygon::toSVG() const {
    vector<Point> points = regularPolygonPoints(
        center_, sides_, sideLength_
    );

    ostringstream os;

    os << "<polygon points=\"";

    for (size_t i = 0; i < points.size(); i++) {
        os << points[i].getX() << "," << points[i].getY();

        if (i + 1 < points.size()) {
            os << " ";
        }
    }

    os << "\" fill=\"none\" stroke=\"" << getColor() << "\" />";

    return os.str();
}

string RegularPolygon::toSvg() const {
    return toSVG();
}

string RegularPolygon::toFileLine() const {
    ostringstream os;

    os << "RegularPolygon " << quoted(getColor()) << " "
        << center_.getX() << " " << center_.getY() << " "
        << sides_ << " " << sideLength_;

    return os.str();
}

unique_ptr<Shape> RegularPolygon::clone() const {
    return make_unique<RegularPolygon>(*this);
}

bool RegularPolygon::intersects(const Shape& other) const {
    vector<Point> polygon = regularPolygonPoints(
        center_, sides_, sideLength_
    );

    if (const Circle* circle = dynamic_cast<const Circle*>(&other)) {
        return circleTriangleIntersect(
            circle->center_, circle->radius_, polygon
        );
    }

    if (const Line* line = dynamic_cast<const Line*>(&other)) {
        return line->intersects(*this);
    }

    if (const Rectangle* rectangle = dynamic_cast<const Rectangle*>(&other)) {
        vector<Point> rect = {
            rectangle->topLeft_,
            Point(rectangle->topLeft_.getX() + rectangle->width_,
                rectangle->topLeft_.getY()),
            Point(rectangle->topLeft_.getX() + rectangle->width_,
                rectangle->topLeft_.getY() + rectangle->height_),
            Point(rectangle->topLeft_.getX(),
                rectangle->topLeft_.getY() + rectangle->height_)
        };

        return polygonsIntersect(polygon, rect);
    }

    if (const Triangle* triangle = dynamic_cast<const Triangle*>(&other)) {
        return polygonsIntersect(
            polygon, { triangle->a_, triangle->b_, triangle->c_ }
        );
    }

    if (const RegularPolygon* otherPolygon =
        dynamic_cast<const RegularPolygon*>(&other)) {

        return polygonsIntersect(
            polygon,
            regularPolygonPoints(
                otherPolygon->center_,
                otherPolygon->sides_,
                otherPolygon->sideLength_
            )
        );
    }

    return false;
}
