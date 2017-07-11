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
/* map matrix class
 * stores a matrix representing a simplicial map
 */

#include "map_matrix.h"
#include "index_matrix.h"
#include "bool_array.h"
#include "../debug.h" //CHECK THIS!
#include <numeric> //for std::accumulate
#include <stdexcept> //for error-checking and debugging

/********** implementation of base class MapMatrix_Base **********/

//implementation of subclass MapMatrixNode
MapMatrix_Base::MapMatrixNode::MapMatrixNode(unsigned row, element value)
    : row_index(row)
    , fpval(value)
    , next(NULL)
{
}

unsigned MapMatrix_Base::MapMatrixNode::get_row()
{
    return row_index;
}

void MapMatrix_Base::MapMatrixNode::set_next(MapMatrixNode* n)
{
    next = n;
}

MapMatrix_Base::MapMatrixNode* MapMatrix_Base::MapMatrixNode::get_next()
{
    return next;
}

void MapMatrix_Base::MapMatrixNode::set_value(element value)
{
    fpval = value;
}

element MapMatrix_Base::MapMatrixNode::get_value()
{
    return fpval;
}

//constructor to create matrix of specified size (all entries zero)
MapMatrix_Base::MapMatrix_Base(unsigned rows, unsigned cols, FFp& field)
    : columns(cols)
    , num_rows(rows)
    , F(field)
{
}

//constructor to create a (square) identity matrix
MapMatrix_Base::MapMatrix_Base(unsigned size, FFp& field)
    : columns(size)
    , num_rows(size)
    , F(field)
{
    for (unsigned i = 0; i < size; i++) {
        MapMatrixNode* newnode = new MapMatrixNode(i, 1);
        columns[i] = newnode;
    }
}

//destructor: deletes all entries in this matrix
MapMatrix_Base::~MapMatrix_Base()
{
    for (unsigned j = 0; j < columns.size(); j++) {
        MapMatrixNode* current = columns[j];
        while (current != NULL) {
            MapMatrixNode* next = current->get_next();
            delete current;
            current = next;
        }
    }
}

//returns the number of columns in the matrix
unsigned MapMatrix_Base::width() const
{
    return columns.size();
}

//returns the number of rows in the matrix
unsigned MapMatrix_Base::height() const
{
    return num_rows;
}

//sets (to value) the entry in row i, column j
void MapMatrix_Base::set(unsigned i, unsigned j, element value)
{
    //set a value of 0 by clearing the entry
    if(value == 0) {
        clear(i, j);
        return;
    }

    //make sure this operation is valid
    if (columns.size() <= j)
        throw std::runtime_error("MapMatrix_Base::set(): attempting to set column past end of matrix");
    if (num_rows <= i)
        throw std::runtime_error("MapMatrix_Base::set(): attempting to set row past end of matrix");

    //if the column is empty, then create a node
    if (columns[j] == NULL) {
        columns[j] = new MapMatrixNode(i, value);
        return;
    }

    //if we get here, the column is not empty
    MapMatrixNode* current = columns[j];

    //see if node that we would insert already exists in the first position
    if (current->get_row() == i) {
        current->set_value(value);
        return;
    }

    //see if we need to insert a new node into the first position
    if (current->get_row() < i) {
        MapMatrixNode* newnode = new MapMatrixNode(i, value);
        newnode->set_next(current);
        columns[j] = newnode;
        return;
    }

    //if we get here, then we must traverse the nodes to find the proper insertion point
    while (current->get_next() != NULL) {
        MapMatrixNode* next = current->get_next();

        if (next->get_row() == i) //then node aready exists
        {
            next->set_value(value);
            return;
        }

        if (next->get_row() < i) //then insert new node between current and next
        {
            MapMatrixNode* newnode = new MapMatrixNode(i, value);
            newnode->set_next(next);
            current->set_next(newnode);

            return;
        }

        //otherwise, move one step
        current = next;
    }

    //if we get here, then append a new node to the end of the list
    MapMatrixNode* newnode = new MapMatrixNode(i, value);
    current->set_next(newnode);
} //end set()

//clears (sets to 0) the entry in row i, column j
void MapMatrix_Base::clear(unsigned i, unsigned j)
{
    //make sure this entry is valid
    if (columns.size() <= j)
        throw std::runtime_error("MapMatrix_Base::clear(): attempting to clear entry in a column past end of matrix");
    if (num_rows <= i)
        throw std::runtime_error("MapMatrix_Base::clear(): attempting to clear entry in a row past end of matrix");

    //if column is empty, then do nothing
    if (columns[j] == NULL)
        return;

    //column is not empty, so get initial node pointer in the column
    MapMatrixNode* current = columns[j];

    //see if the first node is the one we want
    if (current->get_row() < i)
        return; //because the entry in row i must be zero (row entries are sorted in decreasing order)

    if (current->get_row() == i) {
        columns[j] = current->get_next();
        delete current;
        return;
    }

    //traverse the nodes in this column after the first node
    while (current->get_next() != NULL) {
        MapMatrixNode* next = current->get_next();

        if (next->get_row() < i) //then the entry in row i must be zero (row entries are sorted in decreasing order)
            return;

        if (next->get_row() == i) //then we found the row we wanted
        {
            current->set_next(next->get_next());
            delete next;
            return;
        }

        //if we are still looking, then get the next node
        current = next;
    }
} //end clear()

