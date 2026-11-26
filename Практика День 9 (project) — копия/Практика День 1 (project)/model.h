#pragma once
#include <vector>
#include <memory>
#include <random>
#include <cstdint>
#include <string>
#include <utility>
#include <iterator>

enum class GameState {
    Menu,
    Playing,
    Won,
    Lost
};

enum class Direction {
    Left,
    Right,
    Up,
    Down
};

class GameRule {
public:
    virtual ~GameRule() = default;
    virtual bool mergeLine(std::vector<int>& line, long long& score) const = 0;
};

class Classic2048Rule : public GameRule {
public:
    bool mergeLine(std::vector<int>& line, long long& score) const override;
};

class Tile {
    int value_;
public:
    Tile(int value = 0);
    int value() const;
    void setValue(int value);
    bool empty() const;
};

class GameBoard {
    int size_;
    std::vector<std::vector<Tile>> cells_;
public:
    GameBoard(int size = 4);
    int size() const;
    int get(int r, int c) const;
    void set(int r, int c, int value);
    bool empty(int r, int c) const;
    std::vector<std::pair<int, int>> emptyCells() const;
    std::vector<std::vector<int>> values() const;
    void setValues(const std::vector<std::vector<int>>& values);
};

struct GameSnapshot {
    std::vector<std::vector<int>> board;
    long long score = 0;
    GameState state = GameState::Playing;
    bool continueAfterWin = false;
    std::mt19937 rng;
};

class GameModel {
    GameBoard board_;
    long long score_;
    unsigned int seed_;
    std::mt19937 rng_;
    GameState state_;
    bool continueAfterWin_;
    std::shared_ptr<GameRule> rule_;

    void spawnTile();
    bool hasMoves() const;
    int maxTile() const;

public:
    GameModel(int size = 4, unsigned int seed = 1);

    bool move(Direction direction);
    bool canMove(Direction direction) const;
    void startNew(int size, unsigned int seed);
    void setContinueAfterWin(bool value);

    int size() const;
    int get(int r, int c) const;
    long long score() const;
    unsigned int seed() const;
    GameState state() const;
    bool won() const;
    bool lost() const;
    bool canContinue() const;

    GameSnapshot snapshot() const;
    void restore(const GameSnapshot& snapshot);

    Direction bestMove() const;

    static bool mergeLine(std::vector<int>& line, long long& score);
};