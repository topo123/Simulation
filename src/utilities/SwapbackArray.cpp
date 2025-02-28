#include "SwapbackArray.hpp"
#include <type_traits>
#include <stdexcept>

template <typename T>
void swapback_array<T>::push_back(T data){
	if(size < capacity)
	{
		array[size] = data;
		size ++;
	}
	else{
		array.push_back(data);
		size ++;
		capacity ++;
	}
}

template <typename T>
void swapback_array<T>::erase(int index){
	if(size == 0)
	{
		return;
	}

	if constexpr(std::is_pointer<T>::value)
	{
		delete array[index];
	}

	array[index] = array[size - 1];
	size --;
}

template <typename T>
T& swapback_array<T>::operator[](size_t index){
	if(index > size)
	{
		throw::std::out_of_range("Index out of range");
	}

	return array[index];
}