//returns true if entry (i,j) is nonzero, false otherwise
bool MapMatrix_Base::is_nonzero(unsigned i, unsigned j)
{
    //make sure this entry is valid
    if (columns.size() <= j)
        throw std::runtime_error("MapMatrix_Base::is_nonzero(): attempting to check entry in a column past end of matrix");
    if (num_rows <= i)
        throw std::runtime_error("MapMatrix_Base::is_nonzero(): attempting to check entry in a row past end of matrix");

    //get initial node pointer
    MapMatrixNode* np = columns[j];

    //loop while there is another node to check
    while (np != NULL) {
        if (np->get_row() < i) //then we won't find row i because row entrys are sorted in descending order
            return false;

        if (np->get_row() == i) //then we found the row we wanted
            return true;

        //if we are still looking, then get the next node
        np = np->get_next();
    }

    //if we get here, then we didn't find the entry
    return false;
} //end is_nonzero()

//returns the value at entry (i,j)
element MapMatrix_Base::get_entry(unsigned i, unsigned j)
{
    //make sure this entry is valid
    if (columns.size() <= j)
        throw std::runtime_error("MapMatrix_Base::get_entry(): attempting to check entry in a column past end of matrix");
    if (num_rows <= i)
        throw std::runtime_error("MapMatrix_Base::get_entry(): attempting to check entry in a row past end of matrix");

    //get initial node pointer
    MapMatrixNode* np = columns[j];

    //loop while there is another node to check
    while (np != NULL) {
        if (np->get_row() < i) //then we won't find row i because row entrys are sorted in descending order
            return 0;

        if (np->get_row() == i) //then we found the row we wanted
            return np->get_value();

        //if we are still looking, then get the next node
        np = np->get_next();
    }

    //if we get here, then we didn't find the entry
    return 0;
} //end get_entry()

//adds column j to column k
//  RESULT: column j is not changed, column k contains sum of columns j and k (with arithmetic in field F)
void MapMatrix_Base::add_column(unsigned j, unsigned k)
{
    add_multiple(F.id(), j, k);
}

//adds m copies of column j to column k
//  RESULT: column j is not changed, column k contains sum of k and m times columns j (with arithmetic in field F)
void MapMatrix_Base::add_multiple(element m, unsigned j, unsigned k)
{
    //make sure this operation is valid
    if (columns.size() <= j || columns.size() <= k)
        throw std::runtime_error("MapMatrix_Base::add_multiple(): attempting to access column past end of matrix");
    if (j == k)
        throw std::runtime_error("MapMatrix_Base::add_multiple(): adding a column to itself");
    if (F.is_zero(m)) {
        debug() << "Warning: adding zero times a column in MapMatrix_Base::add_multiple()";
        return;
    }

    //pointers
    MapMatrixNode* jnode = columns[j]; //points to next node from column j that we will add to column k
    MapMatrixNode* khandle = NULL; //points to node in column k that was most recently added; will be non-null after first node is added

    //loop through all entries in column j
    while (jnode != NULL) {
        //now it is save to dereference jnode (*jnode)...
        unsigned row = jnode->get_row();

        //compute the element of F that must be added to k[row]
        element mj = F.mul(m, jnode->get_value());

        //loop through entries in column k, starting at the current position
        bool added = false;

        if (columns[k] == NULL) { //then column k is empty, so insert initial node
            MapMatrixNode* newnode = new MapMatrixNode(row, mj);
            columns[k] = newnode;
            khandle = newnode;

            added = true; //proceed with next element from column j
        }

        if (!added && khandle == NULL) { //then we haven't yet added anything to column k (but if we get here, column k is nonempty)
            if ((*columns[k]).get_row() == row) { //then add to this entry in column k
                element sum = F.add(mj, (*columns[k]).get_value());

                if (!F.is_zero(sum)) { //then store sum in this entry of column k
                    (*columns[k]).set_value(sum);
                } else { //then sum is zero, so remove this entry of column k
                    MapMatrixNode* next = (*columns[k]).get_next();
                    delete columns[k];
                    columns[k] = next;
                }
                added = true; //proceed with next element from column j
            } else if ((*columns[k]).get_row() < row) { //then insert new initial node into column k
                MapMatrixNode* newnode = new MapMatrixNode(row, mj);
                newnode->set_next(columns[k]);
                columns[k] = newnode;
                khandle = columns[k];
                added = true; //proceed with next element from column j
            } else { //then move to next node in column k
                khandle = columns[k];
                //now we want to enter the following while loop
            }
        } //end if

        while (!added && (khandle->get_next() != NULL)) //if we get here, both columns[k] and khandle are NOT NULL
        {
            //consider the next node
            MapMatrixNode* next = khandle->get_next();

            if (next->get_row() == row) { //then add to this entry in column k
                element sum = F.add(mj, next->get_value());

                if (!F.is_zero(sum)) { //then store sum in this entry of column k
                    next->set_value(sum);
                } else { //then sum is zero, so remove this entry of column k
                    khandle->set_next(next->get_next());
                    delete next;
                }
                added = true; //proceed with next element from column j
            } else if (next->get_row() < row) { //then insert new node into column k
                MapMatrixNode* newnode = new MapMatrixNode(row, mj);
                newnode->set_next(next);
                khandle->set_next(newnode);
                added = true; //proceed with next element from column j
            } else { //then next->get_row() > row, so move to next node in column k
                khandle = next;
            }
        } //end while

        if (!added && (khandle->get_next() == NULL)) { //then we have reached the end of column k, and we should append a new node
            MapMatrixNode* newnode = new MapMatrixNode(row, mj);
            khandle->set_next(newnode);
            khandle = newnode;
            //added = true, but there are no further if statements
        }

        //move to the next entry in column j
        jnode = jnode->get_next();
    } //end while(jnode != NULL)

} //end add_multiple()

