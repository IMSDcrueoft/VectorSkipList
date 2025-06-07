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

using namespace VSL;

void test1() {
	VectorSkipList<double> skiplist(std::nan(""));

	// test insert
	for (uint64_t i = 0; i < 10; ++i) {
		skiplist.printStructure();
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

	std::cout << "test1 passed!" << std::endl;
}

void test2() {
	// Test large range insertion and sparsity
	VectorSkipList<int> skiplist(-1);
	for (uint64_t i = 0; i < 1000; i += 100) {
		skiplist.setElement(i, static_cast<int>(i * 2));
	}
	for (uint64_t i = 0; i < 1000; ++i) {
		int value = 0;
		skiplist.getElement(i, value);
		if (i % 100 == 0) {
			assert(value == static_cast<int>(i * 2));
		} else {
			assert(value == -1);
		}
	}
	std::cout << "test2 passed!" << std::endl;
}

//void test3() {
//	// Test overwriting existing elements
//	auto none = "none";
//	auto hello = "hello";
//	auto world = "world";
//
//	VectorSkipList<const char**> skiplist(&none);
//	skiplist.setElement(1, &hello);
//	const char** value = nullptr;
//	skiplist.getElement(1, value);
//	assert(value == &hello);
//	skiplist.setElement(1, &world);
//	skiplist.getElement(1, value);
//	assert(value == &world);
//	std::cout << "test3 passed!" << std::endl;
//}

void test4() {
	// Test deletion and reinsertion
	VectorSkipList<int> skiplist(-999);
	skiplist.setElement(10, 42);
	int value = 0;
	skiplist.getElement(10, value);
	assert(value == 42);
	skiplist.setElement(10, -999);
	skiplist.getElement(10, value);
	assert(value == -999);
	skiplist.setElement(10, 100);
	skiplist.getElement(10, value);
	assert(value == 100);
	std::cout << "test4 passed!" << std::endl;
}

void test5() {
	// Test boundary conditions
	VectorSkipList<double> skiplist(std::nan(""));
	double value = 0;
	skiplist.getElement(0, value);
	assert(std::isnan(value));
	skiplist.setElement(0, 3.14);
	skiplist.getElement(0, value);
	assert(value == 3.14);
	skiplist.setElement(UINT64_MAX, 2.71);
	skiplist.getElement(UINT64_MAX, value);
	assert(value == 2.71);
	std::cout << "test5 passed!" << std::endl;
}

// Call in main function
int main() {
	//test1();
	//test2();
	//test3();
	test4();
	test5();
	return 0;
}