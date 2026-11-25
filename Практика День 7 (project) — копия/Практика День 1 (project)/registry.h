#pragma once

#include <utility>
#include "domain.h"
#include <vector>
#include <memory>
#include <string>

template <typename Iterator, typename Function>
Iterator maxBy(Iterator first, Iterator last, Function getValue) {
    if (first == last) {
        return last;
    }

    Iterator best = first;

    for (Iterator current = first; current != last; ++current) {
        if (getValue(*best) < getValue(*current)) {
            best = current;
        }
    }

    return best;
}

class Registry {
private:
    std::vector<std::unique_ptr<Shape>> shapes_;

public:
    void add(std::unique_ptr<Shape> shape);

    bool removeById(int id);

    bool duplicateById(int id);

    void showAll() const;

    int size() const;

    bool empty() const;

    const std::vector<std::unique_ptr<Shape>>& getShapes() const;

    void moveAll(double dx, double dy);
    void scaleAll(double k);
    double totalArea() const;
    std::vector<std::pair<const Shape*, const Shape*>> findIntersections() const;
    const Shape* findSmallestByArea() const;
    
    void sortById();
    void sortByArea();
    void sortByPerimeter();
    void sortByAreaUsingOperator();

    const Shape* findLargestByArea() const;
    const Shape* findLargestByPerimeter() const;

    const Shape* findById(int id) const;

    std::vector<const Shape*> filterByType(const std::string& type) const;

    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);
    void exportToSVG(const std::string& filename) const;
};