/********** implementation of class MapMatrix, for column-sparse matrices **********/

//constructor that sets initial size of matrix
MapMatrix::MapMatrix(unsigned rows, unsigned cols, FFp& field)
    : MapMatrix_Base(rows, cols, field)
{
}

//constructor to create a (square) identity matrix
MapMatrix::MapMatrix(unsigned size, FFp& field)
    : MapMatrix_Base(size, field)
{
}

/*This function needs to be updated to support prime field entries
MapMatrix::MapMatrix(std::initializer_list<std::initializer_list<int>> values)
    : MapMatrix_Base(values.size(),
          std::accumulate(values.begin(), values.end(), 0U,
              [](unsigned max_so_far, const std::initializer_list<int>& row) {
                  return std::max(max_so_far, static_cast<unsigned>(row.size()));
              }))
{
    //TODO: Make this fast once we pick a representation.
    auto row_it = values.begin();
    for (unsigned row = 0; row < values.size(); row++) {
        auto row_values = *row_it;
        ++row_it;
        auto col_it = row_values.begin();
        for (unsigned col = 0; col < row_values.size(); col++) {
            if (*col_it)
                set(row, col);
            ++col_it;
        }
    }
}*/

bool MapMatrix::operator==(MapMatrix& other)
{
    //TODO: make fast once we choose a representation

    if (height() != other.height() || width() != other.width()) //TODO: also compare fields
        return false;
    for (unsigned row = 0; row < height(); row++)
        for (unsigned col = 0; col < width(); col++)
            if (get_entry(row, col) != other.get_entry(row, col))
                return false;

    return true;
}

//destructor: deletes all entries in this matrix (via automatic call to destructor of MapMatrix_Base)
MapMatrix::~MapMatrix()
{
}

//returns the number of columns in the matrix
unsigned MapMatrix::width() const
{
    return MapMatrix_Base::width();
}

//returns the number of rows in the matrix
unsigned MapMatrix::height() const
{
    return MapMatrix_Base::height();
}

//requests that the columns vector have enough capacity for num_cols columns
void MapMatrix::reserve_cols(unsigned num_cols)
{
    columns.reserve(num_cols);
}

//sets (to value) the entry in row i, column j
void MapMatrix::set(unsigned i, unsigned j, element value)
{
    MapMatrix_Base::set(i, j, value);
}

//returns true if entry (i,j) is nonzero, false otherwise
bool MapMatrix::is_nonzero(unsigned i, unsigned j)
{
    return MapMatrix_Base::is_nonzero(i, j);
}

//returns the value at entry (i,j)
element MapMatrix::get_entry(unsigned i, unsigned j)
{
    return MapMatrix_Base::get_entry(i, j);
}

//returns the "low" index in the specified column, or -1 if the column is empty or does not exist
int MapMatrix::low(unsigned j)
{
    //make sure this query is valid
    if (columns.size() <= j)
        throw std::runtime_error("MapMatrix::low(): attempting to check low number of a column past end of matrix");

    //if the column is empty, then return -1
    if (columns[j] == NULL)
        return -1;

    //otherwise, column is non-empty, so return first index (because row indexes are sorted in descending order)
    return (*columns[j]).get_row();
}

//returns true iff column j is empty
bool MapMatrix::col_is_empty(unsigned j)
{
    return (columns[j] == NULL);
}

//adds m copies of column j to column k; column j is not changed
void MapMatrix::add_multiple(element m, unsigned j, unsigned k)
{
    MapMatrix_Base::add_multiple(m, j, k);
}

//adds a multiple of column j to column k, in order to eliminate the low entry in column k
//  RESULT: column j is not changed, column k contains sum of k and m times column j (with aritmetic in field F)
//          and the low entry in column k has been zeroed
void MapMatrix::add_eliminate_low(unsigned j, unsigned k)
{
    //make sure this operation is valid
    if (columns.size() <= j || columns.size() <= k)
        throw std::runtime_error("MapMatrix::add_eliminate_low(): attempting to access column past end of matrix");
    if (columns[j] == NULL || columns[k] == NULL)
    	throw std::runtime_error("MapMatrix::add_eliminate_low(): empty column");

    //compute the multiple of column j to add to column k
    element w = columns[j]->get_value(); //low element in column j
    element a = columns[k]->get_value(); //low element in column k

    element m = F.mul(F.inv(w), F.neg(a)); //satisfies m*w + a = 0 in field F

    //now add the columns
    add_multiple(m, j, k);
} //end add_eliminate_low()

