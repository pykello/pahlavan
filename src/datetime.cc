#include <datetime.h>
#include <iomanip>
using namespace std;

bool Date::operator<(const Date &b) const
{
    if (year != b.year)
        return year < b.year;
    if (month != b.month)
        return month < b.month;
    return day < b.day;
}

ostream &operator<<(std::ostream &output, const Date &d) {
    output << setfill('0') << setw(4) << d.year << "-";
    output << setfill('0') << setw(2) << d.month << "-";
    output << setfill('0') << setw(2) << d.day;
    return output;
}
