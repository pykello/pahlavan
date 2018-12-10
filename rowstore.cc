#include <schema.h>
#include <expr.h>
#include <rowstore.h>
#include <string>
#include <cstring>
using namespace std;

/* explicit template instantiations */
template class AggSum<int>;
template class AggFuncCall<int, int>;

/* AggSum */
template <class inputType>
std::unique_ptr<AggState> AggSum<inputType>::init() {
    return make_unique<AggSumState<inputType>>();
}

template <class inputType>
void AggSum<inputType>::aggregate(AggState *state, const inputType &next) {
    AggSumState<inputType> *sumState = static_cast<AggSumState<inputType>*>(state);
    sumState->sum++;
}

template <class inputType>
inputType AggSum<inputType>::finalize(AggState *state) {
    AggSumState<inputType> *sumState = static_cast<AggSumState<inputType>*>(state);
    return sumState->sum;
}

/* AggFuncCall */
template <class inputType, class resultType>
unique_ptr<AggState> AggFuncCall<inputType, resultType>::init() {
    return func->init();
}

template <class inputType, class resultType>
void AggFuncCall<inputType, resultType>::aggregate(AggState *state, const Tuple& next) {
    func->aggregate(state, expr->eval(next));
}

template <class inputType, class resultType>
void AggFuncCall<inputType, resultType>::addResult(AggState *state, Tuple &tuple) {
    tuple.addAttr(func->finalize(state));
}

template <class inputType, class resultType>
ColumnType AggFuncCall<inputType, resultType>::getType() const {
    return getColumnType<resultType>();
}

/* ExecAgg */
std::vector<Tuple> ExecAgg::eval() {
    std::vector<Tuple> result;
    return result;
}

Schema ExecAgg::getSchema() const {
    Schema schema;
    Schema childSchema = child->getSchema();
    for (size_t i = 0; i < groupBy.size(); i++) {
        schema.push_back(childSchema[i]);
    }
    for (size_t i = 0; i < aggs.size(); i++) {
        schema.push_back(aggs[i]->getType());
    }
    return schema;
}
