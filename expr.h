#ifndef EXPR_H
#define EXPR_H

#include <tuple.h>
#include <vector>
#include <memory>

template <class resultType>
class Expr {
public:
    virtual resultType eval(const Tuple &tuple) = 0;
};

template <class constType>
class ConstExpr: public Expr<constType> {
public:
    ConstExpr(constType val): val(val) {}
    virtual constType eval(const Tuple &tuple) override {
        return val;
    }
private:
    constType val;
};

template <class varType>
class VarExpr: public Expr<varType> {
public:
    VarExpr(int varIndex): varIndex(varIndex) {}
    virtual varType eval(const Tuple &tuple) override {
        return tuple.getAttr<varType>(varIndex);
    }
private:
    int varIndex;
};

template <class resultType>
class MultExpr: public Expr<resultType> {
public:
    MultExpr(std::unique_ptr<Expr<resultType>> left,
             std::unique_ptr<Expr<resultType>> right):
                left(std::move(left)), right(std::move(right)) {}

    virtual resultType eval(const Tuple &tuple) override {
        return left->eval(tuple) * right->eval(tuple);
    }
private:
    std::unique_ptr<Expr<resultType>> left, right;
};

enum CompareOp {
    LT,
    LTE,
    EQ,
    GTE,
    GT
};

template <class childType>
class CompareExpr: public Expr<bool> {
public:
    CompareExpr(std::unique_ptr<Expr<childType>> left,
                std::unique_ptr<Expr<childType>> right, CompareOp op):
                    left(std::move(left)), right(std::move(right)), op(op) {}

    virtual bool eval(const Tuple &tuple) override {
        childType lv = left->eval(tuple), rv = right->eval(tuple);
        switch (op) {
            case LT:
                return lv < rv;
            case LTE:
                return lv <= rv;
            case EQ:
                return lv == rv;
            case GTE: 
                return lv >= rv;
            case GT:
                return lv > rv;
        }
    }
private:
    std::unique_ptr<Expr<childType>> left, right;
    CompareOp op;
};

class AndExpr: public Expr<bool> {
public:
    AndExpr(std::unique_ptr<Expr<bool>> left, std::unique_ptr<Expr<bool>> right):
        left(std::move(left)), right(std::move(right)) {}

    virtual bool eval(const Tuple &tuple) override {
        return left->eval(tuple) && right->eval(tuple);
    }

private:
    std::unique_ptr<Expr<bool>> left, right;
};

class OrExpr: public Expr<bool> {
public:
    OrExpr(std::unique_ptr<Expr<bool>> left, std::unique_ptr<Expr<bool>> right):
        left(std::move(left)), right(std::move(right)) {}

    virtual bool eval(const Tuple &tuple) override {
        return left->eval(tuple) || right->eval(tuple);
    }

private:
    std::unique_ptr<Expr<bool>> left, right;
};

class NotExpr: public Expr<bool> {
public:
    NotExpr(std::unique_ptr<Expr<bool>> child): child(std::move(child)) {}

    virtual bool eval(const Tuple &tuple) override {
        return !child->eval(tuple);
    }

private:
    std::unique_ptr<Expr<bool>> child;
};

#endif
