#include <schema.h>
#include <expr.h>
#include <rowstore.h>
#include <string>
#include <cstring>
using namespace std;

/* explicit template instantiations */
template class AggSum<int>;

/* AggSum */
template <class inputType>
std::unique_ptr<Datum> AggSum<inputType>::init() {
    return make_unique<NumericDatum<inputType>>(0);
}

template <class inputType>
void AggSum<inputType>::aggregate(Datum &state, const Datum &next) {
    auto numericState = static_cast<NumericDatum<inputType> *>(&state);
    numericState->value++;
}

template <class inputType>
std::unique_ptr<Datum> AggSum<inputType>::finalize(Datum &state) {
    return state.clone();
}

/* AggFuncCall */
unique_ptr<Datum> AggFuncCall::init() {
    return func->init();
}

void AggFuncCall::aggregate(Datum &state, const Tuple& next) {
    func->aggregate(state, *(expr->eval(next)));
}

void AggFuncCall::addResult(Datum &state, Tuple &tuple) {
    tuple.push_back(func->finalize(state));
}

/* ExecAgg */
std::vector<Tuple> ExecAgg::eval() {
    std::vector<Tuple> result;
    // TODO
    return result;
}

Schema ExecAgg::getSchema() const {
    Schema schema;
    // TODO
    return schema;
}
