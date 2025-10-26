#include <SexyAppFramework/GL/OpenGLRenderer.h>
#include <SexyAppFramework/SexyAppBase.h>
#include <SexyAppFramework/AutoCrit.h>
#include <SexyAppFramework/SexyMatrix.h>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL3/SDL_video.h>

using namespace Sexy;

//16-bit max value, should be plenty
#define MAX_VERTICES 65536 

const char *gVertexShaderSrc = R"glsl(
#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;
layout(location = 2) in vec4 aColor;

uniform mat4 uProjection;

out vec2 vTexCoord;
out vec4 vColor;

void main() {
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
    vTexCoord = aTex;
    vColor = aColor;
}
)glsl";

const char *gFragmentShaderSrc = R"glsl(
#version 330 core

in vec2 vTexCoord;
in vec4 vColor;

uniform sampler2D uTexture;
uniform bool uUseTexture;

out vec4 FragColor;

void main() {
    if (uUseTexture)
        FragColor = texture(uTexture, vTexCoord) * vColor;
    else
        FragColor = vColor;
}
)glsl";

OpenGLRenderer::OpenGLRenderer(SexyAppBase* theApp): Renderer(theApp)
{
    mApp = theApp;
    mWidth = theApp->mWidth;
    mHeight = theApp->mHeight;
    mDisplayWidth = mWidth;
	mDisplayHeight = mHeight;
	mPresentationRect = Rect(0, 0, mWidth, mHeight);
	mScreenImage = nullptr;
	mHasInitiated = false;
	mIs3D = true;
	mMillisecondsPerFrame = 0;
	mRefreshRate = 0;
    mVBO = 0;
    mVAO = 0;
    mDefaultShader = nullptr;
    mProjection = glm::mat4();
}

OpenGLRenderer::~OpenGLRenderer()
{
    Cleanup();
}

void OpenGLRenderer::Cleanup()
{
    if (mScreenImage)
        delete mScreenImage;

    SDL_GL_DestroyContext(mContext);

    glDeleteBuffers(1, &mVBO);
    glDeleteVertexArrays(1, &mVAO);
}

int OpenGLRenderer::Init()
{
    int aResult = RESULT_OK;

    if (mHasInitiated)
        Cleanup();

    mContext = SDL_GL_CreateContext(mApp->mWindow);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		//Hmm fuck...
        printf("Well uhh, sir...? OpenGL doesn't work");
        aResult = RESULT_FAIL;
		return aResult;
	}

	SDL_GL_MakeCurrent(mApp->mWindow, mContext);

	mDefaultShader = new GLShader();
	mDefaultShader->LoadFromSource(gVertexShaderSrc, gFragmentShaderSrc);

	SetVideoOnlyDraw(false);

	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);

	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);

	glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(GLVertex), nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLVertex), (void *)offsetof(GLVertex, mPos));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLVertex), (void *)offsetof(GLVertex, mTexCoord));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(GLVertex), (void *)offsetof(GLVertex, mColor));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void OpenGLRenderer::UpdateViewport()
{
    //Warning:
    //I don't understand any of this shit.
    //This is from a random blog online
    // https://www.david-amador.com/2013/04/opengl-2d-independent-resolution-rendering/

    int aWindowWidth, aWindowHeight;

    SDL_GetWindowSize(mApp->mWindow, &aWindowWidth, &aWindowHeight);

    float aTargetAspectRatio = (float)mWidth / (float)mHeight;

    int aMaxWidth = aWindowWidth;
    int aMaxHeight = (int)(aMaxWidth / aTargetAspectRatio + 0.5f);

    if (aMaxHeight > aWindowHeight)
    {
        aMaxHeight = aWindowHeight;
        aMaxWidth = (int)(aMaxHeight * aTargetAspectRatio + 0.5f);
    }

    int aViewportX = (aWindowWidth / 2) - (aMaxWidth / 2);
    int aViewportY = (aWindowHeight / 2) - (aMaxHeight / 2);

    glViewport(aViewportX, aViewportY, aMaxWidth, aMaxHeight);

    mPresentationRect = Rect(aViewportX, aViewportY, aMaxWidth, aMaxHeight);

    mProjection = glm::ortho(0.0f, (float)mPresentationRect.mWidth, (float)mPresentationRect.mHeight, 0.0f, -1.0f, 1.0f);
}

bool OpenGLRenderer::Redraw(Rect* theClipRect)
{
    SDL_GL_SwapWindow(mApp->mWindow);

    return true;// !gRendererPreDrawError; later
}

void OpenGLRenderer::SetVideoOnlyDraw(bool videoOnly)
{
    if (mScreenImage)
        delete mScreenImage;
    mScreenImage = new GLImage(this);
    mScreenImage->Create(mWidth, mHeight);
    mScreenImage->SetImageMode(false, false);
}

