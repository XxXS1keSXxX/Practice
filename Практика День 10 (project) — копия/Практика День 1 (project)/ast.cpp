#include "ast.h"

#include <vector>
#include <utility>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <ostream>

namespace {

    std::string lowerAscii(std::string value) {
        for (char& c : value) {
            c = static_cast<char>(
                std::tolower(static_cast<unsigned char>(c))
                );
        }

        return value;
    }

    std::string facultyText(const Student& student) {
        if (student.faculty == Faculty::IT)
            return "ІТ";

        return toString(student.faculty);
    }

    SqlValue fieldValue(
        const Student& student,
        const std::string& field,
        SourcePosition position
    ) {
        if (field == "id")
            return static_cast<double>(student.id);

        if (field == "avggrade")
            return student.avgGrade;

        if (field == "birthyear")
            return static_cast<double>(student.birthDate.year);

        if (field == "name")
            return student.name;

        if (field == "faculty")
            return facultyText(student);

        if (field == "birthdate") {
            return std::to_string(student.birthDate.day) + "." +
                std::to_string(student.birthDate.month) + "." +
                std::to_string(student.birthDate.year);
        }

        throw RuntimeError(
            "Невідоме поле: " + field,
            position
        );
    }

    std::vector<std::string> splitUtf8(const std::string& value) {
        std::vector<std::string> result;

        for (std::size_t i = 0; i < value.size();) {
            unsigned char c =
                static_cast<unsigned char>(value[i]);

            std::size_t length = 1;

            if ((c & 0xE0) == 0xC0)
                length = 2;
            else if ((c & 0xF0) == 0xE0)
                length = 3;
            else if ((c & 0xF8) == 0xF0)
                length = 4;

            length = std::min(length, value.size() - i);

            result.push_back(value.substr(i, length));
            i += length;
        }

        return result;
    }

    bool likeMatch(
        const std::string& text,
        const std::string& pattern
    ) {
        std::vector<std::string> textChars = splitUtf8(text);
        std::vector<std::string> patternChars = splitUtf8(pattern);

        std::size_t i = 0;
        std::size_t j = 0;
        std::size_t star = std::string::npos;
        std::size_t match = 0;

        while (i < textChars.size()) {
            if (
                j < patternChars.size() &&
                (
                    patternChars[j] == "_" ||
                    patternChars[j] == textChars[i]
                    )
                ) {
                ++i;
                ++j;
            }
            else if (
                j < patternChars.size() &&
                patternChars[j] == "%"
                ) {
                star = j++;
                match = i;
            }
            else if (star != std::string::npos) {
                j = star + 1;
                i = ++match;
            }
            else {
                return false;
            }
        }

        while (
            j < patternChars.size() &&
            patternChars[j] == "%"
            ) {
            ++j;
        }

        return j == patternChars.size();
    }

    void addIndent(std::ostream& out, int count) {
        for (int i = 0; i < count; ++i)
            out << "  ";
    }

}

std::string normalizeSqlText(const std::string& value) {
    return lowerAscii(value);
}

bool Expr::isConstant() const {
    return false;
}

bool Expr::constantValue() const {
    return false;
}

ConstantExpr::ConstantExpr(bool value)
    : value_(value) {}

bool ConstantExpr::evaluate(const Student&) const {
    return value_;
}

void ConstantExpr::print(
    std::ostream& out,
    int level
) const {
    addIndent(out, level);

    out << (value_ ? "TRUE" : "FALSE") << '\n';
}

void ConstantExpr::accept(AstVisitor& visitor) const {
    visitor.visit(*this);
}

std::unique_ptr<Expr> ConstantExpr::optimize() {
    return std::make_unique<ConstantExpr>(value_);
}

bool ConstantExpr::isConstant() const {
    return true;
}

bool ConstantExpr::constantValue() const {
    return value_;
}

ComparisonExpr::ComparisonExpr(
    std::string field,
    std::string operation,
    SqlValue value,
    SourcePosition position
)
    : field_(std::move(field)),
    operation_(std::move(operation)),
    value_(std::move(value)),
    position_(position) {}

bool ComparisonExpr::evaluate(const Student& student) const {
    SqlValue actual = fieldValue(
        student,
        field_,
        position_
    );

    if (actual.index() != value_.index()) {
        throw RuntimeError(
            "Несумісні типи поля та значення",
            position_
        );
    }

    if (std::holds_alternative<double>(actual)) {
        double a = std::get<double>(actual);
        double b = std::get<double>(value_);

        if (operation_ == "=") return a == b;
        if (operation_ == "!=" || operation_ == "<>") return a != b;
        if (operation_ == "<") return a < b;
        if (operation_ == ">") return a > b;
        if (operation_ == "<=") return a <= b;
        if (operation_ == ">=") return a >= b;
    }
    else {
        const std::string& a = std::get<std::string>(actual);
        const std::string& b = std::get<std::string>(value_);

        if (operation_ == "=") return a == b;
        if (operation_ == "!=" || operation_ == "<>") return a != b;
        if (operation_ == "<") return a < b;
        if (operation_ == ">") return a > b;
        if (operation_ == "<=") return a <= b;
        if (operation_ == ">=") return a >= b;
    }

    throw RuntimeError(
        "Непідтримуваний оператор порівняння",
        position_
    );
}

