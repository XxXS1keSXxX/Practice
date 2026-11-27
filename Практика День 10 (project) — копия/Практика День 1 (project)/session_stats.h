#pragma once

#include <map>
#include <string>
#include <chrono>

class SessionStats {
private:
    std::map<int, int> sessionCounts_;
    std::map<int, int> totalCounts_;

    long long totalSeconds_ = 0;

    std::chrono::steady_clock::time_point startTime_;

    std::string fileName_;
    bool finished_ = false;

    void load();
    void save();

public:
    explicit SessionStats(const std::string& fileName = "stats.txt");

    void recordMenuChoice(int menuItem);
    void finishSession();
    void showStatistics() const;

    long long getSessionSeconds() const;
};
