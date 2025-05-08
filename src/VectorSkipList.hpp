/*
 * MIT License
 * Copyright (c) 2025 IMSDcrueoft (https://github.com/IMSDcrueoft)
 * See LICENSE file in the root directory for full license text.
*/
#pragma once
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <iostream>

#define VSL_CAPACITY_INIT 4
#define VSL_CAPACITY_LIMIT 32

template<typename Element>
Element* VSL_realloc(Element* pointer, size_t oldCount, size_t newSize) {
	if (oldCount == 0) {
		if (pointer != nullptr) {
			std::free(pointer);
		}
		return nullptr;
	}

	Element* result = std::realloc(pointer, sizeof(Element) * newSize);
	if (result != nullptr) {
		std::cerr << "Memory reallocation failed!\n";
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

			//allocate nodePtrs
			this->nodes = VSL_realloc(this->nodes, 0, this->node_capacity << 1);
			std::fill_n(this->nodes, this->node_capacity << 1, nullptr);
		}

		~SkipListNode() {
			//no ownership
			VSL_realloc(this->nodes, this->node_capacity << 1, 0);
			//free elements
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

		/**
		 * @param index 
		 * @param value 
		 * @param capacityLimit the capacityLimit,the struct itself don't know
		 * @return 
		 */
		bool setElement(const uint8_t index, const T value, const uint8_t capacityLimit) {
			if (index > capacityLimit || this->element_capacity > capacityLimit) return false;

			//grow capacity
			if (index >= this->element_capacity) {
				uint8_t newCapacity = std::min((this->element_capacity != 0) ? (this->element_capacity << 1) : VSL_CAPACITY_INIT, capacityLimit);
				this->elements = VSL_realloc(this->elements, this->element_capacity, newCapacity);
				std::fill_n(this->elements, this->element_capacity, 0);
				this->element_capacity = newCapacity;
			}

			//set value and bitmap
			this->elements[index] = value;
			this->bitMap |= (1 << index);
			this->length = std::max(this->length, index + 1);

			return true;
		}

		/**
		 * @param index 
		 * @param value the reference of value
		 * @return if the value is valid
		 */
		bool getElement(const uint8_t index, T& value) const {
			if (index >= this->element_capacity) return false;
			//set value
			value = this->elements[index];
			return (this->bitMap & (1 << index));
		}

		void increaseLevel() {
			if ((this->level + 1) > this->node_capacity) {
				this->node_capacity <<= 1;
				this->nodes = VSL_realloc(this->nodes, this->node_capacity, this->node_capacity << 1);
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

	T invalid;//you need an invalid default value

	uint64_t width = 0;//the node count
	uint64_t level = 0;//the height

	//you must init,because 0 is invalid seed to xorshift
	uint64_t randState = 0;
	uint64_t xorshift64() {
		this->randState ^= this->randState << 13;
		this->randState ^= this->randState >> 7;
		this->randState ^= this->randState << 17;
		return this->randState;
	}

	SkipListNode<T>* findSkipListNode(const uint64_t index) const {
		if (this->width > 0) {
		}

		return nullptr;
	}
public:
	VectorSkipList(uint64_t seed, const T& invalid) {
		this->invalid = invalid;

		if (seed == 0) {
			std::cerr << "Seed must be non-zero\n";
			seed = UINT64_MAX;
		}
		this->randState = seed;
	}

	~VectorSkipList() {
	}

	bool hasElement(const uint64_t index) const {
		return this->findSkipListNode(index) != nullptr;
	}

	T& getElement(const uint64_t index) const {
		return this->invalid;
	}

	void setElement(const uint64_t index, const T& value) {

	}
};