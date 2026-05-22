#include "order_book.h"
#include <vector>
#include <algorithm>

using namespace NanoMatch;

int main() {
    const int N = 100000000;
 // More iterations for optimized because it's so much faster
    OrderBook opt_book;

    for(int i=0; i<100000; ++i) {
        opt_book.limit_order(i, Side::Buy, 1000 + (i % 500), 10);
        opt_book.limit_order(i + 100000, Side::Sell, 1000 + (i % 500), 10);
    }

    for(int i=0; i<N; ++i) {
        if (i % 2 == 0) {
            opt_book.limit_order(200000+i, Side::Sell, 1000 + (i % 100), 1);
        } else {
            opt_book.limit_order(200000+i, Side::Buy, 900 + (i % 100), 1);
        }
    }
    return 0;
}
