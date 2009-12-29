//---------------------------------------------------
// Name: OpenNero : AssertExt
// Desc:  An assert extension
//---------------------------------------------------

#ifndef _CORE_ASSERT_EXT_H_
#define _CORE_ASSERT_EXT_H_

#include "core/Preprocessor.h"
#include <irrlicht.h>
#include <string>

namespace OpenNero
{
    // Only let this appear in debug builds
#if NERO_DEBUG

    /***********************************************************************************************************************
     *
     * The AssertExt class provides more flexibility to the assertion system for our software. When an assertion
     * fails anywhere in the code base and the irrlicht video driver is currently active, a dialog screen will be presented
     * to the user describing the assert that occurred and prompting for further action by the user.
     *
     * The user has the following choices:
     *
     * Quit        - terminate the application
     * Skip        - allow the program to continue execution for this occurence of the assert
     * Disable     - allow the program to continue execution for this and any following occurences of this assert
     *
     **********************************************************************************************************************/

    // TODO - jbsheblak 21July2007 - at some occurences the font used to display the message will seems to used the most
    //                               recently loaded texture instead of the font texture. Fix this somehow.
    class AssertExt : public irr::IEventReceiver
    {
    public:

        typedef std::string AssertMsg;

    public:

        // set the device to use when displaying the assert message
        static void SetDevice( irr::IrrlichtDevice* device );
        static void ReleaseDevice();
        
        // show an assert to a user to decide whether to quit or not
        static bool ShowAssert( const AssertMsg& msg, bool& assertStatus );

    public:

        AssertExt();        

        // irrlicht event handler (Do not call from our code)
        virtual bool OnEvent(const irr::SEvent& event);

    private:

        // a user's decision on whether to continue the application
        enum AssertVerdict
        {
            kV_Waiting,     ///< waiting for input from user
            kV_Continue,    ///< skip this occurence of the assert
            kV_Disable,     ///< skip this and all occurences of the assert
            kV_Stop         ///< kill the program
        };

    private:

        // singleton
        static AssertExt& instance();

        // let the user choose the verdict
        bool GetVerdict( const AssertMsg& msg, bool& assertStatus );

    private:

        // the device to use when drawing the assert message
        static irr::IrrlichtDevice*     mDevice;

    private:

        // the verdict passed by the user
        AssertVerdict   mVerdict;
    };

#endif // NERO_DEBUG


} // end OpenNero

#endif //end _CORE_ASSERT_EXT_H_
