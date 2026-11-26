#pragma once
#include "model.h"
#include <string>

struct GameConfig {
    int size = 4;
    unsigned int seed = 12345;
    bool ansi = false;
};

GameConfig loadConfig();
bool saveGame(const GameModel& game, const std::string& path);
bool loadGame(GameModel& game, const std::string& path, std::string& error);

void addScore(int size, long long score, const std::string& name);
void showScores(int size);

void startLog(int size, unsigned int seed);
void appendLog(Direction direction);
void appendUndoLog();
void appendRedoLog();
void appendContinueLog();
bool replayLog(GameModel& game, const std::string& path, std::string& error);