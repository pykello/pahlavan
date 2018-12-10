#include <tuple.h>
#include <string>
#include <cstring>
using namespace std;

template <class T>
static void packValue(std::vector<char> &data, const T& v) {
    char bytes[sizeof(T)];
    memcpy(bytes, &v, sizeof(T));
    for (size_t i = 0; i < sizeof(T); i++)
        data.push_back(bytes[i]);
}

template <class T>
static T unpackValue(const std::vector<char> &data, int offset) {
    char bytes[sizeof(T)];
    for (size_t i = 0; i < sizeof(T); i++)
        bytes[i] = data[offset + i];
    T result;
    memcpy(&result, bytes, sizeof(T));
    return result;
}

template <>
void Tuple::addAttr(const string &s) {
    attrIndex.push_back(data.size());
    packValue(data, s.length());
    for (size_t i = 0; i < s.length(); i++)
        data.push_back(s[i]);
}

template <class T>
void Tuple::addAttr(const T& v) {
    attrIndex.push_back(data.size());
    packValue(data, v);
}

template <>
string Tuple::getAttr(int idx) const {
    size_t length = unpackValue<size_t>(data, attrIndex[idx]);
    idx += sizeof(size_t);
    string result;
    for (size_t i = 0; i < length; i++)
        result += data[idx++];
    return result;
}

template <class T>
T Tuple::getAttr(int idx) const {
    return unpackValue<T>(data, attrIndex[idx]);
}

// Explicit instantiations
template int Tuple::getAttr<int>(int) const;
template void Tuple::addAttr<int>(const int& v);

