#ifndef TUPLE_H
#define TUPLE_H

#include <vector>

class Tuple {
public:
    template <class attrType>
    void addAttr(const attrType &v);

    template <class attrType>
    attrType getAttr(int idx) const;

private:
    std::vector<char> data;
    std::vector<int> attrIndex;
};

#endif