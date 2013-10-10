//---------------------------------------------------
// Name: OpenNero : IrrUtil
//  Irrlicht Utility includes
//---------------------------------------------------

#ifndef _CORE_IRRLICHT_UTIL_H_
#define _CORE_IRRLICHT_UTIL_H_

#include <tinyxml.h>    // keeps TinyXML from complaining about isspace()

#include <irrlicht.h>
#include <iostream>

#include "core/Preprocessor.h"
#include "core/Error.h"
#include "core/Common.h"
#include "core/Bitstream.h"

namespace irr
{
    /**
     * Reference increase function called by boost::intrusive_ptr,
     * created to allow boost and irrlicht to work together in ref
     * counted objects
     * @param unknown the irrlicht object to reference count
     */
    inline
    void intrusive_ptr_add_ref( irr::IReferenceCounted* unknown )
    {
        AssertMsg( unknown, "Invalid IReferenceCounted ptr passed into add_ref" );
        unknown->grab();
    }
    
    /**
     * Reference decrease function called by boost::intrusive_ptr,
     * created to allow boost and irrlicht to work together in ref
     * counted objects
     * @param unknown the irrlicht object to reference count
     */
    inline
    void intrusive_ptr_release( irr::IReferenceCounted* unknown )
    {
        AssertMsg( unknown, "Invalid IReferenceCounted ptr passed into ptr_release" );
        unknown->drop();
    }
}

namespace OpenNero {
    /// typedefs for irrlicht types in case we ever decide to move away
    /// @{
    /// 2d integer rectangle
    typedef irr::core::rect<irr::s32>    Rect2i;
    /// 2d real rectangle
    typedef irr::core::rect<irr::f32>    Rect2f;
    /// 2d integer position
    typedef irr::core::position2di       Pos2i;
    /// 2d real position
    typedef irr::core::position2df       Pos2f;
    
    typedef irr::core::vector2df Vector2f;
    typedef irr::core::vector2di Vector2i;
    typedef irr::core::vector3df Vector3f;
    typedef irr::core::vector3di Vector3i;
    typedef irr::core::line2df Line2f;
    typedef irr::core::line2di Line2i;
    typedef irr::core::line3df Line3f;
    typedef irr::core::line3di Line3i;
    typedef irr::core::triangle3df Triangle3f;
    typedef irr::core::triangle3di Triangle3i;
    typedef irr::core::matrix4   Matrix4;
    typedef irr::core::quaternion Quaternion;
    typedef irr::core::aabbox3df BBoxf;
    typedef irr::video::SColor   SColor;
    typedef irr::gui::IGUIFont   Font;
    typedef irr::scene::ISceneManager SceneManager;
    typedef irr::scene::IDynamicMeshBuffer MeshBuffer;
    /// @}
    
