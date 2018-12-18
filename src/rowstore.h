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
    virtual std::vector<TupleP> eval() = 0;
};

class AggFunc {
public:
    virtual DatumP init() = 0;
    virtual void aggregate(Datum &state, const Datum &next) = 0;
    virtual DatumP finalize(Datum &state) = 0;
};

class AggFuncCall{
public:
    AggFuncCall(std::unique_ptr<AggFunc> func,
                std::unique_ptr<Expr> expr):
        func(std::move(func)), expr(std::move(expr)) {}

    DatumP init();
    void aggregate(Datum &state, const Tuple& next);
    void addResult(Datum &state, Tuple &tuple);
private:
    std::unique_ptr<AggFunc> func;
    std::unique_ptr<Expr> expr;
};

template <class inputType>
class AggSum: public AggFunc {
public:
    DatumP init() override;
    void aggregate(Datum &state, const Datum &next) override;
    DatumP finalize(Datum &state) override;

    static std::unique_ptr<AggFuncCall> makeCall(std::unique_ptr<Expr> expr) {
        return std::make_unique<AggFuncCall>(
                    std::make_unique<AggSum<inputType>>(),
                    std::move(expr));
    }
};

class ExecAgg: public ExecNode {
public:
    ExecAgg(std::unique_ptr<ExecNode> child, std::vector<int> groupBy,
            std::vector<std::unique_ptr<AggFuncCall>> aggs):
                child(std::move(child)), groupBy(groupBy), aggs(std::move(aggs)) {}
    std::vector<TupleP> eval() override;
private:
    std::unique_ptr<ExecNode> child;
    std::vector<int> groupBy;
    std::vector<std::unique_ptr<AggFuncCall>> aggs;

    TupleP getGroupKey(const Tuple &tuple);
};

class ExecScan: public ExecNode {
public:
    ExecScan(std::vector<TupleP> tuples): tuples(std::move(tuples)) {}
    std::vector<TupleP> eval() override;
private:
    std::vector<TupleP> tuples;
};

class ExecFilter: public ExecNode {
public:
    ExecFilter(std::unique_ptr<ExecNode> child,
               std::unique_ptr<Expr> expr):
                    child(std::move(child)), expr(std::move(expr)) {}
    std::vector<TupleP> eval() override;
private:
    std::unique_ptr<ExecNode> child;
    std::unique_ptr<Expr> expr;
};

#endif
