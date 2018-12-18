#include "catch.hpp"
#include <expr.h>
#include <tuple.h>
#include <rowstore.h>
#include <memory>
#include <climits>
using namespace std;

const int testdata_1[] = {1, 2,
                          1, 3,
                          2, 1,
                          2, 10,
                          3, 0};
const int rows_1 = 5;
const int cols_1 = 2;

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

TEST_CASE ( "ScanNode", "[rowstore]" ) {
    unique_ptr<ExecNode> scanNode =
        make_unique<ExecScan>(createIntTable(rows_1, cols_1, testdata_1));
    vector<TupleP> result = scanNode->eval();
    REQUIRE ( result.size() == rows_1 );
    for (size_t r = 0; r < rows_1; r++) {
        for (size_t c = 0; c < cols_1; c++) {
            REQUIRE ( fieldValue<int>(result[r], c) == testdata_1[r * cols_1 + c] );
        }
    }
}
