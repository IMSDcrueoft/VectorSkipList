/*
 * MIT License
 * Copyright (c) 2026 IMSDcrueoft (https://github.com/IMSDcrueoft)
 * See LICENSE file in the root directory for full license text.
*/
#include "./src/bvsl.hpp"
#include <cmath>
#include <iostream>
#include <vector>
#include <cassert>
#include <chrono>
#include <map>
#include <random>
#include <algorithm>

constexpr auto testCount = 1000000;
using namespace bvsl;

// Zipf distribution generator for realistic access patterns
class ZipfGenerator {
private:
    std::mt19937_64 rng;
    double alpha;
    std::vector<double> probabilities;

public:
    ZipfGenerator(uint64_t seed, uint64_t n, double alpha = 0.8)
        : rng(seed), alpha(alpha) {
        probabilities.resize(n);
        double sum = 0;
        for (uint64_t i = 1; i <= n; ++i) {
            sum += 1.0 / std::pow(i, alpha);
        }
        double cumulative = 0;
        for (uint64_t i = 0; i < n; ++i) {
            cumulative += 1.0 / std::pow(i + 1, alpha) / sum;
            probabilities[i] = cumulative;
        }
    }

    uint64_t next() {
        double r = std::uniform_real_distribution<double>(0, 1)(rng);
        auto it = std::lower_bound(probabilities.begin(), probabilities.end(), r);
        return std::distance(probabilities.begin(), it);
    }
};

void test1() {
    BitmappedVectorSkipList<uint64_t, double> skiplist(std::nan(""));

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
    BitmappedVectorSkipList<uint64_t, int> skiplist(-1);
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
    BitmappedVectorSkipList<uint64_t, int> skiplist(-999);
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
    BitmappedVectorSkipList<uint64_t, double> skiplist(std::nan(""));
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

// ============= Original Performance Tests =============

void test_performance_stdmap(uint64_t seed) {
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
    const uint64_t a = 6364136223846793005ULL;
    const uint64_t c = 1;
    uint64_t seed_tmp = seed;
    for (uint64_t i = 0; i < N; ++i) {
        seed_tmp = seed_tmp * a + c;
        uint64_t idx = seed_tmp % N;
        auto it = m.find(idx);
        int value = (it != m.end()) ? it->second : -1;
        random_sum += value;
    }
    auto end_random_query = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> random_query_duration = end_random_query - start_random_query;
    std::cout << "[std::map] Random query " << N << " times took: " << random_query_duration.count() << " seconds" << std::endl;
    std::cout << "[std::map] Random query sum: " << random_sum << std::endl;
}

void test_performance(uint64_t seed) {
    const uint64_t N = testCount;
    BitmappedVectorSkipList<uint64_t, int> skiplist(-1);

    auto start_insert = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < N; ++i) {
        skiplist[i] = static_cast<int>(i);
    }
    auto end_insert = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> insert_duration = end_insert - start_insert;
    std::cout << "[vsl] Insert " << N << " elements took: " << insert_duration.count() << " seconds" << std::endl;

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

    auto start_random_query = std::chrono::high_resolution_clock::now();
    int random_sum = 0;
    const uint64_t a = 6364136223846793005ULL;
    const uint64_t c = 1;
    uint64_t seed_tmp = seed;
    for (uint64_t i = 0; i < N; ++i) {
        seed_tmp = seed_tmp * a + c;
        uint64_t idx = seed_tmp % N;
        int value = skiplist[idx];
        random_sum += value;
    }
    auto end_random_query = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> random_query_duration = end_random_query - start_random_query;
    std::cout << "[vsl] Random query " << N << " times took: " << random_query_duration.count() << " seconds" << std::endl;
    std::cout << "[vsl] Random query sum: " << random_sum << std::endl;
}

void test_performance_random_stdmap(uint64_t seedA, uint64_t seedB) {
    const uint64_t N = testCount;
    const uint64_t deleteCount = static_cast<uint64_t>(N * 0.2);
    std::map<uint64_t, int> m;

    auto start_map_insert = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < N; ++i) m[i] = static_cast<int>(i);
    auto end_map_insert = std::chrono::high_resolution_clock::now();
    std::cout << "[std::map] Insert " << N << " : " << (end_map_insert - start_map_insert).count() / 1e9 << "s\n";

    uint64_t seed = seedA, a = 6364136223846793005ULL, c = 1;
    auto start_map_delete = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < deleteCount; ++i) {
        seed = seed * a + c;
        m.erase(seed);
    }
    auto end_map_delete = std::chrono::high_resolution_clock::now();
    std::cout << "[std::map] Delete " << deleteCount << " : " << (end_map_delete - start_map_delete).count() / 1e9 << "s\n";

    seed = seedB;
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

