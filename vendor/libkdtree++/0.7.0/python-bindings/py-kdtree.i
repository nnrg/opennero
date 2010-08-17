/** \file
 * $Id$
 *
 * Provides a Python interface for the libkdtree++.
 *
 * \author Willi Richert <w.richert@gmx.net>
 *
 */

%module kdtree
 //%include exception.i

%{
#define SWIG_FILE_WITH_INIT
#include "py-kdtree.hpp"
%}


%ignore record_t::operator[];
%ignore operator==;
%ignore operator<<;
%ignore KDTree::KDTree::operator=;
%ignore tac;

#define RECORD_2il record_t<2, int, unsigned long long> // cf. py-kdtree.hpp
#define RECORD_4il record_t<4, int, unsigned long long> // cf. py-kdtree.hpp

#define RECORD_1fl record_t<1, float, unsigned long long>
#define RECORD_3fl record_t<3, float, unsigned long long>
#define RECORD_6fl record_t<6, float, unsigned long long>

////////////////////////////////////////////////////////////////////////////////
// TYPE (int, int)
////////////////////////////////////////////////////////////////////////////////
%typemap(in) RECORD_2il (RECORD_2il temp) {
  if (PyTuple_Check($input)) {

    if (PyArg_ParseTuple($input,"(ii)L",  &temp.point[0], &temp.point[1], &temp.data)!=0) 
    {
      $1 = temp;
    } else {
      PyErr_SetString(PyExc_TypeError,"tuple must have 2 elements: (2dim int vector, long value)");
      return NULL;
    }
    
  } else {
    PyErr_SetString(PyExc_TypeError,"expected a tuple.");
    return NULL;
  } 
 }
 
%typemap(in) RECORD_2il::point_t (RECORD_2il::point_t point) {
  if (PyTuple_Check($input)) {
    if (PyArg_ParseTuple($input,"ii",  &point[0], &point[1])!=0)
    {
      $1 = point;
    } else {
      PyErr_SetString(PyExc_TypeError,"tuple must contain 2 ints");
      return NULL;
    }
    
  } else {
    PyErr_SetString(PyExc_TypeError,"expected a tuple.");
    return NULL;
  } 
 }

%typemap(out) RECORD_2il * {
  RECORD_2il * r = $1;
  PyObject* py_result;

  if (r != NULL) {

    py_result = PyTuple_New(2);
    if (py_result==NULL) {
      PyErr_SetString(PyErr_Occurred(),"unable to create a tuple.");
      return NULL;
    }
    
    if (PyTuple_SetItem(py_result, 0, Py_BuildValue("(ii)", r->point[0], r->point[1]))==-1) {
      PyErr_SetString(PyErr_Occurred(),"(a) when setting element");

      Py_DECREF(py_result);
      return NULL;
    }

    if (PyTuple_SetItem(py_result, 1, Py_BuildValue("L", r->data))==-1) {
      PyErr_SetString(PyErr_Occurred(),"(b) when setting element");

      Py_DECREF(py_result);
      return NULL;
    }
  } else {
    py_result = Py_BuildValue("");
  }

  $result = py_result;
 }

%typemap(out) std::vector<RECORD_2il  >*  {
  std::vector<RECORD_2il >* v = $1;

  PyObject* py_result = PyList_New(v->size());
  if (py_result==NULL) {
    PyErr_SetString(PyErr_Occurred(),"unable to create a list.");
    return NULL;
  }
  std::vector<RECORD_2il  >::const_iterator iter = v->begin();

  for (size_t i=0; i<v->size(); i++, iter++) {
    if (PyList_SetItem(py_result, i, Py_BuildValue("(ii)L", (*iter).point[0], (*iter).point[1], (*iter).data))==-1) {
      PyErr_SetString(PyErr_Occurred(),"(c) when setting element");

      Py_DECREF(py_result);
      return NULL;
    } else {
      //std::cout << "successfully set element " << *iter << std::endl;
    }
  }

  $result = py_result;
 }
////////////////////////////////////////////////////////////////////////////////
// TYPE (int, int, int, int)
////////////////////////////////////////////////////////////////////////////////
%typemap(in) RECORD_4il (RECORD_4il temp) {
  if (PyTuple_Check($input)) {

    if (PyArg_ParseTuple($input,"(iiii)L",  &temp.point[0], &temp.point[1], 
    										&temp.point[2], &temp.point[3], 
										    &temp.data)!=0) 
    {
      $1 = temp;
    } else {
      PyErr_SetString(PyExc_TypeError,"tuple must have 4 elements: (4dim int vector, long value)");
      return NULL;
    }
    
  } else {
    PyErr_SetString(PyExc_TypeError,"expected a tuple.");
    return NULL;
  } 
 }
 
