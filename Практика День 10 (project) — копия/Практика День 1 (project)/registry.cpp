#include "registry.h"
#include "factory.h"
#include <fstream>
#include <iostream>
#include <string>
#include "errors.h"
#include <algorithm>
#include <iterator>

using namespace std;

namespace {
    struct ShapePtrLess {
        bool operator()(const unique_ptr<Shape>& first,
            const unique_ptr<Shape>& second) const {
            return *first < *second;
        }
    };
}

void Registry::add(unique_ptr<Shape> shape) {
    if (!shape) {
        throw DomainError("Неможливо додати порожню фігуру!");
    }

    shapes_.push_back(move(shape));
}

bool Registry::removeById(int id) {
    auto it = find_if(shapes_.begin(), shapes_.end(),
        [id](const unique_ptr<Shape>& shape) {
            return shape->getId() == id;
        });

    if (it == shapes_.end()) {
        return false;
    }

    shapes_.erase(it);
    return true;
}

bool Registry::duplicateById(int id) {
    auto it = find_if(shapes_.begin(), shapes_.end(),
        [id](const unique_ptr<Shape>& shape) {
            return shape->getId() == id;
        });

    if (it == shapes_.end()) {
        return false;
    }

    add((*it)->clone());
    return true;
}

void Registry::showAll() const {
    if (shapes_.empty()) {
        cout << "Список фігур порожній.\n";
        return;
    }

    cout << "\n=== СПИСОК ФІГУР ===\n";

    for (const auto& shape : shapes_) {
        cout << *shape << '\n';
    }
}

int Registry::size() const {
    return static_cast<int>(shapes_.size());
}

bool Registry::empty() const {
    return shapes_.empty();
}

const vector<unique_ptr<Shape>>& Registry::getShapes() const {
    return shapes_;
}

void Registry::moveAll(double dx, double dy) {
    for (const auto& shape : shapes_) {
        shape->move(dx, dy);
    }
}

void Registry::sortById() {
    sort(shapes_.begin(), shapes_.end(),
        [](const unique_ptr<Shape>& a, const unique_ptr<Shape>& b) {
            return a->getId() < b->getId();
        });
}

void Registry::scaleAll(double k) {
    if (!isfinite(k) || k <= 0) {
        throw DomainError("Коефіцієнт масштабування повинен бути більшим за 0!");
    }

    for (const auto& shape : shapes_) {
        shape->scale(k);
    }
}

double Registry::totalArea() const {
    double total = 0;

    for (const auto& shape : shapes_) {
        total += shape->area();
    }

    return total;
}

vector<pair<const Shape*, const Shape*>> Registry::findIntersections() const {
    vector<pair<const Shape*, const Shape*>> result;
    const double EPSILON = 1e-10;

    for (size_t i = 0; i < shapes_.size(); i++) {
        for (size_t j = i + 1; j < shapes_.size(); j++) {
            Rect a = shapes_[i]->boundingBox();
            Rect b = shapes_[j]->boundingBox();

            bool intersects =
                a.xMin <= b.xMax + EPSILON &&
                a.xMax + EPSILON >= b.xMin &&
                a.yMin <= b.yMax + EPSILON &&
                a.yMax + EPSILON >= b.yMin;

            if (intersects) {
                result.push_back({ shapes_[i].get(), shapes_[j].get() });
            }
        }
    }

    return result;
}

void Registry::sortByArea() {
    sort(shapes_.begin(), shapes_.end(),
        [](const unique_ptr<Shape>& a, const unique_ptr<Shape>& b) {
            return a->area() < b->area();
        });
}

void Registry::sortByAreaUsingOperator() {
    sort(shapes_.begin(), shapes_.end(), ShapePtrLess{});
}

void Registry::sortByPerimeter() {
    sort(shapes_.begin(), shapes_.end(),
        [](const unique_ptr<Shape>& a, const unique_ptr<Shape>& b) {
            return a->perimeter() < b->perimeter();
        });
}

const Shape* Registry::findLargestByArea() const {
    if (shapes_.empty()) {
        return nullptr;
    }

    auto result = maxBy(
        shapes_.begin(),
        shapes_.end(),
        [](const unique_ptr<Shape>& shape) {
            return shape->area();
        }
    );

    return result->get();
}

const Shape* Registry::findSmallestByArea() const {
    if (shapes_.empty()) {
        return nullptr;
    }

    auto result = min_element(
        shapes_.begin(),
        shapes_.end(),
        [](const unique_ptr<Shape>& a, const unique_ptr<Shape>& b) {
            return a->area() < b->area();
        }
    );

    return result->get();
}

const Shape* Registry::findLargestByPerimeter() const {
    if (shapes_.empty()) {
        return nullptr;
    }

    auto result = maxBy(
        shapes_.begin(),
        shapes_.end(),
        [](const unique_ptr<Shape>& shape) {
            return shape->perimeter();
        }
    );

    return result->get();
}

const Shape* Registry::findById(int id) const {
    auto it = find_if(shapes_.begin(), shapes_.end(),
        [id](const unique_ptr<Shape>& shape) {
            return shape->getId() == id;
        });

    if (it == shapes_.end()) {
        return nullptr;
    }

    return it->get();
}

vector<const Shape*> Registry::filterByType(const string& type) const {
    vector<const Shape*> result;

    for (const auto& shape : shapes_) {
        if (shape->typeName() == type) {
            result.push_back(shape.get());
        }
    }

    return result;
}

void Registry::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);

    if (!file) {
        throw std::runtime_error("Не вдалося відкрити файл для збереження!");
    }

    for (const auto& shape : shapes_) {
        file << shape->toFileLine() << '\n';
    }

    std::cout << "Фігури успішно збережено у файл!\n";
}

void Registry::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);

    if (!file) {
        throw std::runtime_error("Не вдалося відкрити файл для завантаження!");
    }

    Factory factory;
    std::string line;
    int loaded = 0;
    int skipped = 0;

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        try {
            auto shape = factory.create(line);
            add(std::move(shape));
            loaded++;
        }
        catch (const std::exception&) {
            skipped++;
        }
    }

    std::cout << "Завантажено фігур: " << loaded << '\n';
    std::cout << "Пропущено пошкоджених записів: " << skipped << '\n';
}

void Registry::exportToSVG(const std::string& filename) const {
    std::ofstream file(filename);

    if (!file) {
        throw std::runtime_error("Не вдалося створити SVG-файл!");
    }

    double minX = -10;
    double minY = -10;
    double maxX = 10;
    double maxY = 10;

    if (!shapes_.empty()) {
        Rect first = shapes_[0]->boundingBox();

        minX = first.xMin;
        minY = first.yMin;
        maxX = first.xMax;
        maxY = first.yMax;

        for (const auto& shape : shapes_) {
            Rect box = shape->boundingBox();

            minX = min(minX, box.xMin);
            minY = min(minY, box.yMin);
            maxX = max(maxX, box.xMax);
            maxY = max(maxY, box.yMax);
        }
    }

    double padding = 10;
    double width = maxX - minX + padding * 2;
    double height = maxY - minY + padding * 2;

    file << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        << "width=\"1000\" height=\"1000\" "
        << "viewBox=\"" << minX - padding << " "
        << minY - padding << " "
        << width << " " << height << "\">\n";

    for (const auto& shape : shapes_) {
        file << "    " << shape->toSVG() << '\n';
    }

    file << "</svg>\n";

    std::cout << "SVG-файл успішно створено!\n";
}