void test_performance_random(uint64_t seedA, uint64_t seedB) {
    const uint64_t N = testCount;
    const uint64_t deleteCount = static_cast<uint64_t>(N * 0.2);
    BitmappedVectorSkipList<uint64_t, int> skiplist(-1);

    auto start_vsl_insert = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < N; ++i) skiplist[i] = static_cast<int>(i);
    auto end_vsl_insert = std::chrono::high_resolution_clock::now();
    std::cout << "[vsl] Insert " << N << " : " << (end_vsl_insert - start_vsl_insert).count() / 1e9 << "s\n";

    uint64_t seed = seedA, a = 6364136223846793005ULL, c = 1;
    auto start_vsl_delete = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < deleteCount; ++i) {
        seed = seed * a + c;
        skiplist.erase(seed);
    }
    auto end_vsl_delete = std::chrono::high_resolution_clock::now();
    std::cout << "[vsl] Delete " << deleteCount << " : " << (end_vsl_delete - start_vsl_delete).count() / 1e9 << "s\n";

    seed = seedB;
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

// ============= New: Sequential Access Performance Tests =============

void test_performance_sequential_stdmap(uint64_t seed) {
    const uint64_t N = testCount;
    std::map<uint64_t, int> m;

    // Sequential insertion
    auto start_insert = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < N; ++i) {
        m[i] = static_cast<int>(i);
    }
    auto end_insert = std::chrono::high_resolution_clock::now();
    std::cout << "[std::map] Sequential Insert " << N << " : "
        << (end_insert - start_insert).count() / 1e9 << "s\n";

    // Sequential query
    auto start_query = std::chrono::high_resolution_clock::now();
    int sum = 0;
    for (uint64_t i = 0; i < N; ++i) {
        sum += m[i];
    }
    auto end_query = std::chrono::high_resolution_clock::now();
    std::cout << "[std::map] Sequential Query " << N << " : "
        << (end_query - start_query).count() / 1e9 << "s\n";
    std::cout << "[std::map] Sum: " << sum << std::endl;
}

void test_performance_sequential_vsl(uint64_t seed) {
    const uint64_t N = testCount;
    BitmappedVectorSkipList<uint64_t, int> skiplist(-1);

    // Sequential insertion
    auto start_insert = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < N; ++i) {
        skiplist[i] = static_cast<int>(i);
    }
    auto end_insert = std::chrono::high_resolution_clock::now();
    std::cout << "[vsl] Sequential Insert " << N << " : "
        << (end_insert - start_insert).count() / 1e9 << "s\n";

    // Sequential query
    auto start_query = std::chrono::high_resolution_clock::now();
    int sum = 0;
    for (uint64_t i = 0; i < N; ++i) {
        sum += skiplist[i];
    }
    auto end_query = std::chrono::high_resolution_clock::now();
    std::cout << "[vsl] Sequential Query " << N << " : "
        << (end_query - start_query).count() / 1e9 << "s\n";
    std::cout << "[vsl] Sum: " << sum << std::endl;
    std::cout << "[vsl] level " << skiplist.getLevel() << std::endl;
}

// ============= New: Zipf Distribution (Realistic Scenario) Tests =============

void test_performance_zipf_stdmap(uint64_t seed) {
    const uint64_t N = testCount;
    std::map<uint64_t, int> m;
    ZipfGenerator zipf(seed, N, 0.8);

    // Pre-insert all data
    for (uint64_t i = 0; i < N; ++i) {
        m[i] = static_cast<int>(i);
    }

    // Zipf distribution query
    auto start_query = std::chrono::high_resolution_clock::now();
    int sum = 0;
    for (uint64_t i = 0; i < N; ++i) {
        uint64_t idx = zipf.next();
        sum += m[idx];
    }
    auto end_query = std::chrono::high_resolution_clock::now();
    std::cout << "[std::map] Zipf Query (α=0.8) " << N << " : "
        << (end_query - start_query).count() / 1e9 << "s\n";
    std::cout << "[std::map] Sum: " << sum << std::endl;

    // Zipf distribution mixed operations (80% query, 20% update)
    ZipfGenerator zipf2(seed + 1, N, 0.8);
    auto start_mixed = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < N; ++i) {
        uint64_t idx = zipf2.next();
        if (i % 5 == 0) {  // 20% write
            m[idx] = static_cast<int>(i);
        }
        else {  // 80% query
            sum += m[idx];
        }
    }
    auto end_mixed = std::chrono::high_resolution_clock::now();
    std::cout << "[std::map] Zipf Mixed (80/20) " << N << " : "
        << (end_mixed - start_mixed).count() / 1e9 << "s\n";
}

void test_performance_zipf_vsl(uint64_t seed) {
    const uint64_t N = testCount;
    BitmappedVectorSkipList<uint64_t, int> skiplist(-1);
    ZipfGenerator zipf(seed, N, 0.8);

    // Pre-insert all data
    for (uint64_t i = 0; i < N; ++i) {
        skiplist[i] = static_cast<int>(i);
    }

    // Zipf distribution query
    auto start_query = std::chrono::high_resolution_clock::now();
    int sum = 0;
    for (uint64_t i = 0; i < N; ++i) {
        uint64_t idx = zipf.next();
        sum += skiplist[idx];
    }
    auto end_query = std::chrono::high_resolution_clock::now();
    std::cout << "[vsl] Zipf Query (α=0.8) " << N << " : "
        << (end_query - start_query).count() / 1e9 << "s\n";
    std::cout << "[vsl] Sum: " << sum << std::endl;
    std::cout << "[vsl] level " << skiplist.getLevel() << std::endl;

    // Zipf distribution mixed operations (80% query, 20% update)
    ZipfGenerator zipf2(seed + 1, N, 0.8);
    auto start_mixed = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < N; ++i) {
        uint64_t idx = zipf2.next();
        if (i % 5 == 0) {  // 20% write
            skiplist[idx] = static_cast<int>(i);
        }
        else {  // 80% query
            sum += skiplist[idx];
        }
    }
    auto end_mixed = std::chrono::high_resolution_clock::now();
    std::cout << "[vsl] Zipf Mixed (80/20) " << N << " : "
        << (end_mixed - start_mixed).count() / 1e9 << "s\n";
}