//adds a multiple of column j from MapMatrix* other to column k of this matrix, so that the low entry in column k is zeroed
void MapMatrix::add_eliminate_low(MapMatrix* other, unsigned j, unsigned k)
{
    //make sure this operation is valid
    if (other->columns.size() <= j || columns.size() <= k)
        throw std::runtime_error("MapMatrix::add_eliminate_low(other): attempting to access column past end of matrix");
    if (other->columns[j] == NULL || columns[k] == NULL)
        throw std::runtime_error("MapMatrix::add_eliminate_low(other): empty column");

    //compute multiple of column j to add to column k
    element w = other->columns[j]->get_value(); //low element in column j
    element a = columns[k]->get_value(); //low element in column k

    element m = F.mul(F.inv(w), F.neg(a)); //satisfies m*w + a = 0 in field F

    if (F.is_zero(m)) {
        debug() << "Warning: adding zero times a column in MapMatrix::add_eliminate_low(other)";
        return;
    }

    //pointers
    MapMatrixNode* jnode = other->columns[j]; //points to next node from column j that we will add to column k
    MapMatrixNode* khandle = NULL; //points to node in column k that was most recently added; will be non-null after first node is added

    //loop through all entries in column j
    while (jnode != NULL) {
        //now it is save to dereference jnode
        unsigned row = jnode->get_row();

        //compute the element of F that must be added to k[row]
        element mj = F.mul(m, jnode->get_value());

        //loop through entries in column k, starting at the current position
        bool added = false;

        if (columns[k] == NULL) { //then column k is empty, so insert initial node
            MapMatrixNode* newnode = new MapMatrixNode(row, mj);
            columns[k] = newnode;
            khandle = newnode;

            added = true; //proceed with next element from column j
        }
        if (!added && khandle == NULL) { //then we haven't yet added anything to column k (but if we get here, column k is nonempty)
            if ( columns[k]->get_row() == row) { //then remove this node (since 1+1=0)
            	element sum = F.add(mj, columns[k]->get_value());

            	if(!F.is_zero(sum)) { //then store sum in this entry of column k
            		columns[k]->set_value(sum);
            	} else { //then sum is zero, so remove this entry of column k
	                MapMatrixNode* next = columns[k]->get_next();
	                delete columns[k];
    	            columns[k] = next;
    	        }
                added = true; //proceed with next element from column j
            } else if (columns[k]->get_row() < row) { //then insert new initial node into column k
                MapMatrixNode* newnode = new MapMatrixNode(row, mj);
                newnode->set_next(columns[k]);
                columns[k] = newnode;
                khandle = columns[k];
                added = true; //proceed with next element from column j
            } else { //then move to next node in column k
                khandle = columns[k];
                //now we want to enter the following while loop
            }
        } //end if

        while (!added && (khandle->get_next() != NULL)) //if we get here, both columns[k] and khandle are NOT NULL
        {
            //consider the next node
            MapMatrixNode* next = khandle->get_next();

            if (next->get_row() == row) { //then add to this entry in column k
                element sum = F.add(mj, next->get_value());

                if (!F.is_zero(sum)) { //then store sum in this entry of column k
                	next->set_value(sum);
                } else { //then sum is zero, so remove this entry of column k
                    khandle->set_next(next->get_next());
                    delete next;
                }
                added = true; //proceed with next element from column j
            } else if (next->get_row() < row) { //then insert new node into column k
                MapMatrixNode* newnode = new MapMatrixNode(row, mj);
                newnode->set_next(next);
                khandle->set_next(newnode);
                added = true; //proceed with next element from column j
            } else { //then next->get_row() > row, so move to next node in column k
                khandle = next;
            }
        } //end while

        if (!added && (khandle->get_next() == NULL)) { //then we have reached the end of column k, and we should append a new node
            MapMatrixNode* newnode = new MapMatrixNode(row, mj);
            khandle->set_next(newnode);
            khandle = newnode;
            //added = true, but there are no further if statements
        }

        //move to the next entry in column j
        jnode = jnode->get_next();
    } //end while(jnode != NULL)

} //end add_eliminate_low(MapMatrix*, unsigned, unsigned)

//copies NONZERO columns with indexes in [first, last] from other, appending them to this matrix to the right of all existing columns
//  all row indexes in copied columns are increased by offset
void MapMatrix::copy_cols_from(MapMatrix* other, int first, int last, unsigned offset)
{
    for (int j = first; j <= last; j++) {
        MapMatrixNode* other_node = other->columns[j];
        if (other_node != NULL) {
            //create the first node in this column
            MapMatrixNode* cur_node = new MapMatrixNode(other_node->get_row() + offset, other_node->get_value());
            columns.push_back(cur_node);

            //create all other nodes in this column
            other_node = other_node->get_next();
            while (other_node != NULL)
            {
                MapMatrixNode* new_node = new MapMatrixNode(other_node->get_row() + offset, other_node->get_value());
                cur_node->set_next(new_node);
                cur_node = new_node;
                other_node = other_node->get_next();
            }
        }
    }
}//end copy_cols_from()

