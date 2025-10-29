#ifndef SWAPBACKARRAY_H
#define SWAPBACKARRAY_H
#include <vector>

template <typename T>
class swapback_array{
	size_t size;
	size_t capacity;
	std::vector<T> array;

	void push_back(T data);
	void erase(int index);
	T& operator[](size_t index);
};


#endif
