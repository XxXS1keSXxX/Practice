#include "game_task.h"
#include "model.h"
#include "commands.h"
#include "render.h"
#include "save.h"
#include "tests9.h"
#include "menu_utils.h"

#include <iostream>
#include <string>
#include <limits>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <random>
#include <windows.h>

using namespace std;

static string readText(const string& prompt) {
    cout << prompt;

    string text;
    getline(cin, text);

    if (text.empty() && cin.good())
        getline(cin, text);

    return text;
}

static Direction parseDirection(char ch) {
    ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));

    if (ch == 'a') return Direction::Left;
    if (ch == 'd') return Direction::Right;
    if (ch == 'w') return Direction::Up;

    return Direction::Down;
}

static char directionLetter(Direction direction) {
    switch (direction) {
    case Direction::Left: return 'A';
    case Direction::Right: return 'D';
    case Direction::Up: return 'W';
    case Direction::Down: return 'S';
    }

    return 'A';
}

static unsigned int newSeed() {
    auto now = chrono::high_resolution_clock::now()
        .time_since_epoch().count();

    return static_cast<unsigned int>(now);
}

static void finishGame(const GameModel& game) {
    cout << "\nПартію завершено. Рахунок: " << game.score() << "\n";

    string name = readText("Введіть ім'я для таблиці рекордів: ");

    if (name.empty())
        name = "Гравець";

    addScore(game.size(), game.score(), name);
}

static void playGame(GameModel game, bool ansi, bool newGame) {
    History history;

    if (newGame)
        startLog(game.size(), game.seed());

    bool finished = false;

    while (!finished) {
        renderBoard(game, ansi);
        renderGameStatus(game);

        if (game.won()) {
            cout << "Досягнуто 2048! Продовжити гру? (1 - так, 0 - ні): ";

            int answer = readInt("Ваш вибір: ", 0, 1);

            if (answer == 1) {
                game.setContinueAfterWin(true);
                appendContinueLog();
            }
            else {
                finishGame(game);
                break;
            }
        }

        if (game.lost()) {
            finishGame(game);
            break;
        }

        cout << "\nW/A/S/D - рух, U - undo, R - redo\n";
        cout << "H - підказка, SAVE - зберегти, Q - меню\n";

        string command = readText("Команда: ");

        if (command.empty())
            continue;

        for (char& ch : command)
            ch = static_cast<char>(toupper(static_cast<unsigned char>(ch)));

        if (command == "Q") {
            finished = true;
        }
        else if (command == "SAVE") {
            if (saveGame(game, "data/game_1.sav"))
                cout << "Партію збережено.\n";
            else
                cout << "Помилка збереження.\n";
        }
        else if (command == "U") {
            if (history.undo())
                appendUndoLog();
            else
                cout << "Немає ходів для скасування.\n";
        }
        else if (command == "R") {
            if (history.redo())
                appendRedoLog();
            else
                cout << "Немає ходів для повторення.\n";
        }
        else if (command == "H") {
            Direction best = game.bestMove();
            cout << "Рекомендований хід: "
                << directionLetter(best) << "\n";
        }
        else if (command == "W" || command == "A" ||
            command == "S" || command == "D") {

            Direction direction = parseDirection(command[0]);

            bool moved = history.run(
                make_unique<MoveCommand>(game, direction));

            if (moved) {
                appendLog(direction);

                if (game.won() || game.lost())
                    saveGame(game, "data/game_1.sav");
            }
            else {
                cout << "Цей хід нічого не змінює.\n";
            }
        }
        else {
            cout << "Невідома команда.\n";
        }
    }
}

static void newGame(const GameConfig& config) {
    int size = readInt("Розмір поля (3-6): ", 3, 6);
    unsigned int seed = newSeed();

    GameModel game(size, seed);
    playGame(game, config.ansi, true);
}

static void continueGame(const GameConfig& config) {
    GameModel game(config.size, config.seed);
    string error;

    if (!loadGame(game, "data/game_1.sav", error)) {
        cout << error << "\n";
        return;
    }

    cout << "Збережену партію завантажено.\n";
    playGame(game, config.ansi, false);
}

static void demoGame(const GameConfig& config) {
    GameModel game(config.size, config.seed);

    cout << "Демонстраційний режим. Seed: "
        << config.seed << "\n";

    for (int i = 0; i < 100; i++) {
        Direction direction = game.bestMove();

        if (!game.move(direction))
            break;

        if (game.won() || game.lost())
            break;
    }

    renderBoard(game, config.ansi);
    renderGameStatus(game);
    cout << "Демонстрацію завершено.\n";
}

static void replayGame(const GameConfig& config) {
    GameModel game(config.size, config.seed);
    string error;

    if (!replayLog(game, "data/game_1.log", error)) {
        cout << error << "\n";
        return;
    }

    cout << "Відтворення партії завершено.\n";
    renderBoard(game, config.ansi);
    renderGameStatus(game);
}

static void enableAnsi() {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;

    if (GetConsoleMode(handle, &mode))
        SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

void gameMenu() {
    GameConfig config = loadConfig();
    
    if (config.ansi)
        enableAnsi();

    while (true) {
        cout << "\n========== ДЕНЬ 9: ГРА 2048 ==========\n";
        cout << "1. Нова гра\n";
        cout << "2. Завантажити збереження\n";
        cout << "3. Таблиця рекордів\n";
        cout << "4. Відтворити партію\n";
        cout << "5. Автотести\n";
        cout << "6. Демонстраційний режим\n";
        cout << "0. Назад\n";

        int choice = readInt("Ваш вибір: ", 0, 6);

        switch (choice) {
        case 1:
            newGame(config);
            break;

        case 2:
            continueGame(config);
            break;

        case 3:
            showScores(config.size);
            break;

        case 4:
            replayGame(config);
            break;

        case 5:
            runTests9();
            break;

        case 6:
            demoGame(config);
            break;

        case 0:
            return;
        }
    }
}