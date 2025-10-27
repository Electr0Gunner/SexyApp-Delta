#pragma once
#include <SDL3/SDL_video.h>

namespace Sexy
{
    class APITester
    {
    public:
        static bool gOpenGLCheckPerformed;
        static bool gOpenGLCheckResult;

        static bool IsOpenGLAvailable(SDL_Window* theWindow)
        {
            if (gOpenGLCheckPerformed)
                return gOpenGLCheckResult;
            SDL_GLContext context = SDL_GL_CreateContext(theWindow);
            if (context == nullptr)
            {
                gOpenGLCheckResult = false;
                gOpenGLCheckPerformed = true;
                return false;
            }
                

            SDL_GL_DestroyContext(context);

            gOpenGLCheckResult = true;
            gOpenGLCheckPerformed = true;
            return true;
        }
    };

} // namespace Sexy