    /// typedefs for irrlicht intrusive ptrs
    typedef boost::intrusive_ptr<irr::IrrlichtDevice>               IrrlichtDevice_IPtr;
    /// Interface to video driver
    typedef boost::intrusive_ptr<irr::video::IVideoDriver>          IVideoDriver_IPtr;
    /// Interface to scene manager
    typedef boost::intrusive_ptr<irr::scene::ISceneManager>         ISceneManager_IPtr;
    /// Interface to GUI environment
    typedef boost::intrusive_ptr<irr::gui::IGUIEnvironment>         IGuiEnvironment_IPtr;
    /// Interface to Animated mesh
    typedef boost::intrusive_ptr<irr::scene::IAnimatedMesh>         IAnimatedMesh_IPtr;
    /// Interface to mesh buffer
    typedef boost::intrusive_ptr<irr::scene::IDynamicMeshBuffer>    IDynamicMeshBuffer_IPtr;
    /// Interface to Texture
    typedef boost::intrusive_ptr<irr::video::ITexture>		        ITexture_IPtr;
    /// Interface to scene node
    typedef boost::intrusive_ptr<irr::scene::ISceneNode>            ISceneNode_IPtr;
    /// Interface to scene node for terrain
    typedef boost::intrusive_ptr<irr::scene::ITerrainSceneNode>     ITerrainSceneNode_IPtr;
    /// Interface to animated mesh scene node
    typedef boost::intrusive_ptr<irr::scene::IAnimatedMeshSceneNode> IAnimatedMeshSceneNode_IPtr;
    /// Interface to triangle selector
    typedef boost::intrusive_ptr<irr::scene::ITriangleSelector>     ITriangleSelector_IPtr;
    /// Interface to meta triangle selector
    typedef boost::intrusive_ptr<irr::scene::IMetaTriangleSelector> IMetaTriangleSelector_IPtr;
    /// Interface to collision response scene node animator
    typedef boost::intrusive_ptr<irr::scene::ISceneNodeAnimatorCollisionResponse> ISceneNodeAnimatorCollisionResponse_IPtr;
    /// Interface to camera scene node
    typedef boost::intrusive_ptr<irr::scene::ICameraSceneNode>      ICameraSceneNode_IPtr;
    /// Interface to GUI element
    typedef boost::intrusive_ptr<irr::gui::IGUIElement>             IGuiElement_IPtr;
    /// Interface to a GUI image
    typedef boost::intrusive_ptr<irr::gui::IGUIImage>				IGuiImage_IPtr;
    /// Interface to a GUI edit box
    typedef boost::intrusive_ptr<irr::gui::IGUIEditBox>				IGuiEditBox_IPtr;
    /// Interface to a GUI text box
    typedef boost::intrusive_ptr<irr::gui::IGUIStaticText>			IGuiText_IPtr;
    /// Interface to a GUI combo box
    typedef boost::intrusive_ptr<irr::gui::IGUIComboBox>			IGuiComboBox_IPtr;
    /// Interface to a GUI window
    typedef boost::intrusive_ptr<irr::gui::IGUIWindow>				IGuiWindow_IPtr;
    /// Interface to a GUI context menu
    typedef boost::intrusive_ptr<irr::gui::IGUIContextMenu>			IGuiContextMenu_IPtr;
    /// Interface to a GUI button
    typedef boost::intrusive_ptr<irr::gui::IGUIButton>				IGuiButton_IPtr;
    /// Interface to a GUI scrollbar
    typedef boost::intrusive_ptr<irr::gui::IGUIScrollBar>			IGuiScrollBar_IPtr;
    /// Interface to a GUI check box
    typedef boost::intrusive_ptr<irr::gui::IGUICheckBox>            IGuiCheckBox_IPtr;
	
    /// a container of irrlicht ptrs using intrusive ptrs
    struct IrrHandles
    {
        IrrlichtDevice_IPtr	    mpIrrDevice;	///< Irrlicht Device
        IVideoDriver_IPtr		mpVideoDriver;	///< Irrlicht Video Driver
        ISceneManager_IPtr		mpSceneManager;	///< Irrlicht SceneManager
        IGuiEnvironment_IPtr	mpGuiEnv;		///< Irrlicht Gui
        
        /// constructor
        IrrHandles() {}
        
        explicit IrrHandles(IrrlichtDevice_IPtr device);
        
        irr::IrrlichtDevice* getDevice();
        
        const irr::IrrlichtDevice* getDevice() const;
        
        irr::video::IVideoDriver* getVideoDriver();
        
        const irr::video::IVideoDriver* getVideoDriver() const;
        
        irr::scene::ISceneManager* getSceneManager();
        
        const irr::scene::ISceneManager* getSceneManager() const;
        
        irr::gui::IGUIEnvironment* getGuiEnv();
        
        const irr::gui::IGUIEnvironment* getGuiEnv() const;
        
    };
    
    /// a container of irrlicht ptrs using raw ptrs (to stop circular references)
    /// Note: This structure is not safe, do not delete these pointers.
    struct IrrHandles_Weak
    {
        irr::IrrlichtDevice*		    mpIrrDevice;	///< Irrlicht Device
        irr::video::IVideoDriver*		mpVideoDriver;	///< Irrlicht Video Driver
        irr::scene::ISceneManager*	    mpSceneManager;	///< Irrlicht SceneManager
        irr::gui::IGUIEnvironment*		mpGuiEnv;		///< Irrlicht Gui
        
        /// constructor
        IrrHandles_Weak() {}
        
