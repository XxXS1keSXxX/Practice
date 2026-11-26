#pragma once

#include "schema_format.h"

#include <fstream>
#include <filesystem>
#include <vector>
#include <string>

class SchemaStore {
public:
    SchemaStore() = default;
    ~SchemaStore();

    SchemaStore(const SchemaStore&) = delete;
    SchemaStore& operator=(const SchemaStore&) = delete;

    void create(
        const std::filesystem::path& path,
        const Schema& schema,
        const DataTable& rows
    );

    void open(const std::filesystem::path& path);
    void close();

    bool isOpen() const;

    const Schema& schema() const;
    std::uint64_t rowCount() const;

    DataRow readRow(std::uint64_t rowIndex) const;

    DataRow readSelected(
        std::uint64_t rowIndex,
        const std::vector<std::string>& selectedColumns
    ) const;

    DataTable readRange(
        std::uint64_t firstRow,
        std::uint64_t rowCount,
        const std::vector<std::string>& selectedColumns
    ) const;

    void exportCsv(
        const std::filesystem::path& csvPath,
        std::uint64_t firstRow = 0,
        std::uint64_t count = UINT64_MAX,
        const std::vector<std::string>& selectedColumns = {}
    ) const;

    void importCsv(
        const std::filesystem::path& csvPath,
        const std::filesystem::path& binPath,
        const Schema& schema
    );

    void verify() const;

private:
    std::filesystem::path path_;
    mutable std::ifstream file_;
    Schema schema_;
    std::uint64_t rowCount_ = 0;
    std::vector<std::uint64_t> rowOffsets_;
    std::uint64_t dataStart_ = 0;
    std::uint64_t crcOffset_ = 0;

    void buildIndex();
    void skipValue(std::istream& in, const Column& column) const;

    std::vector<std::size_t> resolveColumns(
        const std::vector<std::string>& names
    ) const;
};