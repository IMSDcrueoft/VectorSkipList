/*
 * MIT License
 * Copyright (c) 2025 OpenPlayLabs (https://github.com/OpenPlayLabs)
 * See LICENSE file in the root directory for full license text.
*/
#pragma once
#include "VectorSkipListAllocator.h"

#define VSL_CAPACITY_INIT 4
#define VSL_CAPACITY_LIMIT 32

/**
 * @brief				A vectorized skip list
 * @tparam T			It shouldn't be a particularly short type, otherwise the node is larger than the data
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
	 * @brief			It's just for storing data, so it's struct and not class
	 * @tparam T		It shouldn't be a particularly short type, otherwise the node is larger than the data
	 * 
	 * In order to compress the memory occupied by a single node, we do not apply STL containers
	 */
	template <typename T>
	struct SkipListNode {
		SkipListNode<T>** nodes = nullptr;	//right = level*2 ,left = level * 2 + 1
		T* elements = nullptr;

		uint64_t baseIndex = 0;				//The array is offset by the index, which is almost unmodified
		uint32_t bitMap = 0;				//use bitMap to manage
		uint8_t node_capacity = 0;			//real capacity = *2
		uint8_t level = 0;					//height
		uint8_t element_capacity = 0;		//capacity should not be too large, otherwise the detached/merged elements will be very expensive to copy
		uint8_t length = 0;					//the index of lastItem + 1

	public:
		SkipListNode<T>* getLeftNode(const uint8_t level) {
			return this->nodes[(level << 1) | 1];
		}

		SkipListNode<T>* getRightNode(const uint8_t level) {
			return this->nodes[(level << 1)];
		}

		void setLeftNode(const uint8_t level, const SkipListNode<T>* node) {
			this->nodes[(level << 1) | 1] = node;
		}

		void setRightNode(const uint8_t level, const SkipListNode<T>* node) {
			this->nodes[(level << 1)] = node;
		}
	};

	SkipListNode<T> head;
	SkipListNode<T> tail;

	uint32_t width = 0;//the node count
	uint32_t level = 0;//the height

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