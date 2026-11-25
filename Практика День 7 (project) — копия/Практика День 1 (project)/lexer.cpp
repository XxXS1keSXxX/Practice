#include "lexer.h"

#include <cctype>

ParseError::ParseError(
    const std::string& message,
    SourcePosition position
)
    : std::runtime_error(message), position_(position) {}

SourcePosition ParseError::position() const {
    return position_;
}

RuntimeError::RuntimeError(
    const std::string& message,
    SourcePosition position
)
    : std::runtime_error(message), position_(position) {}

SourcePosition RuntimeError::position() const {
    return position_;
}

Lexer::Lexer(const std::string& source)
    : source_(source) {}

char Lexer::current() const {
    return index_ < source_.size()
        ? source_[index_]
        : '\0';
}

char Lexer::peek(std::size_t offset) const {
    return index_ + offset < source_.size()
        ? source_[index_ + offset]
        : '\0';
}

char Lexer::advance() {
    char c = current();

    if (c == '\0')
        return c;

    ++index_;

    if (c == '\n') {
        ++position_.line;
        position_.column = 1;
    }
    else if (
        (static_cast<unsigned char>(c) & 0xC0) != 0x80
        ) {
        ++position_.column;
    }

    return c;
}

void Lexer::skipWhitespace() {
    while (
        current() != '\0' &&
        std::isspace(static_cast<unsigned char>(current()))
        ) {
        advance();
    }
}

Token Lexer::makeToken(
    TokenType type,
    const std::string& text,
    SourcePosition start
) const {
    return { type, text, start };
}

Token Lexer::readIdentifier() {
    SourcePosition start = position_;
    std::string value;

    while (current() != '\0') {
        unsigned char c =
            static_cast<unsigned char>(current());

        if (
            std::isalnum(c) ||
            c == '_' ||
            c >= 128
            ) {
            value += current();
            advance();
        }
        else {
            break;
        }
    }

    return makeToken(TokenType::Identifier, value, start);
}

Token Lexer::readNumber() {
    SourcePosition start = position_;
    std::string value;
    bool dotSeen = false;

    while (
        std::isdigit(static_cast<unsigned char>(current())) ||
        (current() == '.' && !dotSeen)
        ) {
        if (current() == '.')
            dotSeen = true;

        value += current();
        advance();
    }

    return makeToken(TokenType::Number, value, start);
}

Token Lexer::readString() {
    SourcePosition start = position_;

    advance();

    std::string value;

    while (
        current() != '\0' &&
        current() != '\''
        ) {
        if (current() == '\n') {
            throw ParseError(
                "Рядок не може містити перенесення рядка",
                position_
            );
        }

        if (current() == '\\' && peek() == '\'') {
            advance();
            value += advance();
        }
        else {
            value += advance();
        }
    }

    if (current() != '\'') {
        throw ParseError(
            "Не закрито текстовий рядок",
            start
        );
    }

    advance();

    return makeToken(TokenType::String, value, start);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (current() != '\0') {
        skipWhitespace();

        if (current() == '\0')
            break;

        SourcePosition start = position_;

        unsigned char c =
            static_cast<unsigned char>(current());

        if (
            std::isalpha(c) ||
            c == '_' ||
            c >= 128
            ) {
            tokens.push_back(readIdentifier());
        }
        else if (std::isdigit(c)) {
            tokens.push_back(readNumber());
        }
        else if (current() == '\'') {
            tokens.push_back(readString());
        }
        else {
            char first = advance();

            switch (first) {
            case '*':
                tokens.push_back(
                    makeToken(TokenType::Star, "*", start)
                );
                break;

            case ',':
                tokens.push_back(
                    makeToken(TokenType::Comma, ",", start)
                );
                break;

            case '(':
                tokens.push_back(
                    makeToken(TokenType::LeftParen, "(", start)
                );
                break;

            case ')':
                tokens.push_back(
                    makeToken(TokenType::RightParen, ")", start)
                );
                break;

            case ';':
                tokens.push_back(
                    makeToken(TokenType::Semicolon, ";", start)
                );
                break;

            case '=':
                tokens.push_back(
                    makeToken(TokenType::Equal, "=", start)
                );
                break;

            case '!':
                if (current() == '=') {
                    advance();

                    tokens.push_back(
                        makeToken(TokenType::NotEqual, "!=", start)
                    );
                }
                else {
                    throw ParseError(
                        "Очікувався оператор !=",
                        start
                    );
                }
                break;

            case '<':
                if (current() == '=') {
                    advance();

                    tokens.push_back(
                        makeToken(TokenType::LessEqual, "<=", start)
                    );
                }
                else if (current() == '>') {
                    advance();

                    tokens.push_back(
                        makeToken(TokenType::NotEqual, "<>", start)
                    );
                }
                else {
                    tokens.push_back(
                        makeToken(TokenType::Less, "<", start)
                    );
                }
                break;

            case '>':
                if (current() == '=') {
                    advance();

                    tokens.push_back(
                        makeToken(TokenType::GreaterEqual, ">=", start)
                    );
                }
                else {
                    tokens.push_back(
                        makeToken(TokenType::Greater, ">", start)
                    );
                }
                break;

            default:
                throw ParseError(
                    "Невідомий символ",
                    start
                );
            }
        }
    }

    tokens.push_back({
        TokenType::End,
        "",
        position_
        });

    return tokens;
}