//copies columns with indexes in [first, last] from other, inserting them in this matrix with the same column indexes
void MapMatrix::copy_cols_same_indexes(MapMatrix* other, int first, int last)
{
    for (int j = first; j <= last; j++) {
        MapMatrixNode* other_node = other->columns[j];
        if (other_node != NULL) {
            //create the first node in this column
            MapMatrixNode* cur_node = new MapMatrixNode(other_node->get_row(), other_node->get_value());
            columns[j] = (cur_node);

            //create all other nodes in this column
            other_node = other_node->get_next();
            while (other_node != NULL)
            {
                MapMatrixNode* new_node = new MapMatrixNode(other_node->get_row(), other_node->get_value());
                cur_node->set_next(new_node);
                cur_node = new_node;
                other_node = other_node->get_next();
            }
        }
        else
            columns[j] = NULL; //this shouldn't be necessary
    }
}//end copy_cols_same_indexes()

//removes zero columns from this matrix
//ind_old gives grades of columns before zero columns are removed; new grade info stored in ind_new
//NOTE: ind_old and ind_new must have the same size!
void MapMatrix::remove_zero_cols(IndexMatrix* ind_old, IndexMatrix* ind_new)
{
    int new_col = -1; //new index of rightmost column that has been moved
    int cur_col = 0; //old index of rightmost column considered for move

//    qDebug() << "REMOVING ZERO COLS: (" << columns.size() << "cols)";
//    print();
//    ind_old->print();

    //loop over all grades
    for (unsigned y = 0; y < ind_old->height(); y++) {
        for (unsigned x = 0; x < ind_old->width(); x++) {
            int end_col = ind_old->get(y, x); //index of rightmost column at this grade
            for (; cur_col <= end_col; cur_col++) { //loop over all columns at this grade
                if (columns[cur_col] != NULL) { //then move column
                    new_col++; //new index of this column
                    columns[new_col] = columns[cur_col];
                }
            }
            ind_new->set(y, x, new_col); //rightmost column index for this grade
        }
    }

    //resize the columns vector
    columns.resize(new_col + 1);

//    qDebug() << "RESULTING MATRIX: (" << columns.size() << "cols)";
//    print();
//    ind_new->print();
}//end remove_zero_cols

std::ostream& operator<<(std::ostream& out, const MapMatrix& matrix)
{
    //handle empty matrix
    if (matrix.num_rows == 0 || matrix.columns.size() == 0) {
        out << "        (empty matrix:" << matrix.num_rows << "rows by" << matrix.columns.size() << "columns)";
        return out;
    }

    //create a 2D array of ints to temporarily store the matrix
    bool_array mx(matrix.num_rows, matrix.columns.size());
    for (unsigned i = 0; i < matrix.num_rows; i++)
        for (unsigned j = 0; j < matrix.columns.size(); j++)
            mx.at(i, j) = 0;

    //traverse the linked lists in order to fill the 2D array
    MapMatrix::MapMatrixNode* current;
    for (unsigned j = 0; j < matrix.columns.size(); j++) {
        current = matrix.columns[j];
        while (current != NULL) {
            unsigned row = current->get_row();
            mx.at(row, j) = current->get_value();
            current = current->get_next();
        }
    }

    for (unsigned i = 0; i < matrix.num_rows; i++) {
        out << "        |";
        for (unsigned j = 0; j < matrix.columns.size(); j++) {
            out << " " << mx.at(i, j);
        }
        out << " |\n";
    }
    return out;
}

/********** implementation of class MapMatrix_Perm, supports row swaps (and stores a low array) **********/

MapMatrix_Perm::MapMatrix_Perm(unsigned rows, unsigned cols, FFp& field)
    : MapMatrix(rows, cols, field)
    , perm(rows)
    , mrep(rows)
    , low_by_row(rows, -1)
    , low_by_col(cols, -1) // col_perm(cols)
{
    //initialize permutation vectors to the identity permutation
    for (unsigned i = 0; i < rows; i++) {
        perm[i] = i;
        mrep[i] = i;
    }
}

MapMatrix_Perm::MapMatrix_Perm(unsigned size, FFp& field)
    : MapMatrix(size, field)
    , perm(size)
    , mrep(size)
    , low_by_row(size, -1)
    , low_by_col(size, -1)
{
    //initialize permutation vectors to the identity permutation
    for (unsigned i = 0; i < size; i++) {
        perm[i] = i;
        mrep[i] = i;
    }
}

//copy constructor
MapMatrix_Perm::MapMatrix_Perm(const MapMatrix_Perm& other)
    : MapMatrix(other.height(), other.width(), other.F)
    , perm(other.perm)
    , mrep(other.mrep)
    , low_by_row(other.low_by_row)
    , low_by_col(other.low_by_col)
{
    //copy all matrix entries
    for (unsigned j = 0; j < other.width(); j++) {
        MapMatrixNode* other_node = other.columns[j];
        if (other_node != NULL) {
            //create the first node in this column
            MapMatrixNode* cur_node = new MapMatrixNode(other_node->get_row(), other_node->get_value());
            columns[j] = cur_node;

            //create all other nodes in this column
            other_node = other_node->get_next();
            while (other_node != NULL) {
                MapMatrixNode* new_node = new MapMatrixNode(other_node->get_row(), other_node->get_value());
                cur_node->set_next(new_node);
                cur_node = new_node;
                other_node = other_node->get_next();
            }
        }
    }
}

