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

#include "./bits.hpp"

namespace vsl {
	template<typename T, typename = std::enable_if<std::is_trivial_v<T>&& std::is_standard_layout_v<T>>>
	T* _realloc(T* pointer, size_t oldCount, size_t newSize) {
		if (newSize == 0) {
			if (pointer != nullptr) std::free(pointer);
			return nullptr;
		}

		T* result = static_cast<T*>(std::realloc(pointer, sizeof(T) * newSize));
		if (result == nullptr) {
			std::cerr << "Memory reallocation failed!\n";
			exit(1);
		}

		return result;
	}

	class Xoroshiro64StarStar {
	private:
		uint64_t state;

	public:
		Xoroshiro64StarStar(uint64_t seed = 0x2BD65925FA21F4A3) : state(seed) {}

		void seed(uint64_t seed) {
			state = seed;
		}

		uint64_t next() {
			const uint64_t s0 = state;
			uint64_t s1 = s0 << 55;

			state = s0 ^ (s0 << 23);
			state ^= (state >> 26) ^ (s1 >> 9);

			return ((s0 * 0x9E3779B97F4A7C15) >> 32) * 0xBF58476D1CE4E5B9;
		}
	};

	using bitMap_t = uint32_t;
	constexpr uint64_t capacity_init = 4;
	constexpr uint64_t capacity_limit = sizeof(bitMap_t) * 8;
	constexpr uint64_t index_align = (capacity_limit - 1); // Align to capacity limit

	/**
	 *  When the number of elements in the bottom layer > 2 ^ (current level count), add a new level.
	 *  Conversely, if the number of blocks in the bottom layer < 2 ^ (current level count - 1), remove the topmost level.
	 *  Therefore, even when used as a regular array, it still functions as a skip list with a expected complexity of (log(n / capacityLimit) + 1) to (log(n) + 1).
	 *
	 *  Only inserting sparse and vector too empty creates new nodes
	 *  and when deleted, they are deleted in place instead of splitting
	 *  avoiding the complexity caused by merging and splitting
	 */
	template <typename index_t, typename value_t, typename = std::enable_if<std::is_integral_v<index_t>&& std::is_trivial_v<value_t>&& std::is_standard_layout_v<value_t>>>
	class VectorSkipList {
	protected:
		/**
		 * @brief	It's just for storing data, so it's struct
		 * @tparam value_t	It shouldn't be a particularly short type, otherwise the node is larger than the data
		 *
		 * In order to compress the memory occupied by a single node, we do not apply STL containers
		 */
		struct SkipListNode {
			SkipListNode** nodes = nullptr;		//right = level*2 ,left = level * 2 + 1
			value_t* elements = nullptr;
			index_t baseIndex;					//The array is offset by the index, which is almost unmodified

			bitMap_t bitMap = 0;				//use bitMap to manage

			uint8_t node_capacity;				//real capacity = *2
			uint8_t level;						//height
			uint8_t element_capacity = 0;		//capacity should not be too large, otherwise the detached/merged elements will be very expensive to copy

		public:
			SkipListNode(const index_t baseIndex = 0, const uint8_t level = 0) {
				this->baseIndex = baseIndex;
				this->node_capacity = bits::ceil<uint8_t>(level + 1);
				this->level = level;

				//allocate nodePtrs
				this->nodes = vsl::_realloc(this->nodes, 0, this->node_capacity << 1);
				std::fill_n(this->nodes, this->node_capacity << 1, nullptr);
			}

			~SkipListNode() {
				//no ownership
				vsl::_realloc(this->nodes, this->node_capacity << 1, 0);
				//free elements
				vsl::_realloc(this->elements, this->element_capacity, 0);
			}

			/**
			 * @brief if the node is empty
			 * @return
			 */
			bool isEmpty() {
				return this->bitMap == 0;
			}

			/**
			 * @brief
			 * @param index
			 * @return
			 */
			bool hasElement(const uint8_t index) {
				return (index < this->element_capacity) && bits::get(this->bitMap, index);
			}

			/**
			 * @param index
			 * @param value the reference of value
			 * only when it is valid, the value will be set
			 */
			void getElement(const uint8_t index, value_t& value) const {
				if (index >= this->element_capacity) return;
				//set value
				if (bits::get(this->bitMap, index)) value = this->elements[index];
			}

