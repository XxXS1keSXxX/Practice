#include "parser.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

Parser::Parser(const std::string& source)
    : tokens_(Lexer(source).tokenize()) {}

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

bool Parser::check(TokenType type) const {
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (!check(type))
        return false;

    ++current_;
    return true;
}

bool Parser::matchKeyword(const std::string& keyword) {
    if (peek().type != TokenType::Identifier)
        return false;

    if (normalizeSqlText(peek().text) != keyword)
        return false;

    ++current_;
    return true;
}

const Token& Parser::consume(
    TokenType type,
    const std::string& message
) {
    if (check(type)) {
        ++current_;
        return previous();
    }

    error(message, peek().position);
}

const Token& Parser::consumeKeyword(
    const std::string& keyword,
    const std::string& message
) {
    if (matchKeyword(keyword))
        return previous();

    error(message, peek().position);
}

void Parser::validateField(
    const std::string& field,
    SourcePosition position
) const {
    if (
        field != "id" &&
        field != "name" &&
        field != "faculty" &&
        field != "avggrade" &&
        field != "birthdate" &&
        field != "birthyear"
        ) {
        error("Невідоме поле: " + field, position);
    }
}

bool Parser::isNumericField(const std::string& field) const {
    return field == "id" ||
        field == "avggrade" ||
        field == "birthyear";
}

bool Parser::isStringField(const std::string& field) const {
    return field == "name" ||
        field == "faculty" ||
        field == "birthdate";
}

std::string Parser::parseField() {
    const Token& token = consume(
        TokenType::Identifier,
        "Очікувалося назву поля"
    );

    std::string field = normalizeSqlText(token.text);

    validateField(field, token.position);

    return field;
}

std::unique_ptr<Expr> Parser::parseExpression() {
    return parseOr();
}

std::unique_ptr<Expr> Parser::parseOr() {
    auto expression = parseAnd();

    while (matchKeyword("or")) {
        auto right = parseAnd();

        expression = std::make_unique<LogicalExpr>(
            "OR",
            std::move(expression),
            std::move(right)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parseAnd() {
    auto expression = parseNot();

    while (matchKeyword("and")) {
        auto right = parseNot();

        expression = std::make_unique<LogicalExpr>(
            "AND",
            std::move(expression),
            std::move(right)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parseNot() {
    if (matchKeyword("not")) {
        if (++depth_ > 100) {
            error(
                "Перевищено максимальну глибину виразу",
                previous().position
            );
        }

        auto child = parseNot();
        --depth_;

        return std::make_unique<NotExpr>(
            std::move(child)
        );
    }

    return parsePrimary();
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (match(TokenType::LeftParen)) {
        if (++depth_ > 100) {
            error(
                "Перевищено максимальну глибину виразу",
                previous().position
            );
        }

        auto expression = parseExpression();

        consume(
            TokenType::RightParen,
            "Очікувалася закривальна дужка"
        );

        --depth_;

        return expression;
    }

    const Token& fieldToken = consume(
        TokenType::Identifier,
        "Очікувалося поле або дужка"
    );

    std::string field = normalizeSqlText(fieldToken.text);

    if (field == "true")
        return std::make_unique<ConstantExpr>(true);

    if (field == "false")
        return std::make_unique<ConstantExpr>(false);

    validateField(field, fieldToken.position);

    if (matchKeyword("like")) {
        if (!isStringField(field)) {
            error(
                "LIKE можна застосовувати лише до текстових полів",
                fieldToken.position
            );
        }

        const Token& pattern = consume(
            TokenType::String,
            "Після LIKE очікувався текстовий шаблон"
        );

        return std::make_unique<LikeExpr>(
            field,
            pattern.text,
            fieldToken.position
        );
    }

    std::string operation;

    if (match(TokenType::Equal))
        operation = "=";
    else if (match(TokenType::NotEqual))
        operation = "!=";
    else if (match(TokenType::Less))
        operation = "<";
    else if (match(TokenType::Greater))
        operation = ">";
    else if (match(TokenType::LessEqual))
        operation = "<=";
    else if (match(TokenType::GreaterEqual))
        operation = ">=";
    else {
        error(
            "Очікувався оператор порівняння",
            peek().position
        );
    }

    SqlValue value;

    if (match(TokenType::Number)) {
        if (!isNumericField(field)) {
            error(
                "Несумісні типи: текстове поле порівнюється з числом",
                previous().position
            );
        }

        try {
            value = std::stod(previous().text);
        }
        catch (...) {
            error(
                "Некоректне числове значення",
                previous().position
            );
        }
    }
    else if (match(TokenType::String)) {
        if (!isStringField(field)) {
            error(
                "Несумісні типи: числове поле порівнюється з текстом",
                previous().position
            );
        }

        value = previous().text;
    }
    else {
        error(
            "Очікувалося числове або текстове значення",
            peek().position
        );
    }

    return std::make_unique<ComparisonExpr>(
        field,
        operation,
        std::move(value),
        fieldToken.position
    );
}

SQLQuery Parser::parse() {
    SQLQuery query;

    consumeKeyword(
        "select",
        "Запит повинен починатися з SELECT"
    );

    if (matchKeyword("count")) {
        query.count = true;

        consume(
            TokenType::LeftParen,
            "Після COUNT очікувалася дужка"
        );

        consume(
            TokenType::Star,
            "У COUNT потрібно використовувати *"
        );

        consume(
            TokenType::RightParen,
            "Очікувалася закривальна дужка COUNT"
        );

        query.fields.push_back("COUNT(*)");
    }
    else if (match(TokenType::Star)) {
        query.fields = {
            "id",
            "name",
            "birthdate",
            "faculty",
            "avggrade"
        };
    }
    else {
        query.fields.push_back(parseField());

        while (match(TokenType::Comma)) {
            query.fields.push_back(parseField());
        }
    }

    consumeKeyword(
        "from",
        "Очікувалося FROM"
    );

    const Token& table = consume(
        TokenType::Identifier,
        "Очікувалася назва таблиці"
    );

    if (normalizeSqlText(table.text) != "students") {
        error(
            "Невідома таблиця: " + table.text,
            table.position
        );
    }

    if (matchKeyword("where")) {
        query.where = parseExpression();
    }

    if (matchKeyword("order")) {
        consumeKeyword(
            "by",
            "Після ORDER очікувалося BY"
        );

        query.orderBy = parseField();

        if (matchKeyword("desc")) {
            query.descending = true;
        }
        else {
            matchKeyword("asc");
        }
    }

    if (matchKeyword("limit")) {
        const Token& number = consume(
            TokenType::Number,
            "Після LIMIT очікувалося число"
        );

        if (number.text.find('.') != std::string::npos) {
            error(
                "LIMIT повинен бути цілим числом",
                number.position
            );
        }

        try {
            query.limit = static_cast<std::size_t>(
                std::stoull(number.text)
                );
        }
        catch (...) {
            error(
                "Некоректне значення LIMIT",
                number.position
            );
        }

        query.hasLimit = true;
    }

    match(TokenType::Semicolon);

    consume(
        TokenType::End,
        "Зайві символи після SQL-запиту"
    );

    if (query.where)
        query.where = query.where->optimize();

    return query;
}

[[noreturn]] void Parser::error(
    const std::string& message,
    SourcePosition position
) const {
    throw ParseError(message, position);
}