%typemap(in) RECORD_4il::point_t (RECORD_4il::point_t point) {
  if (PyTuple_Check($input)) {
    if (PyArg_ParseTuple($input,"iiii", &point[0], &point[1], &point[2], &point[3])!=0)
    {
      $1 = point;
    } else {
      PyErr_SetString(PyExc_TypeError,"tuple must contain 4 ints");
      return NULL;
    }
    
  } else {
    PyErr_SetString(PyExc_TypeError,"expected a tuple.");
    return NULL;
  } 
 }

%typemap(out) RECORD_4il * {
  RECORD_4il * r = $1;
  PyObject* py_result;

  if (r != NULL) {

    py_result = PyTuple_New(2);
    if (py_result==NULL) {
      PyErr_SetString(PyErr_Occurred(),"unable to create a tuple.");
      return NULL;
    }
    
    if (PyTuple_SetItem(py_result, 0, Py_BuildValue("(iiii)", 
    		r->point[0], r->point[1], r->point[2], r->point[3]))==-1) {
      PyErr_SetString(PyErr_Occurred(),"(a) when setting element");

      Py_DECREF(py_result);
      return NULL;
    }

    if (PyTuple_SetItem(py_result, 1, Py_BuildValue("L", r->data))==-1) {
      PyErr_SetString(PyErr_Occurred(),"(b) when setting element");

      Py_DECREF(py_result);
      return NULL;
    }
  } else {
    py_result = Py_BuildValue("");
  }

  $result = py_result;
 }

%typemap(out) std::vector<RECORD_4il  >*  {
  std::vector<RECORD_4il >* v = $1;

  PyObject* py_result = PyList_New(v->size());
  if (py_result==NULL) {
    PyErr_SetString(PyErr_Occurred(),"unable to create a list.");
    return NULL;
  }
  std::vector<RECORD_4il  >::const_iterator iter = v->begin();

  for (size_t i=0; i<v->size(); i++, iter++) {
    if (PyList_SetItem(py_result, i, Py_BuildValue("(iiii)L", 
    	(*iter).point[0], (*iter).point[1], (*iter).point[2], (*iter).point[3], 
    	(*iter).data))==-1) {
      PyErr_SetString(PyErr_Occurred(),"(c) when setting element");

      Py_DECREF(py_result);
      return NULL;
    } else {
      //std::cout << "successfully set element " << *iter << std::endl;
    }
  }

  $result = py_result;
 }

////////////////////////////////////////////////////////////////////////////////
// TYPE (float)
////////////////////////////////////////////////////////////////////////////////
%typemap(in) RECORD_1fl (RECORD_1fl temp) {
  if (PyTuple_Check($input)) {

    if (PyArg_ParseTuple($input,"(f)L",  
                         &temp.point[0],
                         &temp.data)!=0) 
    {
      $1 = temp;
    } else {
      PyErr_SetString(PyExc_TypeError,"tuple must have 2 elements: (1dim float tuple, long value)");
      return NULL;
    }
    
  } else {
    PyErr_SetString(PyExc_TypeError,"expected a tuple.");
    return NULL;
  } 
 }

%typemap(in) RECORD_1fl::point_t (RECORD_1fl::point_t point) {
  if (PyTuple_Check($input)) {
    if (PyArg_ParseTuple($input,"f",  
                         &point[0])!=0)
    {
      $1 = point;
    } else {
      PyErr_SetString(PyExc_TypeError,"tuple must contain 1 float");
      return NULL;
    }
    
  } else {
    PyErr_SetString(PyExc_TypeError,"expected a tuple.");
    return NULL;
  } 
 }

