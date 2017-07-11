/**********************************************************************
Copyright 2014-2016 The RIVET Developers. See the COPYRIGHT file at
the top-level directory of this distribution.

This file is part of RIVET.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/
/**
 * \class	MapMatrix and related classes
 * \brief	Stores a matrix representing a simplicial map and provides operations for persistence calculations.
 * \author	Matthew L. Wright
 * \date	February 2014
 * 
 * The MapMatrix class stores a matrix representing a simplicial map, such as a boundary map.
 * Such a matrix is a sparse matrix with entries in the two-element field.
 * This implementation is based on that described in the persistent homology survey paper by Edelsbrunner and Harer.
 * Operations are those necessary for persistence computations.
 *
 * Implementation details: A vector contains pointers to the first element (possibly null) in each column; each column is represented by a linked list.
 * Linked lists connecting entries in each row are not implemented.
 * Each entry in the matrix is an instance of the MapMatrixNode class.
 *
 * The MapMatrix_Base class provides the basic structures and functionality; it is the parent class and is not meant to be instantiated directly.
 * The class MapMatrix inherits MapMatrix_Base and stores matrices in a column-sparse format, designed for basic persistence calcuations.
 * The class MapMatrix_Perm inherits MapMatrix, adding functionality for row and column permutations; it is designed for the reduced matrices of vineyard updates.
 * Lastly, the class MapMatrix_RowPriority_Perm inherits MapMatrix_Base and stores matrices in a row-sparse format with row and column permutations; it is designed for the upper-triangular matrices of vineyard updates.
 */

#ifndef __MapMatrix_H__
#define __MapMatrix_H__

class IndexMatrix;

#include "FFp.hpp" //for prime fields

#include <ostream> //for testing
#include <vector>

//base class simply implements features common to all MapMatrices, whether column-priority or row-priority
//written here using column-priority terminology, but this class is meant to be inherited, not instantiated directly
class MapMatrix_Base {
protected:
    MapMatrix_Base(unsigned rows, unsigned cols, FFp& field); //constructor to create matrix of specified size (all entries zero)
    MapMatrix_Base(unsigned size, FFp& field); //constructor to create a (square) identity matrix
    virtual ~MapMatrix_Base(); //destructor

    virtual unsigned width() const; //returns the number of columns in the matrix
    virtual unsigned height() const; //returns the number of rows in the matrix

    virtual void set(unsigned i, unsigned j, element value); //sets (to value) the entry in row i, column j
    virtual void clear(unsigned i, unsigned j); //clears (sets to 0) the entry in row i, column j
    virtual bool is_nonzero(unsigned i, unsigned j); //returns true if entry (i,j) is nonzero, false otherwise
    virtual element get_entry(unsigned i, unsigned j); //returns the value at entry (i,j)

    virtual void add_column(unsigned j, unsigned k); //adds column j to column k; RESULT: column j is not changed, column k contains sum of columns j and k (with arithmetic in field F)
    virtual void add_multiple(element m, unsigned j, unsigned k); //adds m copies of column j to column k; column j is not changed

    class MapMatrixNode { //subclass for the nodes in the MapMatrix
    public:
        MapMatrixNode(unsigned row, element value); //constructor

        unsigned get_row(); //returns the row index
        void set_next(MapMatrixNode* n); //sets the pointer to the next node in the column
        MapMatrixNode* get_next(); //returns a pointer to the next node in the column
        void set_value(element value); //sets the matrix entry (in prime field F_p) corresponding to this node
        element get_value(); //gets the matrix entry (in prime field F_p) corresponding to this node

    private:
        unsigned row_index; //index of matrix row corresponding to this node
        element fpval;      //nonzero matrix entry corresponding to this node (element of prime field F_p)
        MapMatrixNode* next; //pointer to the next entry in the column containing this node
    };

    std::vector<MapMatrixNode*> columns; //vector of pointers to nodes representing columns of the matrix
    unsigned num_rows; //number of rows in the matrix
    FFp& F; //finite field in which the entries in this matrix live
};

