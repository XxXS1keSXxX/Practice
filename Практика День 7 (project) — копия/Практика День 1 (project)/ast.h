#pragma once

#include "lexer.h"
#include "types_task.h"

#include <memory>
#include <string>
#include <variant>
#include <ostream>

using SqlValue = std::variant<double, std::string>;

class ConstantExpr;
class ComparisonExpr;
class LikeExpr;
class LogicalExpr;
class NotExpr;

class AstVisitor {
public:
    virtual ~AstVisitor() = default;

    virtual void visit(const ConstantExpr& node) = 0;
    virtual void visit(const ComparisonExpr& node) = 0;
    virtual void visit(const LikeExpr& node) = 0;
    virtual void visit(const LogicalExpr& node) = 0;
    virtual void visit(const NotExpr& node) = 0;
};

class Expr {
public:
    virtual ~Expr() = default;

    virtual bool evaluate(const Student& student) const = 0;

    virtual void print(
        std::ostream& out,
        int indent = 0
    ) const = 0;

    virtual void accept(AstVisitor& visitor) const = 0;

    virtual std::unique_ptr<Expr> optimize() = 0;

    virtual bool isConstant() const;
    virtual bool constantValue() const;
};

class ConstantExpr : public Expr {
public:
    explicit ConstantExpr(bool value);

    bool evaluate(const Student& student) const override;

    void print(
        std::ostream& out,
        int indent = 0
    ) const override;

    void accept(AstVisitor& visitor) const override;

    std::unique_ptr<Expr> optimize() override;

    bool isConstant() const override;
    bool constantValue() const override;

private:
    bool value_;
};

class ComparisonExpr : public Expr {
public:
    ComparisonExpr(
        std::string field,
        std::string operation,
        SqlValue value,
        SourcePosition position
    );

    bool evaluate(const Student& student) const override;

    void print(
        std::ostream& out,
        int indent = 0
    ) const override;

    void accept(AstVisitor& visitor) const override;

    std::unique_ptr<Expr> optimize() override;

private:
    std::string field_;
    std::string operation_;
    SqlValue value_;
    SourcePosition position_;
};

class LikeExpr : public Expr {
public:
    LikeExpr(
        std::string field,
        std::string pattern,
        SourcePosition position
    );

    bool evaluate(const Student& student) const override;

    void print(
        std::ostream& out,
        int indent = 0
    ) const override;

    void accept(AstVisitor& visitor) const override;

    std::unique_ptr<Expr> optimize() override;

private:
    std::string field_;
    std::string pattern_;
    SourcePosition position_;
};

class LogicalExpr : public Expr {
public:
    LogicalExpr(
        std::string operation,
        std::unique_ptr<Expr> left,
        std::unique_ptr<Expr> right
    );

    bool evaluate(const Student& student) const override;

    void print(
        std::ostream& out,
        int indent = 0
    ) const override;

    void accept(AstVisitor& visitor) const override;

    std::unique_ptr<Expr> optimize() override;

private:
    std::string operation_;
    std::unique_ptr<Expr> left_;
    std::unique_ptr<Expr> right_;
};

class NotExpr : public Expr {
public:
    explicit NotExpr(std::unique_ptr<Expr> child);

    bool evaluate(const Student& student) const override;

    void print(
        std::ostream& out,
        int indent = 0
    ) const override;

    void accept(AstVisitor& visitor) const override;

    std::unique_ptr<Expr> optimize() override;

private:
    std::unique_ptr<Expr> child_;
};

class NodeCounter : public AstVisitor {
public:
    void visit(const ConstantExpr&) override;
    void visit(const ComparisonExpr&) override;
    void visit(const LikeExpr&) override;
    void visit(const LogicalExpr&) override;
    void visit(const NotExpr&) override;

    int count() const;

private:
    int count_ = 0;
};

std::string normalizeSqlText(const std::string& value);