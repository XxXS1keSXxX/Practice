#pragma once

#include <string>
#include <vector>
#include <stdexcept>

struct SourcePosition {
    int line = 1;
    int column = 1;
};

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& message, SourcePosition position);
    SourcePosition position() const;

private:
    SourcePosition position_;
};

class RuntimeError : public std::runtime_error {
public:
    RuntimeError(const std::string& message, SourcePosition position);
    SourcePosition position() const;

private:
    SourcePosition position_;
};

enum class TokenType {
    Identifier,
    Number,
    String,
    Star,
    Comma,
    LeftParen,
    RightParen,
    Semicolon,
    Equal,
    NotEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    End
};

struct Token {
    TokenType type;
    std::string text;
    SourcePosition position;
};

class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string source_;
    std::size_t index_ = 0;
    SourcePosition position_;

    char current() const;
    char peek(std::size_t offset = 1) const;
    char advance();

    void skipWhitespace();

    Token readIdentifier();
    Token readNumber();
    Token readString();

    Token makeToken(
        TokenType type,
        const std::string& text,
        SourcePosition start
    ) const;
};