//MapMatrix is a column-priority matrix designed for standard persistence calculations
class MapMatrix : public MapMatrix_Base {
public:
    MapMatrix(unsigned rows, unsigned cols, FFp& field); //constructor to create matrix of specified size (all entries zero)
    MapMatrix(unsigned size, FFp& field); //constructor to create a (square) identity matrix
    //MapMatrix(std::initializer_list<std::initializer_list<int>>);
    virtual ~MapMatrix(); //destructor

    friend std::ostream& operator<<(std::ostream&, const MapMatrix&);
    virtual bool operator==(MapMatrix& other);
    unsigned width() const; //returns the number of columns in the matrix
    unsigned height() const; //returns the number of rows in the matrix

    void reserve_cols(unsigned num_cols); //requests that the columns vector have enough capacity for num_cols columns

    virtual void set(unsigned i, unsigned j, element value); //sets (to value) the entry in row i, column j
    virtual bool is_nonzero(unsigned i, unsigned j); //returns true if entry (i,j) is nonzero, false otherwise
    virtual element get_entry(unsigned i, unsigned j); //returns the value at entry (i,j)

    virtual int low(unsigned j); //returns the "low" index in the specified column, or -1 if the column is empty
    bool col_is_empty(unsigned j); //returns true iff column j is empty (for columns that are not empty, this method is faster than low(j))

	void add_multiple(element m, unsigned j, unsigned k); //adds m copies of column j to column k; column j is not changed
    void add_eliminate_low(unsigned j, unsigned k); //adds a multiple of column j to column k, in order to eliminate the low entry in column k; column j is not changed
    void add_eliminate_low(MapMatrix* other, unsigned j, unsigned k); //adds column j from MapMatrix* other to column k of this matrix

    //copies NONZERO columns with indexes in [first, last] from other, appending them to this matrix to the right of all existing columns
    //  all row indexes in copied columns are increased by offset
    void copy_cols_from(MapMatrix* other, int first, int last, unsigned offset);

    //copies columns with indexes in [first, last] from other, inserting them in this matrix with the same column indexes
    void copy_cols_same_indexes(MapMatrix* other, int first, int last);
          
    //removes zero columns from this matrix
    //  ind_old gives grades of columns before zero columns are removed; new grade info stored in ind_new
    void remove_zero_cols(IndexMatrix* ind_old, IndexMatrix* ind_new);
};

//MapMatrix with row/column permutations and low array, designed for "vineyard updates"
class MapMatrix_RowPriority_Perm; //forward declaration
class MapMatrix_Perm : public MapMatrix {
public:
    MapMatrix_Perm(unsigned rows, unsigned cols, FFp& field);
    MapMatrix_Perm(unsigned size, FFp& field);
    MapMatrix_Perm(const MapMatrix_Perm& other); //copy constructor
    ~MapMatrix_Perm();

    void set(unsigned i, unsigned j, element value); //sets (to value) the entry in row i, column j
    bool is_nonzero(unsigned i, unsigned j); //returns true if entry (i,j) is nonzero, false otherwise
    element get_entry(unsigned i, unsigned j); //returns the value at entry (i,j)

    //reduces this matrix, fills the low array, and returns the corresponding upper-triangular matrix for the RU-decomposition
    //  NOTE: only to be called before any rows are swapped!
    MapMatrix_RowPriority_Perm* decompose_RU(); 

    int low(unsigned j); //returns the "low" index in the specified column, or -1 if the column is empty
    int find_low(unsigned l); //returns the index of the column with low l, or -1 if there is no such column

    void swap_rows(unsigned i, bool update_lows); //transposes rows i and i+1, optionally updates low array
    void swap_columns(unsigned j, bool update_lows); //transposes columns j and j+1, optionally updates low array

    //clears the matrix, then rebuilds it from reference with columns permuted according to col_order
    void rebuild(MapMatrix_Perm* reference, std::vector<unsigned>& col_order); 
    
    //clears the matrix, then rebuilds it from reference with columns permuted according to col_order and rows permuted according to row_order
    void rebuild(MapMatrix_Perm* reference, std::vector<unsigned>& col_order, std::vector<unsigned>& row_order); 

    ///FOR TESTING ONLY
    virtual void print(); //prints the matrix to standard output (for testing)
    void check_lows(); //checks for inconsistencies in low arrays

protected:
    std::vector<unsigned> perm; //permutation vector
    std::vector<unsigned> mrep; //inverse permutation vector
    std::vector<int> low_by_row; //stores index of column with each low number, or -1 if no such column exists -- NOTE: only accurate after decompose_RU() is called
    std::vector<int> low_by_col; //stores the low number for each column, or -1 if the column is empty -- NOTE: only accurate after decompose_RU() is called
};

