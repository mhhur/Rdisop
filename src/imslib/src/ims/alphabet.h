#ifndef IMS_ALPHABET_H
#define IMS_ALPHABET_H

#include <vector>
#include <string>
#include <ostream>

#include <ims/element.h>
#include <ims/base/parser/alphabetparser.h>
#include <ims/base/exception/unknowncharacterexception.h>
#include <ims/base/exception/ioexception.h>


namespace ims {

/**
 * @brief Holds an indexed list of bio-chemical elements.
 * 
 * Presents an indexed list of bio-chemical elements of type (or derived from 
 * type) @c Element. Due to indexed structure @c Alphabet can be used similar
 * to @c std::vector, for example to add a new element to @c Alphabet function
 * @c push_back(element_type) can be used. Elements or their properties (such
 * as element's mass) can be accessed by index in a constant time. On the other
 * hand accessing elements by their names takes linear time. Due to this and 
 * also the fact that @c Alphabet is 'heavy-weighted' (consisting of 
 * @c Element -s or their derivatives where the depth of derivation as well is 
 * undefined resulting in possibly 'heavy' access operations) it is recommended
 * not use @c Alphabet directly in operations where fast access to 
 * @c Element 's properties is required. Instead consider to use 
 * 'light-weighted' equivalents, such as @c Weights.
 * 
 * Elements in @c Alphabet can be sorted by the @c Element 's properties: 
 * sequence and mass. When alphabet's data is loaded from file it is 
 * automatically sorted by mass. To load data from file default function
 * @c load(str::string& fname) can be used. Then elements have to be stored 
 * in a flat file @c fname in a predefined format. To read more on this format, 
 * please, @see AlphabetParser. If one wants to load data stored differently or 
 * in its own file format (i.e. xml) one has to define a new parser derived from 
 * @c AlphabetParser and pass its pointer together with the file name to function 
 * @c load(const std::string& fname, AlphabetParser<>* parser). If there is any 
 * error happened while loading data, @c IOException will be thrown. 
 * 
 * @see ComposedElement
 * 
 * @ingroup alphabet
 */
class Alphabet {
	public:
		typedef Element element_type;
		typedef element_type::mass_type mass_type;
		typedef element_type::name_type name_type;
		typedef std::vector<element_type> container;
		typedef container::size_type size_type;
		typedef container::iterator iterator;
		typedef container::const_iterator const_iterator;
		typedef std::vector<name_type> name_container;
		typedef name_container::iterator name_iterator;
		typedef name_container::const_iterator const_name_iterator;
		typedef std::vector<mass_type> mass_container;
		typedef mass_container::iterator mass_iterator;
		typedef mass_container::const_iterator const_mass_iterator;
		typedef std::vector<mass_type> masses_type;

		/**
		 * Empty constructor.
		 */
		Alphabet() { }


		/**
		 * Constructor with elements.
		 *
		 * @param elements Elements to be set
		 */
		Alphabet(const container& elements) :
							elements(elements) { }


		/**
		 * Copy constructor.
		 * 
		 * @param alphabet Alphabet to be assigned
		 */
		Alphabet(const Alphabet &alphabet) :
							elements(alphabet.elements) { }

		/**
		 * Returns the alphabet size.
		 *
		 * @return The size of alphabet.
		 */
		size_type size() const { return elements.size(); }

		/**
		 * Gets the element with index @c index.
		 * @note Operation takes constant time.
		 *
		 * @param Index of the element
		 * @return Element with the given index in alphabet
		 */
		const element_type& getElement(size_type index) const
		{ return elements[index]; }

		/**
		 * Gets the element with the symbol @name. If there is
		 * no such element, throws @c UnknownCharacterException.
		 *
		 * @param name Name of the element.
		 * @return Element with the given name, or if there are no such element
		 * 			throws @c UnknownCharacterException.
		 */
		const element_type& getElement(const name_type& name) const
									throw (UnknownCharacterException);

