/*
 * MIT License
 * Copyright (c) 2025 IMSDcrueoft (https://github.com/IMSDcrueoft)
 * See LICENSE file in the root directory for full license text.
*/
#include "./src/vsl.hpp"
#include <cmath>
#include <iostream>
#include <vector>
#include <cassert>
#include <chrono>
#include <map>

constexpr auto testCount = 1e6;
using namespace vsl;

void test1() {
	VectorSkipList<uint64_t, double> skiplist(std::nan(""));

	// test insert
	for (uint64_t i = 0; i < 10; ++i) {
		skiplist[i] = i * 1.5;
	}

	// test search
	for (uint64_t i = 0; i < 10; ++i) {
		assert(skiplist.has(i));
		double value = skiplist[i];
		assert(value == i * 1.5);
	}

	// find not exist
	assert(!skiplist.has(100));
	double notfound = skiplist[100];
	assert(std::isnan(notfound));

	// test delete
	skiplist.erase(5);
	assert(!skiplist.has(5));
	double deleted = skiplist[5];
	assert(std::isnan(deleted));

	// range test
	skiplist[31] = 99.9;
	assert(skiplist.has(31));
	double edge = skiplist[31];
	assert(edge == 99.9);

	std::cout << "test1 passed!" << std::endl;
}

void test2() {
	// Test large range insertion and sparsity
	VectorSkipList<uint64_t, int> skiplist(-1);
	for (uint64_t i = 0; i < 1000; i += 100) {
		skiplist[i] = static_cast<int>(i * 2);
	}
	for (uint64_t i = 0; i < 1000; ++i) {
		if (i % 100 == 0) {
			assert(skiplist.has(i));
			int value = skiplist[i];
			assert(value == static_cast<int>(i * 2));
		}
		else {
			assert(!skiplist.has(i));
			int value = skiplist[i];
			assert(value == -1);
		}
	}
	std::cout << "test2 passed!" << std::endl;
}

void test3() {
	// Test deletion and reinsertion
	VectorSkipList<uint64_t, int> skiplist(-999);
	skiplist[10] = 42;
	assert(skiplist.has(10));
	int value = skiplist[10];
	assert(value == 42);
	skiplist.erase(10);
	assert(!skiplist.has(10));
	value = skiplist[10];
	assert(value == -999);
	skiplist[10] = 100;
	assert(skiplist.has(10));
	value = skiplist[10];
	assert(value == 100);
	std::cout << "test3 passed!" << std::endl;
}

void test4() {
	// Test boundary conditions
	VectorSkipList<uint64_t, double> skiplist(std::nan(""));
	assert(!skiplist.has(0));
	double value = skiplist[0];
	assert(std::isnan(value));
	skiplist[0] = 3.14;
	assert(skiplist.has(0));
	value = skiplist[0];
	assert(value == 3.14);
	skiplist[UINT64_MAX] = 2.71;
	assert(skiplist.has(UINT64_MAX));
	value = skiplist[UINT64_MAX];
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
	VectorSkipList<uint64_t, int> skiplist(-1);

	// Insert test
	auto start_insert = std::chrono::high_resolution_clock::now();
	for (uint64_t i = 0; i < N; ++i) {
		skiplist[i] = static_cast<int>(i);
	}
	auto end_insert = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> insert_duration = end_insert - start_insert;
	std::cout << "[vsl] Insert " << N << " elements took: " << insert_duration.count() << " seconds" << std::endl;

	// Query test
	auto start_query = std::chrono::high_resolution_clock::now();
	int sum = 0;
	for (uint64_t i = 0; i < N; ++i) {
		int value = skiplist[i];
		sum += value;
	}
	auto end_query = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> query_duration = end_query - start_query;
	std::cout << "[vsl] Query " << N << " elements took: " << query_duration.count() << " seconds" << std::endl;
	std::cout << "[vsl] Query sum: " << sum << std::endl;

	std::cout << "[vsl] level " << skiplist.getLevel() << std::endl;

	// Random access test (using fast LCG)
	auto start_random_query = std::chrono::high_resolution_clock::now();
	int random_sum = 0;
	uint64_t seed = 11451419;
	const uint64_t a = 6364136223846793005ULL;
	const uint64_t c = 1;
	for (uint64_t i = 0; i < N; ++i) {
		seed = seed * a + c;
		uint64_t idx = seed % N;
		int value = skiplist[idx];
		random_sum += value;
	}
	auto end_random_query = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> random_query_duration = end_random_query - start_random_query;
	std::cout << "[vsl] Random query " << N << " times took: " << random_query_duration.count() << " seconds" << std::endl;
	std::cout << "[vsl] Random query sum: " << random_sum << std::endl;

}