std::unique_ptr<ImageData> OpenGLRenderer::CaptureFrameBuffer()
{
    uint8_t* aPixels = new uint8_t[3 * mWidth * mHeight];

    glReadPixels(0, 0, mWidth, mHeight, GL_RGB, GL_UNSIGNED_BYTE, aPixels);

    std::unique_ptr<ImageData> anOutput = std::make_unique<ImageData>();
    anOutput->mWidth = mWidth;
    anOutput->mHeight = mHeight;
    anOutput->mPixels.resize(mWidth * mHeight * 3);

	uint8_t *src = static_cast<uint8_t *>(aPixels);
	uint8_t *dst = anOutput->mPixels.data();

	for (int i = 0; i < mWidth * mHeight * 3; i += 3)
	{
		dst[i + 0] = src[i + 2]; // R
		dst[i + 1] = src[i + 1]; // G
		dst[i + 2] = src[i + 0]; // B
	}

    return anOutput;
}

bool OpenGLRenderer::CreateImageTexture(GPUImage *theImage)
{
	bool wantPurge = false;

	if (theImage->mD3DData == nullptr)
	{
		theImage->mD3DData = new GLTextureData();

		// The actual purging was deferred
		wantPurge = theImage->mPurgeBits;

		AutoCrit aCrit(mCritSect); // Make images thread safe
		mImageSet.insert(static_cast<GLImage *>(theImage));
	}

	GLTextureData *aData = static_cast<GLTextureData*>(theImage->mD3DData);
	aData->CheckCreateTextures(static_cast<GLImage*>(theImage));

	if (wantPurge)
		theImage->PurgeBits();

	return true;
}


void OpenGLRenderer::AddImage(Image *theImage)
{
	AutoCrit anAutoCrit(mCritSect);

	mImageSet.insert((GLImage*)theImage);
}

void OpenGLRenderer::RemoveImage(Image *theImage)
{
	AutoCrit anAutoCrit(mCritSect);

	GLImageSet::iterator anItr = mImageSet.find((GLImage*)theImage);
	if (anItr != mImageSet.end())
		mImageSet.erase(anItr);
}

void OpenGLRenderer::Remove3DData(GPUImage *theImage)
{
	if (theImage->mD3DData != nullptr)
	{
		delete (GLTextureData*)theImage->mD3DData;
		theImage->mD3DData = nullptr;

		AutoCrit aCrit(mCritSect); // Make images thread safe
		mImageSet.erase(static_cast<GLImage*>(theImage));
	}
}
bool OpenGLRenderer::RecoverBits(GPUImage *theImage)
{
	if (theImage->mD3DData == nullptr)
		return false;

	GLTextureData *aData = (GLTextureData *)theImage->mD3DData;
	if (aData->mBitsChangedCount != theImage->mBitsChangedCount) // bits have changed since texture was created
		return false;

	// Reverse the process: copy texture data to theImage
	void *pixels = nullptr;
	glBindTexture(GL_TEXTURE_2D, aData->mTextureID);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	theImage->SetBits((ulong *)pixels, aData->mWidth, aData->mHeight); //TODO: REPLACE ulong with uint32_t

	return true;
}

bool OpenGLRenderer::PreDraw()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(mPresentationRect.mX, mPresentationRect.mY, mPresentationRect.mWidth, mPresentationRect.mHeight);

	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	return true;
}


GLImage* OpenGLRenderer::SetupImage(Image *theImage)
{
	GLImage* aImg = static_cast<GLImage*>(theImage);

	if (aImg->mD3DData == nullptr)
		CreateImageTexture(aImg);
	return aImg;
}

void OpenGLRenderer::ApplyBlendMode(BlendMode theMode)
{
    auto it = gGLBlendModeFuncs.find(theMode);
	if (it == gGLBlendModeFuncs.end())
		return;

	const auto &blend = it->second;

	if (blend.enable_blend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(blend.src, blend.dst);
	}
	else
	{
		glDisable(GL_BLEND);
	}
}