		/**
		 * Gets the symbol of the element with an index @c index in alphabet.
		 *
		 * @param Index of the element.
		 * @return Name of the element.
		 */
		const name_type& getName(size_type index) const;

		/**
		 * Gets monoisotopic mass of the element with the symbol @c name. 
		 * If there is no such element, throws an @c UnknownCharacterException.
		 *
		 * @param name Symbol of the element.
		 * @return Mass of the element, or if there are no element
		 * 			throws @c UnknownCharacterException.
		 * @see getMass(size_type index)
		 */
		mass_type getMass(const name_type& name) const
									throw (UnknownCharacterException);

		/**
		 * Gets mass of the element with an index @index in alphabet.
		 *
		 * @param index Index of the element.
		 * @return Mass of the element.
		 * @see getMass(const std::string& name)
		 */
		mass_type getMass(size_type index) const;

		/**
		 * Gets masses of elements isotopes given by @c isotope_index.
		 * 
		 * @param isotope_index Index of isotope
		 * @return Masses of elements isotopes with the given index.
		 */
		masses_type getMasses(size_type isotope_index = 0) const;

		/**
		 * Gets average masses of elements.
		 * 
		 * @return Average masses of elements.
		 */
		masses_type getAverageMasses() const;

		/**
		 * Returns true if there is an element with symbol 
		 * @c name in the alphabet, false - otherwise.
		 *
		 * @return True, if there is an element with symbol 
		 *         @c name, false - otherwise.
		 */
		bool hasName(const name_type& name) const;

		/**
		 * Adds a new element with symbol @c name and nominal mass @c value 
		 * to the alphabet.
		 *
		 * @param name Symbol of the element to be added.
		 * @param value Nominal mass of the element to be added.
		 * 
		 * @see push_back(const element_type&)
		 */
		void push_back(const name_type& name, mass_type value) {
			push_back(element_type(name, value));
		}

		/**
		 * Adds a new element @c element to the alphabet.
		 *
		 * @param element The @c Element to be added.
		 */
		void push_back(const element_type& element) {
			elements.push_back(element);
		}


		/**
		 * Clears the alphabet data.
		 */
		void clear() { elements.clear(); }


		/**
		 * Sorts the alphabet by names.
		 *
		 * @see sortByValues()
		 */
		virtual void sortByNames();


		/**
		 * Sorts the alphabet by mass values.
		 *
		 * @see sortByNames()
		 */
		virtual void sortByValues();


		/**
		 * Loads the alphabet data from the file @c fname using the default
		 * parser. If there is no file @c fname, throws an @c IOException.
		 *
		 * @param fname The file name to be loaded.
		 * @see load(const std::string& fname, AlphabetParser<>* parser)
		 */
		virtual void load(const std::string& fname) throw (IOException);


		/**
		 * Loads the alphabet data from the file @c fname using @c parser.
		 * If there is no file @c fname found, throws an @c IOException.
		 *
		 * @param fname File name to be loaded.
		 * @param parser Parser to be used by loading.
		 * 
		 * @see load(const std::string& fname)
		 * @see AlphabetParser
		 */
		virtual void load(const std::string& fname, AlphabetParser<>* parser)
			throw (IOException);


		/**
		 * Default destructor.
		 */
		virtual ~Alphabet() { }

	private:
		/**
		 * Elements of the alphabet.
		 */
		container elements;

		/**
		 * Private class-functor to sort out elements in mass ascending order.
		 */
		class MassSortingCriteria {
			public:
				bool operator()(const element_type& el1,
								const element_type& el2) const {
					return el1.getMass() < el2.getMass();
				}
		};

};

/**
 * Prints alphabet to the stream @c os.
 *
 * @param os Output stream to which alphabet is written
 * @param alphabet Alphabet to be written.
 */
std::ostream& operator<<(std::ostream& os, const Alphabet& alphabet);

} // namespace ims

#endif // IMS_ALPHABET_H
