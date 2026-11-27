
#include "session_stats.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;

SessionStats::SessionStats(const string& fileName)
    : fileName_(fileName) {
    startTime_ = chrono::steady_clock::now();
    load();
}

void SessionStats::load() {
    ifstream file(fileName_);

    if (!file.is_open()) {
        return;
    }

    string line;

    while (getline(file, line)) {
        istringstream input(line);

        char type;
        int menuItem;
        int count;
        long long seconds;

        if (!(input >> type)) {
            continue;
        }

        if (type == 'M') {
            if (!(input >> menuItem >> count)) {
                continue;
            }

            if (menuItem < 0 || count < 0) {
                continue;
            }

            totalCounts_[menuItem] += count;
        }
        else if (type == 'T') {
            if (!(input >> seconds)) {
                continue;
            }

            if (seconds < 0) {
                continue;
            }

            totalSeconds_ += seconds;
        }
    }
}

void SessionStats::save() {
    ofstream file(fileName_, ios::trunc);

    if (!file.is_open()) {
        return;
    }

    for (const auto& item : totalCounts_) {
        file << "M " << item.first << " "
            << item.second << '\n';
    }

    file << "T " << totalSeconds_ << '\n';
}

void SessionStats::recordMenuChoice(int menuItem) {
    if (finished_ || menuItem < 0) {
        return;
    }

    sessionCounts_[menuItem]++;
    totalCounts_[menuItem]++;
}

long long SessionStats::getSessionSeconds() const {
    auto now = chrono::steady_clock::now();

    return chrono::duration_cast<chrono::seconds>(
        now - startTime_
    ).count();
}

void SessionStats::finishSession() {
    if (finished_) {
        return;
    }

    finished_ = true;

    long long seconds = getSessionSeconds();

    totalSeconds_ += seconds;

    save();

    cout << "\nТривалість сеансу: "
        << setfill('0')
        << setw(2) << seconds / 60
        << ":"
        << setw(2) << seconds % 60
        << setfill(' ') << '\n';
}

void SessionStats::showStatistics() const {
    cout << "\n===== СТАТИСТИКА СЕАНСУ =====\n";

    vector<pair<int, int>> ranking(
        totalCounts_.begin(),
        totalCounts_.end()
    );

    sort(ranking.begin(), ranking.end(),
        [](const auto& a, const auto& b) {
            if (a.second != b.second) {
                return a.second > b.second;
            }

            return a.first < b.first;
        }
    );

    cout << "\nТоп-3 найчастіших пунктів меню:\n";

    if (ranking.empty()) {
        cout << "Статистика поки відсутня.\n";
    }
    else {
        int limit = min(3, static_cast<int>(ranking.size()));

        for (int i = 0; i < limit; i++) {
            cout << i + 1 << ". Пункт "
                << ranking[i].first
                << " ("
                << ranking[i].second
                << ")\n";
        }
    }

    cout << "\nЗагальний час за всі сеанси: "
        << setfill('0')
        << setw(2) << totalSeconds_ / 60
        << ":"
        << setw(2) << totalSeconds_ % 60
        << setfill(' ') << '\n';

    cout << "Час поточного сеансу: "
        << setfill('0')
        << setw(2) << getSessionSeconds() / 60
        << ":"
        << setw(2) << getSessionSeconds() % 60
        << setfill(' ') << '\n';
}
