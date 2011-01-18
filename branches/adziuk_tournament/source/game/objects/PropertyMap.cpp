//--------------------------------------------------------
// OpenNero : RawTemplate
//  an intermediary for object templates
//--------------------------------------------------------

#include "core/Common.h"
#include "PropertyMap.h"
#include "game/Kernel.h"
#include <boost/tokenizer.hpp>

namespace OpenNero
{
	/// default do nothing constructor
	PropertyMap::PropertyMap() {}

	/**
	 * Constructor that attempts to build the property map from an xml file.
	 * If the building of the map fails, isValidState should return false
	 * @param xmlFile the path to the xml file
	*/
	PropertyMap::PropertyMap( const std::string& xmlFile )
	{
		if( !constructPropertyMap( xmlFile ) )
		{
			mXmlChain.clear();
			AssertMsg( !isValidState(), "Logic Error, should be invalid" );
		}
	}

	/**
	 * Gets the state of the property map. Returns true if we have any xml documents loaded
	 * @return true if we have a valid xml document chain
	*/
	bool PropertyMap::isValidState() const
	{
		return mXmlChain.size() != 0;
	}

	/**
	 * Construct a property map from an xml file
	 * @param xmlFile the path to the xml file. This should be the mod path,
	 *				  we will not add the mod directory to it. We will, however, add the mod
	 *				  path to any file it 'Uses'
	 * @return true if we constructed the property map successfully
	*/	
	bool PropertyMap::constructPropertyMap( const std::string& xmlFile )
	{
		// clear the old map
		mXmlChain.clear();
		Assert( !isValidState() );

		// make our new valid
		if( !parseXmlFile( xmlFile ) )
		{
			mXmlChain.clear();
			Assert( !isValidState() );
			return false;
		}

		// all good
		return true;
	}

	/**
	 * Get all the children of a given spec. In other words, perform a wildcard query for all
	 * children (spec.*) and return the results in a vector of <props,values>. 
	 *
	 * NOTE: THIS WILL RETURN THE PROPS IN ALPHABETICAL ORDER, NOT XML ORDER! The reason
	 * for this is that properties can be spread out among multiple files due to the 'Uses'
	 * function, therefore the ordering of xml elements could be variable:
	 *
	 * example:
     * @verbatim
	 *  foo1.xml
	 *   <Test>
	 *		<A>A</A>
	 *		<B>B</B>
	 *		<C>C</C>
	 *   </Test>
	 *	foo2.xml
	 *   <Test>
	 *		<Uses>foo1.xml</Test>	 
	 *		<C>CC</C>
	 *		<A>AA</B>
	 *   </Test>
     *
	 * getPropChildren( v, "Test" ) => { <A,AA> , <B,B>, <C,CC>, <Uses, foo1.xml> }
     * @endverbatim
	 *
	 * @param outVec the output variable to store the result of this query
	 * @param propertySpec the spec to search for children on
	*/
	void PropertyMap::getPropChildren( ChildPropVector& outVec, const std::string& propertySpec ) const
	{
		// make a map to store our property -> value map
		typedef std::map< std::string, std::string > PropSpecChildMap;
		PropSpecChildMap propSpecChildMap;
		
		XmlDocChain::const_reverse_iterator ritr = mXmlChain.rbegin();
		XmlDocChain::const_reverse_iterator rend = mXmlChain.rend();		

		// iterator through all documents, starting with oldest
		for( ; ritr != rend; ++ritr )
		{
			const TiXmlElement* elem = hasSection_internal( *ritr, propertySpec );
			if( elem )
			{
				const TiXmlElement* child = elem->FirstChildElement();
				while( child )
				{
					// add this property to our map
                    Assert( child->Value() );                    
					std::string prop = child->Value();

                    if( child->GetText() )
                    {   
					    propSpecChildMap[prop] = child->GetText();
                    }

					child = child->NextSiblingElement();
				}
			}
		}

		// now flatten the map
		outVec.clear();

		PropSpecChildMap::const_iterator itr = propSpecChildMap.begin();
		PropSpecChildMap::const_iterator end = propSpecChildMap.end();

		for( ; itr != end; ++itr )
		{
			outVec.push_back( PropValPair( itr->first, itr->second ) );
		}		
	}