%typemap(out) RECORD_1fl * {
  RECORD_1fl * r = $1;
  PyObject* py_result;

  if (r != NULL) {

    py_result = PyTuple_New(2);
    if (py_result==NULL) {
      PyErr_SetString(PyErr_Occurred(),"unable to create a tuple.");
      return NULL;
    }
    
    if (PyTuple_SetItem(py_result, 0, Py_BuildValue("(f)", 
                                                    r->point[0]))==-1) {
      PyErr_SetString(PyErr_Occurred(),"(a) when setting element");

      Py_DECREF(py_result);
      return NULL;
    }

    if (PyTuple_SetItem(py_result, 1, Py_BuildValue("L", r->data))==-1) {
      PyErr_SetString(PyErr_Occurred(),"(b) when setting element");

      Py_DECREF(py_result);
      return NULL;
    }
  } else {
    py_result = Py_BuildValue("");
  }

  $result = py_result;
 }

%typemap(out) std::vector<RECORD_1fl  >*  {
  std::vector<RECORD_1fl >* v = $1;

  PyObject* py_result = PyList_New(v->size());
  if (py_result==NULL) {
    PyErr_SetString(PyErr_Occurred(),"unable to create a list.");
    return NULL;
  }
  std::vector<RECORD_1fl  >::const_iterator iter = v->begin();

  for (size_t i=0; i<v->size(); i++, iter++) {
    if (PyList_SetItem(py_result, i, Py_BuildValue("(f)L", 
                                                   (*iter).point[0], 
                                                   (*iter).data))==-1) {
      PyErr_SetString(PyErr_Occurred(),"(c) when setting element");

      Py_DECREF(py_result);
      return NULL;
    } else {
      //std::cout << "successfully set element " << *iter << std::endl;
    }
  }

  $result = py_result;
 }

////////////////////////////////////////////////////////////////////////////////
// TYPE (float, float, float)
////////////////////////////////////////////////////////////////////////////////
%typemap(in) RECORD_3fl (RECORD_3fl temp) {
  if (PyTuple_Check($input)) {

    if (PyArg_ParseTuple($input,"(fff)L",  
                         &temp.point[0], &temp.point[1],
                         &temp.point[2], 
                         &temp.data)!=0) 
    {
      $1 = temp;
    } else {
      PyErr_SetString(PyExc_TypeError,"tuple must have 2 elements: (3dim float tuple, long value)");
      return NULL;
    }
    
  } else {
    PyErr_SetString(PyExc_TypeError,"expected a tuple.");
    return NULL;
  } 
 }
 
%typemap(in) RECORD_3fl::point_t (RECORD_3fl::point_t point) {
  if (PyTuple_Check($input)) {
    if (PyArg_ParseTuple($input,"fff",  
                         &point[0], &point[1],
                         &point[2])!=0)
    {
      $1 = point;
    } else {
      PyErr_SetString(PyExc_TypeError,"tuple must contain 3 floats");
      return NULL;
    }
    
  } else {
    PyErr_SetString(PyExc_TypeError,"expected a tuple.");
    return NULL;
  } 
 }

%typemap(out) RECORD_3fl * {
  RECORD_3fl * r = $1;
  PyObject* py_result;

  if (r != NULL) {

    py_result = PyTuple_New(2);
    if (py_result==NULL) {
      PyErr_SetString(PyErr_Occurred(),"unable to create a tuple.");
      return NULL;
    }
    
    if (PyTuple_SetItem(py_result, 0, Py_BuildValue("(fff)", 
                                                    r->point[0], r->point[1], r->point[2]))==-1) {
      PyErr_SetString(PyErr_Occurred(),"(a) when setting element");

      Py_DECREF(py_result);
      return NULL;
    }

    if (PyTuple_SetItem(py_result, 1, Py_BuildValue("L", r->data))==-1) {
      PyErr_SetString(PyErr_Occurred(),"(b) when setting element");

      Py_DECREF(py_result);
      return NULL;
    }
  } else {
    py_result = Py_BuildValue("");
  }

  $result = py_result;
 }

%typemap(out) std::vector<RECORD_3fl  >*  {
  std::vector<RECORD_3fl >* v = $1;

  PyObject* py_result = PyList_New(v->size());
  if (py_result==NULL) {
    PyErr_SetString(PyErr_Occurred(),"unable to create a list.");
    return NULL;
  }
  std::vector<RECORD_3fl  >::const_iterator iter = v->begin();

  for (size_t i=0; i<v->size(); i++, iter++) {
    if (PyList_SetItem(py_result, i, Py_BuildValue("(fff)L", 
                                                   (*iter).point[0], (*iter).point[1], 
                                                   (*iter).point[2],
                                                   (*iter).data))==-1) {
      PyErr_SetString(PyErr_Occurred(),"(c) when setting element");

      Py_DECREF(py_result);
      return NULL;
    } else {
      //std::cout << "successfully set element " << *iter << std::endl;
    }
  }

  $result = py_result;
 }

