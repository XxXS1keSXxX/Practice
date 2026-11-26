#include "save.h"
#include "commands.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <cstdint>

using namespace std;

static void writeU32(vector<unsigned char>& data, uint32_t value) {
    for (int i = 0; i < 4; i++)
        data.push_back(static_cast<unsigned char>((value >> (i * 8)) & 255));
}

static void writeU64(vector<unsigned char>& data, uint64_t value) {
    for (int i = 0; i < 8; i++)
        data.push_back(static_cast<unsigned char>((value >> (i * 8)) & 255));
}

static uint32_t readU32(const vector<unsigned char>& data, size_t& pos) {
    uint32_t value = 0;

    for (int i = 0; i < 4; i++)
        value |= static_cast<uint32_t>(data[pos++]) << (i * 8);

    return value;
}

static uint64_t readU64(const vector<unsigned char>& data, size_t& pos) {
    uint64_t value = 0;

    for (int i = 0; i < 8; i++)
        value |= static_cast<uint64_t>(data[pos++]) << (i * 8);

    return value;
}

static uint32_t crc32(const vector<unsigned char>& data, size_t count) {
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < count; i++) {
        crc ^= data[i];

        for (int j = 0; j < 8; j++)
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
    }

    return crc ^ 0xFFFFFFFF;
}

GameConfig loadConfig() {
    filesystem::create_directories("data");

    GameConfig config;
    ifstream file("data/game_config.txt");

    if (!file) {
        ofstream out("data/game_config.txt");
        out << "size=4\n";
        out << "seed=12345\n";
        out << "ansi=false\n";
        return config;
    }

    string line;

    while (getline(file, line)) {
        size_t pos = line.find('=');
        if (pos == string::npos)
            continue;

        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);

        try {
            if (key == "size") {
                int size = stoi(value);
                if (size >= 3 && size <= 6)
                    config.size = size;
            }
            else if (key == "seed") {
                config.seed = static_cast<unsigned int>(stoul(value));
            }
            else if (key == "ansi") {
                config.ansi = (value == "true" || value == "1");
            }
        }
        catch (...) {
        }
    }

    return config;
}

bool saveGame(const GameModel& game, const string& path) {
    filesystem::create_directories("data");

    vector<unsigned char> data;
    data.push_back('G');
    data.push_back('S');
    data.push_back('A');
    data.push_back('V');

    writeU32(data, 1);
    writeU32(data, game.size());
    writeU64(data, static_cast<uint64_t>(game.score()));
    writeU32(data, game.seed());
    writeU32(data, static_cast<uint32_t>(game.state()));
    data.push_back(game.canContinue() ? 1 : 0);

    for (int r = 0; r < game.size(); r++)
        for (int c = 0; c < game.size(); c++)
            writeU32(data, static_cast<uint32_t>(game.get(r, c)));

    GameSnapshot snapshot = game.snapshot();
    ostringstream rngText;
    rngText << snapshot.rng;
    string rngState = rngText.str();

    writeU32(data, static_cast<uint32_t>(rngState.size()));

    for (unsigned char ch : rngState)
        data.push_back(ch);

    uint32_t crc = crc32(data, data.size());
    writeU32(data, crc);

    string tempPath = path + ".tmp";

    ofstream file(tempPath, ios::binary);
    if (!file)
        return false;

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();

    if (!file)
        return false;

    error_code ec;
    filesystem::remove(path, ec);
    filesystem::rename(tempPath, path, ec);

    return !ec;
}

bool loadGame(GameModel& game, const string& path, string& error) {
    ifstream file(path, ios::binary);

    if (!file) {
        error = "Не вдалося відкрити файл збереження.";
        return false;
    }

    vector<unsigned char> data(
        (istreambuf_iterator<char>(file)),
        istreambuf_iterator<char>()
    );

    if (data.size() < 25) {
        error = "Файл занадто короткий.";
        return false;
    }

    if (data[0] != 'G' || data[1] != 'S' ||
        data[2] != 'A' || data[3] != 'V') {
        error = "Невірна сигнатура файлу.";
        return false;
    }

    size_t crcPos = data.size() - 4;
    size_t crcReadPos = crcPos;
    uint32_t savedCrc = readU32(data, crcReadPos);

    if (crc32(data, crcPos) != savedCrc) {
        error = "CRC-32 не збігається. Файл пошкоджено.";
        return false;
    }

    size_t pos = 4;

    uint32_t version = readU32(data, pos);
    if (version != 1) {
        error = "Непідтримувана версія збереження.";
        return false;
    }

    uint32_t size = readU32(data, pos);
    if (size < 3 || size > 6) {
        error = "Невірний розмір поля.";
        return false;
    }

    uint64_t score = readU64(data, pos);
    uint32_t seed = readU32(data, pos);
    uint32_t state = readU32(data, pos);

    if (state > static_cast<uint32_t>(GameState::Lost)) {
        error = "Невірний стан гри.";
        return false;
    }

    bool continueAfterWin = data[pos++] != 0;

    vector<vector<int>> board(size, vector<int>(size));

    for (uint32_t r = 0; r < size; r++)
        for (uint32_t c = 0; c < size; c++) {
            if (pos + 4 > crcPos) {
                error = "Файл пошкоджений.";
                return false;
            }

            board[r][c] = static_cast<int>(readU32(data, pos));

            if (board[r][c] < 0) {
                error = "Невірне значення плитки.";
                return false;
            }
        }

    if (pos + 4 > crcPos) {
        error = "Файл пошкоджений.";
        return false;
    }

    uint32_t rngLength = readU32(data, pos);

    if (pos + rngLength != crcPos) {
        error = "Невірна довжина даних.";
        return false;
    }

    string rngText;
    for (uint32_t i = 0; i < rngLength; i++)
        rngText.push_back(static_cast<char>(data[pos++]));

    GameModel loaded(size, seed);
    GameSnapshot snapshot = loaded.snapshot();

    snapshot.board = board;
    snapshot.score = static_cast<long long>(score);
    snapshot.state = static_cast<GameState>(state);
    snapshot.continueAfterWin = continueAfterWin;

    istringstream rngInput(rngText);
    if (!(rngInput >> snapshot.rng)) {
        error = "Не вдалося відновити генератор випадкових чисел.";
        return false;
    }

    loaded.restore(snapshot);
    game = loaded;

    return true;
}

