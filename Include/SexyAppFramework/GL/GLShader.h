#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

namespace Sexy
{

    class GLShader
    {
    public:
        GLuint mProgramID;

    public:
        GLShader();
        ~GLShader();

        GLuint CompileShader(GLenum theType, const std::string& theSource);
        GLuint GetUniformLocation(const std::string& theName);

        bool LoadFromSource(const std::string& theVertexShader, const std::string& theFragmentShader);

        void Use();

        void SetUniform(const std::string &theName, int theValue);
        void SetUniform(const std::string &theName, float theValue);
        void SetUniform(const std::string &theName, const glm::vec2 &theValue);
        void SetUniform(const std::string &theName, const glm::vec4 &theValue);
        void SetUniform(const std::string &theName, const glm::mat4 &theValue);
    };

} // namespace Sexy
