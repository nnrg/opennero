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

    Quaternion ConvertNeroRotationToIrrlichtQuaternion( const Vector3f& euler )
    {
        Quaternion neroQuat(euler * DEG_2_RAD);
        return Quaternion(neroQuat.X, neroQuat.Z, neroQuat.Y, neroQuat.W);
    }
    
    Vector3f ConvertIrrlichtQuaternionToNeroRotation( const Quaternion& q )
    {
        Quaternion neroQuat(q.X, q.Z, q.Y, q.W);
        Vector3f euler;
        neroQuat.toEuler(euler);
        euler *= RAD_2_DEG;
        return euler;
    }
    
    Vector3f InterpolateNeroRotation( const Vector3f& r1, const Vector3f& r2, F32 f)
    {
        Quaternion q1 = ConvertNeroRotationToIrrlichtQuaternion(r1);
        Quaternion q2 = ConvertNeroRotationToIrrlichtQuaternion(r2);
        Quaternion qSlerp;
        qSlerp.slerp(q1, q2, f);
        return ConvertIrrlichtQuaternionToNeroRotation(qSlerp);
    }
    
    IrrHandles::IrrHandles(IrrlichtDevice_IPtr device)
    : mpIrrDevice(device)
    , mpVideoDriver()
    , mpSceneManager()
    , mpGuiEnv()
    {
        Assert(mpIrrDevice);
        mpVideoDriver = device->getVideoDriver();
        Assert(mpVideoDriver);
        mpSceneManager = device->getSceneManager()->createNewSceneManager();
        Assert(mpSceneManager);
        mpGuiEnv = device->getGUIEnvironment();
        Assert(mpGuiEnv);
    }
    
    irr::IrrlichtDevice* IrrHandles::getDevice()
    {
        return mpIrrDevice.get();
    }
    
    const irr::IrrlichtDevice* IrrHandles::getDevice() const
    {
        return mpIrrDevice.get();
    }
    
    irr::video::IVideoDriver* IrrHandles::getVideoDriver()
    {
        return mpVideoDriver.get();
    }
    
    const irr::video::IVideoDriver* IrrHandles::getVideoDriver() const
    {
        return mpVideoDriver.get();
    }
    
    irr::scene::ISceneManager* IrrHandles::getSceneManager()
    {
        return mpSceneManager.get();
    }
    
    const irr::scene::ISceneManager* IrrHandles::getSceneManager() const
    {
        return mpSceneManager.get();
    }
    
    irr::gui::IGUIEnvironment* IrrHandles::getGuiEnv() 
    {
        return mpGuiEnv.get();
    }
    
    const irr::gui::IGUIEnvironment* IrrHandles::getGuiEnv() const
    {
        return mpGuiEnv.get();
    }

} // end OpenNero
