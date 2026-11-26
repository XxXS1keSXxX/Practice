#pragma once

#include <iostream>
#include <string>
#include <vector>

enum class Faculty {
    IT,
    Economics,
    Law,
    Other
};

struct Date {
    int day;
    int month;
    int year;
};

struct Student {
    int id;
    std::string name;
    Date birthDate;
    Faculty faculty;
    double avgGrade;
};

using StudentList = std::vector<Student>;

std::string toString(Faculty faculty);
void typesTask();
StudentList& getStudents();
