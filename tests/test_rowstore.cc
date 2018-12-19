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
        /* attrs[0] == 2 */
        CompareExpr::make(VarExpr::make(0), ConstExpr::makeInt(2), EQ)
    );

    vector<TupleP> result = filterNode->eval();

    REQUIRE ( result.size() == 2 );
    REQUIRE ( (fieldValue<int>(result[0], 0) == 2 && fieldValue<int>(result[0], 1) == 1) );
    REQUIRE ( (fieldValue<int>(result[1], 0) == 2 && fieldValue<int>(result[1], 1) == 10) );
}

TEST_CASE ( "ExecProject", "[rowstore]" ) {
    /* attrs[1], 2 * attrs[0] */
    std::vector<std::unique_ptr<Expr>> exprs;
    exprs.push_back(VarExpr::make(1));
    exprs.push_back(MultExpr::make(ConstExpr::makeInt(2), VarExpr::make(0)));

    auto projectNode = make_unique<ExecProject>(
        make_unique<ExecScan>(createIntTable(rows_1, cols_1, testdata_1)),
        move(exprs)
    );

    vector<TupleP> result = projectNode->eval();

    REQUIRE ( result.size() == rows_1 );
    for (size_t r = 0; r < rows_1; r++) {
        REQUIRE ( fieldValue<int>(result[r], 0) == testdata_1[r * 2 + 1] );
        REQUIRE ( fieldValue<int>(result[r], 1) == 2 * testdata_1[r * 2] );
    }
}

const std::string lineitem_sample[] = {
    "1|155190|7706|1|17|21168.23|0.04|0.02|N|O|1996-03-13|1996-02-12|1996-03-22|DELIVER IN PERSON|TRUCK|egular courts above the",
    "1|67310|7311|2|36|45983.16|0.09|0.06|N|O|1996-04-12|1996-02-28|1996-04-20|TAKE BACK RETURN|MAIL|ly final dependencies: slyly bold ",
    "1|63700|3701|3|8|13309.60|0.10|0.02|N|O|1996-01-29|1996-03-05|1996-01-31|TAKE BACK RETURN|REG AIR|riously. regular, express dep",
    "1|2132|4633|4|28|28955.64|0.09|0.06|N|O|1996-04-21|1996-03-30|1996-05-16|NONE|AIR|lites. fluffily even de",
    "1|24027|1534|5|24|22824.48|0.10|0.04|N|O|1996-03-30|1996-03-14|1996-04-01|NONE|FOB| pending foxes. slyly re",
    "1|15635|638|6|32|49620.16|0.07|0.02|N|O|1996-01-30|1996-02-07|1996-02-03|DELIVER IN PERSON|MAIL|arefully slyly ex",
    "2|106170|1191|1|38|44694.46|0.00|0.05|N|O|1997-01-28|1997-01-14|1997-02-02|TAKE BACK RETURN|RAIL|ven requests. deposits breach a",
    "3|4297|1798|1|45|54058.05|0.06|0.00|R|F|1994-02-02|1994-01-04|1994-02-23|NONE|AIR|ongside of the furiously brave acco",
    "3|19036|6540|2|49|46796.47|0.10|0.00|R|F|1993-11-09|1993-12-20|1993-11-24|TAKE BACK RETURN|RAIL| unusual accounts. eve",
    "3|128449|3474|3|27|39890.88|0.06|0.07|A|F|1994-01-16|1993-11-22|1994-01-23|DELIVER IN PERSON|SHIP|nal foxes wake. ",
    "3|29380|1883|4|2|2618.76|0.01|0.06|A|F|1993-12-04|1994-01-07|1994-01-01|NONE|TRUCK|y. fluffily pending d",
    "3|183095|650|5|28|32986.52|0.04|0.00|R|F|1993-12-14|1994-01-10|1994-01-01|TAKE BACK RETURN|FOB|ages nag slyly pending",
    "3|62143|9662|6|26|28733.64|0.10|0.02|A|F|1993-10-29|1993-12-18|1993-11-04|TAKE BACK RETURN|RAIL|ges sleep after the caref",
    "4|88035|5560|1|30|30690.90|0.03|0.08|N|O|1996-01-10|1995-12-14|1996-01-18|DELIVER IN PERSON|REG AIR|- quickly regular packages sleep. idly",
    "5|108570|8571|1|15|23678.55|0.02|0.04|R|F|1994-10-31|1994-08-31|1994-11-20|NONE|AIR|ts wake furiously ",
    "5|123927|3928|2|26|50723.92|0.07|0.08|R|F|1994-10-16|1994-09-25|1994-10-19|NONE|FOB|sts use slyly quickly special instruc",
    "5|37531|35|3|50|73426.50|0.08|0.03|A|F|1994-08-08|1994-10-13|1994-08-26|DELIVER IN PERSON|AIR|eodolites. fluffily unusual",
    "6|139636|2150|1|37|61998.31|0.08|0.03|A|F|1992-04-27|1992-05-15|1992-05-02|TAKE BACK RETURN|TRUCK|p furiously special foxes",
    "7|182052|9607|1|12|13608.60|0.07|0.03|N|O|1996-05-07|1996-03-13|1996-06-03|TAKE BACK RETURN|FOB|ss pinto beans wake against th",
    "7|145243|7758|2|9|11594.16|0.08|0.08|N|O|1996-02-01|1996-03-02|1996-02-19|TAKE BACK RETURN|SHIP|es. instructions"
};

