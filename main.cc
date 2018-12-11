#include <schema.h>
#include <rowstore.h>
#include <expr.h>
#include <iostream>
#include <vector>
using namespace std;

int main() {
    // auto mult(make_unique<MultExpr<int>>(
    //             make_unique<VarExpr<int>>(0),
    //             make_unique<VarExpr<int>>(1)));
    // auto cmp(make_unique<CompareExpr<int>>(
    //             make_unique<VarExpr<int>>(0),
    //             make_unique<ConstExpr<int>>(242),
    //             EQ));

    // Tuple tuple;
    // tuple.addAttr<int>(12);
    // tuple.addAttr<int>(13);

    // cout << mult->eval(tuple) << endl;
    // cout << cmp->eval(tuple) << endl;

    // AggFuncCall<int, int> afc(make_unique<AggSum<int>>(), std::move(mult));

    return 0;
}
