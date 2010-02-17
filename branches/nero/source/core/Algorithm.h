//---------------------------------------------------
// Name: OpenNero : Algorithm
// Desc:  various algorithms not provided by std libraries
//---------------------------------------------------

#ifndef _CORE_ALGORITHM_H_
#define _CORE_ALGORITHM_H_

#include <algorithm>

namespace OpenNero
{
    // is_sorted is supported by __gnu_cxx on the linux build, but there is no equivalent
    // on the others builds, so it is added here for uniformity in code expectation.

    /// Determine if a set of values specified by the interval [cur,last) are
    /// sorted in an increasing order as defined by the < operator of the value
    /// @param cur the first iterator of the range
    /// @param last the last iterator of the range (non-inclusive)
    /// @return true if the range of values is sorted in an increasing order
    template <typename ForwardIterator>
    bool is_sorted( ForwardIterator cur, ForwardIterator last )
    {
        if( cur != last )
        {
            for( ForwardIterator prev = cur++; cur != last; ++prev, ++cur )
                if( !(*prev <= *cur) )
                    return false;
        }

        return true;
    }

    /// Determine if a set of values specified by the interval [cur,last) are
    /// sorted in an increasing order as defined by the ScrictWeakOrdering functor
    /// @tparam ForwardIterator the forward iterator for the collection
    /// @tparam StrictWeakOrdering a strict weak ordering comparison operator
    /// @param cur the first iterator of the range
    /// @param last the last iterator of the range (non-inclusive)
    /// @param comp the strict weak comparator
    /// @return true if the range of values is sorted in an increasing order
    template <typename ForwardIterator, typename StrictWeakOrdering>
    bool is_sorted( ForwardIterator cur, ForwardIterator last, StrictWeakOrdering comp )
    {
        if( cur != last )
        {
            for( ForwardIterator prev = cur++; cur != last; ++prev, ++cur )
                if( !comp(*prev,*cur) )
                    return false;
        }

        return true;
    }


} // end OpenNero

#endif //end _CORE_ALGORITHM_H_
