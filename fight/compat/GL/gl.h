/*
 * Moemon Fight: the OpenGL names the engine's OpenGL renderers use. The cabinet draws with the
 * engine's software renderer, so these are never called; compat/mf_gl.c has empty bodies.
 */
#ifndef MF_GL_H
#define MF_GL_H
#define __gl_h_
#define __GL_H__

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;
typedef char GLchar;

#define GL_BACK 0x0405
#define GL_BLEND 0x0BE2
#define GL_CLAMP 0x2900
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_DEPTH_COMPONENT 0x1902
#define GL_EXTENSIONS 0x1F03
#define GL_FRAMEBUFFER_EXT 0x8D40
#define GL_LIGHTING 0x0B50
#define GL_LOGIC_OP 0x0BF1
#define GL_LINEAR 0x2601
#define GL_LINES 0x0001
#define GL_LINE_LOOP 0x0002
#define GL_LUMINANCE 0x1909
#define GL_MAX_TEXTURE_SIZE 0x0D33
#define GL_MODELVIEW 0x1700
#define GL_NEAREST 0x2600
#define GL_NICEST 0x1102
#define GL_NO_ERROR 0
#define GL_ONE 1
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_PERSPECTIVE_CORRECTION_HINT 0x0C50
#define GL_POINTS 0x0000
#define GL_PROJECTION 0x1701
#define GL_RENDERER 0x1F01
#define GL_REPEAT 0x2901
#define GL_REPLACE 0x1E01
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_SCISSOR_TEST 0x0C11
#define GL_TEXTURE_1D 0x0DE0
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_ENV 0x2300
#define GL_TEXTURE_ENV_MODE 0x2200
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_TRIANGLE_STRIP 0x0005
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_UNSIGNED_BYTE 0x1401
#define GL_VENDOR 0x1F00
#define GL_VERSION 0x1F02
#define GL_XOR 0x1506

void glBegin (GLenum mode);
void glBindTexture (GLenum target, GLuint texture);
void glBlendFunc (GLenum sfactor, GLenum dfactor);
void glClear (GLbitfield mask);
void glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void glClearDepth (GLclampd depth);
void glColor3f (GLfloat red, GLfloat green, GLfloat blue);
void glDeleteTextures (GLsizei n, const GLuint *textures);
void glDisable (GLenum cap);
void glEnable (GLenum cap);
void glEnd (void);
void glFlush (void);
void glFrustum (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void glGenTextures (GLsizei n, GLuint *textures);
GLenum glGetError (void);
void glGetIntegerv (GLenum pname, GLint *params);
const GLubyte * glGetString (GLenum name);
void glHint (GLenum target, GLenum mode);
void glLoadIdentity (void);
void glLogicOp (GLenum opcode);
void glMatrixMode (GLenum mode);
void glMultMatrixf (const GLfloat *m);
void glOrtho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void glPixelStorei (GLenum pname, GLint param);
void glReadBuffer (GLenum mode);
void glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
void glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void glScaled (GLdouble x, GLdouble y, GLdouble z);
void glScalef (GLfloat x, GLfloat y, GLfloat z);
void glScissor (GLint x, GLint y, GLsizei width, GLsizei height);
void glTexCoord2f (GLfloat s, GLfloat t);
void glTexEnvf (GLenum target, GLenum pname, GLfloat param);
void glTexImage1D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void glTexParameteri (GLenum target, GLenum pname, GLint param);
void glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void glTranslated (GLdouble x, GLdouble y, GLdouble z);
void glTranslatef (GLfloat x, GLfloat y, GLfloat z);
void glVertex2f (GLfloat x, GLfloat y);
void glVertex3f (GLfloat x, GLfloat y, GLfloat z);
void glViewport (GLint x, GLint y, GLsizei width, GLsizei height);

#endif /* MF_GL_H */