	/**
	 * Do we contain a given property spec?
	 * @param section the property spec to search for
	 * @return true if the property spec exists
	*/
	bool PropertyMap::hasSection( const std::string& section ) const
	{
        return propertySpecQuery(section,false,false);
	}

    /**
     * Does the given property spec contain an inner text value?
     * @param section the property spec to search for a value in
     * @return true if the given spec has a value
    */
    bool PropertyMap::hasValue( const std::string& section ) const
    {
        return propertySpecQuery(section,true,false);
    }

    /**
     * Does the given property spec contain any attributes
     * @param section the property spec to search for attributes in
     * @return true if the given spec has any attributes
    */
    bool PropertyMap::hasAttributes( const std::string& section ) const
    {
        return propertySpecQuery(section,false,true);
    }


    /**
     * Get all the attributes associated with a property spec     
     * @param propertySpec the spec to search for attributes in
     * @return a map of the attributes for the property spec
    */
    PropertyMap::AttributeMap PropertyMap::getAttributes( const std::string& propertySpec ) const
    {
        // clear the old vector
        AttributeMap attMap;        

        // iterate through our xml documents, child to parent looking for the value
        const TiXmlElement* elem = xmlChainHasSection(propertySpec);

        if( elem )
        {
            const TiXmlAttribute* att = elem->FirstAttribute();
            
            while( att )
            {
                AssertWarnMsg( att->Name()  != NULL, "Invalid attribute name when looking at " << propertySpec );
                AssertWarnMsg( att->Value() != NULL, "Invalid attribute value when looking at " << propertySpec );

                // save the attribute
                if( att->Name() && att->Value() )
                {
                    attMap[ att->Name() ] = att->Value();                    
                }

                // move
                att = att->Next();
            }
        }

        return attMap;
    }

    /**
     * A python utility function that will get a string value from a given property spec
     * @param propertySpec the path to search for the value
     * @return a value or "" if nothing found
    */
    std::string PropertyMap::PyGetStringValue( const std::string& propertySpec ) const
    {
        std::string val;
        if( getValue(val, propertySpec) )
            return val;

        return "";
    }
	
	/**
	 * Get the value at a property spec in string form
	 * @param outString the output variable to store the string
	 * @param propertySpec the path to search for the value at
	 * @return true if we got the value string
	*/
	bool PropertyMap::getValueString( std::string& outString, const std::string& propertySpec ) const
	{
		Assert( isValidState() );

		// iterate through our xml documents, child to parent looking for the value
		XmlDocChain::const_iterator itr = mXmlChain.begin();
		XmlDocChain::const_iterator end = mXmlChain.end();		

		for( ; itr != end; ++itr )
		{
			// try to find that spec in this document
			if( getValueString( *itr, outString, propertySpec ) )
				return true;
		}

		// no luck
		outString = "";
		return false;
	}	
		
	/**
	 * Parse an xml file to build the property map. We add the parsed documents
	 * to our xml chain in this method as well as recursing on all 'Uses' elements,
	 * so only one call from construct is needed for this method
	 * @param xmlFile the xml file to load
	 * @return true if we parsed the xml files successfully
	*/
	bool PropertyMap::parseXmlFile( const std::string& xmlFile )
	{
		TiXmlDocument doc;

		// open the file
		if( !doc.LoadFile( xmlFile.c_str() ) )
			return false;

		// add ourselves to the chain
		mXmlChain.push_back(doc);

		// does this use doc contain a Uses section?
		std::string useThis;
		if( getValueString( doc, useThis, "Template.Uses" ) )
		{
			// convert this to a mod path
			useThis = Kernel::findResource( useThis );

			// try to parse this xml file			
			if( !parseXmlFile( useThis ) )
				return false;
		}

		// all good
		return true;
	}

	/**
	 * Try to get the value string some a specific document
	 * @param doc the xml document to search
	 * @param the output variable to store the strig in
	 * @param propertySpec the path to search for in the document
	 * @return true if we got a valid string at that path
	*/
	bool PropertyMap::getValueString( const TiXmlDocument& doc, std::string& outString, const std::string& propertySpec ) const
	{
		// do we have the section?
		const TiXmlElement* elem = hasSection_internal( doc, propertySpec );

		// if the element has text, retrieve it
        if( elem && elem->GetText() )
		{
			outString = elem->GetText();
			return true;
		}
		return false;
	}

