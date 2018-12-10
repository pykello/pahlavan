#ifndef ROWSTORE_H
#define ROWSTORE_H

#include <schema.h>
#include <tuple.h>
#include <expr.h>
#include <memory>

struct RowStore {
    Schema schema;
    std::vector<Tuple> tuples;
};

class ExecNode {
public:
    virtual std::vector<Tuple> eval() = 0;
    virtual Schema getSchema() const = 0;
};

class AggState {};

template <class inputType, class resultType>
class AggFunc {
public:
    virtual std::unique_ptr<AggState> init() = 0;
    virtual void aggregate(AggState *state, const inputType &next) = 0;
    virtual resultType finalize(AggState *state) = 0;
};

template <class inputType>
class AggSumState: public AggState {
public:
    inputType sum = 0;
};

template <class inputType>
class AggSum: public AggFunc<inputType, inputType> {
    std::unique_ptr<AggState> init() override;
    void aggregate(AggState *state, const inputType &next) override;
    inputType finalize(AggState *state) override;
};

class AggFuncCallBase {
public:
    virtual std::unique_ptr<AggState> init() = 0;
    virtual void aggregate(AggState *state, const Tuple& next) = 0;
    virtual void addResult(AggState *state, Tuple &tuple) = 0;
    virtual ColumnType getType() const = 0;
};

template <class inputType, class resultType>
class AggFuncCall: public AggFuncCallBase {
public:
    AggFuncCall(std::unique_ptr<AggFunc<inputType, resultType>> func,
                std::unique_ptr<Expr<inputType>> expr):
        func(std::move(func)), expr(std::move(expr)) {}

    std::unique_ptr<AggState> init() override;
    void aggregate(AggState *state, const Tuple& next) override;
    void addResult(AggState *state, Tuple &tuple) override;
    ColumnType getType() const override;
private:
    std::unique_ptr<AggFunc<inputType, resultType>> func;
    std::unique_ptr<Expr<inputType>> expr;
};

class ExecAgg: public ExecNode {
public:
    ExecAgg(std::unique_ptr<ExecNode> child, std::vector<int> groupBy,
            std::vector<std::unique_ptr<AggFuncCallBase>> aggs):
                child(std::move(child)), groupBy(groupBy), aggs(std::move(aggs)) {}
    std::vector<Tuple> eval() override;
    Schema getSchema() const override;
private:
    std::unique_ptr<ExecNode> child;
    std::vector<int> groupBy;
    std::vector<std::unique_ptr<AggFuncCallBase>> aggs;
};

#endif
