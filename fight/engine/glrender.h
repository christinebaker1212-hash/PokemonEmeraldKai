#ifndef MUGEN_GLRENDER_H
#define MUGEN_GLRENDER_H

/*
 * Types of the OpenGL renderer (glrender.c): GLSL programs and the GLEW
 * function pointers it uses from mugen.exe's own GLEW 1.5.4 copy (filled by
 * glewInit).
 */

/* A GLSL program with its uniform locations (Gl_LoadShader). */
typedef struct GlProgram {
    unsigned int vert;          /* 0x00 vertex shader */
    unsigned int frag;          /* 0x04 fragment shader */
    unsigned int program;       /* 0x08 */
    int texunit0;               /* 0x0C uniform locations */
    int texunit1;               /* 0x10 */
    int mask;                   /* 0x14 */
    int add;                    /* 0x18 */
    int mul;                    /* 0x1C */
    int invertall;              /* 0x20 */
    int colorbal;               /* 0x24 */
    int texsize;                /* 0x28 */
    int src_alphamul;           /* 0x2C */
} GlProgram;
MUGEN_ASSERT_SIZE(GlProgram, 0x30);

/*
 * Programs by variant: palfx (0 none, 1 add/mul, 2 colour balance or
 * inversion) + 3 * (0 plain, 2 stretched / tinted quad).
 */
#define GL_PROGRAM_COUNT 9

/* GLEW function pointer types (__stdcall, as the GL entry points). */
typedef void (__stdcall *GlEnumFn)(unsigned int e);                   /* glActiveTexture, glBlendEquation... */
typedef void (__stdcall *GlUintFn)(unsigned int id);                  /* glUseProgram, glCompileShader... */
typedef void (__stdcall *GlMultiTexCoord2fFn)(unsigned int target, float s, float t);
typedef void (__stdcall *GlMultiTexCoord4fFn)(unsigned int target, float s, float t, float r, float q);
typedef void (__stdcall *GlBlendColorFn)(float r, float g, float b, float a);
typedef void (__stdcall *GlAttachShaderFn)(unsigned int program, unsigned int shader);
typedef unsigned int (__stdcall *GlCreateProgramFn)(void);
typedef unsigned int (__stdcall *GlCreateShaderFn)(unsigned int type);
typedef void (__stdcall *GlGetInfoLogFn)(unsigned int obj, int size, int *length, char *log);
typedef void (__stdcall *GlGetObjectivFn)(unsigned int obj, unsigned int pname, int *params);
typedef int (__stdcall *GlGetUniformLocationFn)(unsigned int program, const char *name);
typedef unsigned char (__stdcall *GlIsShaderFn)(unsigned int shader);
typedef void (__stdcall *GlShaderSourceFn)(unsigned int shader, int count, const char **string, const int *length);
typedef void (__stdcall *GlUniform1fFn)(int location, float v0);
typedef void (__stdcall *GlUniform1iFn)(int location, int v0);
typedef void (__stdcall *GlUniform2fFn)(int location, float v0, float v1);
typedef void (__stdcall *GlUniform3fFn)(int location, float v0, float v1, float v2);
typedef void (__stdcall *GlGenDeleteFn)(int n, unsigned int *ids);    /* glGen/DeleteFramebuffers/RenderbuffersEXT */
typedef unsigned int (__stdcall *GlCheckFramebufferStatusFn)(unsigned int target);
typedef void (__stdcall *GlFramebufferRenderbufferFn)(unsigned int target, unsigned int attachment,
                                                       unsigned int renderbuffer_target, unsigned int renderbuffer);
typedef void (__stdcall *GlFramebufferTexture2DFn)(unsigned int target, unsigned int attachment,
                                                    unsigned int texture_target, unsigned int texture, int level);
typedef void (__stdcall *GlRenderbufferStorageFn)(unsigned int target, unsigned int internal_format, int w, int h);

#endif /* MUGEN_GLRENDER_H */
