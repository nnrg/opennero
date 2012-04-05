/**
   @file exports.cpp

   @brief export OpenNERO scripts into Python

   In order to export more Python classes and/or functions, add your own
   ExportXXXScripts method *AND* call it from ExportScripts
*/

#include "core/Common.h"
#include "core/BoostCommon.h"
#include "scripting/scriptIncludes.h"

// this section has one include for each ExportXXXScripts function
#include "ai/AI.h"
#include "ai/AIManager.h"
#include "ai/AgentBrain.h"
#include "ai/rl/TD.h"
#include "ai/rl/Sarsa.h"
#include "ai/rl/QLearning.h"
#include "ai/Environment.h"
#include "ai/rtneat/rtNEAT.h"
#include "ai/sensors/Sensor.h"
#include "ai/sensors/RaySensor.h"
#include "ai/sensors/RadarSensor.h"
#include "ai/sensors/SensorArray.h"
#include "core/IrrUtil.h"
#include "game/Kernel.h"
#include "game/objects/PropertyMap.h"
#include "scripting/Scheduler.h"
#include "scripting/scripting.h"
#include "game/SimEntityData.h"
#include "game/SimContext.h"
#include "input/IOMapping.h"
#include "render/Camera.h"

// GUI is exported separately in gui/GuiExports.cpp
#include "gui/GuiExports.h"

namespace b = boost;
namespace py = boost::python;

namespace OpenNero {
	namespace scripting {

		/**
         * Export Agent-specific script components
         */
		void ExportAgentBrainScripts()
		{
			// export the interface to python so that we can override its methods there
			py::class_<PyAgentBrain, b::noncopyable, PyAgentBrainPtr>("AgentBrain", "Abstract brain for an AI agent")
				.def("initialize", pure_virtual(&AgentBrain::initialize), "Called before learning starts")
				.def("start", pure_virtual(&AgentBrain::start), "Called at the beginning of a learning episode")
				.def("act", pure_virtual(&AgentBrain::act), "Called for every step of the state-action loop")
				.def("end", pure_virtual(&AgentBrain::end), "Called at the end of a learning episode")
				.def("destroy", pure_virtual(&AgentBrain::destroy), "Called after learning ends")
                .def("add_sensor", &AgentBrain::add_sensor, "Add a sensor for this agent")
                .def("skip", &AgentBrain::Skip, "Causes the next call to act to be skipped")
                .def("teleport", &AgentBrain::Teleport, "Causes the agent to ignore collisions and to be placed exactly where specified by state")
                .def_readonly("step", &AgentBrain::step, "Current step count")
				.def_readonly("episode", &AgentBrain::episode, "Current episode count")
				.def_readonly("fitness", &AgentBrain::fitness, "Cumulative reward for this episode")
				.add_property("state", make_function(&AgentBrain::GetSharedState, return_value_policy<reference_existing_object>()), "Body of the agent");
			// export the interface to python so that we can override its methods there
			py::class_<TDBrain, noncopyable, bases<AgentBrain>, TDBrainPtr >("TDBrain", "Time-Difference RL agent", no_init )
				.def("initialize", &TDBrain::initialize, "Called before learning starts")
				.def("start", &TDBrain::start, "Called at the beginning of a learning episode")
				.def("act", &TDBrain::act, "Called for every step of the state-action loop")
				.def("end", &TDBrain::end, "Called at the end of a learning episode")
				.def("destroy", &TDBrain::destroy, "Called after learning ends")
				.add_property("epsilon", &TDBrain::getEpsilon, &TDBrain::setEpsilon)
				.add_property("alpha", &TDBrain::getAlpha, &TDBrain::setAlpha)
				.add_property("gamma", &TDBrain::getGamma, &TDBrain::setGamma)
				.add_property("state", make_function(&TDBrain::GetSharedState, return_value_policy<reference_existing_object>()), "Body of the agent");
			// export the interface to python so that we can override its methods there
			py::class_<SarsaBrain, bases<TDBrain>, SarsaBrainPtr >("SarsaBrain", "SARSA RL agent", init<double, double, double, double, int, int, int, int>() )
				.def("initialize", &SarsaBrain::initialize, "Called before learning starts")
				.def("start", &SarsaBrain::start, "Called at the beginning of a learning episode")
				.def("act", &SarsaBrain::act, "Called for every step of the state-action loop")
				.def("end", &SarsaBrain::end, "Called at the end of a learning episode")
				.add_property("epsilon", &TDBrain::getEpsilon, &TDBrain::setEpsilon)
				.add_property("alpha", &TDBrain::getAlpha, &TDBrain::setAlpha)
				.add_property("gamma", &TDBrain::getGamma, &TDBrain::setGamma)
				.add_property("state", make_function(&SarsaBrain::GetSharedState, return_value_policy<reference_existing_object>()), "Body of the agent");
			// export the interface to python so that we can override its methods there
			py::class_<QLearningBrain, bases<TDBrain>, QLearningBrainPtr >("QLearningBrain", "Q-Learning RL agent", init<double, double, double, int, int, int, int>() )
				.def("initialize", &QLearningBrain::initialize, "Called before learning starts")
				.def("start", &QLearningBrain::start, "Called at the beginning of a learning episode")
				.def("act", &QLearningBrain::act, "Called for every step of the state-action loop")
				.def("end", &QLearningBrain::end, "Called at the end of a learning episode")
				.def("destroy", &QLearningBrain::destroy, "Called after learning ends")
                .def("to_string", &TDBrain::to_string, "Called to serialize to a string")
                .def("from_string", &TDBrain::from_string, "Called to serialize from a string")
				.add_property("epsilon", &TDBrain::getEpsilon, &TDBrain::setEpsilon)
				.add_property("alpha", &TDBrain::getAlpha, &TDBrain::setAlpha)
				.add_property("gamma", &TDBrain::getGamma, &TDBrain::setGamma)
				.add_property("state", make_function(&QLearningBrain::GetSharedState, return_value_policy<reference_existing_object>()), "Body of the agent");
			;
		}