//MapMatrix stored in row-priority format, with row/column permutations, designed for upper-triangular matrices in vineyard updates
class MapMatrix_RowPriority_Perm : public MapMatrix_Base {
public:
    MapMatrix_RowPriority_Perm(unsigned size, FFp& field); //constructs the identity matrix of specified size
    MapMatrix_RowPriority_Perm(const MapMatrix_RowPriority_Perm& other); //copy constructor
    ~MapMatrix_RowPriority_Perm();

    unsigned width() const; //returns the number of columns in the matrix
    unsigned height() const; //returns the number of rows in the matrix

    void set(unsigned i, unsigned j, element value); //sets (to value) the entry in row i, column j
    void clear(unsigned i, unsigned j); //clears (sets to 0) the entry in row i, column j
    bool is_nonzero(unsigned i, unsigned j); //returns true if entry (i,j) is nonzero, false otherwise
    element get_entry(unsigned i, unsigned j); //returns the value at entry (i,j)

    void add_multiple_row(element m, unsigned j, unsigned k); //adds m copies of row j to row k; row j is not changed

    void swap_rows(unsigned i); //transposes rows i and i+1
    void swap_columns(unsigned j); //transposes columns j and j+1

    ///FOR TESTING ONLY
    void print(); //prints the matrix to qDebug() for testing
    void print_perm(); //prints the permutation vectors to qDebug() for testing

protected:
    std::vector<unsigned> perm; //permutation vector
    std::vector<unsigned> mrep; //inverse permutation vector
};

#endif // __MapMatrix_H__


/*** TESTING MATRICES WITH PRIME FIELD ENTRIES ***/

#include <cstdlib> //for random numbers
#include <iostream>

int main(int argc, char* argv[])
{
    int p = 11;
    unsigned rows = 6;
    unsigned cols = 8;

    FFp F = FFp(p);
    MapMatrix M = MapMatrix(rows, cols, F);
    std::cout << "Working in the field of order " << p << "\n";

    //fill matrix
    std::srand(100);
    for(unsigned r = 0; r < M.height(); r++) {
        for(unsigned c = 0; c < M.width(); c++) {
            M.set(r, c, std::rand() % p);
        }
    }
    std::cout << "Matrix:\n";
    std::cout << M;

    //do some operations
    // std::cout << "Testing column operations:\n";
    // for(unsigned c=1; c < M.width(); c++) {
    //     std::cout << "  Adding " << F.to_element(c) << " times column 1 to column " << c << "\n";
    //     M.add_multiple(F.to_element(c), 0, c);
    // }
    // std::cout << M;

    //reduce the matrix
    std::vector<int> lows(M.width(), -1); //low array
    for (unsigned j = 0; j < M.width(); j++) {
        //while column c is nonempty and its low number is found in the low array, do column operations
        // std::cout << "column " << j << ": low row is " << M.low(j) << "; nonempty: " << (!M.col_is_empty(j)) << "\n";
        while ( !M.col_is_empty(j) && lows[M.low(j)] >= 0) {
        	//std::cout << "checkpoint\n";
            int l = M.low(j);
            int c = lows[l];

            //add a multiple of column c to column j, to clear the low entry in column j
            element w = M.get_entry(l,c);   //low element in column c
            element a = M.get_entry(l,j);   //low element in column j
            element m = F.mul(F.inv(w), F.neg(a)); //satisfies w*m + a = 0 in field F
            // std::cout << "  Adding " << m << " times column " << c << " to column " << j << "\n";
            M.add_multiple(m, c, j);        //perform row operation (add m copies of column c to column j)

            // std::cout << M;
            //M.add_eliminate_low(c, j);
        }

        if (!M.col_is_empty(j)) { //then column is still nonempty, so update lows
            lows[M.low(j)] = j;
            // std::cout << "stored " << j << " in lows[" << M.low(j) << "]\n";
        }

        std::cout << "Finished with column " << j << ":\n" << M;
    }


    return 0;
}