void OpenGLRenderer::Blt(Image *theImage, int theX, int theY, const Rect &theSrcRect, const Color &theColor, int theDrawMode, bool linearFilter = false)
{
    PreDraw();
    ApplyBlendMode(ChooseBlendMode(theDrawMode));
    GLImage* aImg = SetupImage(theImage);

    glm::vec2 p0 = {theX, theY};
	glm::vec2 p1 = {theX + theSrcRect.mWidth, theY};
	glm::vec2 p2 = {theX + theSrcRect.mWidth, theY + theSrcRect.mHeight};
	glm::vec2 p3 = {theX, theY + theSrcRect.mHeight};

	float u0 = (float)theSrcRect.mX / (float)theImage->mWidth;
	float v0 = (float)theSrcRect.mY / (float)theImage->mHeight;
	float u1 = (float)(theSrcRect.mX + theSrcRect.mWidth) / (float)theImage->mWidth;
	float v1 = (float)(theSrcRect.mY + theSrcRect.mHeight) / (float)theImage->mHeight;

	glm::vec2 uv0 = {u0, v0};
	glm::vec2 uv1 = {u1, v0};
	glm::vec2 uv2 = {u1, v1};
	glm::vec2 uv3 = {u0, v1};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f, (float)theColor.mAlpha / 255.0f};

    std::vector<GLVertex> aVertexArray;

	aVertexArray.push_back({p0, uv0, aColor});
	aVertexArray.push_back({p1, uv1, aColor});
	aVertexArray.push_back({p2, uv2, aColor});
	aVertexArray.push_back({p2, uv2, aColor});
	aVertexArray.push_back({p3, uv3, aColor});
	aVertexArray.push_back({p0, uv0, aColor});

    GLShader* aShaderToUse;
   // if (cmd.mShader != nullptr)
    //    aShaderToUse = cmd.mShader;
  //  else
        aShaderToUse = mDefaultShader;

    GLuint aTextureID = static_cast<GLTextureData*>(aImg->mD3DData)->mTextureID;
    aShaderToUse->Use();
    aShaderToUse->SetUniform("uProjection", mProjection);
    aShaderToUse->SetUniform("uUseTexture", (aTextureID != 0));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, aTextureID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, aVertexArray.size() * sizeof(GLVertex), aVertexArray.data());
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)aVertexArray.size());
}

void OpenGLRenderer::BltClipF(Image *theImage, float theX, float theY, const Rect &theSrcRect, const Rect *theClipRect, const Color &theColor, int theDrawMode)
{
    PreDraw();
    ApplyBlendMode(ChooseBlendMode(theDrawMode));
    GLImage* aImg = SetupImage(theImage);

    glm::vec2 p0 = {theX, theY};
	glm::vec2 p1 = {theX + theSrcRect.mWidth, theY};
	glm::vec2 p2 = {theX + theSrcRect.mWidth, theY + theSrcRect.mHeight};
	glm::vec2 p3 = {theX, theY + theSrcRect.mHeight};

	float u0 = (float)theSrcRect.mX / (float)theImage->mWidth;
	float v0 = (float)theSrcRect.mY / (float)theImage->mHeight;
	float u1 = (float)(theSrcRect.mX + theSrcRect.mWidth) / (float)theImage->mWidth;
	float v1 = (float)(theSrcRect.mY + theSrcRect.mHeight) / (float)theImage->mHeight;

	glm::vec2 uv0 = {u0, v0};
	glm::vec2 uv1 = {u1, v0};
	glm::vec2 uv2 = {u1, v1};
	glm::vec2 uv3 = {u0, v1};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f, (float)theColor.mAlpha / 255.0f};

    std::vector<GLVertex> aVertexArray;

	aVertexArray.push_back({p0, uv0, aColor});
	aVertexArray.push_back({p1, uv1, aColor});
	aVertexArray.push_back({p2, uv2, aColor});
	aVertexArray.push_back({p2, uv2, aColor});
	aVertexArray.push_back({p3, uv3, aColor});
	aVertexArray.push_back({p0, uv0, aColor});

    GLShader* aShaderToUse;
   // if (cmd.mShader != nullptr)
    //    aShaderToUse = cmd.mShader;
  //  else
        aShaderToUse = mDefaultShader;

    GLuint aTextureID = static_cast<GLTextureData*>(aImg->mD3DData)->mTextureID;
    aShaderToUse->Use();
    aShaderToUse->SetUniform("uProjection", mProjection);
    aShaderToUse->SetUniform("uUseTexture", (aTextureID != 0));
    glEnable(GL_SCISSOR_TEST);
    if (theClipRect != nullptr)
        glScissor(theClipRect->mX, theClipRect->mY, theClipRect->mWidth, theClipRect->mHeight);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, aTextureID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, aVertexArray.size() * sizeof(GLVertex), aVertexArray.data());
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)aVertexArray.size());
    
    glDisable(GL_SCISSOR_TEST);
}

