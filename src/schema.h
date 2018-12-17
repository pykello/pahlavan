#ifndef SCHEMA_H
#define SCHEMA_H

#include <vector>
#include <string>
#include <datetime.h>

enum ColumnType {
    TYPE_TEXT,
    TYPE_DECIMAL,
    TYPE_INT,
    TYPE_BIGINT,
    TYPE_DATE,
    TYPE_BOOL
};

typedef std::vector<ColumnType> Schema;

template <class T> ColumnType getColumnType();
template <> inline ColumnType getColumnType<int>() { return TYPE_INT; }
template <> inline ColumnType getColumnType<double>() { return TYPE_DECIMAL; }
template <> inline ColumnType getColumnType<long long>() { return TYPE_BIGINT; }
template <> inline ColumnType getColumnType<Date>() { return TYPE_DATE; }
template <> inline ColumnType getColumnType<bool>() { return TYPE_BOOL; }
template <> inline ColumnType getColumnType<std::string>() { return TYPE_TEXT; }

#endif
