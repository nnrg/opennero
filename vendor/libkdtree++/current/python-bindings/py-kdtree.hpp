/** \file
 * Provides a Python interface for the libkdtree++.
 *
 * \author Willi Richert <w.richert@gmx.net>
 *
 *
 * This defines a proxy to a (int, int) -> long long KD-Tree. The long
 * long is needed to save a reference to Python's object id(). Thereby,
 * you can associate Python objects with 2D integer points.
 * 
 * If you want to customize it you can adapt the following: 
 * 
 *  * Dimension of the KD-Tree point vector.
 *    * DIM: number of dimensions.
 *    * operator==() and operator<<(): adapt to the number of comparisons
 *    * py-kdtree.i: Add or adapt all usages of PyArg_ParseTuple() to reflect the 
 *      number of dimensions.
 *    * adapt query_records in find_nearest() and count_within_range()
 *  * Type of points.
 *    * coord_t: If you want to have e.g. floats you have 
 *      to adapt all usages of PyArg_ParseTuple(): Change "i" to "f" e.g.
 *  * Type of associated data. 
 *    * data_t: currently unsigned long long, which is "L" in py-kdtree.i
 *    * PyArg_ParseTuple() has to be changed to reflect changes in data_t
 * 
 */


#ifndef _PY_KDTREE_H_
#define _PY_KDTREE_H_

#include <kdtree++/kdtree.hpp>

#include <iostream>
#include <vector>
#include <limits>

template <size_t DIM, typename COORD_T, typename DATA_T > 
struct record_t {
  static const size_t dim = DIM;
  typedef COORD_T coord_t;
  typedef DATA_T data_t;

  typedef coord_t point_t[dim];

  inline coord_t operator[](size_t const N) const { return point[N]; }

  point_t point;
  data_t data;
};

////////////////////////////////////////////////////////////////////////////////
// Definition of (int, int) points that has an unsigned long long as payload
////////////////////////////////////////////////////////////////////////////////
#define RECORD_2il record_t<2, int, unsigned long long>
#define KDTREE_TYPE_2il KDTree::KDTree<2, RECORD_2il, std::pointer_to_binary_function<RECORD_2il,int,double> >

inline bool operator==(RECORD_2il const& A, RECORD_2il const& B) {
  return A.point[0] == B.point[0] && A.point[1] == B.point[1] && A.data == B.data;
}

std::ostream& operator<<(std::ostream& out, RECORD_2il const& T)
{
  return out << '(' << T.point[0] << ',' << T.point[1] << '|' << T.data << ')';
}

////////////////////////////////////////////////////////////////////////////////
// Definition of (int, int, int, int) points that has an unsigned long long as payload
////////////////////////////////////////////////////////////////////////////////
#define RECORD_4il record_t<4, int, unsigned long long>
#define KDTREE_TYPE_4il KDTree::KDTree<4, RECORD_4il, std::pointer_to_binary_function<RECORD_4il,int,double> >

inline bool operator==(RECORD_4il const& A, RECORD_4il const& B) {
  return A.point[0] == B.point[0] && A.point[1] == B.point[1] && 
  		 A.point[2] == B.point[2] && A.point[3] == B.point[3] && 
  		 A.data == B.data;
}

std::ostream& operator<<(std::ostream& out, RECORD_4il const& T)
{
  return out << '(' << T.point[0] << ',' << T.point[1] << ',' << T.point[2] << ',' << T.point[3] << '|' << T.data << ')';
}

////////////////////////////////////////////////////////////////////////////////
// Definition of (float) points that has an unsigned long long as payload
////////////////////////////////////////////////////////////////////////////////
#define RECORD_1fl record_t<1, float, unsigned long long>
#define KDTREE_TYPE_1fl KDTree::KDTree<1, RECORD_1fl, std::pointer_to_binary_function<RECORD_1fl,int,double> >

inline bool operator==(RECORD_1fl const& A, RECORD_1fl const& B) {
  return A.point[0] == B.point[0] &&
    A.data == B.data;
}


