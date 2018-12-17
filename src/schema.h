#ifndef SCHEMA_H
#define SCHEMA_H

#include <vector>
#include <string>
#include <iostream>

struct Date {
    int year, month, day;
    bool operator<(const Date &b) const {
        if (year != b.year)
            return year < b.year;
        if (month != b.month)
            return month < b.month;
        return day < b.day;
    }
    friend std::ostream &operator<<(std::ostream &output, const Date &d) { 
        output << d.year << "-" << d.month << "-" << d.day;
        return output;            
    }
};

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