        /// copy constructor
        IrrHandles_Weak( const IrrHandles_Weak& handles )
        : mpIrrDevice(handles.mpIrrDevice),
        mpVideoDriver(handles.mpVideoDriver),
        mpSceneManager(handles.mpSceneManager),
        mpGuiEnv(handles.mpGuiEnv)
        {}
        
        /// constructor from IrrHandles
        IrrHandles_Weak( const IrrHandles& handles )
        : mpIrrDevice(handles.mpIrrDevice.get()),
        mpVideoDriver(handles.mpVideoDriver.get()),
        mpSceneManager(handles.mpSceneManager.get()),
        mpGuiEnv(handles.mpGuiEnv.get())
        {}
    };
    
    /// a safe irrlicht grab
    inline
    void SafeIrrGrab( irr::IReferenceCounted* unknown )
    {
        if( unknown )
            unknown->grab();
    }
    
    /// a safe irrlicht drop
    inline
    void SafeIrrDrop( irr::IReferenceCounted* unknown )
    {
        if( unknown )
            unknown->drop();
    }
    
    /// convenience method for storing an irr vector into a bitstream
    template< class T > inline Bitstream&
    operator<<( Bitstream& stream, const irr::core::vector3d<T>& vec )
    {
        return stream << vec.X << vec.Y << vec.Z;
        }
        
        /// convenience method for extracting an irr vector from a bitstream
        template< class T > inline Bitstream&
        operator>>( Bitstream& stream, irr::core::vector3d<T>& vec )
        {
            return stream >> vec.X >> vec.Y >> vec.Z;
        }
        
        /// convenience method for storing an irr color into a bitstream
        inline Bitstream& operator<<( Bitstream& stream, const irr::video::SColor& col )
        {
            stream << col.getAlpha() << col.getRed() << col.getGreen() << col.getBlue();
            return stream;
        }
        
        /// convenience method for extracting an irr color from a bitstream
        inline Bitstream& operator>>( Bitstream& stream, irr::video::SColor& col )
        {
            uint32_t tmp[4] = { 0, 0, 0, 0 };
            stream >> tmp[0] >> tmp[1] >> tmp[2] >> tmp[3];
            col.set( tmp[0], tmp[1], tmp[2], tmp[3] );
            return stream;
        }
        
        /// convert NERO right-handed position (x-y-z) to Irrlicht left-handed (x-z-y)
        template< typename T >
        inline irr::core::vector3d<T> ConvertNeroToIrrlichtPosition( const irr::core::vector3d<T>& vec )
        {
            return irr::core::vector3d<T>( vec.X, vec.Z, vec.Y );
        }
        
        /// convert Irrlicht left-handed (x-z-y) to NERO right-handed position (x-y-z)
        template< typename T >
        inline irr::core::vector3d<T> ConvertIrrlichtToNeroPosition( const irr::core::vector3d<T>& vec )
        {
            // Note: These two functions are inverses
            return ConvertNeroToIrrlichtPosition(vec);
        }
        
        /// convert NERO right-handed degrees Euler rotation to Irrlicht left-handed Euler radian rotation
        template< typename T >
        inline irr::core::vector3d<T> ConvertNeroToIrrlichtRotation( const irr::core::vector3d<T>& vec )
        {
            return irr::core::vector3d<T>( -vec.X, -vec.Z, -vec.Y );
        }
        
        /// convert Irrlicht left-handed radians Euler rotation to NERO right-handed Euler degrees rotation
        template< typename T >
        inline irr::core::vector3d<T> ConvertIrrlichtToNeroRotation( const irr::core::vector3d<T>& vec )
        {
            // Note: These two functions are inverses
            return ConvertNeroToIrrlichtRotation(vec);
        }
        
        Quaternion ConvertNeroRotationToIrrlichtQuaternion( const Vector3f& euler );
        
        Vector3f ConvertIrrlichtQuaternionToNeroRotation( const Quaternion& quat );
        
        Vector3f InterpolateNeroRotation( const Vector3f& r1, const Vector3f& r2, F32 f);
        
        }; // end OpenNero
        
        
#endif //end _CORE_IRRLICHT_UTIL_H_
