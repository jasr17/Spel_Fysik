#pragma once
template <typename T>
class Array
{
private:
	int capacity = 0;
	int size = 0;
	T * elements = nullptr;

	void free();
	void quickSortSub(int left, int right);
public:
	/*Returns the position/index of the item in the array. -1 if not found*/
	int find(const T& item);
	bool isMaxCapacity() const;
	/*Returns total bytes reserved for array (itemBytes*size)*/
	int byteSize() const;
	/*Returns total bytes reserved for array (itemBytes*capacity)*/
	int byteCapacity() const;
	/*Resets memory,size and capacity to zero*/
	void reset();
	/*Increase capacity but not size!*/
	void appendCapacity(int n);
	/*sets size and capacity to a new size, good for predefined sizes*/
	void resize(int _size);
	/*Set every element to same*/
	void fill(T item);
	/*increases capacity if size is equal capacity*/
	bool appendIfNecessary(int n);
	/*Sets capacity to size*/
	void clampCapacity();
	/*Adds an element and increases capacity if necessery*/
	void add(const T& element);
	/*removes element and decrease in size, doesnt change in capacity!*/
	void remove(int index);
	void set(int index,const T& item);
	/*Sorts element from small to large*/
	void quickSort();
	/*Returns size of array. OBSERVE! array might have a capacity for more*/
	int length() const;
	/*Return last element*/
	T& getLast() const;
	T& get(int index) const;
	/*Return pointer to an element*/
	T* getPointer(int n) const;
	/*Returns the array pointer. USE WITH CAUTION!*/
	T* getArrayPointer() const;
	Array operator=(const Array& arr);
	T& operator[](unsigned int index) const;
	Array(const Array& arr);
	Array(int size = 0);
	~Array();
};

template<typename T>
inline void Array<T>::free()
{
	delete[] elements;
	elements = nullptr;
}

template<typename T>
inline int Array<T>::find(const T & item)
{
	int pos = -1;
	for (int i = 0; i < size && pos == -1; i++)
	{
		if (elements[i] == item)
			pos = i;
	}

	return pos;
}

template<typename T>
inline bool Array<T>::isMaxCapacity() const
{
	return (size == capacity);
}

template<typename T>
inline int Array<T>::byteSize() const
{
	return sizeof(T)*size;
}

template<typename T>
inline int Array<T>::byteCapacity() const
{
	return sizeof(T)*capacity;
}

template<typename T>
inline void Array<T>::reset()
{
	free();
	capacity = 0;
	size = 0;
}

template<typename T>
inline void Array<T>::appendCapacity(int index)
{
	if (index > 0) {
		T* newitems = new T[capacity + index];
		for (int i = 0; i < size; i++)
		{
			newitems[i] = elements[i];
		}
		free();
		elements = newitems;
		capacity += index;
	}
}

template<typename T>
inline void Array<T>::resize(int _size)
{
	if (_size > 0) {
		T* newI = new T[_size];
		int m = (size > _size ? _size : size);
		for (int i = 0; i < m; i++)
		{
			newI[i] = elements[i];
		}
		free();
		elements = newI;
		size = _size;
		capacity = size;
	}
	else {
		reset();
	}
}

template<typename T>
inline void Array<T>::fill(T item)
{
	for (int i = 0; i < size; i++)
	{
		elements[i] = item;
	}
}

template<typename T>
inline bool Array<T>::appendIfNecessary(int n)
{
	if (isMaxCapacity()) {
		appendCapacity(n);
		return true;
	}
	return false;
}

template<typename T>
inline void Array<T>::clampCapacity()
{
	resize(size);
}

template<typename T>
inline void Array<T>::quickSortSub(int left, int right)
{
	int i = left, j = right;
	T tmp;
	T pivot = elements[(left + right) / 2];
	/* partition */
	while (i <= j) {
		while (elements[i] < pivot)
			i++;
		while (elements[j] > pivot)
			j--;
		if (i <= j) {
			tmp = elements[i];
			elements[i] = elements[j];
			elements[j] = tmp;
			i++;
			j--;
		}
	};
	/* recursion */
	if (left < j)
		quickSortSub(left, j);
	if (i < right)
		quickSortSub(i, right);
}

template<typename T>
inline void Array<T>::add(const T & item)
{
	if(isMaxCapacity())appendCapacity(1);
	elements[size] = item;
	size++;
}

template<typename T>
inline void Array<T>::remove(int index)
{
	for (int i = index; i < size-1; i++)
	{
		elements[i] = elements[i + 1];
	}
	size--;
}

template<typename T>
inline void Array<T>::set(int index, const T & item)
{
	elements[index] = item;
}

template<typename T>
inline void Array<T>::quickSort()
{
	if(size > 0)
		quickSortSub(0,size-1);
}

template<typename T>
inline int Array<T>::length() const
{
	return size;
}

template<typename T>
inline T & Array<T>::getLast() const
{
	return elements[size - 1];
}

template<typename T>
inline T& Array<T>::get(int index) const
{
	return elements[index];
}

template<typename T>
inline T * Array<T>::getPointer(int index) const
{
	return &elements[index];
}

template<typename T>
inline T * Array<T>::getArrayPointer() const
{
	return elements;
}

template<typename T>
inline Array<T> Array<T>::operator=(const Array<T> & arr)
{
	free();
	capacity = arr.capacity;
	size = arr.size;
	elements = new T[size];
	for (int i = 0; i < size; i++)
	{
		elements[i] = arr.elements[i];
	}
	return *this;
}

template<typename T>
inline T & Array<T>::operator[](unsigned int index) const
{
	return elements[index];
}

template<typename T>
inline Array<T>::Array(const Array<T> & arr)
{
	capacity = arr.capacity;
	size = arr.size;
	elements = new T[size];
	for (int i = 0; i < size; i++)
	{
		elements[i] = arr.elements[i];
	}
}

template<typename T>
inline Array<T>::Array(int size)
{
	if (size > 0) {
		resize(size);
	}
}

template<typename T>
inline Array<T>::~Array()
{
	free();
}
