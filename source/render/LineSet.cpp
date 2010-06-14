//--------------------------------------------------------
// OpenNero : LineSet
//  A collection of line segments in the world
//--------------------------------------------------------

#include "core/Common.h"
#include "render/LineSet.h"
#include "boost/pool/detail/singleton.hpp"

namespace OpenNero
{   
    LineSet::LineSegment::LineSegment( const Vector3f& start, const Vector3f& end, const LineColor& color )
        : mStart(start)
        , mEnd(end)
        , mColor(color)
    {}



    /// Singleton access
    LineSet& LineSet::instance()
    {
        return boost::details::pool::singleton_default<LineSet>::instance();
    }

    /// Ctor - setup the lineset material
    LineSet::LineSet()
    {
        // setup the material for linesets
        mMaterial.MaterialType = irr::video::EMT_SOLID;
        mMaterial.Lighting     = false;
    }

    /// Add a line segment to our list
    void LineSet::AddSegment( const Vector3f& start, const Vector3f& end, const LineColor& color )
    {
        mLineSegments.push_back( LineSegment(start,end,color) );
    }

    /// Remove all of the currently stored line segments
    void LineSet::ClearSegments()
    {
        mLineSegments.clear();
    }

    /// Draw our stored line segments to the screen
    void LineSet::Render( irr::video::IVideoDriver* driver ) const
    {
        using namespace irr;
        using namespace video;
        using namespace core;

        // setup the render state
        driver->setTransform( ETS_WORLD, matrix4( matrix4::EM4CONST_IDENTITY ) );
        driver->setMaterial(mMaterial);

        // render the line segments
        LineSegmentList::const_iterator citr = mLineSegments.begin();
        LineSegmentList::const_iterator cend = mLineSegments.end();

        for( ; citr != cend; ++citr )
        {   
            // segments are assumed to be passed in OpenNero coordinate system,
            // so convert to irrlicht system to draw
            driver->draw3DLine( ConvertNeroToIrrlichtPosition(citr->mStart), 
                                ConvertNeroToIrrlichtPosition(citr->mEnd), 
                                citr->mColor );
        }

	driver->writeImageToFile(driver->createScreenShot(), "/home/pscamman/opennero-read-only/dist/NERO/full-image.jpg", 0);
    }
    
};//end OpenNero