void OpenGLRenderer::BltMirror(Image *theImage, float theX, float theY, const Rect &theSrcRect, const Color &theColor, int theDrawMode, bool linearFilter = false)
{
    PreDraw();
    ApplyBlendMode(ChooseBlendMode(theDrawMode));
    GLImage* aImg = SetupImage(theImage);

    glm::vec2 p0 = {theX, theY};
	glm::vec2 p1 = {theX + theSrcRect.mWidth, theY};
	glm::vec2 p2 = {theX + theSrcRect.mWidth, theY + theSrcRect.mHeight};
	glm::vec2 p3 = {theX, theY + theSrcRect.mHeight};

	float u0 = (float)theSrcRect.mX / (float)theImage->mWidth;
	float v0 = (float)theSrcRect.mY / (float)theImage->mHeight;
	float u1 = (float)(theSrcRect.mX + theSrcRect.mWidth) / (float)theImage->mWidth;
	float v1 = (float)(theSrcRect.mY + theSrcRect.mHeight) / (float)theImage->mHeight;

    std::swap(u0, u1); //That's it. Yep. That's it.

	glm::vec2 uv0 = {u0, v0};
	glm::vec2 uv1 = {u1, v0};
	glm::vec2 uv2 = {u1, v1};
	glm::vec2 uv3 = {u0, v1};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f, (float)theColor.mAlpha / 255.0f};

    std::vector<GLVertex> aVertexArray;

	aVertexArray.push_back({p0, uv0, aColor});
	aVertexArray.push_back({p1, uv1, aColor});
	aVertexArray.push_back({p2, uv2, aColor});
	aVertexArray.push_back({p2, uv2, aColor});
	aVertexArray.push_back({p3, uv3, aColor});
	aVertexArray.push_back({p0, uv0, aColor});

    GLShader* aShaderToUse;
   // if (cmd.mShader != nullptr)
    //    aShaderToUse = cmd.mShader;
  //  else
        aShaderToUse = mDefaultShader;

    GLuint aTextureID = static_cast<GLTextureData*>(aImg->mD3DData)->mTextureID;
    aShaderToUse->Use();
    aShaderToUse->SetUniform("uProjection", mProjection);
    aShaderToUse->SetUniform("uUseTexture", (aTextureID != 0));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, aTextureID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, aVertexArray.size() * sizeof(GLVertex), aVertexArray.data());
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)aVertexArray.size());
}

void OpenGLRenderer::StretchBlt(Image *theImage, const Rect &theDestRect, const Rect &theSrcRect, const Rect *theClipRect, const Color &theColor, int theDrawMode, bool fastStretch, bool mirror = false)
{
    PreDraw();
    ApplyBlendMode(ChooseBlendMode(theDrawMode));
    GLImage* aImg = SetupImage(theImage);

	glm::vec2 p0 = {theDestRect.mX, theDestRect.mY};
	glm::vec2 p1 = {theDestRect.mX + theDestRect.mWidth, theDestRect.mY};
	glm::vec2 p2 = {theDestRect.mX + theDestRect.mWidth, theDestRect.mY + theDestRect.mHeight};
	glm::vec2 p3 = {theDestRect.mX, theDestRect.mY + theDestRect.mHeight};
    
	float u0 = (float)theSrcRect.mX / (float)theImage->mWidth;
	float v0 = (float)theSrcRect.mY / (float)theImage->mHeight;
	float u1 = (float)(theSrcRect.mX + theSrcRect.mWidth) / (float)theImage->mWidth;
	float v1 = (float)(theSrcRect.mY + theSrcRect.mHeight) / (float)theImage->mHeight;

	if (mirror)
	{
		std::swap(u0, u1);
	}

	glm::vec2 uv0 = {u0, v0};
	glm::vec2 uv1 = {u1, v0};
	glm::vec2 uv2 = {u1, v1};
	glm::vec2 uv3 = {u0, v1};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f, (float)theColor.mAlpha / 255.0f};

    std::vector<GLVertex> aVertexArray;

	aVertexArray.push_back({p0, uv0, aColor});
	aVertexArray.push_back({p1, uv1, aColor});
	aVertexArray.push_back({p2, uv2, aColor});
	aVertexArray.push_back({p2, uv2, aColor});
	aVertexArray.push_back({p3, uv3, aColor});
	aVertexArray.push_back({p0, uv0, aColor});

    GLShader* aShaderToUse;
   // if (cmd.mShader != nullptr)
    //    aShaderToUse = cmd.mShader;
  //  else
        aShaderToUse = mDefaultShader;

    GLuint aTextureID = static_cast<GLTextureData*>(aImg->mD3DData)->mTextureID;
    aShaderToUse->Use();
    aShaderToUse->SetUniform("uProjection", mProjection);
    aShaderToUse->SetUniform("uUseTexture", (aTextureID != 0));
    glEnable(GL_SCISSOR_TEST);
    if (theClipRect != nullptr)
        glScissor(theClipRect->mX, theClipRect->mY, theClipRect->mWidth, theClipRect->mHeight);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, aTextureID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, aVertexArray.size() * sizeof(GLVertex), aVertexArray.data());
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)aVertexArray.size());
    
    glDisable(GL_SCISSOR_TEST);
}

//rotation is so annoying...

glm::vec2 RotatePointAroundPivot(const glm::vec2 point, const glm::vec2 center, float angleRad)
{
	float sinValue = sin(angleRad);
	float cosValue = cos(angleRad);

	glm::vec2 translation = point - center;

	glm::vec2 rotation = {translation.x * cosValue - translation.y * sinValue,
						  translation.x * sinValue + translation.y * cosValue};

	return rotation + center;
}

