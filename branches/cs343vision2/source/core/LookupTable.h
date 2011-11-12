//--------------------------------------------------------
// OpenNero : LookupTable
//  a table to map entries to
//--------------------------------------------------------

#ifndef _CORE_LOOKUPTABLE_H_
#define _CORE_LOOKUPTABLE_H_

#include "Common.h"
#include "HashMap.h"

namespace OpenNero 
{   
    /**
     * A lookup table is nearly identical to a map or a hash_map with one slight difference. The lookup table
     * promises to store all of the added entries in linear memory system and give access to where elements are
     * stored. In this way, I can access the elements either through a key or through a direct index. Because the
     * user may expect these index mappings to remain valid throughout the program use, individual elements should 
	 * never be removed from the lookup table, as that may cause an invalidation of the indices. An entire clear
	 * is available though, as long as all parties agree upon it.
     *
     * Note that most of the working methods are declared protected so that later classes can override the public ones
     * without the virtual overhead and still maintain the same functionality.
     *
     * For the template parameters
	 *  Key is the thing you want to do lookups with
	 *  Value is the thing you want to store
	 *  IndexType is the data type you want to index into the table with
	 *		(Make Sure that this is SIGNED, we return -1 on fail)
	 *  Map is the mapping container that maps Keys to IndexTypes
	 */
	 
	// 5/10/07 - __gnu_cxx is a little wonky, hashmap won't work with std::string. Until we resolve this, go with std::map
    //template <typename Key, typename Value, typename IndexType = int32_t, typename Map = hash_map<Key,IndexType> >
    
    template <typename Key, typename Value, typename IndexType = int32_t, typename Map = std::map<Key,IndexType> >
    class LookupTable
    {

    private:
     
        // our index accessor
        typedef Map IndexMap;        

        // vector to hold our procedure handlers (indexed by indices)
        typedef std::vector<Value> EntryVector;

	private:

        IndexMap                    mIndexMap;      ///< Hashmap that maps Keys to indices
        EntryVector                 mEntries;       ///< Container for procedure handlers	

    protected:

		// internal helper methods, override the public ones if you need different functionality

		/**
		 * intern_find finds an entry in the table by its key
		 * @param key the key to search for
		 * @return a number corresponding to an index to access the element
		*/
        IndexType intern_find( const Key& key ) const
		{
			typename IndexMap::const_iterator itr;	

			if( ( itr = mIndexMap.find(key) ) != mIndexMap.end() )        
				return itr->second;        

			return (IndexType)-1;
		}

		/**
		 * intern_add will add a value to the table under a given key
		 * @param key the key to add the value under
		 * @param val the value to add
		 * @return the index of the entry that was added, -1 on fail
		*/
        IndexType intern_add( const Key& key, const Value& val )
		{
			IndexType idx;			

			// if we already are using this key, replace whatever is mapped there
			if( ( idx = intern_find(key) ) != -1 )
			{
				AssertMsg( idx >= 0 && idx < (IndexType)mEntries.size(), "Invalid table index!" );			
				mEntries[idx] = val;
				return idx;
			}		

			// map to the end of the table
			idx = (IndexType)mEntries.size();
			mIndexMap[key] = idx;
			mEntries.push_back(val);

			// return our new index
			return idx;
		}

		/**
		 * Get the value from the table at the index
		 * @param idx the index to look into the table with
		 * @param val the storage reference
		 * @return true if we found the value, false otherwise
		*/
        bool intern_get_by_index( IndexType idx, Value& val ) const
		{
			if( idx >= 0 && idx < (IndexType)mEntries.size() )
			{
				val = mEntries[idx];
				return true;
			}
			return false;
		}    	

    public:		

        /// get the index of this procedure in the table
        IndexType find( const Key& key ) const
		{
			 return intern_find(key);
		}

        /// add a procedure to the table mapped by key
        IndexType add( const Key& key, const Value& val )
		{
		    IndexType ret = intern_add(key,val);
		    Assert( find(key) == ret );
			return ret;
		}

        /// get a procedure from the table using an index
        bool getByIndex( IndexType index, Value& value ) const
		{
			return intern_get_by_index( index, value );
		}

        /// get a procedure from the table using a key
        bool getByKey( const Key& key, Value& value ) const
		{
			return intern_get_by_index( intern_find(key), value );
		}   

		/// clear the table
		void clearAllEntries()
		{
			mIndexMap.clear();
			mEntries.clear();			
		}

        /// get the number of entries in the table
        uint32_t getNumEntries() const
        {
	        return (uint32_t)mEntries.size();
        }
    };
       
	void LookupTable_UnitTest();
    
} //end OpenNero

#endif // _CORE_LOOKUPTABLE_H_