	/**
	 * See if we have a given property spec in an xml document
	 * @param doc the document to search in
	 * @param section the property spec to search for in the doc
	 * @return the TiXmlElement node that contains the section, NULL otherwise
	*/
	const TiXmlElement* PropertyMap::hasSection_internal( const TiXmlDocument& doc, const std::string& section ) const	
	{
		// try to tokenize our section "x.y.z" by .
		std::vector< std::string > props;
		if( !tokenizePropertySpec( props, section ) )
			return NULL;

		Assert( props.size() > 0 );

		// iterate over all the tokens to make sure we have the sections
		std::vector< std::string >::iterator itr = props.begin();
		std::vector< std::string >::iterator end = props.end();

		// get the template node
		const TiXmlNode* node = doc.FirstChild( itr->c_str() );
		if( !node )
			return NULL;

		// convert to an element
		const TiXmlElement* elem = node->ToElement();
		if( !elem )
			return NULL;

		// if we are done, return
		++itr;
		if( itr == end )
			return elem;
		
		// ok, now we can do normal iteration
		for( ; itr != end; ++itr )
		{
			Assert( elem );

			// get the next element
			elem = elem->FirstChildElement( itr->c_str() );
			if( !elem )
				return NULL;
		}		

		// good, we found everything
		return elem;
	}

    /**
     * Search the entire xml chain child to parent looking for a section
     * @param section the property spec to search the chain for
     * @return an xml element if the section is found, NULL otherwise
    */
    const TiXmlElement* PropertyMap::xmlChainHasSection( const std::string& section ) const
    {
        Assert( isValidState() );

        // iterate through our xml documents, child to parent looking for the value
		XmlDocChain::const_iterator itr = mXmlChain.begin();
		XmlDocChain::const_iterator end = mXmlChain.end();		
		
		for( ; itr != end; ++itr )
		{
			// try to find it in this document
			const TiXmlElement* elem = hasSection_internal( *itr, section );

            if( elem )
            {
                return elem;
            }
		}

        return NULL;
    }
	
	/**
	 * Tokenize a given property spec by splitting at '.' using boost::tokenizer
	 * @param outProps the vector of strings to store the tokens in
	 * @param propSpec the spec to split up
	 * @return true if we have valid tokens
	*/
	bool PropertyMap::tokenizePropertySpec( std::vector< std::string >& outProps, const std::string& propSpec ) const
	{
		// adapted from example @ http://www.boost.org/libs/tokenizer/char_separator.htm		
		typedef boost::tokenizer< boost::char_separator<char> > tokenizer;
		boost::char_separator<char> sep(".");
		tokenizer tokens( propSpec, sep );

		// clear the out vector
		outProps.clear();

		// add our specs to the vector
		for( tokenizer::iterator itr = tokens.begin(); itr != tokens.end(); ++itr )
		{
			outProps.push_back(*itr);
		}

		return outProps.size() != 0;
	}

    /**
     * Perform a query on a property spec. By default always checks for existence.
     * @param checkValue if true, checks if the property spec element has an inner text value
     * @param checkAttributes if true, checks if the property spec element has any attributes
     * @return true if the property spec element exists and meets the given checks
    */
    bool PropertyMap::propertySpecQuery( const std::string& section, bool checkValue, bool checkAttributes ) const
    {
        Assert( isValidState() );

		// iterate through our xml documents, child to parent looking for the value
        const TiXmlElement* elem = xmlChainHasSection(section);
            
        // do query checks on element
        return  ( elem != NULL  ) && ( !checkValue || elem->GetText() ) &&  ( !checkAttributes || elem->FirstAttribute() );				
    }

    /// Template specialization to turn the strings "trUe" (or any variation) into true or false
    template <>
    bool PropertyMap::getValue<bool>( bool& outVal, const std::string& propertySpec ) const
	{
        // try to get the string version from the property spec
        std::string sval;
        if( getValueString( sval, propertySpec ) )
        {
            std::transform( sval.begin(), sval.end(), sval.begin(), ::tolower );
            outVal = ( sval == "true" || sval == "1" );
        }
        return false;
    }


} //end OpenNero

