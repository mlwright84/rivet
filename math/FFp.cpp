
/**********************************************************************
Copyright 2014-2017 The RIVET Developers. See the COPYRIGHT file at
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
 * \class    FFp
 * \brief    finite field of order p
 * \author   Matthew L. Wright
 * \date     June 2017
 * 
 * Stores a primitive element, exp and log tables for constant-time operations.
 * Implementation is based on similar implementations in:
 *   Dionysus (http://hg.mrzv.org/Dionysus/file/c54290544583/include/topology/field-arithmetic.h)
 *   Macaulay2 (https://github.com/Macaulay2/M2/blob/master/M2/Macaulay2/e/ZZp.hpp)
 *   Singular (https://github.com/Singular/Sources/blob/spielwiese/libpolys/coeffs/modulop.cc)
 */

//#pragma once --UNCOMMNET AFTER TESTING COMPLETE

#include <vector>

#include <iostream> // TESTING ONLY


class FFp {
public:
    typedef int element;

    FFp(element p = 2) : p_(p)  //similar to Z_mod::initialize_Z_mod(int p) in Macaulay2 -- https://github.com/Macaulay2/M2/blob/master/M2/Macaulay2/e/ZZp.cpp
    {
	std::cout << "creating field of order " << p_ << "\n";

        element x, i, q, n;

        // find a primitive element
        if(p_ == 2) {
            g_ = 1; // 1 is the primitive element in FF_2
        } else {
            x = 1;
            for(i = 2; i < p_ && x < p_ - 1; i++) {  // check if i is a primitive element
            	for(q = i, x = 1; q != 1 && x < p_; q = (q*i) % p_, x++ ) { // if x gets to p-1, then i is a primitive element
            		;
            	}
            }
            g_ = i - 1; // this is the primitive element
        }
	std::cout << "-- primitive element: " << g_ << "\n";

        // initialize exp and log tables
        exp.resize(p_);
        log.resize(p_);
        for(i = 0, n = 1; i < p_ - 1; i++, n = (n * g_) % p_) { // n = g^i (mod p), where g is a primitive root
            exp[i] = n;  // n = g^i (mod p)
            log[n] = i;  // i = log base g of n in this field
        }
        exp[p_ - 1] = 0;  // isn't this unnecessary?
        log[0] = p_ - 1;  // isn't this unnecessary?
	
	std::cout << "exp: ";
	for(i=0; i<p_; i++)
	    std::cout << exp[i] << " ";
	std::cout << "\nlog: ";
	for(i=0; i<p_; i++)
	    std::cout << log[i] << " ";
	std::cout << "\n";
	
    }

    element id() const { return 1; }

    element zero() const { return 0; }

    element to_element(int a) const {
        a = a % p_;
        return ( a < 0 ? a + p_ : a );  // ensures positive value
    }

    element neg(element a) const {
    	return p_ - a;
    }

    element add(element a, element b) const {
    	element t = a + b;
    	return ( t < p_ ? t : t - p_ );  // ensure value is mod p
    }

    element inv(element a) const {
        element x = p_ - 1 - log[a];
        return exp[x];
    }

    element mul(element a, element b) const {
        element x = log[a] + log[b];
        return ( x < p_ ? exp[x] : exp[x - p_] );
    }

    element div(element a, element b) const {
        return mul(a, inv(b));
    }

    bool is_zero(element a) const { return a == 0; }

    element order() const { return p_; }

private:
    element p_;                  // a prime
    element g_;                  // primitive element (multiplicative generator)
    std::vector<element> exp;    // exp[i] = g^i (mod p)
    std::vector<element> log;    // log[n] = log base g of n in this field
};


//// TESTING /////

int main()
{
    int p = 7;
    FFp R = FFp(p);

    std::cout << "Created field with order " << R.order() << "\n";

    std::cout << "Negatives:\n";
    for(int i = 0; i < p; i++) {
        std::cout << "  " << i << "^{-1} = " << R.neg(i) << "\n";
    }
    std::cout << "Multiplicative Inverses:\n";
    for(int i = 0; i < p; i++) {
        std::cout << "  " << i << "^{-1} = " << R.inv(i) << "\n";
    }


}
