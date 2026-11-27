#ifndef MENU_UTILS_H
#define MENU_UTILS_H

#include <string>

int readInt(const std::string& prompt, int min, int max);
double readDouble(const std::string& prompt, double min, double max);

#endif