#include "tests9.h"
#include "model.h"
#include "commands.h"
#include "save.h"
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

static bool testMerge(vector<int> input, vector<int> expected,
    long long expectedScore) {
    long long score = 0;
    GameModel::mergeLine(input, score);
    return input == expected && score == expectedScore;
}

static GameModel makeTestGame() {
    GameModel game(3, 123);
    GameSnapshot s = game.snapshot();

    s.board = {
        { 2, 2, 0 },
        { 0, 0, 0 },
        { 0, 0, 0 }
    };

    s.score = 0;
    s.state = GameState::Playing;
    game.restore(s);

    return game;
}

bool runTests9() {
    int passed = 0;
    int failed = 0;

    auto check = [&](const string& name, bool result) {
        cout << (result ? "[PASS] " : "[FAIL] ") << name << "\n";

        if (result)
            passed++;
        else
            failed++;
        };

    check("Контрольний приклад [2,2,2,2]",
        testMerge({ 2,2,2,2 }, { 4,4,0,0 }, 8));

    check("Контрольний приклад [2,2,4,0]",
        testMerge({ 2,2,4,0 }, { 4,4,0,0 }, 4));

    check("Контрольний приклад [0,2,0,2]",
        testMerge({ 0,2,0,2 }, { 4,0,0,0 }, 4));

    check("Контрольний приклад [4,4,8,0]",
        testMerge({ 4,4,8,0 }, { 8,8,0,0 }, 8));

    check("Плитка зливається лише один раз",
        testMerge({ 2,2,2,0 }, { 4,2,0,0 }, 4));

    check("Рядок без злиття",
        testMerge({ 2,4,8,0 }, { 2,4,8,0 }, 0));

    GameModel game = makeTestGame();
    GameSnapshot before = game.snapshot();

    check("Хід ліворуч змінює поле",
        game.move(Direction::Left));

    GameSnapshot after = game.snapshot();

    check("Після злиття рахунок дорівнює 4",
        game.score() == 4);

    game.restore(before);

    check("Відновлення стану повертає поле та рахунок",
        game.snapshot().board == before.board &&
        game.score() == before.score);

    GameModel commandGame = makeTestGame();
    GameSnapshot commandBefore = commandGame.snapshot();

    History history;
    bool moved = history.run(
        make_unique<MoveCommand>(commandGame, Direction::Left));

    check("Команда виконує хід", moved);

    check("Undo повертає попередній стан", history.undo() &&
        commandGame.snapshot().board == commandBefore.board &&
        commandGame.score() == commandBefore.score);

    check("Redo повторює хід", history.redo() &&
        commandGame.score() == 4);

    GameModel saveTest = makeTestGame();
    saveTest.move(Direction::Left);

    bool saved = saveGame(saveTest, "data/test9.sav");
    GameModel loaded(4, 1);
    string error;

    bool loadedOk = saved &&
        loadGame(loaded, "data/test9.sav", error);

    check("Збереження та завантаження", loadedOk &&
        loaded.score() == saveTest.score() &&
        loaded.size() == saveTest.size() &&
        loaded.snapshot().board == saveTest.snapshot().board);

    if (saved) {
        fstream file("data/test9.sav", ios::binary | ios::in | ios::out);
        file.seekp(10);
        char byte;
        file.read(&byte, 1);
        file.seekp(10);
        byte ^= 1;
        file.write(&byte, 1);
        file.close();

        GameModel damaged(4, 1);
        error.clear();

        check("CRC виявляє пошкоджений файл",
            !loadGame(damaged, "data/test9.sav", error));
    }
    else {
        check("CRC виявляє пошкоджений файл", false);
    }

    startLog(3, 123);
    appendLog(Direction::Left);
    appendLog(Direction::Up);

    GameModel replayed(4, 1);
    error.clear();

    check("Replay читає лог партії",
        replayLog(replayed, "data/game_1.log", error));

    cout << "\nРезультат тестів: "
        << passed << " PASS, "
        << failed << " FAIL\n";

    return failed == 0;
}