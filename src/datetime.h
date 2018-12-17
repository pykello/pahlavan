#ifndef DATETIME_H
#define DATETIME_H

#include <iostream>

struct Date {
    int year, month, day;
    Date(int year, int month, int day): year(year), month(month), day(day) {}
    bool operator<(const Date &b) const;
    friend std::ostream &operator<<(std::ostream &output, const Date &d);
};

#endif