void test_performance_random_stdmap() {
	const uint64_t N = testCount;
	const uint64_t deleteCount = static_cast<uint64_t>(N * 0.2);

	// -------- std::map --------
	std::map<uint64_t, int> m;

	auto start_map_insert = std::chrono::high_resolution_clock::now();
	for (uint64_t i = 0; i < N; ++i) m[i] = static_cast<int>(i);
	auto end_map_insert = std::chrono::high_resolution_clock::now();
	std::cout << "[std::map] Insert " << N << " : " << (end_map_insert - start_map_insert).count() / 1e9 << "s\n";

	uint64_t seed = 11451419, a = 6364136223846793005ULL, c = 1;
	auto start_map_delete = std::chrono::high_resolution_clock::now();
	for (uint64_t i = 0; i < deleteCount; ++i) {
		seed = seed * a + c;
		m.erase(seed);
	}
	auto end_map_delete = std::chrono::high_resolution_clock::now();
	std::cout << "[std::map] Delete " << deleteCount << " : " << (end_map_delete - start_map_delete).count() / 1e9 << "s\n";

	seed = 1919810;
	auto start_map_write_delete = std::chrono::high_resolution_clock::now();
	for (uint64_t i = 0; i < N; ++i) {
		seed = seed * a + c;
		uint64_t idx = seed;
		if (i % 2 == 0) m[idx] = static_cast<int>(i);
		else m.erase(idx);
	}
	auto end_map_write_delete = std::chrono::high_resolution_clock::now();
	std::cout << "[std::map] Write/Delete " << N << " : " << (end_map_write_delete - start_map_write_delete).count() / 1e9 << "s\n";
}

void test_performance_random() {
	const uint64_t N = testCount;
	const uint64_t deleteCount = static_cast<uint64_t>(N * 0.2);

	// -------- VectorSkipList --------
	VectorSkipList<uint64_t, int> skiplist(-1);

	auto start_vsl_insert = std::chrono::high_resolution_clock::now();
	for (uint64_t i = 0; i < N; ++i) skiplist[i] = static_cast<int>(i);
	auto end_vsl_insert = std::chrono::high_resolution_clock::now();
	std::cout << "[vsl] Insert " << N << " : " << (end_vsl_insert - start_vsl_insert).count() / 1e9 << "s\n";

	uint64_t seed = 11451419, a = 6364136223846793005ULL, c = 1;
	auto start_vsl_delete = std::chrono::high_resolution_clock::now();
	for (uint64_t i = 0; i < deleteCount; ++i) {
		seed = seed * a + c;
		skiplist.erase(seed);
	}
	auto end_vsl_delete = std::chrono::high_resolution_clock::now();
	std::cout << "[vsl] Delete " << deleteCount << " : " << (end_vsl_delete - start_vsl_delete).count() / 1e9 << "s\n";

	seed = 1919810;
	auto start_vsl_write_delete = std::chrono::high_resolution_clock::now();
	for (uint64_t i = 0; i < N; ++i) {
		seed = seed * a + c;
		uint64_t idx = seed;
		if (i % 2 == 0) skiplist[idx] = static_cast<int>(i);
		else skiplist.erase(idx);
	}
	auto end_vsl_write_delete = std::chrono::high_resolution_clock::now();
	std::cout << "[vsl] Write/Delete " << N << " : " << (end_vsl_write_delete - start_vsl_write_delete).count() / 1e9 << "s\n";
}

// Call in main function
int main() {
	test1();
	test2();
	test3();
	test4();

	test_performance_stdmap();
	test_performance();

	test_performance_random_stdmap();
	test_performance_random();
	return 0;
}