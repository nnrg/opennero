#ifndef _RENDER_FPSCOUNTER_H_
#define _RENDER_FPSCOUNTER_H_

#include "core/Common.h"

namespace OpenNero
{
    class FPSCounter  
    {
    public:
        FPSCounter();

        //! returns current fps
        S32 getFPS() const;

        //! to be called every frame
        void registerFrame();

    private:

        S32 FPS;
        U32 StartTime;

        U32 FramesCounted;
    };

} // end namespace OpenNero

#endif // _RENDER_FPSCOUNTER_H_