        void ExportSensorScripts()
        {
			py::class_<PySensor, noncopyable, SensorPtr>(
                "Sensor",
                "Abstract sensor base class",
                no_init)
				.def(self_ns::str(self_ns::self))
                ;
            py::class_<RaySensor, noncopyable, bases<Sensor>, RaySensorPtr>(
                "RaySensor",
                "A ray sensor that returns the distance to the closest object it intersects",
                init<double, double, double, double, U32, bool>())
				.def(self_ns::str(self_ns::self))
                ;
            py::implicitly_convertible<RaySensorPtr, SensorPtr>();
            py::class_<RadarSensor, bases<Sensor>, RadarSensorPtr>(
                "RadarSensor",
                "A radar sensor that returns a value based on the number and \
                distance of objects within a sector of space.",
                init<double, double, double, double, double, U32, bool>())
				.def(self_ns::str(self_ns::self))
                ;
            py::implicitly_convertible<RadarSensorPtr, SensorPtr>();
        }

		/// return an action array for python to use
		template<typename T> Actions get_vector(size_t size)
		{
			return std::vector<T>(size);
		}

		size_t hash_value(const FeatureVector& v)
		{
			return boost::hash_value(v);
		}

		size_t hash_value(const StateActionPair& sa_pair)
		{
			return boost::hash_value(sa_pair);
		}

		static bool eq_fv(const FeatureVector& v1, const FeatureVector& v2)
		{ return v1 == v2; }

        /// convert a Python float to a FeatureVector
        struct FeatureVector_from_python_float
        {
            /// the constructor will register this converter with Boost.Python
            FeatureVector_from_python_float()
            {
                py::converter::registry::push_back(
                    &convertible,
                    &construct,
                    py::type_id<FeatureVector>());
            }

            /// determine if the Python object in question can be converted
            /// it has to be a tuple containing three floating point numbers
            static void* convertible(PyObject* obj_ptr)
            {
                if (PyFloat_Check(obj_ptr) || PyInt_Check(obj_ptr))
                {
                    return obj_ptr;
                } else {
                    return 0;
                }
            }
            /// convert a Python object to a FeatureVector object
            static void construct(
                PyObject* obj_ptr,
                py::converter::rvalue_from_python_stage1_data* data)
            {
                // get the value out
                double x = PyFloat_AsDouble(obj_ptr);

                // grab a pointer to memory into which to construct the new FeatureVector
                void* storage = ((py::converter::rvalue_from_python_storage<FeatureVector>*)data)->storage.bytes;

                // in-place construct the Vector3f from the values in the tuple
                new (storage) FeatureVector(1,x);

                // remember the memory chunk
                data->convertible = storage;
            }
        };

