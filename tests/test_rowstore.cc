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

TEST_CASE ( "ExecScan", "[rowstore]" ) {
    auto scanNode = make_unique<ExecScan>(createIntTable(rows_1, cols_1, testdata_1));

    vector<TupleP> result = scanNode->eval();

    REQUIRE ( result.size() == rows_1 );
    for (size_t r = 0; r < rows_1; r++) {
        for (size_t c = 0; c < cols_1; c++) {
            REQUIRE ( fieldValue<int>(result[r], c) == testdata_1[r * cols_1 + c] );
        }
    }
}

TEST_CASE ( "Aggregate Sum(int), a column", "[rowstore]" ) {
    auto scanNode = make_unique<ExecScan>(createIntTable(rows_1, cols_1, testdata_1));

    /* group by first column */
    vector<int> groupBy { 0 };

    vector<unique_ptr<AggFuncCall>> aggFuncCalls;
    aggFuncCalls.push_back(AggSum<int>::makeCall(VarExpr::make(1)));

    auto aggNode = make_unique<ExecAgg>(move(scanNode), groupBy, move(aggFuncCalls)); 

    vector<TupleP> result = aggNode->eval();

    REQUIRE ( result.size() == 3 );
    REQUIRE ( (fieldValue<int>(result[0], 0) == 1 && fieldValue<int>(result[0], 1) == 5) );
    REQUIRE ( (fieldValue<int>(result[1], 0) == 2 && fieldValue<int>(result[1], 1) == 11) );
    REQUIRE ( (fieldValue<int>(result[2], 0) == 3 && fieldValue<int>(result[2], 1) == 0) );
}

TEST_CASE ( "Aggregate Sum(int), whole table as a group", "[rowstore]" ) {
    auto scanNode = make_unique<ExecScan>(createIntTable(rows_1, cols_1, testdata_1));

    /* group by empty tuple, so whole table is a single group */
    vector<int> groupBy {};

    vector<unique_ptr<AggFuncCall>> aggFuncCalls;
    aggFuncCalls.push_back(AggSum<int>::makeCall(VarExpr::make(0)));
    aggFuncCalls.push_back(AggSum<int>::makeCall(VarExpr::make(1)));

    auto aggNode = make_unique<ExecAgg>(move(scanNode), groupBy, move(aggFuncCalls)); 

    vector<TupleP> result = aggNode->eval();

    REQUIRE ( result.size() == 1 );
    REQUIRE ( (fieldValue<int>(result[0], 0) == 9 && fieldValue<int>(result[0], 1) == 16) );
}

TEST_CASE ( "Aggregate Sum(int), a constant", "[rowstore]" ) {
    auto scanNode = make_unique<ExecScan>(createIntTable(rows_1, cols_1, testdata_1));

    /* group by first column */
    vector<int> groupBy { 0 };

    vector<unique_ptr<AggFuncCall>> aggFuncCalls;
    aggFuncCalls.push_back(AggSum<int>::makeCall(ConstExpr::makeInt(1)));

    auto aggNode = make_unique<ExecAgg>(move(scanNode), groupBy, move(aggFuncCalls)); 

    vector<TupleP> result = aggNode->eval();

    REQUIRE ( result.size() == 3 );
    REQUIRE ( (fieldValue<int>(result[0], 0) == 1 && fieldValue<int>(result[0], 1) == 2) );
    REQUIRE ( (fieldValue<int>(result[1], 0) == 2 && fieldValue<int>(result[1], 1) == 2) );
    REQUIRE ( (fieldValue<int>(result[2], 0) == 3 && fieldValue<int>(result[2], 1) == 1) );
}

TEST_CASE ( "ExecFilter", "[rowstore]" ) {
    auto filterNode = make_unique<ExecFilter>(
        make_unique<ExecScan>(createIntTable(rows_1, cols_1, testdata_1)),
        CompareExpr::make(VarExpr::make(0), ConstExpr::makeInt(2), EQ)
    );

    vector<TupleP> result = filterNode->eval();

    REQUIRE ( result.size() == 2 );
    REQUIRE ( (fieldValue<int>(result[0], 0) == 2 && fieldValue<int>(result[0], 1) == 1) );
    REQUIRE ( (fieldValue<int>(result[1], 0) == 2 && fieldValue<int>(result[1], 1) == 10) );

}
