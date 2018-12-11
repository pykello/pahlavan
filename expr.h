#ifndef EXPR_H
#define EXPR_H

#include <tuple.h>
#include <vector>
#include <memory>

class Expr {
public:
    virtual std::unique_ptr<Datum> eval(const Tuple &tuple) = 0;
};

class ConstExpr: public Expr {
public:
    ConstExpr(std::unique_ptr<Datum> val): val(std::move(val)) {}
    std::unique_ptr<Datum> eval(const Tuple &tuple) override {
        return val->clone();
    }
private:
    std::unique_ptr<Datum> val;
};

class VarExpr: public Expr {
public:
    VarExpr(int varIndex): varIndex(varIndex) {}
    std::unique_ptr<Datum> eval(const Tuple &tuple) override {
        return tuple[varIndex]->clone();
    }
private:
    int varIndex;
};

class MultExpr: public Expr {
public:
    MultExpr(std::unique_ptr<Expr> left,
             std::unique_ptr<Expr> right):
                left(std::move(left)), right(std::move(right)) {}

    std::unique_ptr<Datum> eval(const Tuple &tuple) override {
        auto leftResult = left->eval(tuple);
        auto rightResult = right->eval(tuple);
        return leftResult->multiply(rightResult.get());
    }
private:
    std::unique_ptr<Expr> left, right;
};

enum CompareOp {
    LT,
    LTE,
    EQ,
    GTE,
    GT
};

class CompareExpr: public Expr {
public:
    CompareExpr(std::unique_ptr<Expr> left,
                std::unique_ptr<Expr> right, CompareOp op):
                    left(std::move(left)), right(std::move(right)), op(op) {}

    virtual std::unique_ptr<Datum> eval(const Tuple &tuple) override {
        auto lv = left->eval(tuple), rv = right->eval(tuple);
        bool leftIsLess = *lv < *rv;
        bool rightIsLess = *rv < *lv;
        bool eq = !leftIsLess && !rightIsLess;
        bool result = false;
        switch (op) {
            case LT:
                result = leftIsLess;
                break;
            case LTE:
                result = leftIsLess || eq;
                break;
            case EQ:
                result = eq;
                break;
            case GTE: 
                result = rightIsLess || eq;
                break;
            case GT:
                result = rightIsLess;
                break;
        }
        return std::make_unique<BoolDatum>(result);
    }
private:
    std::unique_ptr<Expr> left, right;
    CompareOp op;
};

class AndExpr: public Expr {
public:
    AndExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right):
        left(std::move(left)), right(std::move(right)) {}

    std::unique_ptr<Datum> eval(const Tuple &tuple) override {
        auto lv = left->eval(tuple), rv = right->eval(tuple);
        auto *leftBool = static_cast<const BoolDatum *>(lv.get());
        auto *rightBool = static_cast<const BoolDatum *>(rv.get());
        return std::make_unique<BoolDatum>(leftBool->value && rightBool->value);
    }

private:
    std::unique_ptr<Expr> left, right;
};

class OrExpr: public Expr {
public:
    OrExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right):
        left(std::move(left)), right(std::move(right)) {}

    std::unique_ptr<Datum> eval(const Tuple &tuple) override {
        auto lv = left->eval(tuple), rv = right->eval(tuple);
        auto *leftBool = static_cast<const BoolDatum *>(lv.get());
        auto *rightBool = static_cast<const BoolDatum *>(rv.get());
        return std::make_unique<BoolDatum>(leftBool->value || rightBool->value);
    }

private:
    std::unique_ptr<Expr> left, right;
};

class NotExpr: public Expr {
public:
    NotExpr(std::unique_ptr<Expr> child): child(std::move(child)) {}

    std::unique_ptr<Datum> eval(const Tuple &tuple) override {
        auto cv = child->eval(tuple);
        auto *childBool = static_cast<const BoolDatum *>(cv.get());
        return std::make_unique<BoolDatum>(!childBool->value);
    }

private:
    std::unique_ptr<Expr> child;
};

#endif
