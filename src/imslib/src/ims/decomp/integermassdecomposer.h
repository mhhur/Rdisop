#ifndef IMS_INTEGERMASSDECOMPOSER_H
#define IMS_INTEGERMASSDECOMPOSER_H

#include <vector>
#include <utility>
#include <ims/weights.h>
#include <ims/utils/gcd.h>
#include <ims/decomp/massdecomposer.h>

namespace ims {

/**
 * @brief Implements @c MassDecomposer interface using algorithm and data 
 * structures described in paper "Efficient Mass Decomposition" 
 * S. Böcker, Zs. Lipták, ACM SAC-BIO, 2004.
 *
 * The main idea is instead of using the classical dynamic programming 
 * algorithm, store the residues of the smallest decomposable numbers 
 * for every modulo of the smallest alphabet mass.
 *
 * @param ValueType Type of values to be decomposed.
 * @param DecompositionValueType Type of decomposition elements.
 * 
 * @see ClassicalDPMassDecomposer
 * 
 * @ingroup decomp
 * 
 * @author Anton Pervukhin <Anton.Pervukhin@CeBiTec.Uni-Bielefeld.DE> 
 * @author Marcel Martin <Marcel.Martin@CeBiTec.Uni-Bielefeld.DE>
 * @author Henner Sudek <Henner.Sudek@CeBiTec.Uni-Bielefeld.DE>  
 */
template <typename ValueType = long unsigned int,
		  typename DecompositionValueType = unsigned int>
class IntegerMassDecomposer : 
				public MassDecomposer<ValueType,DecompositionValueType> {
	public:
		/**
		 * Type of value to be decomposed.
		 */
		typedef typename MassDecomposer<ValueType,DecompositionValueType>::
			value_type value_type;

		/**
		 * Type of decomposition value.
		 */
		typedef typename MassDecomposer<ValueType,DecompositionValueType>::
			decomposition_value_type decomposition_value_type;

		/**
		 * Type of decomposition.
		 */
		typedef typename MassDecomposer<ValueType,DecompositionValueType>::
			decomposition_type decomposition_type;

		/**
		 * Type of container for many decompositions.
		 */
		typedef typename MassDecomposer<ValueType,DecompositionValueType>::
			decompositions_type decompositions_type;

		/**
		 * Type of decomposition's size.
		 */
		typedef typename decomposition_type::size_type size_type;

		/**
		 * Constructor with weights.
		 *
		 * @param alphabet Weights over which masses to be decomposed.
		 */
		IntegerMassDecomposer(const Weights& alphabet);

		/**
		 * Returns true if decomposition over the @c mass exists, otherwise - false.
		 *
		 * @param mass Mass to be decomposed.
		 * @return true if decomposition over a given mass exists, otherwise - false.
		 */
		virtual bool exist(value_type mass);

		/**
		 * Gets one possible decomposition for @c mass.
		 *
		 * @param mass Mass to be decomposed.
		 * @return One possible decomposition for a given mass.
		 */
 		virtual decomposition_type getDecomposition(value_type mass);

		/**
		 * Gets all possible decompositions for @c mass.
		 *
		 * @param mass Mass to be decomposed.
		 * @return All possible decompositions for a given mass.
		 */
		virtual decompositions_type getAllDecompositions(value_type mass);

		/**
		 * Gets number of all possible decompositions for a given @c mass.
		 * Since using getAllDecomposition() the usage of this function could 
		 * be @b consuming.
		 * 
		 * @param mass Mass to be decomposed
		 * @return number of decompositions for a given mass.
		 */
		virtual decomposition_value_type getNumberOfDecompositions(value_type mass);
		
	private:
	
		/**
		 * Type of witness vector.
		 */
		typedef std::vector< std::pair<size_type, decomposition_value_type> > witness_vector_type;

		/**
		 * Type of rows of residues table.
		 */
		typedef std::vector<value_type> residues_table_row_type;

		/**
		 * Type of the residues table.
		 */
		typedef std::vector<residues_table_row_type> residues_table_type;

		/**
		 * Weights over which the mass is to be decomposed.
		 */
		Weights alphabet;

		/**
		 * Table with the residues of the smallest decomposable numbers over
		 * every modulo of the smallest alphabet mass are stored.
		 * Corresponds to the Extended Residue Table in the paper.
		 */
		residues_table_type ertable;

		/**
		 * List of the least common multiples. Corresponds to the lcm data structure
		 * in the paper.
		 */
		residues_table_row_type lcms;

		/**
		 * List of the counters for the least common multiples that store the number
		 * how often the smallest alphabet mass fits into the correcponding least
		 * common multiple(lcm).
		 */
		residues_table_row_type mass_in_lcms;

		/**
		 * Represents an infinite value.
		 */
		value_type infty;

		/**
		 * List of the witnesses that is used to find one mass decomposition.
		 * Corresponds to the witness vector w in the paper.
		 */
		witness_vector_type witness_vector;

		/**
		 * Fills the extended residues table.
		 */
		void fillExtendedResidueTable(const Weights& _alphabet, residues_table_row_type& _lcms,
									  residues_table_row_type& _mass_in_lcms, const value_type _infty,
									  witness_vector_type& _witness_vector, residues_table_type& _ertable);

		/**
		 * Collects decompositions for @c mass by recursion. 
		 *
		 * @param mass Mass to be decomposed.
		 * @param alphabetMassIndex An index of the mass in alphabet that is used on this step of recursion.
		 * @param decomposition Decomposition which is calculated on this step of recursion.
		 * @param decompositionStore Container where decompositions are collected.
		 */
		 void collectDecompositionsRecursively(value_type mass, size_type alphabetMassIndex,
				decomposition_type decomposition, decompositions_type& decompositionsStore);
};


template <typename ValueType, typename DecompositionValueType>
IntegerMassDecomposer<ValueType, DecompositionValueType>::IntegerMassDecomposer(
            const Weights& alphabet) : alphabet(alphabet){

	lcms.reserve(alphabet.size());
	lcms.resize(alphabet.size());

	mass_in_lcms.reserve(alphabet.size());
	mass_in_lcms.resize(alphabet.size());

	infty = alphabet.getWeight(0) * alphabet.getWeight(alphabet.size()-1);

	fillExtendedResidueTable(alphabet, lcms, mass_in_lcms, infty, witness_vector, ertable);

}


template <typename ValueType, typename DecompositionValueType>
void IntegerMassDecomposer<ValueType, DecompositionValueType>::fillExtendedResidueTable(
	const Weights& _alphabet, residues_table_row_type& _lcms, residues_table_row_type& _mass_in_lcms,
	const value_type _infty, witness_vector_type& _witnessVector, residues_table_type& _ertable) {

	if (_alphabet.size() < 2) {
		return;
	}
	// caches the most often used mass - smallest mass
	value_type smallestMass = _alphabet.getWeight(0), secondMass = _alphabet.getWeight(1);

	// initializes table: infinity everywhere except in the first field of every column
	_ertable.reserve(_alphabet.size());
	_ertable.assign(_alphabet.size(), std::vector<value_type>(smallestMass, _infty));

	for (size_type i = 0; i < _alphabet.size(); ++i) {
		_ertable[i][0] = 0;
	}

	// initializes witness vector
	_witnessVector.reserve(smallestMass);
	_witnessVector.resize(smallestMass);

	// fills second column (the first one is already correct)
	size_type it_inc = secondMass % smallestMass, witness = 1;
	typename residues_table_row_type::iterator it = _ertable[1].begin() + it_inc;
	value_type mass = secondMass;
	// initializes counter to create a witness vector
	decomposition_value_type counter = 0;
	while (it != _ertable[1].begin()) {
		*it = mass;
		mass += secondMass;
		++counter;
		_witnessVector[it - _ertable[1].begin()] = std::make_pair(witness, counter);
		it += it_inc;
		if (it >= _ertable[1].end()) {
			it -= _ertable[1].size();
		}
	}
	// fills cache variables for i==1
	value_type d = gcd(smallestMass, secondMass);
	_lcms[1] = secondMass*smallestMass / d;
	_mass_in_lcms[1] = smallestMass / d;

	// fills remaining table. i is the column index.
	for (size_type i = 2; i < _alphabet.size(); ++i) {
		// caches often used i-th alphabet mass
		value_type currentMass = _alphabet.getWeight(i);

		value_type d = gcd(smallestMass, currentMass);

		// fills cache for various variables.
		// note that values for i==0 are never assigned since they're unused anyway.
		_lcms[i] = currentMass*smallestMass / d;
		_mass_in_lcms[i] = smallestMass / d;

		// Nijenhuis' improvement: Is currentMass composable with smaller alphabet?
		if (currentMass >= _ertable[i-1][currentMass % smallestMass]) {
			_ertable[i] = _ertable[i-1];
			continue;
		}

		const residues_table_row_type& prev_column = _ertable[i-1];
		residues_table_row_type& cur_column = _ertable[i];

		if (d == 1) {
			// This loop is for the case that the gcd is 1. The optimization used below
			// is not applicable here.

			// p_inc is used to change residue (p) efficiently
			size_type p_inc = currentMass % smallestMass;

			// n is the value that will be written into the table
			value_type n = 0;
			// current residue (in paper variable 'r' is used)
			size_type p = 0;
			// counter for creation of witness vector
			decomposition_value_type counter = 0;

			for (size_type m = smallestMass; m > 0; --m) {
				n += currentMass;
				p += p_inc;
				++counter;
				if (p >= smallestMass) {
					p -= smallestMass;
				}
				if (n > prev_column[p]) {
					n = prev_column[p];
					counter = 0;
				} else {
					_witnessVector[p] = std::make_pair(i, counter);
				}
				cur_column[p] = n;
			}
		}
		else {
			// If we're here, the gcd is not 1. We can use the following cache-optimized
			// version of the algorithm. The trick is to put the iteration over all
			// residue classes into the _inner_ loop.
			//
			// One could see it as going through one column in blocks which are gcd entries long.
			size_type cur = currentMass % smallestMass;
			size_type prev = 0;
			size_type p_inc = cur-d;
			// counters for creation of one witness vector
			std::vector<decomposition_value_type> counters(smallestMass);

			// copies first block from prev_column to cur_column
			for (size_type j = 1; j < d; ++j) {
				cur_column[j] = prev_column[j];
			}

			// first loop: goes through all blocks, updating cur_column for the first time.
			for (size_type m = smallestMass / d; m > 1; m--) {
				// r: current residue class
				for (size_type r = 0; r < d; r++) {

					++counters[cur];
					if (cur_column[prev] + currentMass > prev_column[cur]) {
						cur_column[cur] = prev_column[cur];
						counters[cur] = 0;
					} else {
						cur_column[cur] = cur_column[prev] + currentMass;
						_witnessVector[cur] = std::make_pair(i, counters[cur]);
					}

					prev++;
					cur++;
				}

				prev = cur - d;

				// this does: cur = (cur + currentMass) % smallestMass - d;
				cur += p_inc;
				if (cur >= smallestMass) {
					cur -= smallestMass;
				}
			}

			// second loop:
			bool cont = true;
			while (cont) {
				cont = false;
				prev++;
				cur++;
				++counters[cur];
				for (size_type r = 1; r < d; ++r) {
					if (cur_column[prev] + currentMass < cur_column[cur]) {
						cur_column[cur] = cur_column[prev] + currentMass;
						cont = true;
						_witnessVector[cur] = std::make_pair(i, counters[cur]);
					} else {
						counters[cur] = 0;
					}
					prev++;
					cur++;
				}

				prev = cur - d;

				cur += p_inc;
				if (cur >= smallestMass) {
					cur -= smallestMass;
				}
			}
		}

	}
}


template <typename ValueType, typename DecompositionValueType>
bool IntegerMassDecomposer<ValueType, DecompositionValueType>::
exist(value_type mass) {

	value_type residue = ertable.back().at(mass % alphabet.getWeight(0));
	return (residue != infty && mass >= residue);
}


template <typename ValueType, typename DecompositionValueType>
typename IntegerMassDecomposer<ValueType, DecompositionValueType>::decomposition_type
IntegerMassDecomposer<ValueType, DecompositionValueType>::
getDecomposition(value_type mass) {

	decomposition_type decomposition;
	if (!this->exist(mass)) {
		return decomposition;
	}

	decomposition.reserve(alphabet.size());
	decomposition.resize(alphabet.size());

	// initial mass residue: in FIND-ONE algorithm in paper corresponds variable "r"
	value_type r = mass % alphabet.getWeight(0);
	value_type m = ertable.back().at(r);

	decomposition.at(0) = static_cast<decomposition_value_type>
		((mass - m) / alphabet.getWeight(0));

	while (m != 0) {
		size_type i = witness_vector.at(r).first;
		decomposition_value_type j = witness_vector.at(r).second;
		decomposition.at(i) += j;
		if (m < j*alphabet.getWeight(i)) {
			break;
		}
		m -= j * alphabet.getWeight(i);
		r = m % alphabet.getWeight(0);
	}
	return decomposition;
}


template <typename ValueType, typename DecompositionValueType>
typename IntegerMassDecomposer<ValueType, DecompositionValueType>::decompositions_type
IntegerMassDecomposer<ValueType, DecompositionValueType>::
getAllDecompositions(value_type mass) {
	decompositions_type decompositionsStore;
	decomposition_type decomposition(alphabet.size());
	collectDecompositionsRecursively(mass, alphabet.size()-1, decomposition, decompositionsStore);
	return decompositionsStore;
}


template <typename ValueType, typename DecompositionValueType>
void IntegerMassDecomposer<ValueType, DecompositionValueType>::
collectDecompositionsRecursively(value_type mass, size_type alphabetMassIndex,
 decomposition_type decomposition, decompositions_type& decompositionsStore) {
	if (alphabetMassIndex == 0) {
		value_type numberOfMasses0 = mass / alphabet.getWeight(0);
		if (numberOfMasses0 * alphabet.getWeight(0) == mass) {
			decomposition[0] = static_cast<decomposition_value_type>(
															numberOfMasses0);
			decompositionsStore.push_back(decomposition);
		}
		return;
	}

	// tested: caching these values gives us 15% better performance, at least
	// with aminoacid-mono.masses
	const value_type lcm = lcms[alphabetMassIndex];
	const value_type mass_in_lcm = mass_in_lcms[alphabetMassIndex]; // this is alphabet mass divided by gcd

	value_type mass_mod_alphabet0 = mass % alphabet.getWeight(0); // trying to avoid modulo
	const value_type mass_mod_decrement = alphabet.getWeight(alphabetMassIndex) % alphabet.getWeight(0);

	for (value_type i = 0; i < mass_in_lcm; ++i) {
		// here is the conversion from value_type to decomposition_value_type
		decomposition[alphabetMassIndex] = static_cast<decomposition_value_type>(i);

		// this check is needed because mass could have unsigned type and after reduction on i*alphabetMass will be still be positive but huge
		// and that will end up in unfinite loop
		if (mass < i*alphabet.getWeight(alphabetMassIndex)) {
			break;
		}

		/* r: current residue class. will stay the same in the following loop */
		value_type r = ertable[alphabetMassIndex-1][mass_mod_alphabet0];

		// TODO: if infty was std::numeric_limits<...>... the following 'if' would not be necessary
		if (r != infty) {
			for (value_type m = mass - i * alphabet.getWeight(alphabetMassIndex); m >= r; m -= lcm) {
				/* the condition of the 'for' loop (m >= r) and decrementing the mass
				 * in steps of the lcm ensures that m is decomposable. Therefore
				 * the recursion will result in at least one witness. */
				collectDecompositionsRecursively(m, alphabetMassIndex-1, decomposition, decompositionsStore);
				decomposition[alphabetMassIndex] += mass_in_lcm;
				// this check is needed because mass could have unsigned type and after reduction on i*alphabetMass will be still be positive but huge
				// and that will end up in unfinite loop
				if (m < lcm) {
					break;
				}
			}
		}
		// subtle way of changing the modulo, instead of plain calculation it from (mass - i*currentAlphabetMass) % alphabetMass0 every time
		if (mass_mod_alphabet0 < mass_mod_decrement) {
			mass_mod_alphabet0 += alphabet.getWeight(0) - mass_mod_decrement;
		} else {
			mass_mod_alphabet0 -= mass_mod_decrement;
		}
	}

}

/**
 * Gets number of all possible decompositions for a given @c mass.
 * Since using getAllDecomposition() the usage of this function could 
 * be @b consuming.
 * 
 * @param mass Mass to be decomposed
 * @return number of decompositions for given mass.
 */
template <typename ValueType, typename DecompositionValueType>
typename IntegerMassDecomposer<ValueType, DecompositionValueType>::
decomposition_value_type IntegerMassDecomposer<ValueType, 
DecompositionValueType>::getNumberOfDecompositions(value_type mass) {
	return getAllDecompositions(mass).size();
}


} // namespace ims

#endif // IMS_INTEGERMASSDECOMPOSER_H