		/// @brief export the OpenNERO AI script interface
		void ExportAIScripts()
		{
			// export bound info
			py::class_<Bound>("Bound", "Bounds on a single feature (real or discrete)", init<double, double, bool>())
				.def_readonly("min", &Bound::min, "minimum value")
				.def_readonly("max", &Bound::max, "maximum value")
				.def_readonly("discrete", &Bound::discrete, "values discrete?")
				.def(self_ns::str(self_ns::self));

			// export bounded array info
			py::class_<FeatureVectorInfo>("FeatureVectorInfo", "Describe constraints of a feature vector")
				.def("__len__", &FeatureVectorInfo::size, "Length of the feature vector")
				.def(self_ns::str(self_ns::self))
				.def("min", &FeatureVectorInfo::getMin, "Minimal value for an element")
				.def("max", &FeatureVectorInfo::getMax, "Maximal value for an element")
				.def("discrete", &FeatureVectorInfo::isDiscrete, "Is the element discrete or continuous?")
				.def("bound", &FeatureVectorInfo::getBound, "Spec for a particular feature")
				.def("set_discrete", &FeatureVectorInfo::setDiscrete, "Create a discrete element")
				.def("set_continuous", &FeatureVectorInfo::setContinuous, "Create a continuous element")
				.def("add_discrete", &FeatureVectorInfo::addDiscrete, "Add a discrete element")
				.def("add_continuous", &FeatureVectorInfo::addContinuous, "Add a continuous element")
				.def("add", &FeatureVectorInfo::add, "Add an element")
				.def("validate", &FeatureVectorInfo::validate, "Check whether a feature vector is valid")
				.def("normalize", &FeatureVectorInfo::normalize, "Normalize the feature vector given this info")
				.def("denormalize", &FeatureVectorInfo::denormalize, "Create an instance of a feature vector from a vector of values between 0 and 1")
				.def("get_instance", &FeatureVectorInfo::getInstance, "Create a feature vector based on this information")
				.def("random", &FeatureVectorInfo::getRandom, "Create a random feature vector uniformly distributed within bounds")
				;

			// export std::vector<double>
			py::class_< std::vector<double> > ("DoubleVector", "A vector of real values")
				.def(self_ns::str(self_ns::self))
				.def("__eq__", &eq_fv)
				.def(vector_indexing_suite< std::vector<double> >())
				;

            // ability to convert a single Python float to a FeatureVector
            FeatureVector_from_python_float();

			py::class_<AgentInitInfo>("AgentInitInfo", "Initialization information given to the agent",
                                      init<const FeatureVectorInfo&, const FeatureVectorInfo&, const FeatureVectorInfo&>())
				.def_readonly("sensors", &AgentInitInfo::sensors, "Constraints on the agent's sensor feature vector")
				.def_readonly("actions", &AgentInitInfo::actions, "Constraints on the agent's action feature vector")
				.def_readonly("reward", &AgentInitInfo::reward, "Constraints on the agent's reward")
				.def(self_ns::str(self_ns::self))
				;
		}

		AIPtr getAI(const std::string& name)
		{
			return AIManager::instance().GetAI(name);
		}

		void setAI(const std::string& name, AIPtr ai)
		{
			AIManager::instance().SetAI(name, ai);
		}

		/// enable or disable AI
		void switch_ai(bool state)
		{
			AIManager::instance().SetEnabled(state);
		}

		/// toggle AI between on and off
		void toggle_ai()
		{
			AIManager::instance().SetEnabled(!AIManager::instance().IsEnabled());
		}

		/// enable AI
		void enable_ai()
		{
			AIManager::instance().SetEnabled(true);
		}

		/// disable AI
		void disable_ai()
		{
			AIManager::instance().SetEnabled(false);
		}

		/// reset environment
		void reset_ai()
		{
            AIManager::instance().Reset();
		}

		/// get the currently running environment
		EnvironmentPtr get_environment()
		{
			return AIManager::const_instance().GetEnvironment();
		}

		/// set the environment
		void set_environment(PyEnvironmentPtr env)
		{
			AIManager::instance().SetEnvironment(env);
		}


		/// export AI on/off toggle functions
		void ExportAIManagerScripts()
		{
			// TODO: make these methods more organized
			py::def("switch_ai", &switch_ai, "switch AI");
			py::def("toggle_ai", &toggle_ai, "enable or disable AI");
			py::def("enable_ai", &enable_ai, "enable AI");
			py::def("disable_ai", &disable_ai, "disable AI");
			py::def("reset_ai", &reset_ai, "reset AI");
			py::def("get_environment", &get_environment, "get the current environment");
			py::def("set_environment", &set_environment, "set the current environment");

			py::def("get_ai", &getAI, "return AIPtr");
			py::def("set_ai", &setAI,"set AI ptr");
		}