void OpenGLRenderer::BltRotated(Image *theImage, float theX, float theY, const Rect *theClipRect, const Color &theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY, const Rect &theSrcRect)
{
    PreDraw();
    ApplyBlendMode(ChooseBlendMode(theDrawMode));
    GLImage* aImg = SetupImage(theImage);

	glm::vec2 p0 = {theX, theY};
	glm::vec2 p1 = {theX + theSrcRect.mWidth, theY};
	glm::vec2 p2 = {theX + theSrcRect.mWidth, theY + theSrcRect.mHeight};
	glm::vec2 p3 = {theX, theY + theSrcRect.mHeight};
    
	float u0 = (float)theSrcRect.mX / (float)theImage->mWidth;
	float v0 = (float)theSrcRect.mY / (float)theImage->mHeight;
	float u1 = (float)(theSrcRect.mX + theSrcRect.mWidth) / (float)theImage->mWidth;
	float v1 = (float)(theSrcRect.mY + theSrcRect.mHeight) / (float)theImage->mHeight;

	glm::vec2 uv0 = {u0, v0};
	glm::vec2 uv1 = {u1, v0};
	glm::vec2 uv2 = {u1, v1};
	glm::vec2 uv3 = {u0, v1};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f, (float)theColor.mAlpha / 255.0f};

	float radians = glm::radians(theRot);
	glm::vec2 center = {theRotCenterX + theX, theRotCenterY + theY};
	p0 = RotatePointAroundPivot(p0, center, radians);
	p1 = RotatePointAroundPivot(p1, center, radians);
	p2 = RotatePointAroundPivot(p2, center, radians);
	p3 = RotatePointAroundPivot(p3, center, radians);

    std::vector<GLVertex> aVertexArray;

	aVertexArray.push_back({p0, uv0, aColor});
	aVertexArray.push_back({p1, uv1, aColor});
	aVertexArray.push_back({p2, uv2, aColor});
	aVertexArray.push_back({p2, uv2, aColor});
	aVertexArray.push_back({p3, uv3, aColor});
	aVertexArray.push_back({p0, uv0, aColor});

    GLShader* aShaderToUse;
   // if (cmd.mShader != nullptr)
    //    aShaderToUse = cmd.mShader;
  //  else
        aShaderToUse = mDefaultShader;

    GLuint aTextureID = static_cast<GLTextureData*>(aImg->mD3DData)->mTextureID;
    aShaderToUse->Use();
    aShaderToUse->SetUniform("uProjection", mProjection);
    aShaderToUse->SetUniform("uUseTexture", (aTextureID != 0));
    glEnable(GL_SCISSOR_TEST);
    if (theClipRect != nullptr)
        glScissor(theClipRect->mX, theClipRect->mY, theClipRect->mWidth, theClipRect->mHeight);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, aTextureID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, aVertexArray.size() * sizeof(GLVertex), aVertexArray.data());
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)aVertexArray.size());
    
    glDisable(GL_SCISSOR_TEST);
}

glm::vec2 TransformToGLMPoint(float x, float y, const SexyMatrix3 &m, float aTransX = 0, float aTransY = 0)
{
	glm::vec2 result;
	result.x = m.m00 * x + m.m01 * y + m.m02 + aTransX;
	result.y = m.m10 * x + m.m11 * y + m.m12 + aTransY;
	return result;
}

