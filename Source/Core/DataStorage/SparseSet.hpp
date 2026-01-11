#ifndef SPARSESET_HPP
#define SPARSESET_HPP
#include <iostream>

template <typename T>

class SparseSet {
	const uint32_t maxElements;
	const uint32_t nullElement;
	uint32_t numElements = 0;

public:
	///stores value and where in the sparse array it is stored
	struct DenseElement {
		uint32_t sparseID = 0;
		T val;
	};

	uint32_t *sparse;
	DenseElement *dense;

	///Sparse set
	SparseSet(const uint32_t maxElements) : maxElements(maxElements), nullElement(maxElements) {
		sparse = new uint32_t[maxElements];
		//dense is one larger to make room for null element
		dense = new DenseElement[maxElements+1];

		std::fill_n(sparse, maxElements, nullElement);
	}

	~SparseSet() {
		delete[] sparse;
		sparse = nullptr;
		delete[] dense;
		dense = nullptr;
	}

	///gets the number of elements in the dense array
	uint32_t size() const {
		return numElements;
	}

	///gets the capacity of the dense array
	uint32_t capacity() const {
		return maxElements;
	}

	///add an element to the set
	bool add(const uint32_t id, T value) {
		if (id >= maxElements || sparse[id] != nullElement) return false;

		sparse[id] = numElements;
		dense[numElements] = DenseElement(id, value);

		++numElements;

		return true;
	}

	///delete an element from the set
	bool del(const uint32_t id) {
		if (id == nullElement) return false;

		DenseElement end_dense = dense[numElements-1];

		sparse[end_dense.sparseID] = sparse[id];
		dense[sparse[id]] = end_dense;
		sparse[id] = nullElement;

		--numElements;

		return true;
	}

	///set a preexisting element
	bool set(const uint32_t id, T value) {
		if (id >= maxElements || sparse[id] == nullElement) return false;

		dense[sparse[id]] = DenseElement(id, value);
		return true;
	}

	///get an element with an id
	T get(const uint32_t id) {
		if (id >= maxElements) return T();
		auto val = dense[sparse[id]].val;
		return val;
	}

	///get if set contains an id
	bool contains(const uint32_t id) const {
		if (id >= maxElements) return false;

		return sparse[id] != nullElement;
	}

	///get if set is empty
	bool is_empty() const {
		return numElements == 0;
	}

	///clear sparse and dense arrays
	void clear() {
		numElements = 0;
		delete[] sparse;
		delete[] dense;

		sparse = new uint32_t[maxElements];
		dense = new DenseElement[maxElements+1];

		std::fill_n(sparse, maxElements, nullElement);
	}

	///print map of sparse indexes to dense values
	void print() {
		std::cout << "| id| sparse -> dense |\n";
		for (int i = 0; i < maxElements; i++) {
			if (sparse[i] != nullElement) {
				std::cout << i << "| "<< sparse[i];
                if (sparse[i] < numElements)
                    std:: cout << " -> " << dense[sparse[i]].val << " ";
                std::cout << "\n";
			}

		}
	}

	///print sparse array
    void printSparse() const {
		std::cout << "| id| index |\n";
		for (int i = 0; i < maxElements; i++) {
			if (sparse[i] != nullElement) {
				std::cout << i << "| " << sparse[i] << "\n";
			}
		}
	}

	///print dense array
	void printDense() const {
		std::cout << "| index| value |\n";
		for (int i = 0; i < numElements; i++) {
			std::cout << i << "| " << dense[i].val << "\n";
		}
	}
};

#endif //SPARSESET_HPP
