//--------------------------------------------------------
// OpenNero : GuiImage
//  an image abstraction
//--------------------------------------------------------

#include "core/Common.h"
#include "gui/GuiImage.h"
#include "game/factories/SimFactory.h"
#include "scripting/scripting.h"

namespace OpenNero
{
	/**
	 * Attempt to set the image being used for this piece
	 * @param imagePath the path to use for loading the image
	*/
	void GuiImage::setImage( const std::string imagePath )
	{
        Assert( getGuiElement() );

        SimFactoryPtr fac = getSimFactory();
        IGuiImage_IPtr image = boost::static_pointer_cast<IGUIImage>( getGuiElement() );

        Assert(fac);
        Assert(image);

        ITexture* tex = fac->getIrrFactory().LoadTexture(imagePath);
        if(tex)
        {
            // set the image
            image->setImage(tex);

            // resize our element

            // get the texture size
			const dimension2du tSize = tex->getOriginalSize();
			const rect<s32>    tDim( position2di(0,0), tSize );

            // resize the element to reflect the texture size
            const Rect2i oldBounds = getRelativeBounds();
            setRelativeBounds( Rect2i( oldBounds.UpperLeftCorner, tSize ) );
        }
	}

	void GuiImage::setEnableAlphaChannel( bool enable )
	{
        Assert( getGuiElement() );
        IGuiImage_IPtr image = boost::static_pointer_cast<IGUIImage>( getGuiElement() );
        image->setUseAlphaChannel(enable);
	}

    PYTHON_BINDER( GuiImage )
    {
        using namespace boost;
        using namespace boost::python;

        // ptrs to special overloaded member methods
        _GUI_BASE_PRE_HACK_(GuiImage);

        class_<GuiImage, noncopyable>( "GuiImage", "A basic gui image", no_init )

            // Hack in our gui base methods
            _GUI_BASE_HACK_(GuiImage)

            // export our button methods
            .def("setImage",                &GuiImage::setImage,"Set the image to use" )
            .def("setEnableAlphaChannel",   &GuiImage::setEnableAlphaChannel, "Set whether or not to use the alpha channel")
        ;
    }
}
