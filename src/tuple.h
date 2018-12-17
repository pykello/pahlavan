#ifndef TUPLE_H
#define TUPLE_H

#include <vector>
#include <exception>
#include <memory>
#include <string>
#include <sstream>

class Datum {
public:
    virtual bool operator<(const Datum &other) const = 0;
    virtual std::unique_ptr<Datum> clone() const = 0;
    virtual std::unique_ptr<Datum> multiply(const Datum &other) const = 0;
    virtual std::string toString() const = 0;

    virtual bool operator==(const Datum &other) const {
        return !(*this < other) && !(other < *this);
    }

    virtual bool operator!=(const Datum &other) const {
        return !(*this == other);
    }

    virtual bool operator>=(const Datum &other) const {
        return !(*this < other);
    }

    virtual bool operator>(const Datum &other) const {
        return !(*this < other) && !(*this == other);
    }

    virtual bool operator<=(const Datum &other) const {
        return *this < other || *this == other;
    }
};

template <class T>
class BoxedDatum: public Datum {
public:
    T value;
    BoxedDatum(T value): value(value) {}
    virtual bool operator<(const Datum &other) const override {
        auto otherBoxed = static_cast<const BoxedDatum<T> *>(&other);
        return value < otherBoxed->value;
    }

    virtual std::unique_ptr<Datum> clone() const override {
        return std::make_unique<BoxedDatum<T>>(value);
    }

    virtual std::unique_ptr<Datum> multiply(const Datum &other) const override {
        throw;
    }

    virtual std::string toString() const override {
        std::ostringstream sstream;
        sstream << value;
        return sstream.str();
    }
};

template <class T>
class NumericDatum: public BoxedDatum<T> {
public:
    NumericDatum(T value): BoxedDatum<T>(value) {}
    
    std::unique_ptr<Datum> multiply(const Datum &other) const override {
        auto otherNumeric = static_cast<const NumericDatum<T> &>(other);
        return std::make_unique<NumericDatum<T>>(BoxedDatum<T>::value * otherNumeric.value);
    }
};

typedef NumericDatum<int> IntDatum;
typedef NumericDatum<double> DoubleDatum;
typedef BoxedDatum<bool> BoolDatum;
typedef BoxedDatum<std::string> StringDatum;
typedef std::unique_ptr<Datum> DatumP;
typedef std::vector<DatumP> Tuple;
typedef std::unique_ptr<Tuple> TupleP;

struct compareTupleP {
    bool operator()(const TupleP &a, const TupleP &b) const {
        for (size_t i = 0; i < a->size() && i < b->size(); i++) {
            Datum *datumA = (*a)[i].get();
            Datum *datumB = (*b)[i].get();
            if (*datumA < *datumB)
                return true;
            if (*datumB < *datumA)
                return false;
        }
        return a->size() < b-> size();
    }
};

std::string tupleToString(const Tuple& tuple, char delimiter=',');

#endif
