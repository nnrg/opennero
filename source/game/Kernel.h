//--------------------------------------------------------
// OpenNero : Kernel
//  main kernel of the application
//--------------------------------------------------------

#ifndef _GAME_KERNEL_H_
#define _GAME_KERNEL_H_

#include <vector>
#include "core/Common.h"
#include "core/IrrUtil.h"
#include "core/BoostCommon.h"
#include "core/IrrUtil.h"
#include "core/Preprocessor.h"
#include "input/IOState.h"
#include "utils/Config.h"

namespace OpenNero
{
	class Kernel;
	/// @cond
    BOOST_PTR_DECL( SimContext );
    BOOST_PTR_DECL( GuiBase );
    BOOST_PTR_DECL( Mod );
    /// @endcond

    /**
	 * The kernel is responsible for storing and manipulating the current mod loaded in the game.
	 * It also provides utility functions for the mods to use. Note that it is a singleton.
	*/
	class Kernel : 
        public irr::IEventReceiver
	{
	public:

		/// singleton accessor
		static Kernel&			instance();
		/// const singleton accessor
		static const Kernel&	const_instance();

		/// Convert a mod-relative path to its filesystem path (not necessarily absolute)
		static std::string findResource( const std::string& path, bool ensure_existance = true);

        /// Currently running Mod
        static ModPtr  GetMod();

        /// Current simulation context
        static SimContextPtr GetSimContext();

	protected:

		/// singleton constructor
		Kernel();

	public:

		/// destructor
		~Kernel();

        /// handle a user event
        virtual bool OnEvent(const irr::SEvent& event);

        /// initialize the engine
        bool Initialize( IrrlichtDevice_IPtr device, const AppConfig& appConfig, int argc, char** argv );

		/// get currently running Mod
        ModPtr			getMod();

        /// get currently running Mod (const)
		const ModPtr	getMod() const;

        /// get current configuration profile
		AppConfig&      getAppConfig() { return mAppConfig; }

        /// get current configuration profile (const)
		const AppConfig& getAppConfig() const { return mAppConfig; }

        /// main processing method, makes things step forward
        void ProcessTick();

		/// dispose of the currently loaded Mod's resources
		void flushCurrentMod();

		/// switch to a new Mod
        SimContextPtr	switchMod( const std::string& name, const std::string& mode, const std::string& path );

        /// switch to a new Mod
		SimContextPtr	switchMod( IrrlichtDevice_IPtr device, const std::string& name, const std::string& mode, const std::string& path );

        /// set the resource search path of the current mod ( separated by ':' )
        void setModPath( const std::string& path );

        /// get the resource search path of the current mod ( separated by ':' )
        std::string getModPath();

        /// request a mod switch next frame
        void RequestModSwitch( const std::string& name, const std::string& mode, const std::string& path );

        /// request a mod switch next frame
        void RequestModSwitch( IrrlichtDevice_IPtr device, const std::string& name, const std::string& mode, const std::string& path );
        
        /// Sets the part of the window title after OpenNero - ModName
        void SetWindowCaption(const std::string& caption);

    private:

        /// game/render engine device accessor
		IrrlichtDevice_IPtr getIrrDevice() const;

        /// game/render engine device accessor
        void setIrrDevice( IrrlichtDevice_IPtr dev );

	private:

		IrrlichtDevice_IPtr			mIrrDevice;	///< Irrlicht Rendering device
		ModPtr						mCurMod;	///< Current loaded mod

        struct TransitionInfo
        {
            IrrlichtDevice_IPtr     mDevice;
            std::string             mNewModName;
            std::string             mNewModMode;
            std::string             mNewModDir;
            bool                    mActive;

            TransitionInfo();
        };

        TransitionInfo mTransitionInfo; ///< The transition information

        int             mArgc;           /// number of command line arguments
        char**          mArgv;           /// argv

        AppConfig mAppConfig;
	};

} //end OpenNero

#endif // _GAME_KERNEL_H_