////////////////////////////////////////////////////////////////////////////////
// TYPE (float, float, float, float, float, float)
////////////////////////////////////////////////////////////////////////////////
%typemap(in) RECORD_6fl (RECORD_6fl temp) {
  if (PyTuple_Check($input)) {

    if (PyArg_ParseTuple($input,"(ffffff)L",  
                         &temp.point[0], &temp.point[1],
                         &temp.point[2], &temp.point[3],
                         &temp.point[4], &temp.point[5],
                         &temp.data)!=0) 
    {
      $1 = temp;
    } else {
      PyErr_SetString(PyExc_TypeError,"tuple must have 2 elements: (6dim float tuple, long value)");
      return NULL;
    }
    
  } else {
    PyErr_SetString(PyExc_TypeError,"expected a tuple.");
    return NULL;
  } 
 }
 
%typemap(in) RECORD_6fl::point_t (RECORD_6fl::point_t point) {
  if (PyTuple_Check($input)) {
    if (PyArg_ParseTuple($input,"ffffff",  
                         &point[0], &point[1],
                         &point[2], &point[3],
                         &point[4], &point[5])!=0)
    {
      $1 = point;
    } else {
      PyErr_SetString(PyExc_TypeError,"tuple must contain 6 floats");
      return NULL;
    }
    
  } else {
    PyErr_SetString(PyExc_TypeError,"expected a tuple.");
    return NULL;
  } 
 }

%typemap(out) RECORD_6fl * {
  RECORD_6fl * r = $1;
  PyObject* py_result;

  if (r != NULL) {

    py_result = PyTuple_New(2);
    if (py_result==NULL) {
      PyErr_SetString(PyErr_Occurred(),"unable to create a tuple.");
      return NULL;
    }
    
    if (PyTuple_SetItem(py_result, 0, Py_BuildValue("(ffffff)", 
                                                    r->point[0], r->point[1], 
                                                    r->point[2], r->point[3],
                                                    r->point[4], r->point[5]))==-1) {
      PyErr_SetString(PyErr_Occurred(),"(a) when setting element");

      Py_DECREF(py_result);
      return NULL;
    }

    if (PyTuple_SetItem(py_result, 1, Py_BuildValue("L", r->data))==-1) {
      PyErr_SetString(PyErr_Occurred(),"(b) when setting element");

      Py_DECREF(py_result);
      return NULL;
    }
  } else {
    py_result = Py_BuildValue("");
  }

  $result = py_result;
 }

%typemap(out) std::vector<RECORD_6fl  >*  {
  std::vector<RECORD_6fl >* v = $1;

  PyObject* py_result = PyList_New(v->size());
  if (py_result==NULL) {
    PyErr_SetString(PyErr_Occurred(),"unable to create a list.");
    return NULL;
  }
  std::vector<RECORD_6fl  >::const_iterator iter = v->begin();

  for (size_t i=0; i<v->size(); i++, iter++) {
    if (PyList_SetItem(py_result, i, Py_BuildValue("(ffffff)L", 
                                                   (*iter).point[0], (*iter).point[1], 
                                                   (*iter).point[2], (*iter).point[3], 
                                                   (*iter).point[4], (*iter).point[5], 
                                                   (*iter).data))==-1) {
      PyErr_SetString(PyErr_Occurred(),"(c) when setting element");

      Py_DECREF(py_result);
      return NULL;
    } else {
      //std::cout << "successfully set element " << *iter << std::endl;
    }
  }

  $result = py_result;
 }


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

%include "py-kdtree.hpp"

%template () RECORD_2il;
%template (KDTree_2Int)   PyKDTree<2, int,   unsigned long long>;

%template () RECORD_4il;
%template (KDTree_4Int)   PyKDTree<4, int,   unsigned long long>;

%template () RECORD_1fl;
%template (KDTree_1Float) PyKDTree<1, float, unsigned long long>;

%template () RECORD_3fl;
%template (KDTree_3Float) PyKDTree<3, float, unsigned long long>;

%template () RECORD_6fl;
%template (KDTree_6Float) PyKDTree<6, float, unsigned long long>;
