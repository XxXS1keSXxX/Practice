#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <stdexcept>
#include <vector>
#include <filesystem>

inline void writeU8(std::ostream& out, std::uint8_t value) {
    out.put(static_cast<char>(value));
    if (!out)
        throw std::runtime_error("Помилка запису байта");
}

inline std::uint8_t readU8(std::istream& in) {
    int value = in.get();

    if (value == EOF)
        throw std::runtime_error("Неочікуваний кінець файлу");

    return static_cast<std::uint8_t>(value);
}

inline void writeU32(std::ostream& out, std::uint32_t value) {
    for (int i = 0; i < 4; ++i)
        out.put(static_cast<char>((value >> (i * 8)) & 0xFF));

    if (!out)
        throw std::runtime_error("Помилка запису u32");
}

inline std::uint32_t readU32(std::istream& in) {
    std::uint32_t value = 0;

    for (int i = 0; i < 4; ++i) {
        value |= static_cast<std::uint32_t>(
            readU8(in)
            ) << (i * 8);
    }

    return value;
}

inline void writeU64(std::ostream& out, std::uint64_t value) {
    for (int i = 0; i < 8; ++i)
        out.put(static_cast<char>((value >> (i * 8)) & 0xFF));

    if (!out)
        throw std::runtime_error("Помилка запису u64");
}

inline std::uint64_t readU64(std::istream& in) {
    std::uint64_t value = 0;

    for (int i = 0; i < 8; ++i) {
        value |= static_cast<std::uint64_t>(
            readU8(in)
            ) << (i * 8);
    }

    return value;
}

inline void writeDouble(std::ostream& out, double value) {
    static_assert(sizeof(double) == 8);

    std::uint64_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));

    writeU64(out, bits);
}

inline double readDouble(std::istream& in) {
    std::uint64_t bits = readU64(in);
    double value = 0;

    std::memcpy(&value, &bits, sizeof(value));

    return value;
}

inline void writeString(std::ostream& out, const std::string& value) {
    if (value.size() > UINT32_MAX)
        throw std::runtime_error("Рядок занадто великий");

    writeU32(out, static_cast<std::uint32_t>(value.size()));

    out.write(
        value.data(),
        static_cast<std::streamsize>(value.size())
    );

    if (!out)
        throw std::runtime_error("Помилка запису рядка");
}

inline std::string readString(std::istream& in) {
    std::uint32_t size = readU32(in);

    if (size > 16 * 1024 * 1024)
        throw std::runtime_error("Надмірна довжина рядка");

    std::string value(size, '\0');

    in.read(
        value.data(),
        static_cast<std::streamsize>(size)
    );

    if (!in)
        throw std::runtime_error("Не вдалося прочитати рядок");

    return value;
}

inline std::uint32_t crc32(
    const unsigned char* data,
    std::size_t size,
    std::uint32_t crc = 0
) {
    crc = ~crc;

    for (std::size_t i = 0; i < size; ++i) {
        crc ^= data[i];

        for (int k = 0; k < 8; ++k) {
            crc = (crc >> 1) ^
                (0xEDB88320u & (0u - (crc & 1u)));
        }
    }

    return ~crc;
}

inline std::uint32_t crc32File(
    const std::filesystem::path& path,
    std::uint64_t byteCount
) {
    std::ifstream in(path, std::ios::binary);

    if (!in)
        throw std::runtime_error("Не вдалося відкрити файл для CRC");

    std::vector<unsigned char> buffer(8192);
    std::uint64_t remaining = byteCount;
    std::uint32_t crc = 0;

    while (remaining > 0) {
        std::size_t amount = static_cast<std::size_t>(
            remaining < buffer.size()
            ? remaining
            : buffer.size()
            );

        in.read(
            reinterpret_cast<char*>(buffer.data()),
            static_cast<std::streamsize>(amount)
        );

        if (!in)
            throw std::runtime_error("Помилка читання файлу для CRC");

        crc = crc32(buffer.data(), amount, crc);
        remaining -= amount;
    }

    return crc;
}

inline void hexdump(
    const std::filesystem::path& path,
    std::size_t byteCount
) {
    std::ifstream in(path, std::ios::binary);

    if (!in) {
        std::cout << "Не вдалося відкрити файл.\n";
        return;
    }

    std::cout << "\nHEX-DUMP\n";

    std::size_t offset = 0;

    while (offset < byteCount && in) {
        unsigned char bytes[16]{};
        std::size_t count = 0;

        while (count < 16 && offset + count < byteCount) {
            int value = in.get();

            if (value == EOF)
                break;

            bytes[count++] = static_cast<unsigned char>(value);
        }

        if (count == 0)
            break;

        std::cout << std::hex << std::uppercase
            << std::setw(8) << std::setfill('0')
            << offset << "  ";

        for (std::size_t i = 0; i < 16; ++i) {
            if (i < count)
                std::cout << std::setw(2)
                << static_cast<int>(bytes[i]) << ' ';
            else
                std::cout << "   ";
        }

        std::cout << " |";

        for (std::size_t i = 0; i < count; ++i) {
            char c = static_cast<char>(bytes[i]);

            std::cout << (
                c >= 32 && c <= 126 ? c : '.'
                );
        }

        std::cout << "|\n";
        offset += count;
    }

    std::cout << std::dec << std::setfill(' ');
}