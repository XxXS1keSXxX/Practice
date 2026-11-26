#pragma once

#include "parser.h"

#include <string>
#include <vector>

struct QueryResult {
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> rows;
};

QueryResult executeQuery(const SQLQuery& query);

void printQueryResult(const QueryResult& result);

void parserMenu();