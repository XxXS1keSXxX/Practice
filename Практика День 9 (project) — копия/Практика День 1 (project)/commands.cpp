#include "commands.h"

MoveCommand::MoveCommand(GameModel& model, Direction direction)
    : model_(model), direction_(direction), executed_(false) {}

bool MoveCommand::execute() {
    if (executed_) {
        model_.restore(after_);
        return true;
    }

    before_ = model_.snapshot();

    if (!model_.move(direction_))
        return false;

    after_ = model_.snapshot();
    executed_ = true;
    return true;
}

void MoveCommand::undo() {
    if (executed_)
        model_.restore(before_);
}

Direction MoveCommand::direction() const {
    return direction_;
}

bool History::run(std::unique_ptr<Command> command) {
    if (!command->execute())
        return false;

    done_.push_back(std::move(command));
    undone_.clear();
    return true;
}

bool History::undo() {
    if (done_.empty())
        return false;

    done_.back()->undo();
    undone_.push_back(std::move(done_.back()));
    done_.pop_back();
    return true;
}

bool History::redo() {
    if (undone_.empty())
        return false;

    undone_.back()->execute();
    done_.push_back(std::move(undone_.back()));
    undone_.pop_back();
    return true;
}

void History::clear() {
    done_.clear();
    undone_.clear();
}