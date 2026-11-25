
#include "factory.h"
#include "errors.h"

#include <sstream>
#include <iomanip>
#include <stdexcept>

using namespace std;

namespace {
    double readNumber(const string& value) {
        size_t position = 0;
        double number = stod(value, &position);

        if (position != value.size()) {
            throw DomainError("Некоректне числове значення!");
        }

        return number;
    }
}

Factory::Factory() {

    creators_["Line"] = [](const vector<string>& f) {
        if (f.size() != 5) {
            throw DomainError("Некоректні дані лінії!");
        }

        return make_unique<Line>(
            Point(readNumber(f[1]), readNumber(f[2])),
            Point(readNumber(f[3]), readNumber(f[4])),
            f[0]
        );
        };

    creators_["Circle"] = [](const vector<string>& f) {
        if (f.size() != 4) {
            throw DomainError("Некоректні дані кола!");
        }

        return make_unique<Circle>(
            Point(readNumber(f[1]), readNumber(f[2])),
            readNumber(f[3]),
            f[0]
        );
        };

    creators_["Rectangle"] = [](const vector<string>& f) {
        if (f.size() != 5) {
            throw DomainError("Некоректні дані прямокутника!");
        }

        return make_unique<Rectangle>(
            Point(readNumber(f[1]), readNumber(f[2])),
            readNumber(f[3]),
            readNumber(f[4]),
            f[0]
        );
        };

    creators_["Square"] = [](const vector<string>& f) {
        if (f.size() != 4) {
            throw DomainError("Некоректні дані квадрата!");
        }

        return make_unique<Square>(
            Point(readNumber(f[1]), readNumber(f[2])),
            readNumber(f[3]),
            f[0]
        );
        };

    creators_["Triangle"] = [](const vector<string>& f) {
        if (f.size() != 7) {
            throw DomainError("Некоректні дані трикутника!");
        }

        return make_unique<Triangle>(
            Point(readNumber(f[1]), readNumber(f[2])),
            Point(readNumber(f[3]), readNumber(f[4])),
            Point(readNumber(f[5]), readNumber(f[6])),
            f[0]
        );
        };

    creators_["RegularPolygon"] = [](const vector<string>& f) {
        if (f.size() != 5) {
            throw DomainError("Некоректні дані правильного багатокутника!");
        }

        return make_unique<RegularPolygon>(
            Point(readNumber(f[1]), readNumber(f[2])),
            stoi(f[3]),
            readNumber(f[4]),
            f[0]
        );
        };
}

unique_ptr<Shape> Factory::create(const string& line) const {
    istringstream input(line);

    string type;
    string color;

    if (!(input >> type >> quoted(color))) {
        throw DomainError("Некоректний формат запису!");
    }

    vector<string> fields;
    string value;

    fields.push_back(color);

    while (input >> value) {
        fields.push_back(value);
    }

    auto it = creators_.find(type);

    if (it == creators_.end()) {
        throw DomainError("Невідомий тип фігури: " + type);
    }

    return it->second(fields);
}
