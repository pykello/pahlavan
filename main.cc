#include <schema.h>
#include <rowstore.h>
#include <expr.h>
#include <iostream>
#include <vector>
using namespace std;

TupleP createIntTuple(size_t n, const int* values) {
    TupleP result = make_unique<Tuple>();
    for (size_t i = 0; i < n; i++)
        result->push_back(make_unique<IntDatum>(values[i]));
    return result;
}

vector<TupleP> createIntTable(size_t rows, size_t cols, const int *values) {
    vector<TupleP> result;
    for (size_t i = 0; i < rows; i++) {
        result.push_back(createIntTuple(cols, values));
        values += cols;
    }
    return result;
}

int main() {
    int t1_values[] = {1, 2,
                       1, 3,
                       2, 1,
                       2, 10};
    unique_ptr<ExecNode> scanNode =
        make_unique<ExecScan>(createIntTable(4, 2, t1_values));
    vector<int> groupBy { 0 };
    unique_ptr<AggFuncCall> sumFuncCall =
        make_unique<AggFuncCall>(
            make_unique<AggSum<int>>(),
            make_unique<VarExpr>(1)
        );
    vector<unique_ptr<AggFuncCall>> aggFuncCalls;
    aggFuncCalls.push_back(move(sumFuncCall));
    unique_ptr<ExecNode> aggNode =
        make_unique<ExecAgg>(move(scanNode), groupBy, move(aggFuncCalls)); 
    vector<TupleP> result = aggNode->eval();
    cout << result.size() << endl;
    return 0;
}
