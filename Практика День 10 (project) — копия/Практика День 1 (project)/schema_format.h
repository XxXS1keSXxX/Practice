#pragma once
#pragma once

#include "binio.h"

#include <memory>
#include <string>
#include <vector>
#include <variant>

enum class ColumnType : std::uint8_t {
    Int32 = 1,
    Int64 = 2,
    Double = 3,
    Bool = 4,
    String = 5,
    Date = 6
};

struct DateValue {
    std::uint16_t year = 0;
    std::uint8_t month = 0;
    std::uint8_t day = 0;
};

using CellValue = std::variant<
    std::int32_t,
    std::int64_t,
    double,
    bool,
    std::string,
    DateValue
>;

struct ColumnInfo {
    std::string name;
    ColumnType type;
};

class Column {
public:
    explicit Column(std::string name);
    virtual ~Column() = default;

    const std::string& name() const;

    virtual ColumnType type() const = 0;
    virtual CellValue parse(const std::string& text) const = 0;
    virtual void write(std::ostream& out, const CellValue& value) const = 0;
    virtual CellValue read(std::istream& in) const = 0;
    virtual std::string toText(const CellValue& value) const = 0;

protected:
    std::string name_;
};

class IntColumn : public Column {
public:
    using Column::Column;

    ColumnType type() const override;
    CellValue parse(const std::string& text) const override;
    void write(std::ostream& out, const CellValue& value) const override;
    CellValue read(std::istream& in) const override;
    std::string toText(const CellValue& value) const override;
};

class Int64Column : public Column {
public:
    using Column::Column;

    ColumnType type() const override;
    CellValue parse(const std::string& text) const override;
    void write(std::ostream& out, const CellValue& value) const override;
    CellValue read(std::istream& in) const override;
    std::string toText(const CellValue& value) const override;
};

class DoubleColumn : public Column {
public:
    using Column::Column;

    ColumnType type() const override;
    CellValue parse(const std::string& text) const override;
    void write(std::ostream& out, const CellValue& value) const override;
    CellValue read(std::istream& in) const override;
    std::string toText(const CellValue& value) const override;
};

class BoolColumn : public Column {
public:
    using Column::Column;

    ColumnType type() const override;
    CellValue parse(const std::string& text) const override;
    void write(std::ostream& out, const CellValue& value) const override;
    CellValue read(std::istream& in) const override;
    std::string toText(const CellValue& value) const override;
};

class StringColumn : public Column {
public:
    using Column::Column;

    ColumnType type() const override;
    CellValue parse(const std::string& text) const override;
    void write(std::ostream& out, const CellValue& value) const override;
    CellValue read(std::istream& in) const override;
    std::string toText(const CellValue& value) const override;
};

class DateColumn : public Column {
public:
    using Column::Column;

    ColumnType type() const override;
    CellValue parse(const std::string& text) const override;
    void write(std::ostream& out, const CellValue& value) const override;
    CellValue read(std::istream& in) const override;
    std::string toText(const CellValue& value) const override;
};

class Schema {
public:
    Schema() = default;
    explicit Schema(const std::string& text);

    void parse(const std::string& text);

    const std::string& text() const;
    std::size_t size() const;

    const Column& column(std::size_t index) const;
    const ColumnInfo& info(std::size_t index) const;

    int findColumn(const std::string& name) const;

    std::vector<std::string> columnNames() const;

private:
    std::string text_;
    std::vector<ColumnInfo> info_;
    std::vector<std::unique_ptr<Column>> columns_;
};

using DataRow = std::vector<CellValue>;
using DataTable = std::vector<DataRow>;

std::string cellToText(
    const Column& column,
    const CellValue& value
);