void OpenGLRenderer::BltTransformed(Image *theImage, const Rect *theClipRect, const Color &theColor, int theDrawMode, const Rect &theSrcRect, const SexyMatrix3 &theTransform, bool linearFilter, float theX = 0, float theY = 0, bool center = false)
{
    PreDraw();
    ApplyBlendMode(ChooseBlendMode(theDrawMode));
    GLImage* aImg = SetupImage(theImage);

	float aWidth = static_cast<float>(theSrcRect.mWidth);
	float aHeight = static_cast<float>(theSrcRect.mHeight);

	glm::vec2 origin = {0.0f, 0.0f};
	if (center)
		origin = {aWidth * 0.5f, aHeight * 0.5f};

	glm::vec2 localP0 = {-origin.x, -origin.y};
	glm::vec2 localP1 = {aWidth - origin.x, -origin.y};
	glm::vec2 localP2 = {aWidth - origin.x, aHeight - origin.y};
	glm::vec2 localP3 = {-origin.x, aHeight - origin.y};

	glm::vec2 p0 = TransformToGLMPoint(localP0.x, localP0.y, theTransform, theX, theY);
	glm::vec2 p1 = TransformToGLMPoint(localP1.x, localP1.y, theTransform, theX, theY);
	glm::vec2 p2 = TransformToGLMPoint(localP2.x, localP2.y, theTransform, theX, theY);
	glm::vec2 p3 = TransformToGLMPoint(localP3.x, localP3.y, theTransform, theX, theY);

	float u0 = (float)theSrcRect.mX / (float)theImage->mWidth;
	float v0 = (float)theSrcRect.mY / (float)theImage->mHeight;
	float u1 = (float)(theSrcRect.mX + theSrcRect.mWidth) / (float)theImage->mWidth;
	float v1 = (float)(theSrcRect.mY + theSrcRect.mHeight) / (float)theImage->mHeight;

	glm::vec2 uv0 = {u0, v0};
	glm::vec2 uv1 = {u1, v0};
	glm::vec2 uv2 = {u1, v1};
	glm::vec2 uv3 = {u0, v1};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f,
						(float)theColor.mAlpha / 255.0f};


    std::vector<GLVertex> aVertexArray;

	aVertexArray.push_back({p0, uv0, aColor});
	aVertexArray.push_back({p1, uv1, aColor});
	aVertexArray.push_back({p2, uv2, aColor});
	aVertexArray.push_back({p2, uv2, aColor});
	aVertexArray.push_back({p3, uv3, aColor});
	aVertexArray.push_back({p0, uv0, aColor});

    GLShader* aShaderToUse;
   // if (cmd.mShader != nullptr)
    //    aShaderToUse = cmd.mShader;
  //  else
        aShaderToUse = mDefaultShader;

    GLuint aTextureID = static_cast<GLTextureData*>(aImg->mD3DData)->mTextureID;
    aShaderToUse->Use();
    aShaderToUse->SetUniform("uProjection", mProjection);
    aShaderToUse->SetUniform("uUseTexture", (aTextureID != 0));
    glEnable(GL_SCISSOR_TEST);
    if (theClipRect != nullptr)
        glScissor(theClipRect->mX, theClipRect->mY, theClipRect->mWidth, theClipRect->mHeight);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, aTextureID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, aVertexArray.size() * sizeof(GLVertex), aVertexArray.data());
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)aVertexArray.size());
    
    glDisable(GL_SCISSOR_TEST);
}

void OpenGLRenderer::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color &theColor, int theDrawMode)
{
    PreDraw();
    ApplyBlendMode(ChooseBlendMode(theDrawMode));

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f, (float)theColor.mAlpha / 255.0f};

    std::vector<GLVertex> aVertexArray;

	aVertexArray.push_back({{theStartX, theStartY}, {}, aColor});
	aVertexArray.push_back({{theEndX, theEndY}, {}, aColor});

    GLShader* aShaderToUse;
   // if (cmd.mShader != nullptr)
    //    aShaderToUse = cmd.mShader;
  //  else
        aShaderToUse = mDefaultShader;

    aShaderToUse->Use();
    aShaderToUse->SetUniform("uProjection", mProjection);
    aShaderToUse->SetUniform("uUseTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBufferSubData(GL_ARRAY_BUFFER, 0, aVertexArray.size() * sizeof(GLVertex), aVertexArray.data());
    glDrawArrays(GL_LINES, 0, (GLsizei)aVertexArray.size());
}

void OpenGLRenderer::FillRect(const Rect &theRect, const Color &theColor, int theDrawMode)
{
    PreDraw();
    ApplyBlendMode(ChooseBlendMode(theDrawMode));
    
	glm::vec2 p0 = {theRect.mX, theRect.mY};
	glm::vec2 p1 = {theRect.mX + theRect.mWidth, theRect.mY};
	glm::vec2 p2 = {theRect.mX + theRect.mWidth, theRect.mY + theRect.mHeight};
	glm::vec2 p3 = {theRect.mX, theRect.mY + theRect.mHeight};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f, (float)theColor.mAlpha / 255.0f};

    std::vector<GLVertex> aVertexArray;

	aVertexArray.push_back({p0, {}, aColor});
	aVertexArray.push_back({p1, {}, aColor});
	aVertexArray.push_back({p2, {}, aColor});
	aVertexArray.push_back({p2, {}, aColor});
	aVertexArray.push_back({p3, {}, aColor});
	aVertexArray.push_back({p0, {}, aColor});

    GLShader* aShaderToUse;
   // if (cmd.mShader != nullptr)
    //    aShaderToUse = cmd.mShader;
  //  else
        aShaderToUse = mDefaultShader;

    aShaderToUse->Use();
    aShaderToUse->SetUniform("uProjection", mProjection);
    aShaderToUse->SetUniform("uUseTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBufferSubData(GL_ARRAY_BUFFER, 0, aVertexArray.size() * sizeof(GLVertex), aVertexArray.data());
    glDrawArrays(GL_LINES, 0, (GLsizei)aVertexArray.size());
}