MapMatrix_Perm::~MapMatrix_Perm()
{
}

//sets (to value) the entry in row i, column j
//NOTE: to be used for matrix construction only; does not update low array
void MapMatrix_Perm::set(unsigned i, unsigned j, element value)
{
    MapMatrix::set(mrep[i], j, value);
}

//returns true if entry (i,j) is nonzero, false otherwise
bool MapMatrix_Perm::is_nonzero(unsigned i, unsigned j)
{
    return MapMatrix::is_nonzero(mrep[i], j);
}

//returns the value at entry (i,j)
element MapMatrix_Perm::get_entry(unsigned i, unsigned j)
{
    return MapMatrix::get_entry(mrep[i], j);
}

//reduces this matrix and returns the corresponding upper-triangular matrix for the RU-decomposition
//NOTE -- only to be called before any rows are swapped!
MapMatrix_RowPriority_Perm* MapMatrix_Perm::decompose_RU()
{
    //create the matrix U
    MapMatrix_RowPriority_Perm* U = new MapMatrix_RowPriority_Perm(columns.size(), F); //NOTE: must be deleted later!

    //loop through columns
    for (unsigned j = 0; j < columns.size(); j++) {
        //while column j is nonempty and its low number is found in the low array, do column operations
        while (columns[j] != NULL && low_by_row[columns[j]->get_row()] >= 0) {
            int c = low_by_row[columns[j]->get_row()];

            //add a multiple of column c to column j, to clear the low entry in column j
            element w = columns[c]->get_value();   //low element in column c
            element a = columns[j]->get_value();   //low element in column j
            element m = F.mul(F.inv(w), F.neg(a)); //satisfies m*w + a = 0 in field F
            add_multiple(m, c, j);                 //perform row operation on R (add m copies of column c to column j)
            U->add_multiple_row(F.neg(m), j, c);   //perform the opposite row operation on U (subtract m copies of row j from row c)
        }

        if (columns[j] != NULL) { //then column is still nonempty, so update lows
            low_by_col[j] = columns[j]->get_row();
            low_by_row[columns[j]->get_row()] = j;
        }
    }

    //return the matrix U
    return U;
} //end decompose_RU()

//returns the row index of the lowest entry in the specified column, or -1 if the column is empty
int MapMatrix_Perm::low(unsigned j)
{
    return low_by_col[j];
}

//returns the index of the column with low l, or -1 if there is no such column
int MapMatrix_Perm::find_low(unsigned l)
{
    return low_by_row[l];
}

//transposes rows i and i+1
//NOTE: this causes low array to be incorrect iff there are columns k and l with low(k)=i, low(l)=i+1, and M[i,l]=1  (as in Vineyards, Case 1.1)
//      the user must detect this and do a column operation to restore the matrix to a reduced state!
void MapMatrix_Perm::swap_rows(unsigned i, bool update_lows)
{
    //get original row indexes of these rows
    unsigned a = mrep[i];
    unsigned b = mrep[i + 1];

    //swap entries in permutation and inverse permutation arrays
    unsigned temp = perm[a]; ///TODO: why do I do this? isn't temp == i?
    perm[a] = perm[b];
    perm[b] = temp;

    mrep[i] = b;
    mrep[i + 1] = a;

    //update low arrays
    if (update_lows) {
        int l = low_by_row[i];
        int k = low_by_row[i + 1];

        low_by_row[i] = k;
        low_by_row[i + 1] = l;

        if (l != -1)
            low_by_col[l] = i + 1;
        if (k != -1)
            low_by_col[k] = i;
    }
} //end swap_rows()

//transposes columns j and j+1
//NOTE: this does not update low arrays! user must do this via swap_lows()
void MapMatrix_Perm::swap_columns(unsigned j, bool update_lows)
{
    //swap columns
    MapMatrixNode* temp = columns[j];
    columns[j] = columns[j + 1];
    columns[j + 1] = temp;

    //update low arrays
    if (update_lows) {
        int l = low_by_col[j];
        int k = low_by_col[j + 1];

        low_by_col[j] = k;
        low_by_col[j + 1] = l;

        if (l != -1)
            low_by_row[l] = j + 1;
        if (k != -1)
            low_by_row[k] = j;
    }
}

