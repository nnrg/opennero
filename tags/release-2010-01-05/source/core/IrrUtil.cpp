//---------------------------------------------------
// Name: OpenNero : IrrUtil
//  Irrlicht Utility includes
//---------------------------------------------------

#include "core/Common.h"
#include "core/IrrUtil.h"
#include "core/IrrSerialize.h"
#include "scripting/scriptIncludes.h"

namespace OpenNero
{
    /// the pickling suite for the Vector class
    template <typename T>
    struct irr_vector3d_pickle_suite : boost::python::pickle_suite
    {
        /// 3D vector from Irrlicht
        typedef irr::core::vector3d<T> VectorType;
        /// Python Tuple
        typedef boost::python::tuple   PyTuple;
        /// Python Object
        typedef boost::python::object  PyObject;


        /// Create a python tuple from a vector
        static PyTuple getinitargs(const VectorType& v)
        {   
            return boost::python::make_tuple( v.X, v.Y, v.Z );
        }

        /// Return a python tuple representing a python objects state
        static PyTuple getstate(PyObject obj)
        {   
            VectorType const& v = extract<VectorType const&>(obj)();
            return boost::python::make_tuple(obj.attr("__dict__"), v.X, v.Y, v.Z );
        }

        /// Set the state of a python object
        static void setstate( PyObject obj, PyTuple state)
        {   
            using namespace boost::python;

            // get the vector from the py object
            VectorType& v = extract<VectorType&>(obj)();
            
            // make sure the tuple is proper format            
            if (len(state) != 4)
            {
              PyErr_SetObject(PyExc_ValueError, ("expected 2-item tuple in call to __setstate__; got %s" % state).ptr() );
              throw_error_already_set();
            }
            
            // restore the object's __dict__
            dict d = extract<dict>(obj.attr("__dict__"))();
            d.update(state[0]);
            
            // restore the internal state of the C++ object
            v.X = extract<T>(state[1]);
            v.Y = extract<T>(state[2]);
            v.Z = extract<T>(state[3]);
        }
        
        /// Ensure that getstae manages the objects dict properly
        static bool getstate_manages_dict() { return true; }
    };

    // the pickling suite for the SColor class
    struct irr_SColor_pickle_suite : boost::python::pickle_suite
    {
        typedef boost::python::tuple   PyTuple;
        typedef boost::python::object  PyObject;

        static PyTuple getinitargs(const SColor& col)
        {   
            return boost::python::make_tuple( col.getAlpha(), col.getRed(), col.getGreen(), col.getBlue() );
        }

        static PyTuple getstate(PyObject obj)
        {   
            SColor const& col = extract<SColor const&>(obj)();
            return boost::python::make_tuple(obj.attr("__dict__"), col.getAlpha(), col.getRed(), col.getGreen(), col.getBlue() );
        }

        static void setstate( PyObject obj, PyTuple state)
        {   
            using namespace boost::python;

            // get the color from the py object
            SColor& col = extract<SColor&>(obj)();
            
            // make sure the tuple is proper format            
            if (len(state) != 5)
            {
              PyErr_SetObject(PyExc_ValueError, ("expected 4-item tuple in call to __setstate__; got %s" % state).ptr() );
              throw_error_already_set();
            }
            
            // restore the object's __dict__
            dict d = extract<dict>(obj.attr("__dict__"))();
            d.update(state[0]);
            
            // restore the internal state of the C++ object
            col.setAlpha( extract<uint32_t>(state[1]) );
            col.setRed( extract<uint32_t>(state[2]) );
            col.setGreen( extract<uint32_t>(state[3]) );
            col.setBlue( extract<uint32_t>(state[4]) );
        }

        static bool getstate_manages_dict() { return true; }
    };

    namespace
    {
        template <typename PosClass, typename PosType>
        void ExportPos2( const char* name, const char* desc )
        {
            class_<PosClass>(name, desc, init<PosType,PosType>() )
                .def(init<>())
                .def_readwrite("x", &PosClass::X)
                .def_readwrite("y", &PosClass::Y)
                .def(self += self)
                .def(self + self)
                .def(self -= self)
                .def(self - self)
            ;
        }

        template <typename RectClass, typename RectType, typename PosClass>
        void ExportRect2( const char* name, const char* desc )
        {
            class_<RectClass>(name, desc, init<RectType,RectType,RectType,RectType>() )
                .def( init<PosClass,PosClass>() )
                .def( init<>() )
                .def_readwrite( "UpperLeftCorner", &RectClass::UpperLeftCorner )
                .def_readwrite( "LowerRightCorner", &RectClass::LowerRightCorner )
            ;
        }
    }

    /// export the Irrlicht utilities to python
    PYTHON_BINDER( IrrUtil )
    {
        using namespace boost::python;            

        // a vector class
        class_<Vector3f>("Vector3f", "A three-dimensional vector", init<float32_t, float32_t, float32_t>())
            .def_readwrite("x", &Vector3f::X)
            .def_readwrite("y", &Vector3f::Y)
            .def_readwrite("z", &Vector3f::Z)
            .def(-self)
            .def(self += self)
            .def(self + self)
            .def(self -= self)
            .def(self - self)
            .def(self *= float32_t())
            .def(self * float32_t())
            .def(self / float32_t())
            .def(self /= float32_t())
            .def("getDistanceFrom", &Vector3f::getDistanceFrom, "Get distance from another point")            
            .def("normalize", &Vector3f::normalize, return_value_policy<reference_existing_object>(), "Normalize this vector")
            .def("getLength", &Vector3f::getLength, "length of this vector")
            .def("dotProduct", &Vector3f::dotProduct, "dot product with another vector")
            .def("crossProduct", &Vector3f::crossProduct, "cross product with another vector")
            .def("setLength", &Vector3f::setLength, return_value_policy<reference_existing_object>(), "sets the length of the vector to a new value")
            .def(self_ns::str(self))
            .def_pickle(irr_vector3d_pickle_suite<float32_t>())
        ;

        // position2D classes
        ExportPos2< Pos2i, int32_t >  ( "Pos2i", "A two dimensional integer position." );
        ExportPos2< Pos2f, float32_t >( "Pos2f", "A two dimensional float position." );

        // rectangle classes
        ExportRect2< Rect2i, int32_t, Pos2i >  ( "Rect2i", "An integer rectangle." );
        ExportRect2< Rect2f, float32_t, Pos2f >( "Rect2f", "A float rectangle." );

        // export SColor
        class_<SColor>("Color", "An argb unsigned byte color", init<uint32_t,uint32_t,uint32_t,uint32_t>() )
            .def( init<>() )
            .add_property( "r", &SColor::getRed,   &SColor::setRed )
            .add_property( "g", &SColor::getGreen, &SColor::setGreen )
            .add_property( "b", &SColor::getBlue,  &SColor::setBlue )
            .add_property( "a", &SColor::getAlpha, &SColor::setAlpha )
            .def_pickle(irr_SColor_pickle_suite())
        ;
    }
        
    
} // end OpenNero
