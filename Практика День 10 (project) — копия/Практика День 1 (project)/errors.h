#pragma once

#include <stdexcept>

class DomainError : public std::invalid_argument {
public:
    using std::invalid_argument::invalid_argument;
};