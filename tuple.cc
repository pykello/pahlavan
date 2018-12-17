#include <tuple.h>
#include <string>
#include <cstring>
using namespace std;

std::string tupleToString(const Tuple& tuple, char delimiter) {
    std::string result;
    bool first = true;
    for (const DatumP &datum: tuple) {
        if (!first) {
            result += delimiter;
        }
        result += datum->toString();
        first = false;
    }
    return result;
}