		/// Export World-specific script components
		void ExportEnvironmentScripts()
		{
			// export the interface to python so that we can override its methods there
			py::class_<PyEnvironment, noncopyable, PyEnvironmentPtr >("Environment", "Abstract base class for implementing an environment")
				.def("get_agent_info", pure_virtual(&Environment::get_agent_info), "Get the blueprint for creating new agents")
				.def("sense", pure_virtual(&Environment::sense), "sense the agent's current environment" )
				.def("is_episode_over", pure_virtual(&Environment::is_episode_over), "is the episode over for the specified agent?")
				.def("step", pure_virtual(&Environment::step), "Get a step for an agent")
				.def("cleanup", pure_virtual(&Environment::cleanup), "Clean up when the environment is removed")
				.def("reset", pure_virtual(&Environment::reset), "reset the environment to its initial state");

			py::implicitly_convertible<PyEnvironmentPtr, EnvironmentPtr >();
		}

		/// Export RTNEAT related classes and functions to Python
		void ExportRTNEATScripts()
		{
			// export Network
			py::class_<PyNetwork, PyNetworkPtr>("Network", "an artificial neural network", no_init )
				.def("load_sensors", &PyNetwork::load_sensors, "load sensor values into the network")
				.def("activate", &PyNetwork::activate, "activate the network for one or more steps until signal reaches output")
				.def("flush", &PyNetwork::flush, "flush the network by clearing its internal state")
				.def("get_outputs", &PyNetwork::get_outputs, "get output values from the network")
				.def(self_ns::str(self_ns::self));

			// export Organism
			py::class_<PyOrganism, PyOrganismPtr>("Organism", "a phenotype and a genotype for a neural network", no_init)
				.add_property("net", &PyOrganism::GetNetwork, "neural network (phenotype)")
				.add_property("id", &PyOrganism::GetId, "evolution-wide unique id of the organism")
				.add_property("fitness", &PyOrganism::GetFitness, &PyOrganism::SetFitness, "organism fitness (non-negative real)")
				.add_property("time_alive", &PyOrganism::GetTimeAlive, &PyOrganism::SetTimeAlive, "organism time alive (integer, non negative)")
                .def_readwrite("champion", &PyOrganism::champion, "is the organism a champion of the species?")
                .add_property("species_id", &PyOrganism::GetSpeciesId, "the id of the species of the organism")
				.def("save", &PyOrganism::Save, "save the organism to file")
				.def(self_ns::str(self_ns::self));

			// export AI base class
			py::class_<AI, AIPtr, noncopyable>("AI", "AI algorithm", no_init);

			// export RTNEAT interface
			py::class_<RTNEAT, bases<AI>, RTNEATPtr>("RTNEAT", init<const std::string&, const std::string&, S32, const RewardInfo&>())
				.def(init<const std::string&, S32, S32, S32, F32, const RewardInfo&>())
				.def("get_organism", &RTNEAT::get_organism, "evolve a new organism and return it")
                .def("release_organism", &RTNEAT::release_organism, "release the organism after the agent is done")
                .def("ready", &RTNEAT::ready, "return true iff RTNEAT is ready to produce a new organism")
                .def("has_organism", &RTNEAT::has_organism, "return true iff RTNEAT has an organism for this agent")
                .def("set_weight", &RTNEAT::set_weight, "set weight i to value f")
                .def("set_lifetime", &RTNEAT::set_lifetime, "set the lifetime of an agent")
				.def("save_population", &RTNEAT::save_population, "save the population to a file")
                .def("enable_evolution", &RTNEAT::enable_evolution, "turn evolution on")
                .def("disable_evolution", &RTNEAT::disable_evolution, "turn evolution off");
		}

		/// the pickling suite for the Vector class
		template <typename T>
		struct irr_vector3d_pickle_suite : py::pickle_suite
		{
			/// 3D vector from Irrlicht
			typedef irr::core::vector3d<T> VectorType;
			/// Python Tuple
			typedef py::tuple   PyTuple;
			/// Python Object
			typedef py::object  PyObject;

			/// Create a python tuple from a vector
			static PyTuple getinitargs(const VectorType& v)
			{
				return py::make_tuple( v.X, v.Y, v.Z );
			}

			/// Return a python tuple representing a python objects state
			static PyTuple getstate(PyObject obj)
			{
				VectorType const& v = extract<VectorType const&>(obj)();
				return py::make_tuple(obj.attr("__dict__"), v.X, v.Y, v.Z );
			}

