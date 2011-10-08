//--------------------------------------------------------
// OpenNero : RawTemplate
//  an intermediary for object templates
//--------------------------------------------------------

#ifndef _GAME_OBJECTS_RAWTEMPLATE_H_
#define _GAME_OBJECTS_RAWTEMPLATE_H_

#include "tinyxml.h"		// for Xml parsing
#include "core/Common.h"
#include "core/IrrUtil.h"
#include "boost/lexical_cast.hpp"

#include <map>

namespace OpenNero
{
	/**
     * PropertyMap is a class that stores property values at given property
     * specs (paths) for xml files. A user can query these values by specifying
     * a property spec to search for. A property spec is defined by a series
     * of tokens seperated by a '.' . Each token represents an xml element.
     * The syntax A.B.C means that C is a child element of B and B is a child
     * element of A. From this we can quickly specify hierarchies of xml element paths.
     *
     * System.Child.Render means:
     * @verbatim
     * <System>
     *		<Child>
     *			<Render>Value</Render>
     *		</Child>
     *	</System>
     * </>
     * @endverbatim
     */
	class PropertyMap
	{
	public:

		/// a <spec, value> pair (ex: System.PI => <PI, 3.14> )
		typedef std::pair< std::string, std::string > PropValPair;

		/// a vector of PropValPairs
		typedef std::vector< PropValPair > ChildPropVector;

        /// a mapping of attribute name to value
        typedef std::map< std::string, std::string > AttributeMap;

	public:

		// constructors
		PropertyMap();
		PropertyMap( const std::string& xmlFile );

		// are we in a valid state?
		bool isValidState() const;

		// build a property map from a given xml file
		bool constructPropertyMap( const std::string& xmlFile );

		// get all of the children of a given property spec
		// (similar to a wildcard, return ALL children of a given path)
		// NOTE: THIS WILL RETURN THE PROPS IN ALPHABETICAL ORDER, NOT XML ORDER!
		void getPropChildren( ChildPropVector& outVec, const std::string& propertySpec ) const;

		// do we contain this property spec?
		bool hasSection( const std::string& section ) const;

        // does this property spec contain a text value?
        bool hasValue( const std::string& section ) const;

        // does this property spec contain any attributes
        bool hasAttributes( const std::string& section ) const;

		// get a value in a specific format from a property spec with a given converter
		template<typename T, typename Converter >
		bool getValue( T& outVal, const std::string& propertySpec, Converter c = Converter() ) const;

		// get a value in a specific format from a property spec using the default converter
		template<typename T>
		bool getValue( T& outVal, const std::string& propertySpec ) const;

        // get a map of all the attributes at a property spec
        AttributeMap getAttributes( const std::string& propertySpec ) const;

    public:

        // Python methods
        std::string PyGetStringValue( const std::string& propertySpec ) const;

	private:
		
		// get the string value resting at this property spec
		bool getValueString( std::string& outString, const std::string& propertySpec ) const;

		// parse an xml file to build a property map
		bool parseXmlFile( const std::string& xmlFile );		

		// get a value string from a specific xml document
		bool getValueString( const TiXmlDocument& doc, std::string& outString, const std::string& propertySpec ) const;

		// does a specific xml document have a given section?
		const TiXmlElement* hasSection_internal( const TiXmlDocument& doc, const std::string& section ) const;

        // does our xml document chain have a given section?
        const TiXmlElement* xmlChainHasSection( const std::string& section ) const;

		// tokenize a spec
		bool tokenizePropertySpec( std::vector< std::string >& outProps, const std::string& propSpec ) const;

        // performance a query on a property spec
        bool propertySpecQuery( const std::string& section, bool checkValue, bool checkAttributes ) const;

	private:

		// a vector of xml documents
		typedef std::vector<TiXmlDocument>	XmlDocChain;

	private:

		/// The xml document chain, stored from child to parent
		/// (if A uses B uses C then stored A,B,C since C is most ancestral )
		XmlDocChain					mXmlChain;
	};	    

    /**
     * gets the string value at the property spec and then uses a special function
     * object converter to transform the string to the T object. The function object
     * should define an operator as follows:
     *
     *  bool operator() ( T&, const std::string& )
     *
     * @param outVal the output variable
     * @param propertySpec the path to look for the string value at
     * @param c the converter function object
    */
    template<typename T, typename Converter >
    bool PropertyMap::getValue( T& outVal, const std::string& propertySpec, Converter c ) const
    {
        std::string sval;
        if( getValueString( sval, propertySpec ) )
        {
            if( c( outVal, sval ) )
                return true;            
        }
        return false;
    }

	/**
	 * gets the string value at the property spec and then converts that string to T 
	 * using a boost::lexical_cast. To ensure that new T values can be converted,
     * define:
     *      std::istream& operator>>(std::istream& stream, T& obj)
     * where T is your datatype
     *
	 * @param outVal the output variable to store the result value
	 * @param propertySpec the path to the property value	 
	 * @return true if we found the property spec value
	*/
    template<typename T>
	bool PropertyMap::getValue( T& outVal, const std::string& propertySpec ) const
	{
        // try to get the string version from the property spec
        std::string sval;
        if( getValueString( sval, propertySpec ) )
        {
            // try to convert, if we cannot catch the exception
            try
            {
                outVal = boost::lexical_cast<T>(sval);
                return true;
            }

            // dont do anything with the exception,
            // let's just return false
            catch( bad_lexical_cast&){}
        }
        return false;
    }

    /// let's define a specialization for the bool so that we can return true for "TRUE", "TrUe", ... , or "1"
    template<> bool PropertyMap::getValue<bool>( bool& outVal, const std::string& propertySpec ) const;	

} //end OpenNero

#endif // _GAME_OBJECTS_RAWTEMPLATE_H_
