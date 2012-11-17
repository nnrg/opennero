//---------------------------------------------------
// Name: OpenNero : IdNameMap
// Desc:  A template container of objects that is indexed by a name and a unique id
//---------------------------------------------------

#ifndef _CORE_IDNAMEMAP_H_
#define _CORE_IDNAMEMAP_H_

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>
#include <string>
#include <vector>
#include <iostream>

#include "core/ONTypes.h"

namespace OpenNero
{

    using namespace boost::multi_index;

    /// A helper structure to hold things with their names and ids
    template<class T>
    struct IdNameWrapper
    {
            U32 id; ///< id
            std::string name; ///< name
            T payload; ///< a useful payload

            /// create a new id-name object
            /// @param id_ a numeric id
            /// @param name_ a string name
            /// @param payload_ a useful payload
            IdNameWrapper(U32 id_, const std::string& name_, T payload_) :
                id(id_), name(name_), payload(payload_)
            {
            }

            /// human-readable output
            std::ostream& operator<<(std::ostream& os)
            {
                os << id << " "<< name << " "<< std::endl;
                return os;
            }
    };

    /// used to tag the payload index
    struct seq {};

    /// used to tag the name index
    struct name {};

    /// used to tag the id index
    struct id {};

    /// IdNameRegistry is just a trick to allow us to use templates for typedefs
    /// we can now make a IdNameRegistry<Foo>::Set of Foo that is accessible by name and id.
    template<class T>
    struct IdNameRegistry
    {
            typedef IdNameWrapper<T> Wrapper; ///< type of wrapper

            /// Now we define a multi-index for the various payloads
            /// It is accessible as a collection of its payload type, by
            /// its unique name, or by its unique ID
            typedef multi_index_container <
            Wrapper,
            indexed_by <
                // This is just a list of payloads
                sequenced< tag<seq> >,
                // but it can also be accessed by hashed name
                hashed_non_unique< tag<name>, BOOST_MULTI_INDEX_MEMBER(Wrapper, std::string, name) >,
                // or by hashed unique id
                hashed_unique< tag<id>, BOOST_MULTI_INDEX_MEMBER(Wrapper, U32, id) >
                >
            > Set;

            /// this is what a list of these objects might look like to the world
            typedef std::vector<T> ListInterface;

    };

    template<typename Tag, typename MultiIndexContainer>
    void print_out_by(const MultiIndexContainer& s,
                      Tag* =0 /* fixes a MSVC++ 6.0 bug with implicit template function parms */)
    {
        /* obtain a reference to the index tagged by Tag */

        const typename boost::multi_index::index<MultiIndexContainer,Tag>::type
            & i=get<Tag>(s);

        typedef typename MultiIndexContainer::value_type value_type;

        /* dump the elements of the index to cout */

        std::copy(i.begin(), i.end(), std::ostream_iterator<value_type>(std::cout));
    }

}

#endif // #ifndef _CORE_IDNAMEMAP_H_
