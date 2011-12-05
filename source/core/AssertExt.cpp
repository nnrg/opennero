//---------------------------------------------------
// Name: OpenNero : AssertExt
// Desc:  An assert extension
//---------------------------------------------------

#include "core/Common.h"
#include "core/AssertExt.h"
#include "core/ONTypes.h"
#include "boost/lexical_cast.hpp"
#include <cassert>

namespace OpenNero
{
#if NERO_DEBUG

    using namespace irr;

    irr::IrrlichtDevice* AssertExt::mDevice;    

    void AssertExt::SetDevice( irr::IrrlichtDevice* device )
    {
        mDevice = device;
        if(mDevice)
            mDevice->grab();
    }

    void AssertExt::ReleaseDevice()
    {
        if( mDevice )
        {
            mDevice->drop();
            mDevice = NULL;
        }        
    }    

    bool AssertExt::ShowAssert( const AssertMsg& msg, bool& assertStatus )
    {
        return instance().GetVerdict(msg, assertStatus);
    }

    AssertExt::AssertExt()
        : mVerdict(kV_Waiting)
    {}        

    bool AssertExt::OnEvent(const SEvent& event)
    {
        // check for escape or space
        switch( event.EventType )
        {
        case EET_KEY_INPUT_EVENT:
            {
                // can we handle it?
                if( event.KeyInput.Key == KEY_ESCAPE )
                {
                    mVerdict = kV_Stop;                    
                }

                else if( event.KeyInput.Key == KEY_SPACE )
                {
                    mVerdict = kV_Continue;
                }

                else if( event.KeyInput.Key == KEY_KEY_D )
                {
                    mVerdict = kV_Disable;
                }
                break;
            }
        default:
            break;
        }

        return true;
    }

    AssertExt& AssertExt::instance()
    {
		static AssertExt assert_ext;
		return assert_ext;
    }

    bool AssertExt::GetVerdict( const AssertMsg& msg, bool& assertStatus )
    {
        // make sure we have a device
        if( !mDevice )
            return false;        

        // grab the drawing tools we need as well
        irr::video::IVideoDriver* driver = mDevice->getVideoDriver();
        irr::gui::IGUIFont* font = mDevice->getGUIEnvironment()->getFont( "common/data/gui/fonthaettenschweiler.bmp" );

        if( !driver || !font )
            return false;

        // save the old event receiver and plug outself in
        IEventReceiver* oldReceiver = mDevice->getEventReceiver();
        mDevice->setEventReceiver(this);

        // convert to a wstring
        std::wstring wMsg = boost::lexical_cast<std::wstring>(msg.c_str());

        // setup our dimensions
        irr::core::dimension2du screenSize = driver->getScreenSize();

        irr::core::dimension2du  msgSize = font->getDimension(wMsg.c_str());
        irr::core::rect<int32_t> msgPos;
        msgPos.UpperLeftCorner.X = (screenSize.Width - msgSize.Width)/2;
        msgPos.UpperLeftCorner.Y = (screenSize.Height - msgSize.Height)/2;
        msgPos.LowerRightCorner.X = msgPos.UpperLeftCorner.X + msgSize.Width;
        msgPos.LowerRightCorner.Y = msgPos.UpperLeftCorner.Y + 50;

        std::wstring wOptions = L"Assert Options:     Quit [ Escape ]             Disable [ D ]             Continue [ Space ]";

        irr::core::dimension2du optionsSize = font->getDimension(wOptions.c_str());

        irr::core::rect<int32_t> optionsPos;
        optionsPos.UpperLeftCorner.X = (screenSize.Width - optionsSize.Width)/2;
        optionsPos.UpperLeftCorner.Y = msgPos.LowerRightCorner.Y + 40;
        optionsPos.LowerRightCorner.X = optionsPos.UpperLeftCorner.X + optionsSize.Width;
        optionsPos.LowerRightCorner.Y = optionsPos.UpperLeftCorner.Y + 50;        

        // reset our verdict
        mVerdict = kV_Waiting;

        // run our loop
        while( mDevice->run() )
        {  
            // should we break?
            if( mVerdict == kV_Stop )
            {
                NERO_BREAK;
                mDevice->setEventReceiver(oldReceiver);
                return false;
            }

            // should we continue?
            if( mVerdict == kV_Continue )
            {
                mDevice->setEventReceiver(oldReceiver);
                return true;
            }

            // should we disable this assert?
            if( mVerdict == kV_Disable )
            {
                mDevice->setEventReceiver(oldReceiver);
                assertStatus = false;
                return true;
            }
            
            // draw the text
            driver->beginScene( true, false, irr::video::SColor( 255, 0, 0, 255) );
            
            font->draw( wMsg.c_str(), msgPos, irr::video::SColor(255,255,255,255), true, true, 0 );
            font->draw( wOptions.c_str(), optionsPos, irr::video::SColor(255,255,255,255), true, true, 0 );
            
            driver->endScene();
            
            mDevice->yield();
        }

        // somehow the device died...
        return true;
    }


#endif // NERO_DEBUG


} // end OpenNero
