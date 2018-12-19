#include <tuple.h>
#include <string>
#include <cstring>
#include <vector>
using namespace std;

static string escapeString(const string &s, char delimiter);
static vector<string> tokenize(const string &s, char delimiter);
static DatumP datumFromString(const string &s, ColumnType type);
static bool boolFromString(const string &s);
static Date dateFromString(const string &s);

string tupleToString(const Tuple& tuple, char delimiter) {
    string result;
    bool first = true;
    for (const DatumP &datum: tuple) {
        if (!first) {
            result += delimiter;
        }
        result += escapeString(datum->toString(), delimiter);
        first = false;
    }
    return result;
}

TupleP tupleFromString(const string &s, const Schema &schema, char delimiter) {
    vector<string> tokens = tokenize(s, delimiter);
    if (tokens.size() != schema.size()) {
        throw;
    }
    TupleP result = make_unique<Tuple>();
    for (size_t i = 0; i < tokens.size(); i++) {
        result->push_back(datumFromString(tokens[i], schema[i]));
    }
    return result;
}

vector<TupleP> parseTuples(const string* data, int row_count,
                           const Schema &schema, char delimiter)
{
    vector<TupleP> result;
    for (size_t i = 0; i < row_count; i++)
        result.push_back(tupleFromString(data[i], schema, delimiter));
    return result;
}

static string escapeString(const string &s, char delimiter) {
    string result;
    for (char c: s) {
        if (c == delimiter || c == '\\') {
            result += "\\";
            result += c;
        } else {
            result += c;
        }
    }
    return result;
}

static vector<string> tokenize(const string &s, char delimiter) {
    vector<string> result;
    string current;
    size_t i = 0;
    for (i = 0; i < s.length(); i++) {
        if (s[i] == delimiter) {
           result.push_back(current);
           current = "";
        } else if (s[i] == '\\' && i + 1 < s.length()) {
            current += s[i + 1];
            i++;
        } else {
            current += s[i];
        }
    }
    result.push_back(current);
    return result;
}

static DatumP datumFromString(const string &s, ColumnType type) {
    switch (type) {
        case TYPE_TEXT:
            return make_unique<StringDatum>(s);
        case TYPE_DECIMAL:
            return make_unique<DoubleDatum>(atof(s.c_str()));
        case TYPE_INT:
            return make_unique<IntDatum>(atoi(s.c_str()));
        case TYPE_BIGINT:
            return make_unique<BigIntDatum>(atoll(s.c_str()));
        case TYPE_DATE:
            return make_unique<DateDatum>(dateFromString(s));
        case TYPE_BOOL:
            return make_unique<BoolDatum>(boolFromString(s));
    }
    return NULL;
}

static bool boolFromString(const string &s) {
    if (s == "1" || s == "true" || s == "True" || s == "TRUE")
        return true;
    return false;
}

static Date dateFromString(const string &s) {
    Date result(0, 0, 0);
    vector<string> tokens = tokenize(s, '-');
    result.year = atoi(tokens[0].c_str());
    result.month = atoi(tokens[1].c_str());
    result.day = atoi(tokens[2].c_str());
    return result;
}
