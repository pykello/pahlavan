#ifndef DATETIME_H
#define DATETIME_H

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

#endif
