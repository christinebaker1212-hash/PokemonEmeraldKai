/* Moemon Fight: empty OpenGL entry points (see compat/GL/gl.h). */
#include <GL/gl.h>
#include <GL/glu.h>

void glBegin(GLenum mode)
{
}

void glBindTexture(GLenum target, GLuint texture)
{
}

void glBlendFunc(GLenum sfactor, GLenum dfactor)
{
}

void glClear(GLbitfield mask)
{
}

void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
}

void glClearDepth(GLclampd depth)
{
}

void glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
}

void glDeleteTextures(GLsizei n, const GLuint *textures)
{
}

void glDisable(GLenum cap)
{
}

void glEnable(GLenum cap)
{
}

void glEnd(void)
{
}

void glFlush(void)
{
}

void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
}

void glGenTextures(GLsizei n, GLuint *textures)
{
}

GLenum glGetError(void)
{
    return 0;
}

void glGetIntegerv(GLenum pname, GLint *params)
{
}

const GLubyte * glGetString(GLenum name)
{
    return 0;
}

void glHint(GLenum target, GLenum mode)
{
}

void glLoadIdentity(void)
{
}

void glLogicOp(GLenum opcode)
{
}

void glMatrixMode(GLenum mode)
{
}

void glMultMatrixf(const GLfloat *m)
{
}

void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
}

void glPixelStorei(GLenum pname, GLint param)
{
}

void glReadBuffer(GLenum mode)
{
}

void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
}

void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
}

void glScaled(GLdouble x, GLdouble y, GLdouble z)
{
}

void glScalef(GLfloat x, GLfloat y, GLfloat z)
{
}

void glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
}

void glTexCoord2f(GLfloat s, GLfloat t)
{
}

void glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
}

void glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
}

void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
}

void glTexParameteri(GLenum target, GLenum pname, GLint param)
{
}

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
}

void glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
}

void glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
}

void glVertex2f(GLfloat x, GLfloat y)
{
}

void glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
}

void gluOrtho2D(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top)
{
}
