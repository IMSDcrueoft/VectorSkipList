/*
 * MIT License
 * Copyright (c) 2025 IMSDcrueoft (https://github.com/IMSDcrueoft)
 * See LICENSE file in the root directory for full license text.
*/
#include "./src/VectorSkipList.hpp"
#include <cmath>
#include <iostream>
#include <vector>
#include <cassert>

int main() {
    using namespace VSL;
    VectorSkipList<double> skiplist(std::nan(""));

    // test insert
    for (uint64_t i = 0; i < 10; ++i) {
        skiplist.setElement(i, i * 1.5);
    }

    // test search
    for (uint64_t i = 0; i < 10; ++i) {
        double value = 0;
        skiplist.getElement(i, value);
        assert(value == i * 1.5);
        std::cout << "Index " << i << " = " << value << std::endl;
    }

    // find not exist
    double notfound = 123;
    skiplist.getElement(100, notfound);
    assert(std::isnan(notfound));
    std::cout << "Index 100 = NaN? " << std::isnan(notfound) << std::endl;

    // test delete
    skiplist.setElement(5, std::nan(""));
    double deleted = 0;
    skiplist.getElement(5, deleted);
    assert(std::isnan(deleted));
    std::cout << "Index 5 deleted, value = NaN? " << std::isnan(deleted) << std::endl;

    // range test
    skiplist.setElement(31, 99.9);
    double edge = 0;
    skiplist.getElement(31, edge);
    assert(edge == 99.9);
    std::cout << "Index 31 = " << edge << std::endl;

    std::cout << "All tests passed!" << std::endl;
    return 0;
}