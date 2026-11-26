#include "model.h"
#include <algorithm>

bool Classic2048Rule::mergeLine(std::vector<int>& line, long long& score) const {
    std::vector<int> values;

    for (int value : line) {
        if (value != 0)
            values.push_back(value);
    }

    std::vector<int> result;
    for (size_t i = 0; i < values.size(); i++) {
        if (i + 1 < values.size() && values[i] == values[i + 1]) {
            int merged = values[i] * 2;
            result.push_back(merged);
            score += merged;
            i++;
        }
        else {
            result.push_back(values[i]);
        }
    }

    result.resize(line.size(), 0);

    bool changed = result != line;
    line = result;
    return changed;
}

Tile::Tile(int value) : value_(value) {}

int Tile::value() const {
    return value_;
}

void Tile::setValue(int value) {
    value_ = value;
}

bool Tile::empty() const {
    return value_ == 0;
}

GameBoard::GameBoard(int size)
    : size_(size), cells_(size, std::vector<Tile>(size)) {}

int GameBoard::size() const {
    return size_;
}

int GameBoard::get(int r, int c) const {
    return cells_[r][c].value();
}

void GameBoard::set(int r, int c, int value) {
    cells_[r][c].setValue(value);
}

bool GameBoard::empty(int r, int c) const {
    return cells_[r][c].empty();
}

std::vector<std::pair<int, int>> GameBoard::emptyCells() const {
    std::vector<std::pair<int, int>> result;

    for (int r = 0; r < size_; r++)
        for (int c = 0; c < size_; c++)
            if (empty(r, c))
                result.push_back({ r, c });

    return result;
}

std::vector<std::vector<int>> GameBoard::values() const {
    std::vector<std::vector<int>> result(size_, std::vector<int>(size_));

    for (int r = 0; r < size_; r++)
        for (int c = 0; c < size_; c++)
            result[r][c] = get(r, c);

    return result;
}

void GameBoard::setValues(const std::vector<std::vector<int>>& values) {
    for (int r = 0; r < size_; r++)
        for (int c = 0; c < size_; c++)
            set(r, c, values[r][c]);
}

GameModel::GameModel(int size, unsigned int seed)
    : board_(size), score_(0), seed_(seed), rng_(seed),
    state_(GameState::Playing), continueAfterWin_(false),
    rule_(std::make_shared<Classic2048Rule>()) {
    spawnTile();
    spawnTile();
}

void GameModel::startNew(int size, unsigned int seed) {
    if (size < 3 || size > 6)
        size = 4;

    board_ = GameBoard(size);
    score_ = 0;
    seed_ = seed;
    rng_.seed(seed);
    state_ = GameState::Playing;
    continueAfterWin_ = false;

    spawnTile();
    spawnTile();
}

void GameModel::spawnTile() {
    auto empty = board_.emptyCells();

    if (empty.empty())
        return;

    int index = static_cast<int>(rng_() % empty.size());
    int value = (rng_() % 10 == 0) ? 4 : 2;

    board_.set(empty[index].first, empty[index].second, value);
}

bool GameModel::mergeLine(std::vector<int>& line, long long& score) {
    Classic2048Rule rule;
    return rule.mergeLine(line, score);
}

bool GameModel::move(Direction direction) {
    if (state_ == GameState::Lost)
        return false;

    if (state_ == GameState::Won && !continueAfterWin_)
        return false;

    bool changed = false;
    int n = board_.size();

    for (int i = 0; i < n; i++) {
        std::vector<int> line(n);

        for (int j = 0; j < n; j++) {
            int r, c;

            if (direction == Direction::Left) {
                r = i; c = j;
            }
            else if (direction == Direction::Right) {
                r = i; c = n - 1 - j;
            }
            else if (direction == Direction::Up) {
                r = j; c = i;
            }
            else {
                r = n - 1 - j; c = i;
            }

            line[j] = board_.get(r, c);
        }

        long long gained = 0;
        bool lineChanged = rule_->mergeLine(line, gained);
        score_ += gained;
        changed = changed || lineChanged;

        for (int j = 0; j < n; j++) {
            int r, c;

            if (direction == Direction::Left) {
                r = i; c = j;
            }
            else if (direction == Direction::Right) {
                r = i; c = n - 1 - j;
            }
            else if (direction == Direction::Up) {
                r = j; c = i;
            }
            else {
                r = n - 1 - j; c = i;
            }

            board_.set(r, c, line[j]);
        }
    }

    if (!changed)
        return false;

    spawnTile();

    if (maxTile() >= 2048)
        state_ = GameState::Won;
    else if (!hasMoves())
        state_ = GameState::Lost;
    else
        state_ = GameState::Playing;

    return true;
}

bool GameModel::canMove(Direction direction) const {
    GameModel copy = *this;
    return copy.move(direction);
}

bool GameModel::hasMoves() const {
    if (!board_.emptyCells().empty())
        return true;

    int n = board_.size();

    for (int r = 0; r < n; r++)
        for (int c = 0; c < n; c++) {
            if (r + 1 < n && board_.get(r, c) == board_.get(r + 1, c))
                return true;
            if (c + 1 < n && board_.get(r, c) == board_.get(r, c + 1))
                return true;
        }

    return false;
}

int GameModel::maxTile() const {
    int result = 0;

    for (int r = 0; r < size(); r++)
        for (int c = 0; c < size(); c++)
            result = std::max(result, get(r, c));

    return result;
}

void GameModel::setContinueAfterWin(bool value) {
    continueAfterWin_ = value;

    if (value && state_ == GameState::Won) {
        state_ = GameState::Playing;

        if (!hasMoves())
            state_ = GameState::Lost;
    }
}

int GameModel::size() const {
    return board_.size();
}

int GameModel::get(int r, int c) const {
    return board_.get(r, c);
}

long long GameModel::score() const {
    return score_;
}

unsigned int GameModel::seed() const {
    return seed_;
}

GameState GameModel::state() const {
    return state_;
}

bool GameModel::won() const {
    return state_ == GameState::Won;
}

bool GameModel::lost() const {
    return state_ == GameState::Lost;
}

bool GameModel::canContinue() const {
    return continueAfterWin_;
}

GameSnapshot GameModel::snapshot() const {
    GameSnapshot s;
    s.board = board_.values();
    s.score = score_;
    s.state = state_;
    s.continueAfterWin = continueAfterWin_;
    s.rng = rng_;
    return s;
}

void GameModel::restore(const GameSnapshot& s) {
    board_.setValues(s.board);
    score_ = s.score;
    state_ = s.state;
    continueAfterWin_ = s.continueAfterWin;
    rng_ = s.rng;
}

Direction GameModel::bestMove() const {
    Direction directions[] = {
        Direction::Left, Direction::Right,
        Direction::Up, Direction::Down
    };

    Direction best = Direction::Left;
    long long bestScore = -1;

    for (Direction d : directions) {
        GameModel copy = *this;
        long long oldScore = copy.score();

        if (copy.move(d)) {
            long long gained = copy.score() - oldScore;
            long long empty = 0;

            for (int r = 0; r < copy.size(); r++)
                for (int c = 0; c < copy.size(); c++)
                    if (copy.get(r, c) == 0)
                        empty++;

            long long value = gained * 100 + empty;

            if (value > bestScore) {
                bestScore = value;
                best = d;
            }
        }
    }

    return best;
}