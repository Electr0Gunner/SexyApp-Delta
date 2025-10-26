#include <SexyAppFramework/GL/GLShader.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Sexy;

GLShader::GLShader()
{
}

GLShader::~GLShader()
{
    if (mProgramID)
        glDeleteProgram(mProgramID);
}
    
bool GLShader::LoadFromSource(const std::string &theVertexSource, const std::string &theFragmentSource)
{
	GLuint aVertexShader = CompileShader(GL_VERTEX_SHADER, theVertexSource);
	GLuint aFragmentShader = CompileShader(GL_FRAGMENT_SHADER, theFragmentSource);

	if (!aVertexShader || !aFragmentShader)
		return false;

	mProgramID = glCreateProgram();
	glAttachShader(mProgramID, aVertexShader);
	glAttachShader(mProgramID, aFragmentShader);
	glLinkProgram(mProgramID);

	GLint success;
	glGetProgramiv(mProgramID, GL_LINK_STATUS, &success);
	// TODO: ADD SUCCESS CHECK

	glDeleteShader(aVertexShader);
	glDeleteShader(aFragmentShader);
	return true;
}

void GLShader::Use()
{
    glUseProgram(mProgramID);
}

GLuint GLShader::GetUniformLocation(const std::string &name)
{
	GLuint pos = glGetUniformLocation(mProgramID, name.c_str());
	return pos;
}

void GLShader::SetUniform(const std::string &theName, int value)
{
	glUniform1i(GetUniformLocation(theName), value);
}
void GLShader::SetUniform(const std::string &theName, float value)
{
	glUniform1f(GetUniformLocation(theName), value);
}
void GLShader::SetUniform(const std::string &theName, const glm::vec2 &value)
{
	glUniform2f(GetUniformLocation(theName), value.x, value.y);
}
void GLShader::SetUniform(const std::string &theName, const glm::vec4 &theValue)
{
	glUniform4f(GetUniformLocation(theName), theValue.x, theValue.y, theValue.z, theValue.w);
}
void GLShader::SetUniform(const std::string &theName, const glm::mat4 &theValue)
{
	glUniformMatrix4fv(GetUniformLocation(theName), 1, GL_FALSE, glm::value_ptr(theValue));
}