void OpenGLRenderer::DrawTriangle(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor, int theDrawMode)
{
    PreDraw();
    ApplyBlendMode(ChooseBlendMode(theDrawMode));

	glm::vec2 vert0 = {p1.x, p1.y};
	glm::vec2 vert1 = {p2.x, p2.y};
	glm::vec2 vert2 = {p3.x, p3.y};
	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f, (float)theColor.mAlpha / 255.0f};

    std::vector<GLVertex> aVertexArray;

	aVertexArray.push_back({vert0, {p1.u, p1.v}, aColor});
	aVertexArray.push_back({vert1, {p2.u, p2.v}, aColor});
	aVertexArray.push_back({vert2, {p3.u, p3.v}, aColor});

    GLShader* aShaderToUse;
   // if (cmd.mShader != nullptr)
    //    aShaderToUse = cmd.mShader;
  //  else
        aShaderToUse = mDefaultShader;

    aShaderToUse->Use();
    aShaderToUse->SetUniform("uProjection", mProjection);
    aShaderToUse->SetUniform("uUseTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBufferSubData(GL_ARRAY_BUFFER, 0, aVertexArray.size() * sizeof(GLVertex), aVertexArray.data());
    glDrawArrays(GL_LINES, 0, (GLsizei)aVertexArray.size());
}

void OpenGLRenderer::DrawTriangleTex(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor, int theDrawMode, Image *theTexture, bool blend = true)
{
    PreDraw();
    ApplyBlendMode(ChooseBlendMode(theDrawMode));
    GLImage *aImg = SetupImage(theTexture);

	glm::vec2 vert0 = {p1.x, p1.y};
	glm::vec2 vert1 = {p2.x, p2.y};
	glm::vec2 vert2 = {p3.x, p3.y};
	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f, (float)theColor.mAlpha / 255.0f};

    std::vector<GLVertex> aVertexArray;

	aVertexArray.push_back({vert0, {p1.u, p1.v}, aColor});
	aVertexArray.push_back({vert1, {p2.u, p2.v}, aColor});
	aVertexArray.push_back({vert2, {p3.u, p3.v}, aColor});

    GLShader* aShaderToUse;
   // if (cmd.mShader != nullptr)
    //    aShaderToUse = cmd.mShader;
  //  else
        aShaderToUse = mDefaultShader;

    GLuint aTextureID = static_cast<GLTextureData*>(aImg->mD3DData)->mTextureID;
    aShaderToUse->Use();
    aShaderToUse->SetUniform("uProjection", mProjection);
    aShaderToUse->SetUniform("uUseTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, aTextureID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, aVertexArray.size() * sizeof(GLVertex), aVertexArray.data());
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)aVertexArray.size());
}

void OpenGLRenderer::DrawTrianglesTex(const TriVertex theVertices[][3], int theNumTriangles, const Color &theColor, int theDrawMode, Image *theTexture, float tx = 0, float ty = 0, bool blend = true)
{
	for (int aTriangleNum = 0; aTriangleNum < theNumTriangles; aTriangleNum++)
	{
		TriVertex v0 = theVertices[aTriangleNum][0];
		TriVertex v1 = theVertices[aTriangleNum][1];
		TriVertex v2 = theVertices[aTriangleNum][2];

		v0.x += tx;
		v0.y += ty;
		v1.x += tx;
		v1.y += ty;
		v2.x += tx;
		v2.y += ty;

		DrawTriangleTex(v0, v1, v2, theColor, theDrawMode, theTexture, blend);
	}
}
void OpenGLRenderer::DrawTrianglesTexStrip(const TriVertex theVertices[], int theNumTriangles, const Color &theColor, int theDrawMode, Image *theTexture, float tx = 0, float ty = 0, bool blend = true)
{
	TriVertex aList[100][3];
	int aTriNum = 0;
	while (aTriNum < theNumTriangles)
	{
		int aMaxTriangles = std::min(100, theNumTriangles - aTriNum);
		for (int i = 0; i < aMaxTriangles; i++)
		{
			aList[i][0] = theVertices[aTriNum];
			aList[i][1] = theVertices[aTriNum + 1];
			aList[i][2] = theVertices[aTriNum + 2];
			aTriNum++;
		}
		DrawTrianglesTex(aList, aMaxTriangles, theColor, theDrawMode, theTexture, tx, ty, blend);
	}
}

void OpenGLRenderer::FillPoly(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty)
{
	if (theNumVertices < 3)
		return;

	for (int i = 1; i < theNumVertices - 1; ++i)
	{
		TriVertex v0, v1, v2;

		v0.x = theVertices[0].mX + tx;
		v0.y = theVertices[0].mY + ty;

		v1.x = theVertices[i].mX + tx;
		v1.y = theVertices[i].mY + ty;

		v2.x = theVertices[i + 1].mX + tx;
		v2.y = theVertices[i + 1].mY + ty;

        glEnable(GL_SCISSOR_TEST);
        if (theClipRect != nullptr)
            glScissor(theClipRect->mX, theClipRect->mY, theClipRect->mWidth, theClipRect->mHeight);
		DrawTriangle(v0, v1, v2, theColor, theDrawMode);
	}
    glDisable(GL_SCISSOR_TEST);
}

