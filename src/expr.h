#ifndef EXPR_H
#define EXPR_H

#include <tuple.h>
#include <vector>
#include <memory>

class Expr {
public:
    virtual Datum *eval(const Tuple &tuple) = 0;
};

class ConstExpr: public Expr {
public:
    ConstExpr(std::unique_ptr<Datum> val): val(std::move(val)) {}
    Datum *eval(const Tuple &tuple) override {
        return val.get();
    }

    static std::unique_ptr<ConstExpr> makeInt(int value) {
        return std::make_unique<ConstExpr>(std::make_unique<IntDatum>(value));
    }

    static std::unique_ptr<ConstExpr> makeDecimal(double value) {
        return std::make_unique<ConstExpr>(std::make_unique<DoubleDatum>(value));
    }

    template <class valueType>
    static std::unique_ptr<ConstExpr> makeBoxed(valueType value) {
        return std::make_unique<ConstExpr>(std::make_unique<BoxedDatum<valueType>>(value));
    }
private:
    std::unique_ptr<Datum> val;
};

class VarExpr: public Expr {
public:
    VarExpr(int varIndex): varIndex(varIndex) {}
    Datum *eval(const Tuple &tuple) override {
        return tuple[varIndex].get();
    }

    static std::unique_ptr<VarExpr> make(int attr) {
        return std::make_unique<VarExpr>(attr);
    }
private:
    int varIndex;
};

class MultExpr: public Expr {
public:
    MultExpr(std::unique_ptr<Expr> left,
             std::unique_ptr<Expr> right):
                left(std::move(left)), right(std::move(right)) {}

    Datum *eval(const Tuple &tuple) override {
        auto leftResult = left->eval(tuple);
        auto rightResult = right->eval(tuple);
        lastResult = move(leftResult->multiply(*rightResult));
        return lastResult.get();
    }

    static std::unique_ptr<MultExpr> make(std::unique_ptr<Expr> left,
                                          std::unique_ptr<Expr> right) {
        return std::make_unique<MultExpr>(std::move(left), std::move(right));
    }
private:
    std::unique_ptr<Expr> left, right;
    std::unique_ptr<Datum> lastResult;
};

enum CompareOp {
    LT,
    LTE,
    EQ,
    GTE,
    GT
};

static std::unique_ptr<BoolDatum> True = std::make_unique<BoolDatum>(true);
static std::unique_ptr<BoolDatum> False = std::make_unique<BoolDatum>(false);

class CompareExpr: public Expr {
public:
    CompareExpr(std::unique_ptr<Expr> left,
                std::unique_ptr<Expr> right, CompareOp op):
                    left(std::move(left)), right(std::move(right)), op(op) {}

    virtual Datum *eval(const Tuple &tuple) override {
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
        return result ? True.get() : False.get();
    }

    static std::unique_ptr<CompareExpr> make(std::unique_ptr<Expr> left,
                                             std::unique_ptr<Expr> right,
                                             CompareOp op)
    {
        return std::make_unique<CompareExpr>(std::move(left), std::move(right), op);
    }
private:
    std::unique_ptr<Expr> left, right;
    CompareOp op;
};

class AndExpr: public Expr {
public:
    AndExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right):
        left(std::move(left)), right(std::move(right)) {}

    Datum *eval(const Tuple &tuple) override {
        auto lv = left->eval(tuple), rv = right->eval(tuple);
        auto *leftBool = static_cast<const BoolDatum *>(lv);
        auto *rightBool = static_cast<const BoolDatum *>(rv);
        return (leftBool->value && rightBool->value) ? True.get() : False.get();
    }

    static std::unique_ptr<AndExpr> make(std::unique_ptr<Expr> left,
                                         std::unique_ptr<Expr> right)
    {
        return std::make_unique<AndExpr>(std::move(left), std::move(right));
    }

private:
    std::unique_ptr<Expr> left, right;
};

class OrExpr: public Expr {
public:
    OrExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right):
        left(std::move(left)), right(std::move(right)) {}

    Datum *eval(const Tuple &tuple) override {
        auto lv = left->eval(tuple), rv = right->eval(tuple);
        auto *leftBool = static_cast<const BoolDatum *>(lv);
        auto *rightBool = static_cast<const BoolDatum *>(rv);
        return (leftBool->value || rightBool->value) ? True.get() : False.get();
    }

private:
    std::unique_ptr<Expr> left, right;
};

class NotExpr: public Expr {
public:
    NotExpr(std::unique_ptr<Expr> child): child(std::move(child)) {}

    Datum *eval(const Tuple &tuple) override {
        auto cv = child->eval(tuple);
        auto *childBool = static_cast<const BoolDatum *>(cv);
        return (!childBool->value) ? True.get() : False.get();
    }

private:
    std::unique_ptr<Expr> child;
};

#endif
