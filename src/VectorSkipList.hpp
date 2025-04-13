/*
 * MIT License
 * Copyright (c) 2025 IMSDcrueoft (https://github.com/IMSDcrueoft)
 * See LICENSE file in the root directory for full license text.
*/
#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <algorithm>

#define VSL_CAPACITY_INIT 4
#define VSL_CAPACITY_LIMIT 32

template<typename T>
T* VSL_realloc(T* pointer, uint64_t oldSize, uint64_t newSize) {
	if (oldSize == 0) {
		if (pointer != nullptr) {
			std::free(pointer);
		}
		return nullptr;
	}

	T* result = std::realloc(pointer, sizeof(T) * newSize);
	if (result != nullptr) {
		fprintf(stderr, "Memory reallocation failed!\n");
		exit(1);
	}
	return result;
}

/**
 * @tparam T	It shouldn't be a particularly short type, otherwise the node is larger than the data
 * 
 *  When the number of elements in the bottom layer > 2 ^ (current level count), add a new level.
 *  Conversely, if the number of blocks in the bottom layer < 2 ^ (current level count - 1), remove the topmost level.
 *  Therefore, even when used as a regular array, it still functions as a skip list with a expected complexity of (log(n / capacityLimit) + 1) to (log(n) + 1).
 * 
 *  Only inserting sparse and vector too empty creates new nodes
 *  and when deleted, they are deleted in place instead of splitting
 *  avoiding the complexity caused by merging and splitting
 */
template <typename T>
class VectorSkipList {
protected:
	/**
	 * @brief	It's just for storing data, so it's struct
	 * @tparam T	It shouldn't be a particularly short type, otherwise the node is larger than the data
	 * 
	 * In order to compress the memory occupied by a single node, we do not apply STL containers
	 */
	template <typename T>
	struct SkipListNode {
		SkipListNode<T>** nodes = nullptr;	//right = level*2 ,left = level * 2 + 1
		T* elements = nullptr;

		uint64_t baseIndex;				//The array is offset by the index, which is almost unmodified
		uint32_t bitMap;				//use bitMap to manage
		uint8_t node_capacity;			//real capacity = *2
		uint8_t level;					//height
		uint8_t element_capacity;		//capacity should not be too large, otherwise the detached/merged elements will be very expensive to copy
		uint8_t length;					//the index of last valid item + 1

	public:
		SkipListNode(const uint64_t baseIndex = 0, const uint8_t level = 1) {
			this->nodes = nullptr;
			this->elements = nullptr;
			this->baseIndex = baseIndex;
			this->bitMap = 0;
			this->node_capacity = level;
			this->level = level;
			this->element_capacity = 0;//Sentinel nodes do not store data
			this->length = 0;

			this->nodes = VSL_realloc(this->nodes, 0, this->node_capacity << 1);
			std::fill_n(this->nodes, this->node_capacity << 1, nullptr);
		}

		~SkipListNode() {
			//no ownership
			VSL_realloc(this->nodes, this->node_capacity << 1, 0);
			VSL_realloc(this->elements, this->element_capacity, 0);

			this->nodes = nullptr;
			this->elements = nullptr;
			this->baseIndex = 0;
			this->bitMap = 0;
			this->node_capacity = 0;
			this->level = 0;
			this->element_capacity = 0;
			this->length = 0;
		}

		bool setElement(const uint8_t index, const T value, const uint8_t capacityLimit) {
			if (index > capacityLimit || this->element_capacity > capacityLimit) return false;

			if (index >= this->element_capacity) {
				uint8_t newCapacity = std::min((this->element_capacity != 0) ? (this->element_capacity << 1) : VSL_CAPACITY_INIT, capacityLimit);
				this->elements = VSL_realloc(this->elements, this->element_capacity, newCapacity);
				std::fill_n(this->elements, this->element_capacity, 0);
				this->element_capacity = newCapacity;
			}

			this->elements[index] = value;
			this->bitMap |= (1 << index);
			this->length = std::max(this->length, index + 1);

			return true;
		}

		bool getElement(const uint8_t index, T& value) const {
			if (index >= this->element_capacity) return false;

			if (this->bitMap & (1 << index)) {
				value = this->elements[index];
				return true;
			}
			else {
				return false;
			}
		}

		void increaseLevel() {
			if ((this->level + 1) > this->node_capacity) {
				this->node_capacity <<= 1;
				this->nodes = VSL_realloc(this->nodes, 0, this->node_capacity << 1);
				std::fill_n(this->nodes + this->node_capacity, this->node_capacity, nullptr);
			}
			++this->level;
		}

		void decreaseLevel() {
			--this->level;
			std::fill_n(this->nodes + (this->level << 1), 2, nullptr);
		}

		SkipListNode<T>* getLeftNode(const uint8_t level) const {
			return this->nodes[(level << 1) | 1];
		}

		SkipListNode<T>* getRightNode(const uint8_t level) const {
			return this->nodes[(level << 1)];
		}

		void setLeftNode(const uint8_t level, const SkipListNode<T>* node) {
			this->nodes[(level << 1) | 1] = node;
		}

		void setRightNode(const uint8_t level, const SkipListNode<T>* node) {
			this->nodes[(level << 1)] = node;
		}
	};

	SkipListNode<T> sentryHead;
	SkipListNode<T> sentryTail;

	uint64_t width = 0;//the node count
	uint64_t level = 0;//the height

	T& getItemByIndex(uint64_t index) {
		//not implete yet

		T val = 0;
		return val;
	}

public:
	VectorSkipList() {
	}

	~VectorSkipList() {
	}

	T& operator[] (uint64_t index) {
		return this->getItemByIndex(index);
	}

	const T& operator[] (uint64_t index) const {
		return this->getItemByIndex(index);
	}
};