void addScore(int size, long long score, const string& name) {
    filesystem::create_directories("data");

    struct Record {
        int size;
        long long score;
        string name;
    };

    vector<Record> records;
    ifstream file("data/game_scores.txt");
    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        Record record;
        string scoreText;

        if (getline(ss, line, ';') &&
            getline(ss, scoreText, ';') &&
            getline(ss, record.name)) {
            try {
                record.size = stoi(line);
                record.score = stoll(scoreText);
                records.push_back(record);
            }
            catch (...) {
            }
        }
    }

    records.push_back({ size, score, name });

    sort(records.begin(), records.end(),
        [](const Record& a, const Record& b) {
            if (a.size != b.size)
                return a.size < b.size;
            return a.score > b.score;
        });

    ofstream out("data/game_scores.txt");

    for (const auto& record : records)
        out << record.size << ';'
        << record.score << ';'
        << record.name << '\n';
}

void showScores(int size) {
    ifstream file("data/game_scores.txt");

    cout << "\nРекорди для поля " << size << "x" << size << ":\n";

    string line;
    int place = 1;

    while (getline(file, line)) {
        stringstream ss(line);
        string sizeText, scoreText, name;

        if (getline(ss, sizeText, ';') &&
            getline(ss, scoreText, ';') &&
            getline(ss, name)) {
            try {
                if (stoi(sizeText) == size)
                    cout << place++ << ". " << name
                    << " — " << scoreText << "\n";
            }
            catch (...) {
            }
        }
    }

    if (place == 1)
        cout << "Рекордів поки немає.\n";
}

static char directionChar(Direction direction) {
    switch (direction) {
    case Direction::Left: return 'L';
    case Direction::Right: return 'R';
    case Direction::Up: return 'U';
    case Direction::Down: return 'D';
    }

    return 'L';
}

void startLog(int size, unsigned int seed) {
    filesystem::create_directories("data");

    ofstream file("data/game_1.log");
    file << "2048LOG;" << size << ";" << seed << "\n";
}

void appendLog(Direction direction) {
    ofstream file("data/game_1.log", ios::app);
    file << directionChar(direction) << "\n";
}

void appendUndoLog() {
    ofstream file("data/game_1.log", ios::app);
    file << "UNDO\n";
}

void appendRedoLog() {
    ofstream file("data/game_1.log", ios::app);
    file << "REDO\n";
}

void appendContinueLog() {
    ofstream file("data/game_1.log", ios::app);
    file << "CONTINUE\n";
}

bool replayLog(GameModel& game, const string& path, string& error) {
    ifstream file(path);

    if (!file) {
        error = "Не вдалося відкрити лог партії.";
        return false;
    }

    string header;
    if (!getline(file, header)) {
        error = "Лог порожній.";
        return false;
    }

    stringstream ss(header);
    string tag, sizeText, seedText;

    if (!getline(ss, tag, ';') ||
        !getline(ss, sizeText, ';') ||
        !getline(ss, seedText)) {
        error = "Невірний заголовок логу.";
        return false;
    }

    if (tag != "2048LOG") {
        error = "Невірний формат логу.";
        return false;
    }

    int size;
    unsigned int seed;

    try {
        size = stoi(sizeText);
        seed = static_cast<unsigned int>(stoul(seedText));
    }
    catch (...) {
        error = "Невірні параметри логу.";
        return false;
    }

    if (size < 3 || size > 6) {
        error = "Невірний розмір поля у логу.";
        return false;
    }

    GameModel replay(size, seed);
    History history;
    string line;

    while (getline(file, line)) {
        if (line == "L")
            history.run(make_unique<MoveCommand>(replay, Direction::Left));
        else if (line == "R")
            history.run(make_unique<MoveCommand>(replay, Direction::Right));
        else if (line == "U")
            history.run(make_unique<MoveCommand>(replay, Direction::Up));
        else if (line == "D")
            history.run(make_unique<MoveCommand>(replay, Direction::Down));
        else if (line == "UNDO")
            history.undo();
        else if (line == "REDO")
            history.redo();
        else if (line == "CONTINUE")
            replay.setContinueAfterWin(true);
        else {
            error = "Невідома команда у логу.";
            return false;
        }
    }

    game = replay;
    return true;
}