#pragma once

#include "domain.h"
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>

class Factory {
public:
    using Creator = std::function<std::unique_ptr<Shape>(const std::vector<std::string>&)>;

    Factory();

    std::unique_ptr<Shape> create(const std::string& line) const;

private:
    std::map<std::string, Creator> creators_;
};