const size_t lineitem_rows = 20;

const Schema lineitem_schema {
    TYPE_BIGINT, TYPE_BIGINT, TYPE_BIGINT, TYPE_INT, TYPE_DECIMAL, TYPE_DECIMAL,
    TYPE_DECIMAL, TYPE_DECIMAL, TYPE_TEXT, TYPE_TEXT, TYPE_DATE, TYPE_DATE,
    TYPE_DATE, TYPE_TEXT, TYPE_TEXT, TYPE_TEXT
};

const int l_quantity = 4;
const int l_extendedprice = 5;
const int l_discount = 6;
const int l_shipdate = 10;

TEST_CASE ( "TPCH6", "[rowstore]" ) {
    auto scanNode = make_unique<ExecScan>(
        parseTuples(lineitem_sample, lineitem_rows, lineitem_schema, '|'));
    /* l_shipdate >= '1994-01-01' */
    unique_ptr<Expr> filterExpr = CompareExpr::make(
        VarExpr::make(l_shipdate),
        ConstExpr::makeBoxed<Date>(Date(1994, 1, 1)),
        GTE);
    /* AND l_shipdate < '1995-01-01' */
    filterExpr = AndExpr::make(
        move(filterExpr),
        CompareExpr::make(VarExpr::make(l_shipdate),
                          ConstExpr::makeBoxed<Date>(Date(1995, 1, 1)), LT)
    );
    /* AND l_discount >= 0.06 - 0.01 */
    filterExpr = AndExpr::make(
        move(filterExpr),
        CompareExpr::make(VarExpr::make(l_discount),
                          ConstExpr::makeDecimal(0.06 - 0.01 - 1e-6), GTE)
    );
    /* AND l_discount <= 0.06 + 0.01 */
    filterExpr = AndExpr::make(
        move(filterExpr),
        CompareExpr::make(VarExpr::make(l_discount),
                          ConstExpr::makeDecimal(0.06 + 0.01 + 1e-6), LTE)
    );
    /* AND l_quantity < 100.0 */
    filterExpr = AndExpr::make(
        move(filterExpr),
        CompareExpr::make(VarExpr::make(l_quantity),
                          ConstExpr::makeDecimal(100.0), LT)
    );
    auto filterNode = make_unique<ExecFilter>(move(scanNode), move(filterExpr));
    
    /* sum(l_extendedprice * l_discount) */
    vector<unique_ptr<AggFuncCall>> aggFuncCalls;
    aggFuncCalls.push_back(AggSum<double>::makeCall(
        MultExpr::make(VarExpr::make(l_extendedprice), VarExpr::make(l_discount))
    ));

    /* group by empty tuple, so whole table is a single group */
    vector<int> groupBy {};

    auto aggNode = make_unique<ExecAgg>(move(filterNode), groupBy, move(aggFuncCalls));

    vector<TupleP> result = aggNode->eval();
    REQUIRE ( result.size() == 1 );
    REQUIRE ( tupleToString(*result[0]) == "9187.61" );
}
