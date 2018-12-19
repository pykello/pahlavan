#include <schema.h>
#include <rowstore.h>
#include <expr.h>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
using namespace std;

const Schema lineitem_schema {
    TYPE_BIGINT, TYPE_BIGINT, TYPE_BIGINT, TYPE_INT, TYPE_DECIMAL, TYPE_DECIMAL,
    TYPE_DECIMAL, TYPE_DECIMAL, TYPE_TEXT, TYPE_TEXT, TYPE_DATE, TYPE_DATE,
    TYPE_DATE, TYPE_TEXT, TYPE_TEXT, TYPE_TEXT
};

const int l_quantity = 4;
const int l_extendedprice = 5;
const int l_discount = 6;
const int l_shipdate = 10;

static vector<TupleP> readLineitem();
static unique_ptr<ExecNode> tpchQuery6(vector<TupleP> tuples);

int main() {
    clock_t c1 = clock();
    unique_ptr<ExecNode> q6 = tpchQuery6(readLineitem());
    cout << "Loaded!" << endl;
    clock_t c2 = clock();
    vector<TupleP> result = q6->eval();
    clock_t c3 = clock();
    cout << fixed << showpoint << setprecision(6);
    cout << "Rows: " << result.size() << endl;
    cout << "Load time: " << (c2 - c1) * (1.0 / CLOCKS_PER_SEC) << " s" << endl;
    cout << "Query time: " << (c3 - c2) * (1.0 / CLOCKS_PER_SEC) << " s" << endl;
    for (const TupleP &tuple: result)
        cout << tupleToString(*tuple) << endl;
    return 0;
}

static vector<TupleP> readLineitem() {
    vector<TupleP> result;
    string line;
    while (getline(cin, line)) {
        result.push_back(tupleFromString(line, lineitem_schema, '|'));
    }
    return result;
}

static unique_ptr<ExecNode> tpchQuery6(vector<TupleP> tuples) {
    auto scanNode = make_unique<ExecScan>(move(tuples));
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
                          ConstExpr::makeDecimal(24 - 1e-6), LT)
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
    return aggNode;
}
