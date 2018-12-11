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

class AggFunc {
public:
    virtual std::unique_ptr<Datum> init() = 0;
    virtual void aggregate(Datum &state, const Datum &next) = 0;
    virtual std::unique_ptr<Datum> finalize(Datum &state) = 0;
};

template <class inputType>
class AggSum: public AggFunc {
    std::unique_ptr<Datum> init() override;
    void aggregate(Datum &state, const Datum &next) override;
    std::unique_ptr<Datum> finalize(Datum &state) override;
};

class AggFuncCall{
public:
    AggFuncCall(std::unique_ptr<AggFunc> func,
                std::unique_ptr<Expr> expr):
        func(std::move(func)), expr(std::move(expr)) {}

    std::unique_ptr<Datum> init();
    void aggregate(Datum &state, const Tuple& next);
    void addResult(Datum &state, Tuple &tuple);
private:
    std::unique_ptr<AggFunc> func;
    std::unique_ptr<Expr> expr;
};

class ExecAgg: public ExecNode {
public:
    ExecAgg(std::unique_ptr<ExecNode> child, std::vector<int> groupBy,
            std::vector<std::unique_ptr<AggFuncCall>> aggs):
                child(std::move(child)), groupBy(groupBy), aggs(std::move(aggs)) {}
    std::vector<Tuple> eval() override;
    Schema getSchema() const override;
private:
    std::unique_ptr<ExecNode> child;
    std::vector<int> groupBy;
    std::vector<std::unique_ptr<AggFuncCall>> aggs;
};

#endif