			/**
			 * @param index
			 * @param value
			 */
			void setElement(const uint8_t index, const value_t& value) {
				if (index >= vsl::capacity_limit) return;

				//grow capacity
				if (index >= this->element_capacity) {
					uint8_t newCapacity = (this->element_capacity != 0) ? (this->element_capacity << 1) : vsl::capacity_init;
					while (index >= newCapacity) newCapacity <<= 1;

					this->elements = vsl::_realloc(this->elements, this->element_capacity, newCapacity);
					this->element_capacity = newCapacity;
				}

				//set value and bitmap
				this->elements[index] = value;
				bits::set_one(this->bitMap, index);
			}

			/**
			 * @brief logic delete
			 * @param index
			 */
			void deleteElement(const uint8_t index) {
				if (index >= this->element_capacity) return;
				bits::set_zero(this->bitMap, index);
			}

			void increaseLevel() {
				if ((this->level + 1) >= this->node_capacity) {
					uint8_t newCapacity = this->node_capacity << 1;
					this->nodes = vsl::_realloc(this->nodes, this->node_capacity << 1, newCapacity << 1);
					this->node_capacity = newCapacity;
				}
				++this->level;
				std::fill_n(this->nodes + (this->level << 1), 2, nullptr);
			}

			void decreaseLevel() {
				--this->level;
			}

			SkipListNode* getLeftNode(const uint8_t level) const {
				return this->nodes[(level << 1) | 1];
			}

			SkipListNode* getRightNode(const uint8_t level) const {
				return this->nodes[(level << 1)];
			}

			void setLeftNode(const uint8_t level, SkipListNode* node) {
				this->nodes[(level << 1) | 1] = node;
			}

			void setRightNode(const uint8_t level, SkipListNode* node) {
				this->nodes[(level << 1)] = node;
			}

			static bool isIndexValid(const uint64_t index) {
				return index < vsl::capacity_limit;
			}
		};

	private:
		//thread local caches,shared among instances,coroutines danger
		static inline thread_local SkipListNode* leftPathNodes[32];

	protected:
		vsl::Xoroshiro64StarStar rng;

		SkipListNode sentryHead;
		SkipListNode sentryTail;

		uint64_t width = 0;//the node count
		int64_t level = 0;//the height

		value_t invalid;//you need an invalid default value

		//check if need add level
		void increaseLevel() {
			// 1. level up sentry
			this->sentryHead.increaseLevel();
			this->sentryTail.increaseLevel();
			++this->level;

			// 2. get nodes witch level == this->level - 1
			SkipListNode* node = this->sentryHead.getRightNode(this->level - 1);
			SkipListNode* left = &this->sentryHead;

			while (node->level < (this->level - 1)) {
				node = node->getRightNode(this->level - 1);
			}

			//at least one node
			bool promoted = false;

			while (node != &this->sentryTail) {
				// 50% percent
				if ((this->rng.next() & 1) || !promoted) {
					node->increaseLevel();
					// connect node
					node->setLeftNode(this->level, left);
					left->setRightNode(this->level, node);
					left = node;

					promoted = true;
				}
				node = node->getRightNode(this->level - 1);
			}

			//connect
			left->setRightNode(this->level, &this->sentryTail);
			this->sentryTail.setLeftNode(this->level, left);
		}

		//check if need sub level
		void decreaseLevel() {
			//level down all node that level == this.level
			SkipListNode* node = &this->sentryHead;

			while (node != nullptr) {
				SkipListNode* right = node->getRightNode(this->level);
				node->decreaseLevel();
				node = right;
			}

			--this->level;
		}

		/**
		 * @brief
		 * @return
		 */
		uint8_t getRandomLevel() {
			//limit level [0-31]
			const auto count = bits::ctz64(this->rng.next()) & 31;
			return (count <= this->level) ? count : this->level;
		}

		/**
		 * @brief
		 * @param index
		 */
		SkipListNode* findLeftNode(const index_t index) const {
			SkipListNode* node = const_cast<SkipListNode*>(&this->sentryHead);
			auto curLevel = this->level;

			while (curLevel >= 0) {
				auto next = node->getRightNode(curLevel);
				// check next node, if it is nullptr, then go down a level
				if (next != &this->sentryTail && next->baseIndex <= index) {
					node = next;
				}
				else {
					leftPathNodes[curLevel] = node;
					--curLevel;
				}
			}

			return leftPathNodes[0];
		}

