//--------------------------------------------------------
// OpenNero : LineSet
//  A collection of line segments in the world
//--------------------------------------------------------

#ifndef _GAME_RENDER_LINE_SET_H_
#define _GAME_RENDER_LINE_SET_H_

#include "core/ONTypes.h"
#include "core/IrrUtil.h"
#include <vector>

namespace OpenNero
{   
    /// The LineSet is a collection of colored line segments in 3D space
    class LineSet
    {
    public:

        /// the color of a line
        typedef SColor LineColor;

    public:

        // singleton access
        static LineSet& instance();

    public:

        LineSet();

        // segment management
        void AddSegment( const Vector3f& start, const Vector3f& end, const LineColor& color );
        void ClearSegments();
        
        // draw the segments to the screen
        void Render( irr::video::IVideoDriver* driver ) const;

    private:

        /// an individual line segment entry
        struct LineSegment
        {
            Vector3f            mStart; ///< The start of the line segment
            Vector3f            mEnd;   ///< The end of the line segment
            LineColor           mColor; ///< The color of the line segment

            LineSegment() {}
            LineSegment( const Vector3f& start, const Vector3f& end, const LineColor& color );
        };

        /// a collection of line segments
        typedef std::vector<LineSegment> LineSegmentList;

    private:

        /// our current line segments
        LineSegmentList         mLineSegments;

        /// the material to use for our line segments
        irr::video::SMaterial   mMaterial;
    };

};//end OpenNero

#endif // _GAME_RENDER_LINE_SET_H_
