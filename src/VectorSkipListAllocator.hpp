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
#include <cstdio>
#include <cstdlib>
#include <cstddef>

void* VSL_realloc(void* pointer, size_t oldSize, size_t newSize) {
	if (oldSize == 0) {
		if (pointer != nullptr) {
			std::free(pointer);
		}
		return nullptr;
	}

	void* result = std::realloc(pointer, newSize);
	if (result == nullptr) {
		fprintf(stderr, "Memory reallocation failed!\n");
		exit(1);
	}
	return result;
}