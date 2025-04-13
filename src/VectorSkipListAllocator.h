/*
 * MIT License
 * Copyright (c) 2025 OpenPlayLabs (https://github.com/OpenPlayLabs)
 * See LICENSE file in the root directory for full license text.
 * 
 * 
 * override this file to change new/delete realloc/free
 * this file is only for cpp
*/
#pragma once
#ifdef __cplusplus
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstddef>

template <typename T>
void* VSL_realloc(void* pointer, uint64_t oldCount, uint64_t newCount) {
	if (newCount == 0) {
		if (pointer != nullptr) {
			std::free(pointer);
		}
		return nullptr;
	}

	void* result = std::realloc(pointer, sizeof(T) * newCount);
	if (result == nullptr) {
		fprintf(stderr, "Memory reallocation failed!\n");
		exit(1);
	}
	return result;
}
#endif