/*
 * Moemon Fight: GLEW's function pointers and extension flags, which the engine's OpenGL renderers
 * read. They stay empty: the cabinet draws with the software renderer (compat/GL/gl.h).
 */
#include "mugen.h"

GlEnumFn __glewActiveTexture;
GlMultiTexCoord2fFn __glewMultiTexCoord2f;
GlMultiTexCoord4fFn __glewMultiTexCoord4f;
GlBlendColorFn __glewBlendColor;
GlEnumFn __glewBlendEquation;
GlAttachShaderFn __glewAttachShader;
GlUintFn __glewCompileShader;
GlCreateProgramFn __glewCreateProgram;
GlCreateShaderFn __glewCreateShader;
GlUintFn __glewDeleteProgram;
GlUintFn __glewDeleteShader;
GlGetInfoLogFn __glewGetProgramInfoLog;
GlGetObjectivFn __glewGetProgramiv;
GlGetInfoLogFn __glewGetShaderInfoLog;
GlGetObjectivFn __glewGetShaderiv;
GlGetUniformLocationFn __glewGetUniformLocation;
GlIsShaderFn __glewIsShader;
GlUintFn __glewLinkProgram;
GlShaderSourceFn __glewShaderSource;
GlUniform1fFn __glewUniform1f;
GlUniform1iFn __glewUniform1i;
GlUniform2fFn __glewUniform2f;
GlUniform3fFn __glewUniform3f;
GlUintFn __glewUseProgram;
GlBindFramebufferFn __glewBindFramebufferEXT;
GlBindFramebufferFn __glewBindRenderbufferEXT;
GlCheckFramebufferStatusFn __glewCheckFramebufferStatusEXT;
GlGenDeleteFn __glewDeleteRenderbuffersEXT;
GlFramebufferRenderbufferFn __glewFramebufferRenderbufferEXT;
GlFramebufferTexture2DFn __glewFramebufferTexture2DEXT;
GlGenDeleteFn __glewGenFramebuffersEXT;
GlGenDeleteFn __glewGenRenderbuffersEXT;
GlRenderbufferStorageFn __glewRenderbufferStorageEXT;
unsigned char __GLEW_VERSION_2_0;
unsigned char __GLEW_ARB_fragment_program;
unsigned char __GLEW_ARB_texture_non_power_of_two;
unsigned char __GLEW_EXT_blend_logic_op;
unsigned char __GLEW_EXT_framebuffer_object;

/* glewInit fails: no OpenGL here */
unsigned int glewInit(void)
{
    return 1;
}