//clears the matrix, then rebuilds it from reference with columns permuted according to col_order
//  NOTE: reference should have the same size as this matrix!
//  col_order is a map: (column index in reference matrix) -> (column index in rebuilt matrix)
void MapMatrix_Perm::rebuild(MapMatrix_Perm* reference, std::vector<unsigned>& col_order)
{
    //clear the matrix
    for (unsigned j = 0; j < columns.size(); j++) {
        MapMatrixNode* current = columns[j];
        while (current != NULL) {
            MapMatrixNode* next = current->get_next();
            delete current;
            current = next;
        }
    }

    //reset low arrays
    for (unsigned i = 0; i < num_rows; i++)
        low_by_row[i] = -1;
    for (unsigned j = 0; j < columns.size(); j++)
        low_by_col[j] = -1;

    //reset permutation vectors
    for (unsigned i = 0; i < num_rows; i++) {
        perm[i] = i;
        mrep[i] = i;
    }

    //build the new matrix
    for (unsigned j = 0; j < columns.size(); j++) {
        //copy column j from reference into column col_order[j] of this matrix
        MapMatrixNode* ref_node = reference->columns[j];
        if (ref_node != NULL) {
            //create the first node in this column
            MapMatrixNode* cur_node = new MapMatrixNode(ref_node->get_row(), ref_node->get_value());
            columns[col_order[j]] = cur_node;

            //create all other nodes in this column
            ref_node = ref_node->get_next();
            while (ref_node != NULL) {
                MapMatrixNode* new_node = new MapMatrixNode(ref_node->get_row(), ref_node->get_value());
                cur_node->set_next(new_node);
                cur_node = new_node;
                ref_node = ref_node->get_next();
            }
        }
    }
} //end rebuild()

//clears the matrix, then rebuilds it from reference with columns permuted according to col_order and rows permuted according to row_order
//  NOTE: reference should have the same size as this matrix!
//  col_order is a map: (column index in reference matrix) -> (column index in rebuilt matrix) and similarly for row_order
void MapMatrix_Perm::rebuild(MapMatrix_Perm* reference, std::vector<unsigned>& col_order, std::vector<unsigned>& row_order)
{
    ///TESTING: check the permutation
    //std::vector<bool> check(columns.size(), false);
    //for (unsigned j = 0; j < columns.size(); j++)
    //    check[col_order[j]] = true;
    //for (unsigned j = 0; j < columns.size(); j++)
    //    if (check[j] == false) {
    //        debug() << "ERROR: column permutation skipped" << j;
    //    }

    //clear the matrix
    for (unsigned j = 0; j < columns.size(); j++) {
        MapMatrixNode* current = columns[j];
        while (current != NULL) {
            MapMatrixNode* next = current->get_next();
            delete current;
            current = next;
        }
        columns[j] = NULL; ///TODO: CHECK -- HAVE I AVOIDED A MEMORY LEAK HERE?
    }

    //reset low arrays
    for (unsigned i = 0; i < num_rows; i++)
        low_by_row[i] = -1;
    for (unsigned j = 0; j < columns.size(); j++)
        low_by_col[j] = -1;

    //reset permutation vectors
    for (unsigned i = 0; i < num_rows; i++) {
        perm[i] = i;
        mrep[i] = i;
    }

    //build the new matrix
    for (unsigned j = 0; j < columns.size(); j++) {
        MapMatrixNode* ref_node = reference->columns[j];
        while (ref_node != NULL) {
            MapMatrix::set(row_order[ref_node->get_row()], col_order[j], ref_node->get_value());
            ref_node = ref_node->get_next();
        }
    }
} //end rebuild()

//function to print the matrix to standard output, for testing purposes
void MapMatrix_Perm::print()
{
    //handle empty matrix
    if (num_rows == 0 || columns.size() == 0) {
        debug() << "        (empty matrix:" << num_rows << "rows by" << columns.size() << "columns)";
        return;
    }

    //create a 2D array of booleans to temporarily store the matrix
    bool_array mx(num_rows, columns.size());
    for (unsigned i = 0; i < num_rows; i++)
        for (unsigned j = 0; j < columns.size(); j++)
            mx.at(i, j) = 0;

    //traverse the linked lists in order to fill the 2D array
    MapMatrixNode* current;
    for (unsigned j = 0; j < columns.size(); j++) {
        current = columns[j];
        while (current != NULL) {
            unsigned row = current->get_row();
            mx.at(perm[row], j) = current->get_value();
            current = current->get_next();
        }
    }

    //print the matrix
    for (unsigned i = 0; i < num_rows; i++) {
        Debug qd = debug(true);
        qd << "        |";
        for (unsigned j = 0; j < columns.size(); j++) {
            qd << " " << mx.at(i, j);
        }
        qd << " |";
    }
} //end print()

//check for inconsistencies in low arrays, for testing purposes
void MapMatrix_Perm::check_lows()
{
    for (unsigned i = 0; i < num_rows; i++) {
        if (low_by_row[i] != -1) {
            if (low_by_col[low_by_row[i]] != static_cast<int>(i))
                debug() << "===>>> ERROR: INCONSISTNECY IN LOW ARRAYS";
        }
    }
    for (unsigned j = 0; j < columns.size(); j++) {
        //find the lowest entry in column j
        int lowest = -1;
        if (columns[j] != NULL) {
            //consider the first node
            MapMatrixNode* current = columns[j];
            lowest = perm[current->get_row()];

            //consider all following nodes
            current = current->get_next();
            while (current != NULL) {
                if (static_cast<int>(perm[current->get_row()]) > lowest)
                    lowest = perm[current->get_row()];

                current = current->get_next();
            }
        }

        //does this match low_by_col[j]?
        if (lowest != low_by_col[j])
            debug() << "===>>> ERROR IN low_by_col[" << j << "]";
        else if (lowest != -1) {
            if (low_by_row[lowest] != static_cast<int>(j))
                debug() << "===>>> ERROR: INCONSISTNECY IN LOW ARRAYS";
        }
    }
}

