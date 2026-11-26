#pragma once
#include "model.h"
#include <memory>
#include <vector>

class Command {
public:
    virtual ~Command() = default;
    virtual bool execute() = 0;
    virtual void undo() = 0;
};

class MoveCommand : public Command {
    GameModel& model_;
    Direction direction_;
    GameSnapshot before_;
    GameSnapshot after_;
    bool executed_;

public:
    MoveCommand(GameModel& model, Direction direction);
    bool execute() override;
    void undo() override;
    Direction direction() const;
};

class History {
    std::vector<std::unique_ptr<Command>> done_;
    std::vector<std::unique_ptr<Command>> undone_;

public:
    bool run(std::unique_ptr<Command> command);
    bool undo();
    bool redo();
    void clear();
};