void ComparisonExpr::print(
    std::ostream& out,
    int level
) const {
    addIndent(out, level);

    out << field_ << " " << operation_ << " ";

    if (std::holds_alternative<double>(value_))
        out << std::get<double>(value_);
    else
        out << "'" << std::get<std::string>(value_) << "'";

    out << '\n';
}

void ComparisonExpr::accept(AstVisitor& visitor) const {
    visitor.visit(*this);
}

std::unique_ptr<Expr> ComparisonExpr::optimize() {
    return std::make_unique<ComparisonExpr>(
        field_,
        operation_,
        value_,
        position_
    );
}

LikeExpr::LikeExpr(
    std::string field,
    std::string pattern,
    SourcePosition position
)
    : field_(std::move(field)),
    pattern_(std::move(pattern)),
    position_(position) {}

bool LikeExpr::evaluate(const Student& student) const {
    SqlValue actual = fieldValue(
        student,
        field_,
        position_
    );

    if (!std::holds_alternative<std::string>(actual)) {
        throw RuntimeError(
            "LIKE можна застосовувати лише до текстових полів",
            position_
        );
    }

    return likeMatch(
        std::get<std::string>(actual),
        pattern_
    );
}

void LikeExpr::print(
    std::ostream& out,
    int level
) const {
    addIndent(out, level);

    out << field_ << " LIKE '" << pattern_ << "'\n";
}

void LikeExpr::accept(AstVisitor& visitor) const {
    visitor.visit(*this);
}

std::unique_ptr<Expr> LikeExpr::optimize() {
    return std::make_unique<LikeExpr>(
        field_,
        pattern_,
        position_
    );
}

LogicalExpr::LogicalExpr(
    std::string operation,
    std::unique_ptr<Expr> left,
    std::unique_ptr<Expr> right
)
    : operation_(std::move(operation)),
    left_(std::move(left)),
    right_(std::move(right)) {}

bool LogicalExpr::evaluate(const Student& student) const {
    if (operation_ == "AND") {
        return left_->evaluate(student) &&
            right_->evaluate(student);
    }

    return left_->evaluate(student) ||
        right_->evaluate(student);
}

void LogicalExpr::print(
    std::ostream& out,
    int level
) const {
    addIndent(out, level);

    out << operation_ << '\n';

    left_->print(out, level + 1);
    right_->print(out, level + 1);
}

void LogicalExpr::accept(AstVisitor& visitor) const {
    visitor.visit(*this);

    left_->accept(visitor);
    right_->accept(visitor);
}

std::unique_ptr<Expr> LogicalExpr::optimize() {
    left_ = left_->optimize();
    right_ = right_->optimize();

    if (left_->isConstant()) {
        bool value = left_->constantValue();

        if (operation_ == "AND") {
            return value
                ? std::move(right_)
                : std::make_unique<ConstantExpr>(false);
        }

        return value
            ? std::make_unique<ConstantExpr>(true)
            : std::move(right_);
    }

    if (right_->isConstant()) {
        bool value = right_->constantValue();

        if (operation_ == "AND") {
            return value
                ? std::move(left_)
                : std::make_unique<ConstantExpr>(false);
        }

        return value
            ? std::make_unique<ConstantExpr>(true)
            : std::move(left_);
    }

    return std::make_unique<LogicalExpr>(
        operation_,
        std::move(left_),
        std::move(right_)
    );
}

NotExpr::NotExpr(std::unique_ptr<Expr> child)
    : child_(std::move(child)) {}

bool NotExpr::evaluate(const Student& student) const {
    return !child_->evaluate(student);
}

void NotExpr::print(
    std::ostream& out,
    int level
) const {
    addIndent(out, level);

    out << "NOT\n";

    child_->print(out, level + 1);
}

void NotExpr::accept(AstVisitor& visitor) const {
    visitor.visit(*this);

    child_->accept(visitor);
}

std::unique_ptr<Expr> NotExpr::optimize() {
    child_ = child_->optimize();

    if (child_->isConstant()) {
        return std::make_unique<ConstantExpr>(
            !child_->constantValue()
        );
    }

    return std::make_unique<NotExpr>(
        std::move(child_)
    );
}

void NodeCounter::visit(const ConstantExpr&) {
    ++count_;
}

void NodeCounter::visit(const ComparisonExpr&) {
    ++count_;
}

void NodeCounter::visit(const LikeExpr&) {
    ++count_;
}

void NodeCounter::visit(const LogicalExpr&) {
    ++count_;
}

void NodeCounter::visit(const NotExpr&) {
    ++count_;
}

int NodeCounter::count() const {
    return count_;
}