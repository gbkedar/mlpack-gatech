/**
 * @author Parikshit Ram (pram@cc.gatech.edu)
 * @file math_functions.h
 *
 * This file has certain functions that find the
 * highest or lowest element in an array or
 * in a row of a matrix and returns them
 *
 */
#include <mlpack/core.h>

/**
 * Finds the index of the minimum element
 * in each row of a matrix
 *
 * Example use:
 * @code
 * Matrix& mat;
 * size_t indices[mat.n_rows()];
 *
 * ...
 *
 * min_element(mat, indices);
 * @endcode
 */

void min_element(Matrix& element, size_t *indices) {

	size_t last = element.n_cols() - 1;
	size_t first, lowest;
	size_t i;

	for (i = 0; i < element.n_rows(); i++) {

		first = lowest = 0;
		if (first == last) {
			indices[i] = last;
		}
		while (++first <= last) {
			if (element.get(i, first) < element.get(i, lowest)) {
				lowest = first;
			}
		}
		indices[i] = lowest;
	}
	return;
}

/**
 * Returns the index of the maximum element
 * in a float array 'array' of length 'length'.
 *
 * Example use:
 * @code
 * size_t length, index;
 * float array[length];
 * ...
 * index = max_element_index(array, length);
 * @endcode
 */
int max_element_index(float *array, int length) {

	int last = length - 1;
	int first = 0;
       	int highest = 0;

	if (first == last) {
		return last;
	}
	while (++first <= last) {
		if (array[first] > array[highest]) {
			highest = first;
		}
	}
	return highest;
}

/**
 * Finds the index of the maximum element in an arraylist
 * of floats
 *
 * Example use:
 * @code
 * size_t index;
 * ArrayList<float> array;
 * ...
 * index = max_element_index(array);
 * @endcode
 */
int max_element_index(ArrayList<float>& array){

        int last = array.size() - 1;
        int first = 0;
       	int highest = 0;

	if (first == last) {
                return last;
        }
        while (++first <= last) {
                if (array[first] > array[highest]) {
                        highest = first;
                }
	}
	return highest;
}

/**
 * Returns the index of the maximum element in
 * an arraylist of doubles
 *
 * Example use:
 * @code
 * size_t index;
 * ArrayList<double> array;
 * ...
 * index = max_element_index(array);
 * @endcode
 */
int max_element_index(ArrayList<double>& array) {

        int last = array.size() - 1;
        int first = 0;
       	int highest = 0;

	if (first == last) {
                return last;
        }
        while (++first <= last) {
                if(array[first] > array[highest]) {
                        highest = first;
                }
	}
	return highest;
}