			/// Set the state of a python object
			static void setstate( PyObject obj, PyTuple state)
			{
				// get the vector from the py object
				VectorType& v = py::extract<VectorType&>(obj)();

				// make sure the tuple is proper format
				if (py::len(state) != 4)
                    {
                        PyErr_SetObject(PyExc_ValueError, ("expected 2-item tuple in call to __setstate__; got %s" % state).ptr() );
                        py::throw_error_already_set();
                    }

				// restore the object's __dict__
				py::dict d = py::extract<py::dict>(obj.attr("__dict__"))();
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
		struct irr_SColor_pickle_suite : py::pickle_suite
		{
			typedef py::tuple   PyTuple;
			typedef py::object  PyObject;

			static PyTuple getinitargs(const SColor& col)
			{
				return py::make_tuple( col.getAlpha(), col.getRed(), col.getGreen(), col.getBlue() );
			}

			static PyTuple getstate(PyObject obj)
			{
				SColor const& col = extract<SColor const&>(obj)();
				return py::make_tuple(obj.attr("__dict__"), col.getAlpha(), col.getRed(), col.getGreen(), col.getBlue() );
			}

			static void setstate( PyObject obj, PyTuple state)
			{
				// get the color from the py object
				SColor& col = py::extract<SColor&>(obj)();

				// make sure the tuple is proper format
				if (py::len(state) != 5)
                    {
                        PyErr_SetObject(PyExc_ValueError, ("expected 4-item tuple in call to __setstate__; got %s" % state).ptr() );
                        py::throw_error_already_set();
                    }

				// restore the object's __dict__
				py::dict d = py::extract<py::dict>(obj.attr("__dict__"))();
				d.update(state[0]);

				// restore the internal state of the C++ object
				col.setAlpha( py::extract<uint32_t>(state[1]) );
				col.setRed( py::extract<uint32_t>(state[2]) );
				col.setGreen( py::extract<uint32_t>(state[3]) );
				col.setBlue( py::extract<uint32_t>(state[4]) );
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

        /// convert Python tuples to Vector3f objects
        struct Vector3f_from_python_tuple
        {
            /// the constructor will register this converter with Boost.Python
            Vector3f_from_python_tuple()
            {
                py::converter::registry::push_back(
                    &convertible,
                    &construct,
                    py::type_id<Vector3f>());
            }

            /// determine if the Python object in question can be converted
            /// it has to be a tuple containing three floating point numbers
            static void* convertible(PyObject* obj_ptr)
            {
                if (PyTuple_Check(obj_ptr) && PyTuple_Size(obj_ptr) == 3) {
                    if (PyFloat_Check(PyTuple_GetItem(obj_ptr, 0)) &&
                        PyFloat_Check(PyTuple_GetItem(obj_ptr, 1)) &&
                        PyFloat_Check(PyTuple_GetItem(obj_ptr, 2)))
                    {
                        return obj_ptr;
                    }
                    else
                    {
                        return 0;
                    }
                } else {
                    return obj_ptr;
                }
            }

            /// convert a Python object to a Vector3f object
            static void construct(
                PyObject* obj_ptr,
                py::converter::rvalue_from_python_stage1_data* data)
            {
                // get the x, y and z components of the tuple as doubles
                double x = PyFloat_AsDouble(PyTuple_GetItem(obj_ptr, 0));
                double y = PyFloat_AsDouble(PyTuple_GetItem(obj_ptr, 1));
                double z = PyFloat_AsDouble(PyTuple_GetItem(obj_ptr, 2));

                // grab a pointer to memory into which to construct the new Vector3f
                void* storage = ((py::converter::rvalue_from_python_storage<Vector3f>*)data)->storage.bytes;

                // in-place construct the Vector3f from the values in the tuple
                new (storage) Vector3f(x,y,z);

                // remember the memory chunk
                data->convertible = storage;
            }
        };

		/// export the Irrlicht utilities to python
		void ExportIrrUtilScripts()
		{
			// a vector class
			py::class_<Vector3f>("Vector3f", "A three-dimensional vector", init<float32_t, float32_t, float32_t>())
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

            // A converter from a Python tuple to a Vector3f
            Vector3f_from_python_tuple();

			// position2D classes
			ExportPos2< Pos2i, int32_t >  ( "Pos2i", "A two dimensional integer position." );
			ExportPos2< Pos2f, float32_t >( "Pos2f", "A two dimensional float position." );

			// rectangle classes
			ExportRect2< Rect2i, int32_t, Pos2i >  ( "Rect2i", "An integer rectangle." );
			ExportRect2< Rect2f, float32_t, Pos2f >( "Rect2f", "A float rectangle." );

			// export SColor
			py::class_<SColor>("Color", "An argb unsigned byte color", init<uint32_t,uint32_t,uint32_t,uint32_t>() )
				.def( init<>() )
				.add_property( "r", &SColor::getRed,   &SColor::setRed )
				.add_property( "g", &SColor::getGreen, &SColor::setGreen )
				.add_property( "b", &SColor::getBlue,  &SColor::setBlue )
				.add_property( "a", &SColor::getAlpha, &SColor::setAlpha )
				.def_pickle(irr_SColor_pickle_suite())
				;
		}

        /// request a switch to a new mod with the specified mod path
        void switchMod( const std::string& modName, const std::string& modDir )
        {
            Kernel::instance().RequestModSwitch(modName,modDir);
        }

        /// convert mod-relative path to filesystem path
        std::string findResource(const std::string& path)
        {
            return Kernel::instance().findResource(path);
        }

        std::string getModPath()
        {
            return Kernel::instance().getModPath();
        }

        void setModPath(const std::string& path)
        {
            Kernel::instance().setModPath(path);
        }

        void setWindowCaption(const std::string& caption)
        {
            Kernel::instance().SetWindowCaption(caption);
        }

		void ExportKernelScripts()
		{
			py::def( "switchMod", &switchMod, "Switch the kernel to a new mod");
			py::def( "findResource", &findResource, "Convert mod-relative path to filesystem path");
			py::def( "getModPath", &getModPath, "get the resource search path of the current mod ( separated by ':' )");
			py::def( "setModPath", &setModPath, "set the resource search path of the current mod ( separated by ':' )");
            py::def( "setWindowCaption", &setWindowCaption, "set the last part of the window caption to display a custom message");
		}

        void ExportPropertyMapScripts()
        {
            // export the map
            typedef std::map< std::string, std::string > StringToStringMap;

            py::class_< StringToStringMap >( "StringToStringMap", "A mapping of strings to strings" )
                .def(map_indexing_suite< StringToStringMap >() )
                ;

            // export property map
            py::class_<PropertyMap>("PropertyMap", "A quick utility for polling an xml file")
                .def("construct",   &PropertyMap::constructPropertyMap, "Creating a property map from an xml file path")
                .def("get_value", &PropertyMap::PyGetStringValue,       "Get a string value from a property spec" )
                .def("get_attributes", &PropertyMap::getAttributes,     "Get the attributes at a given property spec")
                .def("has_attributes", &PropertyMap::hasAttributes,     "Check if the given property map spec contains attributes")
                .def("has_value", &PropertyMap::hasValue,               "Check if the given property map spec contains a value")
                .def("has_section", &PropertyMap::hasSection,           "Check if the given property map spec exists")
                ;
        }

        Scheduler::EventId schedule( uint32_t timeOffset, const Scheduler::ScriptCommand& command )
        {
            Scheduler& scheduler = ScriptingEngine::instance().GetScheduler();
            return scheduler.ScheduleEvent( timeOffset, command );
        }

        bool cancel( const Scheduler::EventId& id )
        {
            Scheduler& scheduler = ScriptingEngine::instance().GetScheduler();
            return scheduler.CancelEvent(id);
        }


        /// export scheduler methods into Python API
        void ExportSchedulerScripts()
        {
            py::def( "schedule",
                 &schedule,
                 "Schedule an event to execute in some time offset. schedule(offset,command)");
            py::def( "cancel",
                 &cancel,
                 "Cancel an event from executing. cancel( eventId )" );
        }

        /// redirect standard output to message-level logging
        class PyStdLogWriter {
            std::string s;
        public:
            PyStdLogWriter() : s() {}
            /// write messge to log
            void write(std::string msg)
            {
                if (msg == "\n")
                    {
                        LOG_F_MSG( "python", s );
                        s = "";
                    }
                else
                    {
                        s += msg;
                    }
            }
            /// flush log to final destination
            void flush() {}
            /// close log
            void close() {}
        };

        /// redirect standard error to error-level logging
        class PyErrLogWriter {
            std::string s;
        public:
            PyErrLogWriter() : s() {}
            /// write a message to log
            void write(std::string msg)
            {
                if (msg == "\n")
                    {
                        LOG_F_MSG( "python", s );
                        s = "";
                    }
                else
                    {
                        s += msg;
                    }
            }
            /// flush log to final destination
            void flush() {}
            /// close log
            void close() {}
        };

        /// export scripting engine to Python
        void ExportScriptingEngineScripts()
        {
            py::class_<ScriptingEngine>("ScriptingEngine");
            py::class_<PyStdLogWriter>("StdLogWriter")
                .def("write", &PyStdLogWriter::write, "write message to the OpenNERO log")
                .def("close", &PyStdLogWriter::close, "close the python log writer")
                .def("flush", &PyStdLogWriter::flush, "flush the OpenNERO log");
            py::class_<PyErrLogWriter>("ErrLogWriter")
                .def("write", &PyErrLogWriter::write, "write message to the OpenNERO log")
                .def("close", &PyErrLogWriter::close, "close the python log writer")
                .def("flush", &PyErrLogWriter::flush, "flush the OpenNERO log");
        }

        void ExportSimEntityDataScripts()
        {
            py::class_<SimEntityData>("SimEntityData", no_init)
                .add_property("position",
                              make_function(&SimEntityData::GetPosition, return_value_policy<copy_const_reference>()),
                              &SimEntityData::SetPosition)
                .add_property("velocity",
                              make_function(&SimEntityData::GetVelocity, return_value_policy<copy_const_reference>()),
                              &SimEntityData::SetVelocity)
                .add_property("rotation",
                              make_function(&SimEntityData::GetRotation, return_value_policy<copy_const_reference>()),
                              &SimEntityData::SetRotation)
                .add_property("acceleration",
                              make_function(&SimEntityData::GetAcceleration, return_value_policy<copy_const_reference>()),
                              &SimEntityData::SetAcceleration)
                .add_property("label",
                              make_function(&SimEntityData::GetLabel, return_value_policy<copy_const_reference>()),
                              &SimEntityData::SetLabel)
                .add_property("color",
                              make_function(&SimEntityData::GetColor, return_value_policy<copy_const_reference>()),
                              &SimEntityData::SetColor)
                .add_property("type",
                              &SimEntityData::GetType,
                              &SimEntityData::SetType)
                .add_property("collision",
                              &SimEntityData::GetCollision,
                              &SimEntityData::SetCollision)
                .add_property("id", &SimEntityData::GetId)
                .add_property("animation", &SimEntityData::GetAnimation, &SimEntityData::SetAnimation)
                .add_property("animation_speed", &SimEntityData::GetAnimationSpeed, &SimEntityData::SetAnimationSpeed)
                ;

            py::class_<SimDataVector>("SimDataVector", "A vector of SimEntityData")
                .def(vector_indexing_suite<SimDataVector>())
                ;
        }

        /// get the current simulation context
        SimContext& GetSimContext()
        {
            return *(Kernel::GetSimContext());
        }

        BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(addObject_overloads, AddObject, 2, 7)

        BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(addSkyBox_overloads, AddSkyBox, 1, 2)

        BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(addLightSource_overloads, AddLightSource, 2, 3)

        BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(findInRay_overloads, PyFindInRay, 2, 6)

        void ExportSimContextScripts()
        {
            py::class_<SimContext>("SimContext", "The simulation context from an XML file", no_init )
                .def("addAxes",
                     &SimContext::AddAxes,
                     "Add a set of Cartesian axes from an XML file")
                .def("setFog",
                     &SimContext::SetFog,
                     "Set the fog mode")
                .def("addCamera",
                     &SimContext::AddCamera,
                     "Create and add a camera to the context and return camera")
                .def("addLightSource",
                     &SimContext::AddLightSource,
                     addLightSource_overloads("Add a light source to the scene"))
                .def("addSkyBox",
                     &SimContext::AddSkyBox,
                     addSkyBox_overloads("Add a sky box consisting of 6 images starting with arg0 and ending with arg1"))
                .def("addObject",
                     &SimContext::AddObject,
                     addObject_overloads("Create an object on the server and broadcast to clients"))
                .def("removeObject",
                     &SimContext::RemoveObject,
                     "Remove an object from the server and broadcast to clients")
                .def("getGuiManager",
                     &SimContext::GetGuiManager,
                     "Return the gui manager for the context")
                .def("killGame",
                     &SimContext::KillGame,
                     "Kill the game")
                .def("setInputMapping",
                     &SimContext::SetInputMapping,
                     "Set the io map to use" )
                .def("findInRay",
                     &SimContext::PyFindInRay,
                     findInRay_overloads("Find the first object that intersects the specified ray (origin:Vector3f, target:Vector3f, [int])") )
                .def("getClickedPosition",
                     &SimContext::GetClickedPosition,
                     "Approximate 3d position of the mouse click")
                .def("getClickedEntityId",
                     &SimContext::GetClickedEntityId,
                     "Return the id of the entity that was clicked")
                .def("getMousePosition",
                     &SimContext::GetMousePosition,
                     "Get the current position of the mouse")
                .def("setObjectPosition",
                     &SimContext::SetObjectPosition,
                     "Set the position of an object specified by its id")
                .def("setObjectRotation",
                     &SimContext::SetObjectRotation,
                     "Set the rotation of an object specified by its id")
                .def("setObjectScale",
                     &SimContext::SetObjectScale,
                     "Set the scale of an object specified by its id")
                .def("setObjectLabel",
                     &SimContext::SetObjectLabel,
                     "Set the label of an object specified by its id")
                .def("setObjectColor",
                     &SimContext::SetObjectColor,
                     "Set the color of an object specified by its id")
                .def("setObjectAnimation",
                     &SimContext::SetObjectAnimation,
                     "Set the animation of the object specified by its id")
                .def("getObjectPosition",
                     &SimContext::GetObjectPosition,
                     "Get the position of an object specified by its id")
                .def("getObjectRotation",
                     &SimContext::GetObjectRotation,
                     "Get the rotation of an object specified by its id")
                .def("getObjectScale",
                     &SimContext::GetObjectScale,
                     "Get the scale of an object specified by its id")
                .def("getObjectLabel",
                     &SimContext::GetObjectLabel,
                     "Get the label of an object specified by its id")
                .def("getObjectColor",
                     &SimContext::GetObjectColor,
                     "Get the color of an object specified by its id")
                .def("getObjectBBMinEdge",
                     &SimContext::GetObjectBBMinEdge,
                     "Get the bounding box min edge of an object specified by its id")
                .def("getObjectBBMaxEdge",
                     &SimContext::GetObjectBBMaxEdge,
                     "Get the bounding box max edge of an object specified by its id")
                .def("getActiveCamera",
                     &SimContext::getActiveCamera,
                     "Get the current active camera")
                .def("transformVector",
                     &SimContext::TransformVector,
                     "Transform the given vector by the matrix of the object specified by id")
                .add_property("delay", &SimContext::GetFrameDelay, &SimContext::SetFrameDelay)
                ;

            // this is how Python can access the C++ reference to SimContext
            py::def("getSimContext", &GetSimContext, return_value_policy<reference_existing_object>());
        }

        /// Python Binder Method for the IOMap
        void ExportIOMappingScripts()
        {
            py::class_<PyIOMap>("PyIOMap", "The python input mappings", init<>() )
                .def("BindKey", &PyIOMap::BindKey, "Bind a keyboard key to a python action" )
                .def("BindMouseButton", &PyIOMap::BindMouseButton, "Bind a mouse button to a python action" )
                .def("BindMouseAction", &PyIOMap::BindMouseAction, "Bind a mouse button to a python action" )
                .def("ClearMappings", &PyIOMap::ClearMappings, "Clear all python mappings" )
                ;
        }

        /// export camera API to script
        void ExportCameraScripts()
        {
            py::class_<Camera,CameraPtr>("Camera", "A camera in a sim context", no_init)
                .def("setPosition",
                     &Camera::setPosition,
                     "Set the current position of the camera" )
                .def("setRotation",
                     &Camera::setRotation,
                     "Set the euler rotation angles of the camera" )
                .def("setTarget",
                     &Camera::setTarget,
                     "Set the point the camera is looking at" )
                .def("setFarPlane",
                     &Camera::setFarPlane,
                     "Set the far plane of the perspective projection of the camera" )
                .def("setEdgeScroll",
                     &Camera::setEdgeScroll,
                     "Set whether or not the camera edge scrolls" )
                .def("attach",
                     &Camera::attach,
                     "Attach to sim object by id" )
				.def("snapshot",
					 &Camera::snapshot,
					 "Take a snapshot of the current screen")
                ;
        }

        void ExportAppConfigScripts()
        {
            py::class_<AppConfig>("AppConfig", no_init)
                .def_readonly("title", &AppConfig::Title)
                .def_readonly("logfile", &AppConfig::LogFile)
                .def_readonly("logconfigfile", &AppConfig::LogConfigFile)
                .def_readonly("rendertype", &AppConfig::RenderType)
                .def_readonly("startmod", &AppConfig::StartMod)
                .def_readonly("startmoddir", &AppConfig::StartModDir)
                .def_readonly("startcommand", &AppConfig::StartCommand)
                .def_readonly("width", &AppConfig::Width)
                .def_readonly("height", &AppConfig::Height)
                .def_readonly("bpp", &AppConfig::BPP)
                .def_readonly("fullscreen", &AppConfig::FullScreen)
                .def_readonly("stencilbufer", &AppConfig::StencilBuffer)
                .def_readonly("randomseeds", &AppConfig::RandomSeeds)
                ;

            py::def("getAppConfig", &GetAppConfig, return_value_policy<reference_existing_object>());
        }

        void ExportScripts()
        {
            ExportAppConfigScripts();
            ExportAIScripts();
            ExportAIManagerScripts();
            ExportAgentBrainScripts();
            ExportSensorScripts();
            ExportEnvironmentScripts();
            ExportRTNEATScripts();
            ExportIrrUtilScripts();
            ExportKernelScripts();
            ExportPropertyMapScripts();
            ExportGuiScripts();
            ExportSchedulerScripts();
            ExportSimEntityDataScripts();
            ExportSimContextScripts();
            ExportIOMappingScripts();
            ExportCameraScripts();
            ExportScriptingEngineScripts();
        }
     }
 }
