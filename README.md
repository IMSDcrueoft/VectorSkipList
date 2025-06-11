# Vectorized Block Skip List
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/IMSDcrueoft/VectorSkipList)

A block-based skip list with dynamically sized blocks, designed to represent sparse array structures efficiently.

## Features

- **Block-based structure**: Dynamically sized blocks optimize memory usage for sparse data
- **Skip list implementation**: Provides O(log n) time complexity for search, insertion, and deletion
- **Sparse array representation**: Efficiently handles arrays with large gaps between elements
- **Memory optimization**: Compressed node structure minimizes memory overhead
- **Dynamic level adjustment**: Automatically adjusts skip list height based on element count

## License

MIT License

See [LICENSE](LICENSE) file for full license text.

## Implementation Details

### Key Components

1. **SkipListNode**:
   - Manages a block of elements with bitmask tracking
   - Implements dynamic capacity growth
   - Handles left/right connections at multiple levels

2. **VectorSkipList**:
   - Main skip list container class
   - Uses head and tail sentry nodes
   - Implements automatic level adjustment
   - Provides element access and modification

3. **Xoroshiro64StarStar**:
   - Pseudorandom number generator
   - Used for probabilistic level assignment

### Technical Specifications

- Uses bitmaps to track valid elements within blocks
- Dynamically adjusts block capacities (4 to 64 elements)
- Automatically manages skip list levels based on element count
- Provides both logical and physical deletion
- Compared to the red-black tree of std::map, VSL's memory footprint is lower

## Usage

### Basic Operations

```cpp
// Create a skip list with an invalid marker value
VectorSkipList<int> skipList(-1);

// Set elements
skipList[100] = 42;  // Set value 42 at index 100
skipList[500] = 73;  // Set value 73 at index 500

// Get elements
int value;
value = skipList[100];  // value will be 42
value = skipList[200];  // value will be -1 (invalid)

// Delete elements
skipList.erase(100);  // Logical deletion
skipList.has(100);    // check
