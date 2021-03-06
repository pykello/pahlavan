#include <schema.h>
#include <expr.h>
#include <rowstore.h>
#include <string>
#include <cstring>
#include <map>
using namespace std;

/* explicit template instantiations */
template class AggSum<int>;
template class AggSum<double>;
template class AggSum<long long>;

/* Exec Node */
vector<TupleP> ExecNode::eval() {
    vector<TupleP> result;
    while (true) {
        Tuple* next = nextTuple();
        if (next) {
            result.push_back(cloneTuple(*next));
        } else {
            break;
        }
    }
    return result;
}

/* AggSum */
template <class inputType>
DatumP AggSum<inputType>::init() {
    return make_unique<NumericDatum<inputType>>(0);
}

template <class inputType>
void AggSum<inputType>::aggregate(Datum &state, const Datum &next) {
    auto numericState = static_cast<NumericDatum<inputType> *>(&state);
    auto *nextNumber = static_cast<const NumericDatum<inputType> *>(&next);
    numericState->value += nextNumber->value;
}

template <class inputType>
DatumP AggSum<inputType>::finalize(Datum &state) {
    return state.clone();
}

/* AggFuncCall */
DatumP AggFuncCall::init() {
    return func->init();
}

void AggFuncCall::aggregate(Datum &state, const Tuple& next) {
    func->aggregate(state, *(expr->eval(next)));
}

void AggFuncCall::addResult(Datum &state, Tuple &tuple) {
    tuple.push_back(func->finalize(state));
}

/* ExecAgg */
std::vector<TupleP> ExecAgg::eval() {
    if (groupBy.size() == 0)
        return evalSingleGroup();
    map<TupleP, TupleP, compareTupleP> aggState;

    Tuple *tuple;
    while ((tuple = child->nextTuple())) {
        TupleP groupKey = getGroupKey(*tuple);
        auto it = aggState.find(groupKey);
        /*
         * if we already have a group with the same key, use that
         * otherwise initialize a group.
         */
        Tuple *currentState;
        if (it != aggState.end()) {
            currentState = it->second.get();
        } else {
            TupleP initialState = make_unique<Tuple>();
            currentState = initialState.get();
            for (const auto &agg: aggs)
                currentState->push_back(agg->init());
            aggState[move(groupKey)] = move(initialState);
        }
        /* Now add the current tuple to the group. */
        for (int i = 0; i < aggs.size(); i++) {
            aggs[i]->aggregate(*(*currentState)[i], *tuple);
        }
    }

    /*
     * Loop over all groups, then first add the group key,
     * and then add aggregate results.
     */
    std::vector<TupleP> result;
    for (const pair<const TupleP, TupleP> &p: aggState) {
        TupleP resultTuple = make_unique<Tuple>();
        for (const DatumP &datum: *(p.first)) {
            resultTuple->push_back(datum->clone());
        }
        for (int i = 0; i < aggs.size(); i++) {
            Datum &state = *((*(p.second))[i]);
            aggs[i]->addResult(state, *resultTuple);
        }
        result.push_back(move(resultTuple));
    }
    return result;
}

vector<TupleP> ExecAgg::evalSingleGroup() {
    TupleP state = make_unique<Tuple>();
    for (const auto &agg: aggs)
        state->push_back(agg->init());
    Tuple *tuple;
    while (tuple = child->nextTuple()) {
        for (int i = 0; i < aggs.size(); i++) {
            aggs[i]->aggregate(*((*state)[i]), *tuple);
        }
    }
    TupleP resultTuple = make_unique<Tuple>();
    for (int i = 0; i < aggs.size(); i++) {
        Datum &s = *((*state)[i]);
        aggs[i]->addResult(s, *resultTuple);
    }
    std::vector<TupleP> result;
    result.push_back(move(resultTuple));
    return result;
}

Tuple* ExecAgg::nextTuple() {
    if (!tuplesCalculated) {
        tuples = eval();
        tuplesCalculated = true;
    }
    if (nextTupleIndex < tuples.size())
        return tuples[nextTupleIndex++].get();
    return NULL;
}

TupleP ExecAgg::getGroupKey(const Tuple &tuple) {
    TupleP key = make_unique<Tuple>();
    for (int idx: groupBy)
        key->push_back(tuple[idx]->clone());
    return key;
}

/* ExecScan */
Tuple* ExecScan::nextTuple() {
    if (nextTupleIndex < tuples.size())
        return tuples[nextTupleIndex++].get();
    return NULL;
}

/* ExecFilter */
Tuple* ExecFilter::nextTuple() {
    Tuple* tuple;
    while ((tuple = child->nextTuple())) {
        Datum *exprResult = expr->eval(*tuple);
        auto exprResultBool = static_cast<const BoolDatum *>(exprResult);
        if (exprResultBool->value) {
            return tuple;
        }
    }
    return NULL;
}

/* ExecProject */
Tuple* ExecProject::nextTuple() {
    Tuple* tuple = child->nextTuple();
    if (!tuple)
        return NULL;
    lastTuple = make_unique<Tuple>();
    for (const auto &expr: exprs) {
        DatumP v = expr->eval(*tuple)->clone();
        lastTuple->push_back(move(v));
    }
    return lastTuple.get();
}

/* ExecCount */
Tuple* ExecCount::nextTuple() {
    if (evaluated)
        return NULL;
    int count = 0;
    while (child->nextTuple()) {
        count++;
    }
    result.push_back(make_unique<IntDatum>(count));
    evaluated = true;
    return &result;
}