/********** implementation of class MapMatrix_RowPriority_Perm **********/

MapMatrix_RowPriority_Perm::MapMatrix_RowPriority_Perm(unsigned size, FFp& field)
    : MapMatrix_Base(size, field)
    , perm(size)
    , mrep(size)
{
    //initialize permutation vectors to the identity permutation
    for (unsigned i = 0; i < size; i++) {
        perm[i] = i;
        mrep[i] = i;
    }
}

//copy constructor
MapMatrix_RowPriority_Perm::MapMatrix_RowPriority_Perm(const MapMatrix_RowPriority_Perm& other)
    : MapMatrix_Base(other.height(), other.F)
    , perm(other.perm)
    , mrep(other.mrep)
{
    //copy all matrix entries
    for (unsigned j = 0; j < other.height(); j++) {
        MapMatrixNode* other_node = other.columns[j];
        if (other_node != NULL) {
            //create the first node in this column
            MapMatrixNode* cur_node = new MapMatrixNode(other_node->get_row(), other_node->get_value());
            columns[j] = cur_node;

            //create all other nodes in this column
            other_node = other_node->get_next();
            while (other_node != NULL) {
                MapMatrixNode* new_node = new MapMatrixNode(other_node->get_row(), other_node->get_value());
                cur_node->set_next(new_node);
                cur_node = new_node;
                other_node = other_node->get_next();
            }
        }
    }
}

MapMatrix_RowPriority_Perm::~MapMatrix_RowPriority_Perm()
{
}

unsigned MapMatrix_RowPriority_Perm::width() const
{
    return MapMatrix_Base::height();
}

unsigned MapMatrix_RowPriority_Perm::height() const
{
    return MapMatrix_Base::width();
}

void MapMatrix_RowPriority_Perm::set(unsigned i, unsigned j, element value)
{
    MapMatrix_Base::set(mrep[j], i, value);
}

void MapMatrix_RowPriority_Perm::clear(unsigned i, unsigned j)
{
    MapMatrix_Base::clear(mrep[j], i);
}

bool MapMatrix_RowPriority_Perm::is_nonzero(unsigned i, unsigned j)
{
    return MapMatrix_Base::is_nonzero(mrep[j], i);
}

element MapMatrix_RowPriority_Perm::get_entry(unsigned i, unsigned j)
{
    return MapMatrix_Base::get_entry(mrep[j], i);
}

//adds row j to row k; RESULT: row j is not changed, row k contains sum of rows j and k (with mod-2 arithmetic)
void MapMatrix_RowPriority_Perm::add_multiple_row(element m, unsigned j, unsigned k)
{
    return MapMatrix_Base::add_multiple(m, j, k);
}

//transposes rows i and i+1
void MapMatrix_RowPriority_Perm::swap_rows(unsigned i)
{
    MapMatrixNode* temp = columns[i];
    columns[i] = columns[i + 1];
    columns[i + 1] = temp;
}

//transposes columns j and j+1
void MapMatrix_RowPriority_Perm::swap_columns(unsigned j)
{
    //get original indexes of these columns
    unsigned a = mrep[j];
    unsigned b = mrep[j + 1];

    //swap perm[a] and perm[b]
    unsigned temp = perm[a];
    perm[a] = perm[b];
    perm[b] = temp;

    //swap mrep[i] and mrep[i+1]
    mrep[j] = b;
    mrep[j + 1] = a;
}

//prints the matrix to debug(), for testing
//this function is identical to MapMatrix::print(), with rows and columns transposed
void MapMatrix_RowPriority_Perm::print()
{
    //handle empty matrix
    if (num_rows == 0 || columns.size() == 0) {
        debug() << "        (empty matrix:" << columns.size() << "rows by" << num_rows << "columns)";
        return;
    }

    //create a 2D array of booleans to temporarily store the matrix
    bool_array mx(columns.size(), num_rows);
    for (unsigned i = 0; i < columns.size(); i++)
        for (unsigned j = 0; j < num_rows; j++)
            mx.at(i, j) = 0;

    //traverse the linked lists in order to fill the 2D array
    MapMatrixNode* current;
    for (unsigned j = 0; j < columns.size(); j++) {
        current = columns[j];
        while (current != NULL) {
            unsigned row = current->get_row();
            mx.at(j, perm[row]) = current->get_value();
            current = current->get_next();
        }
    }

    //print the matrix
    for (unsigned i = 0; i < num_rows; i++) {
        Debug qd = debug(true);
        qd << "        |";
        for (unsigned j = 0; j < columns.size(); j++) {
            qd << " " << mx.at(i, j);
        }
        qd << " |";
    }
} //end print()

//prints the permutation vectors to debug() for testing
void MapMatrix_RowPriority_Perm::print_perm()
{
    Debug qd = debug();
    qd << " ==== Perm:";
    for (unsigned i = 0; i < perm.size(); i++)
        qd << perm[i];
    qd << "\n ==== Mrep:";
    for (unsigned i = 0; i < mrep.size(); i++)
        qd << mrep[i];
}
