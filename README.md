# Bitmapped Vector Skip List (BVSL)

[![MIT License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/IMSDcrueoft/VectorSkipList)

A **memory‑efficient sparse array** that combines skip list indexing with block‑based storage.  
Each block is a **dynamically sized vector** tracked by a bitmap — offering better cache locality and significantly lower memory overhead than traditional tree‑based structures.

## ✨ Features

- **Block‑based storage** – Elements are grouped into blocks (up to 32 slots), reducing pointer overhead and improving spatial locality.
- **Bitmap indexing** – A `uint32_t` bitmap marks which slots inside a block are occupied, enabling O(1) presence checks and compact memory layout.
- **Skip list indexing** – Blocks are organized as a skip list, providing expected O(log n) search, insert, and delete.
- **Adaptive sizing** – Block capacities grow dynamically up to a fixed limit (default 32), balancing memory use and performance.
- **Self‑adjusting levels** – Skip list height automatically scales with the number of blocks, not individual elements.
- **Low memory footprint** – For 1 million elements, BVSL uses ~14 MB vs. ~123 MB for `std::map` (red‑black tree).

## 📊 Performance

| Operation (1 M elements) | `std::map` | `BVSL` |
|--------------------------|------------|------|
| Insert                   | 0.130 s    | 0.034 s |
| Sequential query         | 0.040 s    | 0.027 s |
| Random query             | 0.265 s    | 0.195 s |
| Delete (200 k)           | 0.021 s    | 0.005 s |
| Memory                   | 123 MB     | 14 MB   |

> Mixed insert/delete workloads are still a strength of `std::map`; BVSL excels in sparse array scenarios where deletions are rare.

## 🚀 Quick Start

```cpp
#include "vbsl.hpp"

// Create a sparse array with -1 as "empty" value
vbsl::BitmappedVectorSkipList<int, int> arr(-1);

// Write elements
arr[100] = 42;
arr[500] = 73;
arr[1'000'000] = 999;   // large indices are fine

// Read elements
int val = arr[100];     // 42
val = arr[200];         // -1 (empty)

// Delete
arr.erase(100);
bool exists = arr.has(100);   // false
```

## 🧠 Design Highlights

### Bitmapped Blocks
Each `SkipListNode` manages a small vector (up to 32 slots) using a bitmap:
- `bitMap` – 32‑bit mask indicating occupied slots.
- `elements` – Contiguous array of values.
- `baseIndex` – Starting index of the block (always aligned to 32).

### Dynamic Growth
- Block capacity starts at 4 and doubles when needed, but never exceeds 32.
- This keeps block size small, improves cache efficiency, and avoids huge copy costs.

### Skip List Over Blocks
- Indexing is performed over **blocks**, not individual elements.
- Height is determined by block count: `height = log2(number_of_blocks)`.
- Expected search cost: O(log (n / 32)) – flatter than a traditional skip list.

### Memory‑Aware Pointers
- Left and right pointers for all levels are stored interleaved in a single `nodes` array: `right = level*2`, `left = level*2+1`.
- Reduces allocation overhead and improves pointer locality.

### Thread‑Local Path Cache
- A `thread_local` array caches the search path, eliminating recursion and repeated allocations.

## 🔧 Implementation Details

| Component               | Description |
|-------------------------|-------------|
| `SkipListNode`          | Block containing a bitmap, value vector, and level pointers. |
| `BitmappedVectorSkipList`        | Main container with sentinel head/tail and automatic level adjustment. |
| `Xoroshiro64StarStar`   | Fast RNG for probabilistic level assignment. |
| `bits.hpp`              | Optimized bit operations (popcount, ctz, ceiling powers of two). |

## 📄 License

MIT License – see the [LICENSE](LICENSE) file for details.