		/**
		 * @brief
		 * @param index
		 * @return
		 */
		SkipListNode* insertNode(const index_t index) {
			//make node
			const auto level = this->getRandomLevel();
			SkipListNode* newNode = new SkipListNode(index, level);

			//connect
			SkipListNode* left = nullptr, * right = nullptr;

			//sentry level is ennough right now
			for (auto i = 0; i <= level; ++i) {
				left = leftPathNodes[i];
				right = left->getRightNode(i);

				newNode->setLeftNode(i, left);
				left->setRightNode(i, newNode);
				newNode->setRightNode(i, right);
				right->setLeftNode(i, newNode);
			}

			++this->width;
			if (this->width > (1ULL << this->level)) {
				increaseLevel();
			}

			return newNode;
		}

		/**
		 * @brief
		 * @param node
		 */
		void removeNode(SkipListNode* node) {
			SkipListNode* left = nullptr, * right = nullptr;

			for (auto i = 0; i <= node->level; ++i) {
				left = (leftPathNodes[i] != node) ? leftPathNodes[i] : node->getLeftNode(i);
				right = node->getRightNode(i);

				left->setRightNode(i, right);
				right->setLeftNode(i, left);
			}

			delete node;
			--this->width;

			constexpr auto minLevel = 6;
			if (this->level < minLevel || this->width >((1ULL << this->level) - (1ULL << minLevel))) return;
			this->decreaseLevel();
		}
	public:
		/**
		 * @brief
		 * @param invalid invalid value, it should be a default value that is not used in the data
		 */
		VectorSkipList(const value_t& invalid) {
			this->invalid = invalid;

			this->sentryHead.setRightNode(0, &this->sentryTail);
			this->sentryTail.setLeftNode(0, &this->sentryHead);
		}

		/**
		 * @brief
		 * @param seed
		 * @param invalid invalid value, it should be a default value that is not used in the data
		 */
		VectorSkipList(const value_t& invalid, uint64_t seed) {
			this->invalid = invalid;

			this->sentryHead.setRightNode(0, &this->sentryTail);
			this->sentryTail.setLeftNode(0, &this->sentryHead);

			rng.seed(seed);
		}

		~VectorSkipList() {
			// release one by one
			SkipListNode* node = this->sentryHead.getRightNode(0);
			while (node != nullptr && node != &this->sentryTail) {
				SkipListNode* next = node->getRightNode(0);
				delete node;
				node = next;
			}
			// no need to free sentry node
		}

		int64_t getLevel() {
			return this->level;
		}

		/**
		 * @brief
		 * @param index
		 * @return
		 */
		bool has(const index_t index) const {
			if (this->width == 0) return false;

			SkipListNode* node = this->findLeftNode(index);
			// now node is the maximum node with baseIndex <= index
			if (node != &this->sentryHead && node->baseIndex <= index && SkipListNode::isIndexValid(index - node->baseIndex)) {
				return node->hasElement(index - node->baseIndex);
			}
			return false;
		}

		/**
		 * @brief
		 * @param index
		 */
		bool erase(const index_t index) {
			if (this->width == 0) return false;

			SkipListNode* node = this->findLeftNode(index);
			// now node is the maximum node with baseIndex <= index
			if (node != &this->sentryHead && node->baseIndex <= index && SkipListNode::isIndexValid(index - node->baseIndex)) {
				uint8_t offset = static_cast<uint8_t>(index - node->baseIndex);
				if (node->hasElement(offset)) {
					node->deleteElement(offset);

					//remove node
					if (node->isEmpty()) this->removeNode(node);
					return true;
				}
			}
			return false;
		}

		/**
		 * @brief
		 * @param index
		 * @return
		 */
		value_t& operator[](const index_t index) {
			SkipListNode* node = this->findLeftNode(index);

			if (node != &this->sentryHead && node->baseIndex <= index && SkipListNode::isIndexValid(index - node->baseIndex)) {
				uint8_t offset = static_cast<uint8_t>(index - node->baseIndex);
				if (!node->hasElement(offset)) {
					node->setElement(offset, this->invalid);
				}

				return node->elements[offset];
			}

			//align to capacity
			const index_t offsetIndex = index & index_align;

			SkipListNode* newNode = this->insertNode(index - offsetIndex);
			newNode->setElement(offsetIndex, this->invalid);
			return newNode->elements[offsetIndex];
		}

		/**
		 * @brief
		 * @param index
		 * @return
		 */
		const value_t& operator[](const index_t index) const {
			SkipListNode* node = this->findLeftNode(index);
			if (node != &this->sentryHead && node->baseIndex <= index && SkipListNode::isIndexValid(index - node->baseIndex)) {
				uint8_t offset = static_cast<uint8_t>(index - node->baseIndex);

				if (node->hasElement(offset)) {
					return node->elements[offset];
				}
			}

			return this->invalid;
		}
	};
}