#include "render.h"
#include <iostream>
#include <iomanip>

void renderBoard(const GameModel& game, bool ansi) {
    const char* reset = "\x1b[0m";
    const char* color = "\x1b[36m";

    int n = game.size();

    std::cout << "\n";

    for (int r = 0; r < n; r++) {
        for (int c = 0; c < n; c++) {
            int value = game.get(r, c);

            if (ansi && value != 0)
                std::cout << color;

            if (value == 0)
                std::cout << std::setw(7) << ".";
            else
                std::cout << std::setw(7) << value;

            if (ansi)
                std::cout << reset;
        }

        std::cout << "\n";
    }

    std::cout << "\nРахунок: " << game.score() << "\n";
}

void renderGameStatus(const GameModel& game) {
    if (game.won())
        std::cout << "Вітаємо! Ви досягли 2048!\n";
    else if (game.lost())
        std::cout << "Гру завершено. Більше немає можливих ходів.\n";
}