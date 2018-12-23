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

struct Lineitem {
    long long orderkey;
    long long partkey;
    long long suppkey;
    int linenumber;
    double quantity;
    double extendedprice;
    double discount;
};

const int l_orderkey = 0;
const int l_partkey = 1;
const int l_suppkey = 2;
const int l_linenumber = 3;
const int l_quantity = 4;
const int l_extendedprice = 5;
const int l_discount = 6;
const int l_shipdate = 10;

static vector<TupleP> readLineitem();
static unique_ptr<ExecNode> tpchQuery6(vector<TupleP> tuples);
static double tpchQuery6Manual(vector<TupleP> &tuples);
static double sum_discount(const vector<Lineitem> &v);
static double sum_discount(const vector<unique_ptr<Lineitem>> &v);
static double sum_discount(const vector<vector<Datum*>> &tuples);
static vector<Lineitem> convertLineitem(const vector<TupleP> &tuples);
static vector<unique_ptr<Lineitem>> convertLineitem2(const vector<Lineitem> &v);
static vector<vector<Datum*>> omitPointers(const vector<TupleP> &tuples);

void scenario1() {
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
}

void scenario2() {
    clock_t c1 = clock();
    vector<TupleP> tuples = readLineitem();
    cout << "Loaded!" << endl;
    clock_t c2 = clock();
    double result = tpchQuery6Manual(tuples);
    clock_t c3 = clock();
    cout << fixed << showpoint << setprecision(6);
    cout << "Load time: " << (c2 - c1) * (1.0 / CLOCKS_PER_SEC) << " s" << endl;
    cout << "Query time: " << (c3 - c2) * (1.0 / CLOCKS_PER_SEC) << " s" << endl;
    cout << result << endl;
}

void scenario3() {
    clock_t c1 = clock();
    vector<TupleP> tuples = readLineitem();
    vector<Lineitem> v = convertLineitem(tuples);
    cout << "Loaded!" << endl;
    clock_t c2 = clock();
    double result = sum_discount(v);
    clock_t c3 = clock();
    cout << fixed << showpoint << setprecision(6);
    cout << "Load time: " << (c2 - c1) * (1.0 / CLOCKS_PER_SEC) << " s" << endl;
    cout << "Query time: " << (c3 - c2) * (1.0 / CLOCKS_PER_SEC) << " s" << endl;
    cout << result << endl;
}

void scenario4() {
    clock_t c1 = clock();
    vector<TupleP> tuples = readLineitem();
    vector<unique_ptr<Lineitem>> v = convertLineitem2(convertLineitem(tuples));
    cout << "Loaded!" << endl;
    clock_t c2 = clock();
    double result = sum_discount(v);
    clock_t c3 = clock();
    cout << fixed << showpoint << setprecision(6);
    cout << "Load time: " << (c2 - c1) * (1.0 / CLOCKS_PER_SEC) << " s" << endl;
    cout << "Query time: " << (c3 - c2) * (1.0 / CLOCKS_PER_SEC) << " s" << endl;
    cout << result << endl;
}

void scenario5() {
    clock_t c1 = clock();
    vector<TupleP> tuples = readLineitem();
    auto v = omitPointers(tuples);
    cout << "Loaded!" << endl;
    clock_t c2 = clock();
    double result = sum_discount(v);
    clock_t c3 = clock();
    cout << fixed << showpoint << setprecision(6);
    cout << "Load time: " << (c2 - c1) * (1.0 / CLOCKS_PER_SEC) << " s" << endl;
    cout << "Query time: " << (c3 - c2) * (1.0 / CLOCKS_PER_SEC) << " s" << endl;
    cout << result << endl;
}

int main() {
    scenario5();
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

static vector<Lineitem> convertLineitem(const vector<TupleP> &tuples) {
    vector<Lineitem> v;
    for (const TupleP &tuple: tuples) {
        Lineitem item;
        auto orderkey = static_cast<const BigIntDatum *>((*tuple)[l_orderkey].get());
        auto partkey = static_cast<const BigIntDatum *>((*tuple)[l_partkey].get());
        auto suppkey = static_cast<const BigIntDatum *>((*tuple)[l_suppkey].get());
        auto linenumber = static_cast<const IntDatum *>((*tuple)[l_linenumber].get());
        auto quantity = static_cast<const DoubleDatum *>((*tuple)[l_quantity].get());
        auto extendedprice = static_cast<const DoubleDatum *>((*tuple)[l_extendedprice].get());
        auto discount = static_cast<const DoubleDatum *>((*tuple)[l_discount].get());

        item.orderkey = orderkey->value;
        item.partkey = partkey->value;
        item.suppkey = suppkey->value;
        item.linenumber = linenumber->value;
        item.quantity = quantity->value;
        item.extendedprice = extendedprice->value;
        item.discount = discount->value;
        v.emplace_back(item);
    }
    return move(v);
}

static vector<unique_ptr<Lineitem>> convertLineitem2(const vector<Lineitem> &v) {
    vector<unique_ptr<Lineitem>> result;
    for (const Lineitem &item: v) {
        result.push_back(make_unique<Lineitem>(item));
    }
    return result;
}

static vector<vector<Datum*>> omitPointers(const vector<TupleP> &tuples) {
    vector<vector<Datum*>> result;
    for(const TupleP& tuple: tuples) {
        vector<Datum*> v;
        for(const DatumP &datum: *tuple) {
            v.push_back(datum.get());
        }
        result.push_back(move(v));
    }
    return result;
}

static double sum_discount(const vector<Lineitem> &v) {
    double result = 0;
    for (const Lineitem& item: v) {
        result += item.discount;
    }
    return result;
}

static double sum_discount(const vector<unique_ptr<Lineitem>> &v) {
    double result = 0;
    for (const unique_ptr<Lineitem>& item: v) {
        result += item->discount;
    }
    return result;
}

static double sum_discount(const vector<vector<Datum*>> &tuples) {
    double result = 0;
    for (const vector<Datum*>& tuple: tuples) {
        auto discount = static_cast<DoubleDatum *>(tuple[l_discount]);
        result += discount->value;
    }
    return result;
}

static double tpchQuery6Manual(vector<TupleP> &tuples) {
    double result = 0;
    for (const TupleP &tuple: tuples) {
        auto discount = static_cast<DoubleDatum *>((*tuple)[l_discount].get());
        result += discount->value;
    }
    return result;
}

static unique_ptr<ExecNode> tpchQuery6(vector<TupleP> tuples) {
    auto scanNode = make_unique<ExecScan>(move(tuples));
    /* sum(l_extendedprice * l_discount) */
    vector<unique_ptr<AggFuncCall>> aggFuncCalls;
    aggFuncCalls.push_back(AggSum<double>::makeCall(
        VarExpr::make(l_discount)
    ));

    /* group by empty tuple, so whole table is a single group */
    vector<int> groupBy {};

    auto aggNode = make_unique<ExecAgg>(move(scanNode), groupBy, move(aggFuncCalls));

    return aggNode;
}