void OpenGLRenderer::BltTexture(Texture *theTexture, const Rect &theSrcRect, const Rect &theDestRect, const Color &theColor, int theDrawMode)
{
    PreDraw();
    ApplyBlendMode(ChooseBlendMode(theDrawMode));
    GLTextureData* aTex = static_cast<GLTextureData*>(theTexture);

	glm::vec2 p0 = {theDestRect.mX, theDestRect.mY};
	glm::vec2 p1 = {theDestRect.mX + theDestRect.mWidth, theDestRect.mY};
	glm::vec2 p2 = {theDestRect.mX + theDestRect.mWidth, theDestRect.mY + theDestRect.mHeight};
	glm::vec2 p3 = {theDestRect.mX, theDestRect.mY + theDestRect.mHeight};
    
	float u0 = (float)theSrcRect.mX / (float)aTex->mWidth;
	float v0 = (float)theSrcRect.mY / (float)aTex->mHeight;
	float u1 = (float)(theSrcRect.mX + theSrcRect.mWidth) / (float)aTex->mWidth;
	float v1 = (float)(theSrcRect.mY + theSrcRect.mHeight) / (float)aTex->mHeight;

	glm::vec2 uv0 = {u0, v0};
	glm::vec2 uv1 = {u1, v0};
	glm::vec2 uv2 = {u1, v1};
	glm::vec2 uv3 = {u0, v1};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f, (float)theColor.mAlpha / 255.0f};

    std::vector<GLVertex> aVertexArray;

	aVertexArray.push_back({p0, uv0, aColor});
	aVertexArray.push_back({p1, uv1, aColor});
	aVertexArray.push_back({p2, uv2, aColor});
	aVertexArray.push_back({p2, uv2, aColor});
	aVertexArray.push_back({p3, uv3, aColor});
	aVertexArray.push_back({p0, uv0, aColor});

    GLShader* aShaderToUse;
   // if (cmd.mShader != nullptr)
    //    aShaderToUse = cmd.mShader;
  //  else
        aShaderToUse = mDefaultShader;

    GLuint aTextureID = aTex->mTextureID;
    aShaderToUse->Use();
    aShaderToUse->SetUniform("uProjection", mProjection);
    aShaderToUse->SetUniform("uUseTexture", (aTextureID != 0));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, aTextureID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, aVertexArray.size() * sizeof(GLVertex), aVertexArray.data());
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)aVertexArray.size());
}

GLTextureData::GLTextureData()
{
	mWidth = 0;
	mHeight = 0;
	mBitsChangedCount = 0;
	mTextureID = 0;
}

GLTextureData::~GLTextureData()
{
	ReleaseTextures();
}

void GLTextureData::ReleaseTextures()
{
	if (mTextureID != 0)
		glDeleteTextures(1, &mTextureID);
}

void GLTextureData::CreateTextures(GLImage *theImage)
{
	theImage->DeleteSWBuffers(); // we don't need the software buffers anymore
	theImage->CommitBits();

	bool createTexture = false;

	// only recreate the texture if the dimensions or image data have changed
	if (mWidth != theImage->mWidth || mHeight != theImage->mHeight || mBitsChangedCount != theImage->mBitsChangedCount)
	{
		ReleaseTextures();
		createTexture = true;
	}

	int aWidth = theImage->GetWidth();
	int aHeight = theImage->GetHeight();

	if (createTexture)
	{
		glGenTextures(1, &mTextureID);
		glBindTexture(GL_TEXTURE_2D, mTextureID);

		bool doNearestFilter = theImage->mD3DFlags & TextureFlags_NearestFiltering;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, doNearestFilter ? GL_NEAREST : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, doNearestFilter ? GL_NEAREST : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, aWidth, aHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, theImage->GetBits());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else if (mBitsChangedCount != theImage->mBitsChangedCount)
	{
		void *bits = theImage->GetBits();
		if (bits)
		{
			glGenTextures(1, &mTextureID);
			glBindTexture(GL_TEXTURE_2D, mTextureID);

			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, aWidth, aHeight, GL_BGRA, GL_UNSIGNED_BYTE, bits);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			//TODO: add some proper error catching...
            assert(false);
		}
	}

	mWidth = theImage->mWidth;
	mHeight = theImage->mHeight;
	mBitsChangedCount = theImage->mBitsChangedCount;
}

void GLTextureData::CheckCreateTextures(GLImage *theImage)
{
	if (mTextureID != 0)
	{
		if (mWidth != theImage->mWidth || mHeight != theImage->mHeight ||
			mBitsChangedCount != theImage->mBitsChangedCount)
			CreateTextures(theImage);
		return;
	}
	CreateTextures(theImage);
}

int GLTextureData::GetMemSize()
{
	int aSize = 0;

	aSize = 4 * mWidth * mHeight; // TODO: ADD MORE PIXEL FORMATS

	return aSize;
}