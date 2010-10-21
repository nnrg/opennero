//--------------------------------------------------------
// OpenNero : GuiButton
//  a button abstraction
//--------------------------------------------------------

#include "core/Common.h"
#include "gui/GuiButton.h"
#include "game/factories/IrrFactory.h"
#include "game/factories/SimFactory.h"

namespace OpenNero
{	
	// Set the image path of this button and try to load all related images.	
    void GuiButton::setImages( const std::string& imgBase )
    {
        SimFactoryPtr fac = getSimFactory();
        IGuiButton_IPtr button = boost::static_pointer_cast<IGUIButton>( getGuiElement() );

        Assert( fac );
        Assert( button );        
        
        IrrFactory& irrFac = fac->getIrrFactory();
        
		// construct our three state image paths
        const std::string nPath = imgBase + "_n.png";
		const std::string nPathJPG = imgBase + "_n.jpg";
        const std::string hPath = imgBase + "_h.png";
        const std::string hPathJPG = imgBase + "_h.jpg";
        const std::string dPath = imgBase + "_d.png";
        const std::string dPathJPG = imgBase + "_d.jpg";						

		// try loading the normal texture
        if( ITexture* tex = irrFac.LoadTexture( imgBase ) )
        {               
			// get the texture size
			const dimension2du tSize = tex->getOriginalSize();
			const rect<s32>    tDim( position2di(0,0), tSize );

			// set the image
            button->setImage(tex, tDim);

            // resize the element to reflect the texture size      
            const Rect2i oldBounds = getRelativeBounds();
            setRelativeBounds( Rect2i( oldBounds.UpperLeftCorner, tSize ) );            
        }
        else
            LOG_F_WARNING("gui", "could not load texture file " << imgBase);

		// try loading the normal texture
        if( ITexture* tex = irrFac.LoadTexture( nPath ) )
        {               
			// get the texture size
			const dimension2du tSize = tex->getOriginalSize();
			const rect<s32>    tDim( position2di(0,0), tSize );

			// set the image
            button->setImage(tex, tDim);

            // resize the element to reflect the texture size      
            const Rect2i oldBounds = getRelativeBounds();
            setRelativeBounds( Rect2i( oldBounds.UpperLeftCorner, tSize ) );            
        }
        else
            LOG_F_WARNING("gui", "could not load texture file " << nPath);
        
		// try loading the normal texture
        if( ITexture* tex = irrFac.LoadTexture( nPathJPG ) )
        {               
			// get the texture size
			const dimension2du tSize = tex->getOriginalSize();
			const rect<s32>    tDim( position2di(0,0), tSize );

			// set the image
            button->setImage(tex, tDim);

            // resize the element to reflect the texture size      
            const Rect2i oldBounds = getRelativeBounds();
            setRelativeBounds( Rect2i( oldBounds.UpperLeftCorner, tSize ) );            
        }
        else
            LOG_F_WARNING("gui", "could not load texture file " << nPathJPG);

		// try loading the hover texture
        if( ITexture* tex = irrFac.LoadTexture( hPath ) )
        {
			// get the texture size
			const dimension2du tSize = tex->getOriginalSize();
			const rect<s32>    tDim( position2di(0,0), tSize );

			// set the image
            button->setPressedImage(tex, tDim);

            // resize the element to reflect the texture size      
            const Rect2i oldBounds = getRelativeBounds();
            setRelativeBounds( Rect2i( oldBounds.UpperLeftCorner, tSize ) );            

        }        		
        else
            LOG_F_WARNING("gui", "could not load texture file " << hPath);

		// try loading the normal texture
        if( ITexture* tex = irrFac.LoadTexture( hPathJPG ) )
        {               
			// get the texture size
			const dimension2du tSize = tex->getOriginalSize();
			const rect<s32>    tDim( position2di(0,0), tSize );

			// set the image
            button->setImage(tex, tDim);

            // resize the element to reflect the texture size      
            const Rect2i oldBounds = getRelativeBounds();
            setRelativeBounds( Rect2i( oldBounds.UpperLeftCorner, tSize ) );            
        }
        else
            LOG_F_WARNING("gui", "could not load texture file " << hPathJPG);
        
		// try loading the depressed texture
        if( ITexture* tex = irrFac.LoadTexture( dPath ) )
        {
			// get the texture size
			const dimension2du tSize = tex->getOriginalSize();
			const rect<s32>    tDim( position2di(0,0), tSize );

			// set the image
            button->setPressedImage(tex, tDim);

            // resize the element to reflect the texture size      
            const Rect2i oldBounds = getRelativeBounds();
            setRelativeBounds( Rect2i( oldBounds.UpperLeftCorner, tSize ) );            

        }        		
        else
            LOG_F_WARNING("gui", "could not load texture file " << dPath);

		// try loading the normal texture
        if( ITexture* tex = irrFac.LoadTexture( dPathJPG ) )
        {               
			// get the texture size
			const dimension2du tSize = tex->getOriginalSize();
			const rect<s32>    tDim( position2di(0,0), tSize );

			// set the image
            button->setImage(tex, tDim);

            // resize the element to reflect the texture size      
            const Rect2i oldBounds = getRelativeBounds();
            setRelativeBounds( Rect2i( oldBounds.UpperLeftCorner, tSize ) );            
        }
        else
            LOG_F_WARNING("gui", "could not load texture file " << dPathJPG);
    }
}
