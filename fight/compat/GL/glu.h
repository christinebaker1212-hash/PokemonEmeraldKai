/* Moemon Fight: the one GLU call the engine makes (never reached: see compat/GL/gl.h). */
#ifndef MF_GLU_H
#define MF_GLU_H
#include <GL/gl.h>
void gluOrtho2D(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top);
#endif
