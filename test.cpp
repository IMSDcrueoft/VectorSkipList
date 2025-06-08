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
#include <chrono>
#include <map>

constexpr auto testCount = 5e6;
using namespace VSL;

void test1() {
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
	}

	// find not exist
	double notfound = 123;
	skiplist.getElement(100, notfound);
	assert(std::isnan(notfound));

	// test delete
	skiplist.setElement(5, std::nan(""));
	double deleted = 0;
	skiplist.getElement(5, deleted);
	assert(std::isnan(deleted));

	// range test
	skiplist.setElement(31, 99.9);
	double edge = 0;
	skiplist.getElement(31, edge);
	assert(edge == 99.9);

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

void test3() {
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
	std::cout << "test3 passed!" << std::endl;
}

void test4() {
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
	std::cout << "test4 passed!" << std::endl;
}

void test_performance_stdmap() {
	const uint64_t N = testCount;
	std::map<uint64_t, int> m;

	auto start_insert = std::chrono::high_resolution_clock::now();
	for (uint64_t i = 0; i < N; ++i) {
		m[i] = static_cast<int>(i);
	}
	auto end_insert = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> insert_duration = end_insert - start_insert;
	std::cout << "[std::map] Insert " << N << " elements took: " << insert_duration.count() << " seconds" << std::endl;

	auto start_query = std::chrono::high_resolution_clock::now();
	int sum = 0;
	for (uint64_t i = 0; i < N; ++i) {
		auto it = m.find(i);
		int value = (it != m.end()) ? it->second : -1;
		sum += value;
	}
	auto end_query = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> query_duration = end_query - start_query;
	std::cout << "[std::map] Query " << N << " elements took: " << query_duration.count() << " seconds" << std::endl;
	std::cout << "[std::map] Query sum: " << sum << std::endl;

	// Random access test (using fast LCG)
	auto start_random_query = std::chrono::high_resolution_clock::now();
	int random_sum = 0;
	uint64_t seed = 11451419;
	const uint64_t a = 6364136223846793005ULL;
	const uint64_t c = 1;
	for (uint64_t i = 0; i < N; ++i) {
		seed = seed * a + c;
		uint64_t idx = seed % N;
		auto it = m.find(idx);
		int value = (it != m.end()) ? it->second : -1;
		random_sum += value;
	}
	auto end_random_query = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> random_query_duration = end_random_query - start_random_query;
	std::cout << "[std::map] Random query " << N << " times took: " << random_query_duration.count() << " seconds" << std::endl;
	std::cout << "[std::map] Random query sum: " << random_sum << std::endl;
}

void test_performance() {
	// Performance test: insert and query a large number of elements
	const uint64_t N = testCount;
	VectorSkipList<int> skiplist(-1);

	// Insert test
	auto start_insert = std::chrono::high_resolution_clock::now();
	for (uint64_t i = 0; i < N; ++i) {
		skiplist.setElement(i, static_cast<int>(i));
	}
	auto end_insert = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> insert_duration = end_insert - start_insert;
	std::cout << "Insert " << N << " elements took: " << insert_duration.count() << " seconds" << std::endl;

	// Query test
	auto start_query = std::chrono::high_resolution_clock::now();
	int sum = 0;
	for (uint64_t i = 0; i < N; ++i) {
		int value = 0;
		skiplist.getElement(i, value);
		sum += value;
	}
	auto end_query = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> query_duration = end_query - start_query;
	std::cout << "[VSL] Query " << N << " elements took: " << query_duration.count() << " seconds" << std::endl;
	std::cout << "[VSL] Query sum: " << sum << std::endl;

	std::cout << "[VSL] level " << skiplist.getLevel() << std::endl;

	// Random access test (using fast LCG)
	auto start_random_query = std::chrono::high_resolution_clock::now();
	int random_sum = 0;
	uint64_t seed = 11451419;
	const uint64_t a = 6364136223846793005ULL;
	const uint64_t c = 1;
	for (uint64_t i = 0; i < N; ++i) {
		seed = seed * a + c;
		uint64_t idx = seed % N;
		int value = 0;
		skiplist.getElement(idx, value);
		random_sum += value;
	}
	auto end_random_query = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> random_query_duration = end_random_query - start_random_query;
	std::cout << "[VSL] Random query " << N << " times took: " << random_query_duration.count() << " seconds" << std::endl;
	std::cout << "[VSL] Random query sum: " << random_sum << std::endl;

}
// Call in main function
int main() {
	test1();
	test2();
	test3();
	test4();

	test_performance_stdmap();
	test_performance();
	return 0;
}