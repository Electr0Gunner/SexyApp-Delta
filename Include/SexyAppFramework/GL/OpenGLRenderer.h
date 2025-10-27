#pragma once

#include <SexyAppFramework/Renderer.h>
#include <SexyAppFramework/GL/GLImage.h>
#include <SexyAppFramework/GL/GLShader.h>

#include <glad/glad.h>
#include <SDL3/SDL_video.h>

#include <unordered_map>

namespace Sexy
{
    class SexyAppBase;

    struct GLBlendFunc
    {
        GLenum src;
        GLenum dst;
        bool enable_blend = true;
    };

    struct GLVertex
    {
        glm::vec2 mPos;
        glm::vec2 mTexCoord;
        glm::vec4 mColor;
    };

    struct GLDrawCommand
    {
        GLenum mPrimitiveType;
        GLuint mTextureID;
        BlendMode mBlendMode;
        std::vector<GLVertex> mVertices;
        GLShader *mShader = nullptr;
        const Rect *mClipRect = nullptr;
    };

    typedef std::set<GLImage* > GLImageSet;

    inline const std::unordered_map<BlendMode, GLBlendFunc> gGLBlendModeFuncs = {
        {BlendMode::BLENDMODE_BLEND, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, true}},
        {BlendMode::BLENDMODE_ADD, {GL_SRC_ALPHA, GL_ONE, true}},
        {BlendMode::BLENDMODE_MUL, {GL_DST_COLOR, GL_ZERO, true}}};


    struct GLTextureData : public Texture
    {
    public:
        GLuint mTextureID;

        GLTextureData();
        ~GLTextureData();

        virtual void ReleaseTextures();

        virtual void CreateTextures(GLImage *theImage);
        virtual void CheckCreateTextures(GLImage *theImage);

        virtual int GetMemSize();
    };

    class OpenGLRenderer: public Renderer
    {
    public:
        GLuint mVAO;
        GLuint mVBO;
        SDL_GLContext mContext;
        GLShader* mDefaultShader;
        GLImageSet mImageSet;
        glm::mat4 mProjection;
        std::vector<GLDrawCommand> mCommandBuffer;
    public:
        OpenGLRenderer(SexyAppBase* theApp);
        ~OpenGLRenderer();

       virtual void Cleanup();

        virtual void AddImage(Image *theImage);
        virtual void RemoveImage(Image *theImage);
        virtual void Remove3DData(MemoryImage *theImage);

        virtual GPUImage *NewGPUImage() 
        {
            return new GLImage(this);
        };

        virtual void UpdateViewport();
        virtual int Init();

        virtual bool Redraw(Rect *theClipRect);
        virtual void SetVideoOnlyDraw(bool videoOnly);

        virtual std::unique_ptr<ImageData> CaptureFrameBuffer();

        virtual bool PreDraw();

        virtual bool CreateImageTexture(GPUImage *theImage);
        virtual bool RecoverBits(MemoryImage *theImage);

        virtual std::string GetErrorString();

        virtual void Blt(Image *theImage, int theX, int theY, const Rect &theSrcRect, const Color &theColor,
                        int theDrawMode, bool linearFilter = false);

        virtual void BltClipF(Image *theImage, float theX, float theY, const Rect &theSrcRect, const Rect *theClipRect,
                            const Color &theColor, int theDrawMode);

        virtual void BltMirror(Image *theImage, float theX, float theY, const Rect &theSrcRect, const Color &theColor,
                            int theDrawMode, bool linearFilter = false);

        virtual void StretchBlt(Image *theImage, const Rect &theDestRect, const Rect &theSrcRect, const Rect *theClipRect,
                                const Color &theColor, int theDrawMode, bool fastStretch, bool mirror = false);

        virtual void BltRotated(Image *theImage, float theX, float theY, const Rect *theClipRect, const Color &theColor,
                                int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY,
                                const Rect &theSrcRect);

        virtual void BltTransformed(Image *theImage, const Rect *theClipRect, const Color &theColor, int theDrawMode,
                                    const Rect &theSrcRect, const SexyMatrix3 &theTransform, bool linearFilter, float theX = 0,
                                    float theY = 0, bool center = false);

        virtual void DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color &theColor,
                            int theDrawMode);

        virtual void FillRect(const Rect &theRect, const Color &theColor, int theDrawMode);

        virtual void DrawTriangle(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor,
                                int theDrawMode);

        virtual void DrawTriangleClipped(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor, const Rect *theClipRect, int theDrawMode);

        virtual void DrawTriangleTex(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor,
                                    int theDrawMode, Image *theTexture, bool blend = true);

        virtual void DrawTrianglesTex(const TriVertex theVertices[][3], int theNumTriangles, const Color &theColor,
                                    int theDrawMode, Image *theTexture, float tx = 0, float ty = 0, bool blend = true);

        virtual void DrawTrianglesTexStrip(const TriVertex theVertices[], int theNumTriangles, const Color &theColor,
                                        int theDrawMode, Image *theTexture, float tx = 0, float ty = 0,
                                        bool blend = true);

        virtual void FillPoly(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor,
                            int theDrawMode, int tx, int ty);

        virtual void BltTexture(Texture *theTexture, const Rect &theSrcRect, const Rect &theDestRect, const Color &theColor,
                                int theDrawMode);

        //Renderer specific
        GLImage* SetupImage(Image *theImage);
        void ApplyBlendMode(BlendMode theMode);
        void AddCommand(const GLDrawCommand &command);

    };
} // namespace Sexy