// ============= New: Range Query Performance Tests =============

void test_performance_range_stdmap() {
    const uint64_t N = testCount;
    std::map<uint64_t, int> m;

    for (uint64_t i = 0; i < N; ++i) {
        m[i] = static_cast<int>(i);
    }

    auto start_range = std::chrono::high_resolution_clock::now();
    int sum = 0;
    for (uint64_t i = 0; i < 1000; ++i) {
        uint64_t start = i * 1000;
        uint64_t end = start + 500;
        auto it = m.lower_bound(start);
        while (it != m.end() && it->first < end) {
            sum += it->second;
            ++it;
        }
    }
    auto end_range = std::chrono::high_resolution_clock::now();
    std::cout << "[std::map] Range queries (500 elements x 1000) : "
        << (end_range - start_range).count() / 1e9 << "s\n";
    std::cout << "[std::map] Range sum: " << sum << std::endl;
}

void test_performance_range_vsl() {
    const uint64_t N = testCount;
    BitmappedVectorSkipList<uint64_t, int> skiplist(-1);

    for (uint64_t i = 0; i < N; ++i) {
        skiplist[i] = static_cast<int>(i);
    }

    auto start_range = std::chrono::high_resolution_clock::now();
    int sum = 0;
    for (uint64_t i = 0; i < 1000; ++i) {
        uint64_t start = i * 1000;
        uint64_t end = start + 500;
        for (uint64_t j = start; j < end; ++j) {
            sum += skiplist[j];
        }
    }
    auto end_range = std::chrono::high_resolution_clock::now();
    std::cout << "[vsl] Range queries (500 elements x 1000) : "
        << (end_range - start_range).count() / 1e9 << "s\n";
    std::cout << "[vsl] Range sum: " << sum << std::endl;
}

// ============= New: Batch Operation Performance Tests =============

void test_performance_batch_stdmap() {
    const uint64_t N = testCount;
    std::map<uint64_t, int> m;

    // Batch insert (1000 per batch)
    auto start_batch = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < N; i += 1000) {
        for (uint64_t j = 0; j < 1000; ++j) {
            m[i + j] = static_cast<int>(i + j);
        }
    }
    auto end_batch = std::chrono::high_resolution_clock::now();
    std::cout << "[std::map] Batch insert (1000/batch) : "
        << (end_batch - start_batch).count() / 1e9 << "s\n";
}

void test_performance_batch_vsl() {
    const uint64_t N = testCount;
    BitmappedVectorSkipList<uint64_t, int> skiplist(-1);

    // Batch insert (1000 per batch)
    auto start_batch = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < N; i += 1000) {
        for (uint64_t j = 0; j < 1000; ++j) {
            skiplist[i + j] = static_cast<int>(i + j);
        }
    }
    auto end_batch = std::chrono::high_resolution_clock::now();
    std::cout << "[vsl] Batch insert (1000/batch) : "
        << (end_batch - start_batch).count() / 1e9 << "s\n";
}

int main() {
    // Basic functionality tests
    test1();
    test2();
    test3();
    test4();

    // Generate random seeds using system time and other sources
    auto now = std::chrono::high_resolution_clock::now();
    uint64_t seedA = now.time_since_epoch().count();
    now = std::chrono::high_resolution_clock::now(); // Get fresh time
    uint64_t seedB = now.time_since_epoch().count() ^ 0x5DEECE66DLL;

    std::cout << "\n========== Original Performance Tests (Random Access) ==========\n";
    test_performance_stdmap(seedA);
    test_performance(seedA);
    test_performance_random_stdmap(seedA, seedB);
    test_performance_random(seedA, seedB);

    std::cout << "\n========== New: Sequential Access Performance Tests ==========\n";
    test_performance_sequential_stdmap(seedA);
    test_performance_sequential_vsl(seedA);

    std::cout << "\n========== New: Zipf Distribution (Realistic Scenario) ==========\n";
    test_performance_zipf_stdmap(seedA);
    test_performance_zipf_vsl(seedA);

    std::cout << "\n========== New: Range Query Performance Tests ==========\n";
    test_performance_range_stdmap();
    test_performance_range_vsl();

    std::cout << "\n========== New: Batch Operation Performance Tests ==========\n";
    test_performance_batch_stdmap();
    test_performance_batch_vsl();

    return 0;
}