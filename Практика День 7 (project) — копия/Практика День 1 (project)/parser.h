#pragma once

#include "ast.h"
#include "lexer.h"

#include <memory>
#include <string>
#include <vector>

struct SQLQuery {
    std::vector<std::string> fields;
    bool count = false;
    std::unique_ptr<Expr> where;
    std::string orderBy;
    bool descending = false;
    bool hasLimit = false;
    std::size_t limit = 0;
};

class Parser {
public:
    explicit Parser(const std::string& source);

    SQLQuery parse();

private:
    std::vector<Token> tokens_;
    std::size_t current_ = 0;
    int depth_ = 0;

    const Token& peek() const;
    const Token& previous() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool matchKeyword(const std::string& keyword);
    const Token& consume(TokenType type, const std::string& message);
    const Token& consumeKeyword(
        const std::string& keyword,
        const std::string& message
    );

    std::string parseField();
    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseOr();
    std::unique_ptr<Expr> parseAnd();
    std::unique_ptr<Expr> parseNot();
    std::unique_ptr<Expr> parsePrimary();

    void validateField(
        const std::string& field,
        SourcePosition position
    ) const;

    bool isNumericField(const std::string& field) const;
    bool isStringField(const std::string& field) const;

    [[noreturn]] void error(
        const std::string& message,
        SourcePosition position
    ) const;
};