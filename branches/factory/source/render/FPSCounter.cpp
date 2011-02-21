#include "core/Common.h"
#include "core/ONTime.h"
#include "render/FPSCounter.h"
#include "irrlicht.h"

namespace OpenNero
{
    FPSCounter::FPSCounter()
    :	FPS(60), StartTime(0), FramesCounted(0)
    {

    }

    /// returns current fps
    S32 FPSCounter::getFPS() const
    {
        return FPS;
    }

    /// to be called every frame
    void FPSCounter::registerFrame()
    {
        ++FramesCounted;
        U32 now = U32(GetStaticTimer().getMilliseconds());
        const U32 milliseconds = now - StartTime;

        if (milliseconds >= 1500)
        {
            const F32 invMilli = irr::core::reciprocal ( (F32) milliseconds );
		
            FPS = irr::core::ceil32 ( ( 1000 * FramesCounted ) * invMilli );

            FramesCounted = 0;
            StartTime = now;
        }
    }
} // end namespace OpenNero

