#ifndef TUPLE_H
#define TUPLE_H

#include <vector>
#include <exception>
#include <memory>

class Datum {
public:
    virtual bool operator<(const Datum &other) const = 0;
    virtual std::unique_ptr<Datum> clone() const = 0;
    virtual std::unique_ptr<Datum> multiply(const Datum *other) const = 0;
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

    virtual std::unique_ptr<Datum> multiply(const Datum *other) const override {
        throw;
    }
};

template <class T>
class NumericDatum: public BoxedDatum<T> {
public:
    NumericDatum(T value): BoxedDatum<T>(value) {}
    
    std::unique_ptr<Datum> multiply(const Datum *other) const override {
        auto otherNumeric = static_cast<const NumericDatum<T> *>(other);
        return std::make_unique<NumericDatum<T>>(BoxedDatum<T>::value * otherNumeric->value);
    }
};

typedef NumericDatum<int> IntDatum;
typedef BoxedDatum<bool> BoolDatum;
typedef std::vector<std::unique_ptr<Datum>> Tuple;

#endif