////////////////////////////////////////////////////////////////////////////////
// Definition of (float, float, float) points that has an unsigned long long as payload
////////////////////////////////////////////////////////////////////////////////
#define RECORD_3fl record_t<3, float, unsigned long long>
#define KDTREE_TYPE_3fl KDTree::KDTree<3, RECORD_3fl, std::pointer_to_binary_function<RECORD_3fl,int,double> >

inline bool operator==(RECORD_3fl const& A, RECORD_3fl const& B) {
  return A.point[0] == B.point[0] && A.point[1] == B.point[1] && 
    A.point[2] == B.point[2] &&
    A.data == B.data;
}

////////////////////////////////////////////////////////////////////////////////
// Definition of (float, float, float, float, float, float) points that has an unsigned long long as payload
////////////////////////////////////////////////////////////////////////////////
#define RECORD_6fl record_t<6, float, unsigned long long>
#define KDTREE_TYPE_6fl KDTree::KDTree<6, RECORD_6fl, std::pointer_to_binary_function<RECORD_6fl,int,double> >

inline bool operator==(RECORD_6fl const& A, RECORD_6fl const& B) {
  return A.point[0] == B.point[0] && A.point[1] == B.point[1] && 
    A.point[2] == B.point[2] && A.point[3] == B.point[3] && 
    A.point[4] == B.point[4] && A.point[5] == B.point[5] && 
    A.data == B.data;
}

////////////////////////////////////////////////////////////////////////////////
// END OF TYPE SPECIFIC DEFINITIONS
////////////////////////////////////////////////////////////////////////////////


template <class RECORD_T>
inline double tac(RECORD_T r, int k) { return r[k]; }

template <size_t DIM, typename COORD_T, typename DATA_T > 
class PyKDTree {
public:

  typedef record_t<DIM, COORD_T, DATA_T> RECORD_T;
  typedef KDTree::KDTree<DIM, RECORD_T, std::pointer_to_binary_function<RECORD_T,int,double> > TREE_T;
  TREE_T tree;

  PyKDTree() : tree(std::ptr_fun(tac<RECORD_T>)) {  };

  void add(RECORD_T T) { tree.insert(T); };

  /**
     Exact erase.
  */
  bool remove(RECORD_T T) { 
    bool removed = false;

    typename TREE_T::const_iterator it = tree.find_exact(T);
    if (it!=tree.end()) {
      tree.erase_exact(T); 
      removed = true;
    }
    return removed;
  };

  int size(void) { return tree.size(); }

  void optimize(void) { tree.optimise(); }
  
  RECORD_T* find_exact(RECORD_T T) {
    RECORD_T* found = NULL;
    typename TREE_T::const_iterator it = tree.find_exact(T);
    if (it!=tree.end())
      found = new RECORD_T(*it);

    return found;
  }

  size_t count_within_range(typename RECORD_T::point_t T, typename TREE_T::distance_type range) {
    RECORD_T query_record;
    memcpy(query_record.point, T, sizeof(COORD_T)*DIM);

    return tree.count_within_range(query_record, range);
  }

  std::vector<RECORD_T > find_within_range(typename RECORD_T::point_t T, typename TREE_T::distance_type range) {
    RECORD_T query_record;
    memcpy(query_record.point, T, sizeof(COORD_T)*DIM);

    std::vector<RECORD_T> v;
    tree.find_within_range(query_record, range, std::back_inserter(v));
    return v;
  }

  RECORD_T* find_nearest (typename RECORD_T::point_t T) {
    RECORD_T* found = NULL;
    RECORD_T query_record;
    memcpy(query_record.point, T, sizeof(COORD_T)*DIM);

    std::pair<typename TREE_T::const_iterator, typename TREE_T::distance_type> best = 
      tree.find_nearest(query_record, std::numeric_limits<typename TREE_T::distance_type>::max());

    if (best.first!=tree.end()) {
      found = new RECORD_T(*best.first);
    }
    return found;
  }

  std::vector<RECORD_T >* get_all() {
    std::vector<RECORD_T>* v = new std::vector<RECORD_T>;

    for (typename TREE_T::const_iterator iter=tree.begin(); iter!=tree.end(); ++iter) {
      v->push_back(*iter);
    }

    return v;
  }

  size_t __len__() { return tree.size(); }
};
#endif //_PY_KDTREE_H_
