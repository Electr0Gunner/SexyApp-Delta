#pragma once

#include <SexyAppFramework/Common.h>
#include <SexyAppFramework/Image.h>
#include <SexyAppFramework/GPUImage.h>
#include <SexyAppFramework/CritSect.h>
#include <SexyAppFramework/Graphics.h>
#include <memory>

namespace Sexy
{
    class SexyAppBase;

    enum BlendMode
    {
        BLENDMODE_NONE = 0,
        BLENDMODE_BLEND,
        BLENDMODE_ADD,
        BLENDMODE_MUL,
    };

    struct ImageData
    {
        int mWidth;
        int mHeight;
        std::vector<uint8_t> mPixels; // The pixels are in the RGBA8 format
    };

    enum TextureFlags
    {
        TextureFlags_MinimizeNumSubdivisions = 0x0001,		// subdivide image into fewest possible textures (may use more memory)
        TextureFlags_Use64By64Subdivisions = 0x0002,		// good to use with image strips so the entire texture isn't pulled in when drawing just a piece
        TextureFlags_UseA4R4G4B4 = 0x0004,		            // images with not too many color gradients work well in this format
        TextureFlags_UseA8R8G8B8 = 0x0008,		            // non-alpha images will be stored as R5G6B5 by default so use this option if you want a 32-bit non-alpha image
        TextureFlags_NearestFiltering = 0x0016              //use the nearest filtering for texture scaling.
    };

    enum PixelFormat
    {
        PixelFormat_Unknown = 0,
        PixelFormat_A8R8G8B8,
        PixelFormat_A4R4G4B4,
        PixelFormat_R5G6B5,
        PixelFormat_Palette8,
    };

    enum RendererResult
    {
        RESULT_FAIL = 0,
        RESULT_OK,
    };

    class Texture
    {
    public:

        int mWidth;
        int mHeight;
        int mBitsChangedCount;
        PixelFormat mPixelFormat;

        Texture() = default;
        ~Texture() = default;

        virtual void ReleaseTextures() {};

        virtual int GetMemSize() {return 0;};
    };

    class Renderer
    {
    public:
        SexyAppBase* mApp;
       	CritSect mCritSect;
        int mWidth;
        int mHeight;
        int mDisplayWidth;
        int mDisplayHeight;
        int mVideoOnlyDraw;

        bool mIs3D;
        bool mHasInitiated;

        Rect mPresentationRect;
        int mRefreshRate;
        int mMillisecondsPerFrame;

        GPUImage *mScreenImage;

		int mRGBBits = 0;
		ulong mRedMask = 0;
		ulong mGreenMask = 0;
		ulong mBlueMask = 0;
		int mRedBits = 0;
		int mGreenBits = 0;
		int mBlueBits = 0;
		int mRedShift = 0;
		int mGreenShift = 0;
		int mBlueShift = 0;
    public:
        Renderer(SexyAppBase* theApp) {};
        ~Renderer() = default;	
        virtual void Cleanup() = 0;

        virtual void AddImage(Image *theImage) = 0;
        virtual void RemoveImage(Image *theImage) = 0;
        virtual void Remove3DData(MemoryImage *theImage) = 0;

        virtual GPUImage *NewGPUImage() = 0;

        virtual GPUImage *GetScreenImage()
        {
            return mScreenImage;
        }
        virtual void UpdateViewport() = 0;
        virtual int Init() = 0;

        virtual bool Redraw(Rect *theClipRect) = 0;
        virtual void SetVideoOnlyDraw(bool videoOnly) = 0;

        virtual std::unique_ptr<ImageData> CaptureFrameBuffer() = 0;

        virtual bool PreDraw() = 0;

        virtual bool CreateImageTexture(GPUImage *theImage) = 0;
        virtual bool RecoverBits(MemoryImage *theImage) = 0;

        virtual std::string GetErrorString() { return "";};

        virtual BlendMode ChooseBlendMode(int theDrawMode)
        {
            switch (theDrawMode)
            {
                case Graphics::DRAWMODE_ADDITIVE:
                    return BLENDMODE_ADD;
                case Graphics::DRAWMODE_NORMAL:
                default:
                    return BLENDMODE_BLEND;
            }
        }

        // Draw Funcs
        virtual void Blt(Image *theImage, int theX, int theY, const Rect &theSrcRect, const Color &theColor,
                        int theDrawMode, bool linearFilter = false) = 0;
        virtual void BltClipF(Image *theImage, float theX, float theY, const Rect &theSrcRect, const Rect *theClipRect,
                            const Color &theColor, int theDrawMode) = 0;
        virtual void BltMirror(Image *theImage, float theX, float theY, const Rect &theSrcRect, const Color &theColor,
                            int theDrawMode, bool linearFilter = false) = 0;
        virtual void StretchBlt(Image *theImage, const Rect &theDestRect, const Rect &theSrcRect, const Rect *theClipRect,
                                const Color &theColor, int theDrawMode, bool fastStretch, bool mirror = false) = 0;
        virtual void BltRotated(Image *theImage, float theX, float theY, const Rect *theClipRect, const Color &theColor,
                                int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY,
                                const Rect &theSrcRect) = 0;
        virtual void BltTransformed(Image *theImage, const Rect *theClipRect, const Color &theColor, int theDrawMode,
                                    const Rect &theSrcRect, const SexyMatrix3 &theTransform, bool linearFilter, float theX = 0,
                                    float theY = 0, bool center = false) = 0;
        virtual void DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color &theColor,
                            int theDrawMode) = 0;
        virtual void FillRect(const Rect &theRect, const Color &theColor, int theDrawMode) = 0;
        virtual void DrawTriangle(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor,
                                int theDrawMode) = 0;
        virtual void DrawTriangleTex(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor,
                                    int theDrawMode, Image *theTexture, bool blend = true) = 0;
        virtual void DrawTrianglesTex(const TriVertex theVertices[][3], int theNumTriangles, const Color &theColor,
                                    int theDrawMode, Image *theTexture, float tx = 0, float ty = 0,
                                    bool blend = true) = 0;
        virtual void DrawTrianglesTexStrip(const TriVertex theVertices[], int theNumTriangles, const Color &theColor,
                                        int theDrawMode, Image *theTexture, float tx = 0, float ty = 0,
                                        bool blend = true) = 0;
        virtual void FillPoly(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor,
                            int theDrawMode, int tx, int ty) = 0;

        virtual void BltTexture(Texture *theTexture, const Rect &theSrcRect, const Rect &theDestRect, const Color &theColor,
                                int theDrawMode) = 0;
    };
extern bool gRendererPreDrawError;
    
} // namespace Sexy
