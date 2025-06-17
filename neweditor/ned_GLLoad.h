/*
 THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF OUTRAGE
 ENTERTAINMENT, INC. ("OUTRAGE").  OUTRAGE, IN DISTRIBUTING THE CODE TO
 END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
 ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
 IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
 SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
 FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
 CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
 AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
 COPYRIGHT 1996-2000 OUTRAGE ENTERTAINMENT, INC.  ALL RIGHTS RESERVED.
 */
 #include "gl.h"
#include "module.h"

#ifdef MAINCODE
#define EXTERN(x,y) x y=NULL
#else
#define EXTERN(x,y) extern x y
#endif

//#define STATIC_OPENGL

#ifdef STATIC_OPENGL
#define dwglCreateContext wglCreateContext
#define dwglDeleteContext wglDeleteContext
#define dwglMakeCurrent wglMakeCurrent
#define dwglGetProcAddress wglGetProcAddress
#define dglAlphaFunc glAlphaFunc
#define dglBegin glBegin
#define dglBindTexture glBindTexture
#define dglBlendFunc glBlendFunc
#define dglClear glClear
#define dglClearColor glClearColor
#define dglClearDepth glClearDepth
#define dglColor3ub glColor3ub
#define dglColor4ub glColor4ub
#define dglColor4f glColor4f
#define dglColorPointer glColorPointer
#define dglDeleteTextures glDeleteTextures
#define dglDepthFunc glDepthFunc
#define dglDepthMask glDepthMask
#define dglDepthRange glDepthRange
#define dglDisable glDisable
#define dglDisableClientState glDisableClientState
#define dglDrawArrays glDrawArrays
#define dglDrawPixels glDrawPixels
#define dglEnable glEnable
#define dglEnableClientState glEnableClientState
#define dglEnd glEnd
#define dglFogf glFogf
#define dglFogfv glFogfv
#define dglFogi glFogi
#define dglGetString glGetString
#define dglHint glHint
#define dglLoadIdentity glLoadIdentity
#define dglMatrixMode glMatrixMode
#define dglOrtho glOrtho
#define dglPixelStorei glPixelStorei
#define dglPixelTransferi glPixelTransferi
#define dglPolygonOffset glPolygonOffset
#define dglReadPixels glReadPixels
#define dglScissor glScissor
#define dglShadeModel glShadeModel
#define dglTexCoordPointer glTexCoordPointer
#define dglTexEnvf glTexEnvf
#define dglTexImage2D glTexImage2D
#define dglTexParameteri glTexParameteri
#define dglTexSubImage2D glTexSubImage2D
#define dglVertex2i glVertex2i
#define dglVertex3f glVertex3f
#define dglVertexPointer glVertexPointer
#define dglViewport glViewport
#else
typedef void(__stdcall *glAlphaFunc_fp )(GLenum func, GLclampf ref);
typedef void(__stdcall *glBegin_fp )(GLenum mode);
typedef void(__stdcall *glBindTexture_fp )( GLenum target, GLuint texture );
typedef void(__stdcall *glBlendFunc_fp )(GLenum sfactor, GLenum dfactor);
typedef void(__stdcall *glClear_fp )(GLbitfield mask);
typedef void(__stdcall *glClearColor_fp )(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void(__stdcall *glClearDepth_fp )(GLclampd depth);
typedef void(__stdcall *glColor3ub_fp )(GLubyte red, GLubyte green, GLubyte blue);
typedef void(__stdcall *glColor4ub_fp )(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
typedef void(__stdcall *glColor4f_fp )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void(__stdcall *glColorPointer_fp )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void(__stdcall *glDeleteTextures_fp) (GLsizei n, const GLuint *textures);
typedef void(__stdcall *glDepthFunc_fp )(GLenum func);
typedef void(__stdcall *glDepthMask_fp )(GLboolean flag);
typedef void(__stdcall *glDepthRange_fp )(GLclampd zNear, GLclampd zFar);
typedef void(__stdcall *glDisable_fp )(GLenum cap);
typedef void(__stdcall *glDisableClientState_fp )(GLenum array);
typedef void(__stdcall *glDrawArrays_fp )(GLenum mode, GLint first, GLsizei count);
typedef void(__stdcall *glDrawPixels_fp )(GLsizei width,GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void(__stdcall *glEnable_fp )(GLenum cap);
typedef void(__stdcall *glEnableClientState_fp )(GLenum array);
typedef void(__stdcall *glEnd_fp )(void);
typedef void(__stdcall *glFogf_fp )(GLenum pname, GLfloat param);
typedef void(__stdcall *glFogfv_fp )(GLenum pname, const GLfloat *params);
typedef void(__stdcall *glFogi_fp )(GLenum pname, GLint param);
typedef const GLubyte *(__stdcall *glGetString_fp )(GLenum name);
typedef void(__stdcall *glHint_fp )(GLenum target, GLenum mode);
typedef void(__stdcall *glLoadIdentity_fp )(void);
typedef void(__stdcall *glMatrixMode_fp )(GLenum mode);
typedef void(__stdcall *glOrtho_fp )(GLdouble,GLdouble,GLdouble,GLdouble,GLdouble,GLdouble);
typedef void(__stdcall *glPixelStorei_fp )(GLenum pname, GLint param);
typedef void(__stdcall *glPixelTransferi_fp )(GLenum pname, GLint param);
typedef void(__stdcall *glPolygonOffset_fp )(GLfloat factor, GLfloat units);
typedef void(__stdcall *glReadPixels_fp )(GLint, GLint,GLsizei, GLsizei, GLenum,GLenum,GLvoid *);
typedef void(__stdcall *glScissor_fp )(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void(__stdcall *glShadeModel_fp )(GLenum mode);
typedef void(__stdcall *glTexCoordPointer_fp )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void(__stdcall *glTexEnvf_fp )(GLenum target, GLenum pname, GLfloat param);
typedef void(__stdcall *glTexImage2D_fp )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void(__stdcall *glTexParameteri_fp )(GLenum target, GLenum pname, GLint param);
typedef void(__stdcall *glTexSubImage2D_fp )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void(__stdcall *glVertex2i_fp )(GLint,GLint);
typedef void(__stdcall *glVertex3f_fp )(GLfloat,GLfloat,GLfloat);
typedef void(__stdcall *glVertexPointer_fp )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void(__stdcall *glViewport_fp )(GLint x, GLint y, GLsizei width, GLsizei height);
typedef HGLRC(__stdcall *wglCreateContext_fp )(HDC);
typedef BOOL(__stdcall *wglDeleteContext_fp )(HGLRC);
typedef BOOL(__stdcall *wglMakeCurrent_fp )(HDC,HGLRC);
typedef PROC(__stdcall *wglGetProcAddress_fp )(LPCSTR);

EXTERN(wglCreateContext_fp,dwglCreateContext);
EXTERN(wglDeleteContext_fp,dwglDeleteContext);
EXTERN(wglMakeCurrent_fp,dwglMakeCurrent);
EXTERN(wglGetProcAddress_fp,dwglGetProcAddress);

EXTERN(glAlphaFunc_fp,dglAlphaFunc);
EXTERN(glBegin_fp,dglBegin);
EXTERN(glBindTexture_fp,dglBindTexture);
EXTERN(glBlendFunc_fp,dglBlendFunc);
EXTERN(glClear_fp,dglClear);
EXTERN(glClearColor_fp,dglClearColor);
EXTERN(glClearDepth_fp,dglClearDepth);
EXTERN(glColor3ub_fp,dglColor3ub);
EXTERN(glColor4ub_fp,dglColor4ub);
EXTERN(glColor4f_fp,dglColor4f);
EXTERN(glColorPointer_fp,dglColorPointer);
EXTERN(glDeleteTextures_fp,dglDeleteTextures);
EXTERN(glDepthFunc_fp,dglDepthFunc);
EXTERN(glDepthMask_fp,dglDepthMask);
EXTERN(glDepthRange_fp,dglDepthRange);
EXTERN(glDisable_fp,dglDisable);
EXTERN(glDisableClientState_fp,dglDisableClientState);
EXTERN(glDrawArrays_fp,dglDrawArrays);
EXTERN(glDrawPixels_fp,dglDrawPixels);
EXTERN(glEnable_fp,dglEnable);
EXTERN(glEnableClientState_fp,dglEnableClientState);
EXTERN(glEnd_fp,dglEnd);
EXTERN(glFogf_fp,dglFogf);
EXTERN(glFogfv_fp,dglFogfv);
EXTERN(glFogi_fp,dglFogi);
EXTERN(glGetString_fp,dglGetString);
EXTERN(glHint_fp,dglHint);
EXTERN(glLoadIdentity_fp,dglLoadIdentity);
EXTERN(glMatrixMode_fp,dglMatrixMode);
EXTERN(glOrtho_fp,dglOrtho);
EXTERN(glPixelStorei_fp,dglPixelStorei);
EXTERN(glPixelTransferi_fp,dglPixelTransferi);
EXTERN(glPolygonOffset_fp,dglPolygonOffset);
EXTERN(glReadPixels_fp,dglReadPixels);
EXTERN(glScissor_fp,dglScissor);
EXTERN(glShadeModel_fp,dglShadeModel);
EXTERN(glTexCoordPointer_fp,dglTexCoordPointer);
EXTERN(glTexEnvf_fp,dglTexEnvf);
EXTERN(glTexImage2D_fp,dglTexImage2D);
EXTERN(glTexParameteri_fp,dglTexParameteri);
EXTERN(glTexSubImage2D_fp,dglTexSubImage2D);
EXTERN(glVertex2i_fp,dglVertex2i);
EXTERN(glVertex3f_fp,dglVertex3f);
EXTERN(glVertexPointer_fp,dglVertexPointer);
EXTERN(glViewport_fp,dglViewport);
#endif

#ifdef MAINCODE
static module OpenGLDLLInst;
#endif

#ifdef MAINCODE
module *LoadOpenGLDLL(char *dllname)
{
#ifndef STATIC_OPENGL
	mprintf ((0,"Loading OpenGL dll...\n"));

	if(!mod_LoadModule(&OpenGLDLLInst,dllname))
	{
		int err = mod_GetLastError();
		mprintf ((0,"Couldn't open module called %s\n",dllname));
		return NULL;
	}

	dglAlphaFunc = (glAlphaFunc_fp)mod_GetSymbol(&OpenGLDLLInst,"glAlphaFunc",255);
	if(! dglAlphaFunc) goto dll_error;

	dglBegin = (glBegin_fp)mod_GetSymbol(&OpenGLDLLInst,"glBegin",255);
	if(! dglBegin) goto dll_error;

	dglBindTexture = (glBindTexture_fp)mod_GetSymbol(&OpenGLDLLInst,"glBindTexture",255);
	if(! dglBindTexture) goto dll_error;
	
	dglBlendFunc = (glBlendFunc_fp)mod_GetSymbol(&OpenGLDLLInst,"glBlendFunc",255);
	if(! dglBlendFunc) goto dll_error;

	dglClear = (glClear_fp)mod_GetSymbol(&OpenGLDLLInst,"glClear",255);
	if(! dglClear) goto dll_error;

	dglClearColor = (glClearColor_fp)mod_GetSymbol(&OpenGLDLLInst,"glClearColor",255);
	if(! dglClearColor) goto dll_error;

	dglClearDepth = (glClearDepth_fp)mod_GetSymbol(&OpenGLDLLInst,"glClearDepth",255);
	if(! dglClearDepth) goto dll_error;

	dglColor3ub = (glColor3ub_fp)mod_GetSymbol(&OpenGLDLLInst,"glColor3ub",255);
	if(! dglColor3ub) goto dll_error;

	dglColor4ub = (glColor4ub_fp)mod_GetSymbol(&OpenGLDLLInst,"glColor4ub",255);
	if(! dglColor4ub) goto dll_error;

	dglColor4f = (glColor4f_fp)mod_GetSymbol(&OpenGLDLLInst,"glColor4f",255);
	if(! dglColor4f) goto dll_error;

	dglColorPointer = (glColorPointer_fp)mod_GetSymbol(&OpenGLDLLInst,"glColorPointer",255);
	if(! dglColorPointer) goto dll_error;

	dglDeleteTextures = (glDeleteTextures_fp)mod_GetSymbol(&OpenGLDLLInst,"glDeleteTextures",255);
	if(! dglDeleteTextures) goto dll_error;

	dglDepthFunc = (glDepthFunc_fp)mod_GetSymbol(&OpenGLDLLInst,"glDepthFunc",255);
	if(! dglDepthFunc) goto dll_error;

	dglDepthMask = (glDepthMask_fp)mod_GetSymbol(&OpenGLDLLInst,"glDepthMask",255);
	if(! dglDepthMask) goto dll_error;

	dglDepthRange = (glDepthRange_fp)mod_GetSymbol(&OpenGLDLLInst,"glDepthRange",255);
	if(! dglDepthRange) goto dll_error;

	dglDisable = (glDisable_fp)mod_GetSymbol(&OpenGLDLLInst,"glDisable",255);
	if(! dglDisable) goto dll_error;

	dglDisableClientState = (glDisableClientState_fp)mod_GetSymbol(&OpenGLDLLInst,"glDisableClientState",255);
	if(! dglDisableClientState) goto dll_error;

	dglDrawArrays = (glDrawArrays_fp)mod_GetSymbol(&OpenGLDLLInst,"glDrawArrays",255);
	if(! dglDrawArrays) goto dll_error;

	dglDrawPixels = (glDrawPixels_fp)mod_GetSymbol(&OpenGLDLLInst,"glDrawPixels",255);
	if(! dglDrawPixels) goto dll_error;

	dglEnable = (glEnable_fp)mod_GetSymbol(&OpenGLDLLInst,"glEnable",255);
	if(! dglEnable) goto dll_error;

	dglEnableClientState = (glEnableClientState_fp)mod_GetSymbol(&OpenGLDLLInst,"glEnableClientState",255);
	if(! dglEnableClientState) goto dll_error;

	dglEnd = (glEnd_fp)mod_GetSymbol(&OpenGLDLLInst,"glEnd",255);
	if(! dglEnd) goto dll_error;

	dglFogf = (glFogf_fp)mod_GetSymbol(&OpenGLDLLInst,"glFogf",255);
	if(! dglFogf) goto dll_error;

	dglFogfv = (glFogfv_fp)mod_GetSymbol(&OpenGLDLLInst,"glFogfv",255);
	if(! dglFogfv) goto dll_error;

	dglFogi = (glFogi_fp)mod_GetSymbol(&OpenGLDLLInst,"glFogi",255);
	if(! dglFogi) goto dll_error;

	dglGetString = (glGetString_fp)mod_GetSymbol(&OpenGLDLLInst,"glGetString",255);
	if(! dglGetString) goto dll_error;

	dglHint = (glHint_fp)mod_GetSymbol(&OpenGLDLLInst,"glHint",255);
	if(! dglHint) goto dll_error;

	dglLoadIdentity = (glLoadIdentity_fp)mod_GetSymbol(&OpenGLDLLInst,"glLoadIdentity",255);
	if(! dglLoadIdentity) goto dll_error;

	dglMatrixMode = (glMatrixMode_fp)mod_GetSymbol(&OpenGLDLLInst,"glMatrixMode",255);
	if(! dglMatrixMode) goto dll_error;
	
	dglOrtho = (glOrtho_fp)mod_GetSymbol(&OpenGLDLLInst,"glOrtho",255);
	if(! dglOrtho) goto dll_error;
	
	dglPixelStorei = (glPixelStorei_fp)mod_GetSymbol(&OpenGLDLLInst,"glPixelStorei",255);
	if(! dglPixelStorei) goto dll_error;

	dglPixelTransferi = (glPixelTransferi_fp)mod_GetSymbol(&OpenGLDLLInst,"glPixelTransferi",255);
	if(! dglPixelTransferi) goto dll_error;
	
	dglPolygonOffset = (glPolygonOffset_fp)mod_GetSymbol(&OpenGLDLLInst,"glPolygonOffset",255);
	if(! dglPolygonOffset) goto dll_error;

	dglReadPixels = (glReadPixels_fp)mod_GetSymbol(&OpenGLDLLInst,"glReadPixels",255);
	if(! dglReadPixels) goto dll_error;
	
	dglScissor = (glScissor_fp)mod_GetSymbol(&OpenGLDLLInst,"glScissor",255);
	if(! dglScissor) goto dll_error;
	
	dglShadeModel = (glShadeModel_fp)mod_GetSymbol(&OpenGLDLLInst,"glShadeModel",255);
	if(! dglShadeModel) goto dll_error;
	
	dglTexCoordPointer = (glTexCoordPointer_fp)mod_GetSymbol(&OpenGLDLLInst,"glTexCoordPointer",255);
	if(! dglTexCoordPointer) goto dll_error;

	dglTexEnvf = (glTexEnvf_fp)mod_GetSymbol(&OpenGLDLLInst,"glTexEnvf",255);
	if(! dglTexEnvf) goto dll_error;
	
	dglTexImage2D = (glTexImage2D_fp)mod_GetSymbol(&OpenGLDLLInst,"glTexImage2D",255);
	if(! dglTexImage2D) goto dll_error;
	
	dglTexParameteri= (glTexParameteri_fp)mod_GetSymbol(&OpenGLDLLInst,"glTexParameteri",255);
	if(! dglTexParameteri) goto dll_error;
	
	dglTexSubImage2D = (glTexSubImage2D_fp)mod_GetSymbol(&OpenGLDLLInst,"glTexSubImage2D",255);
	if(! dglTexSubImage2D) goto dll_error;
	
	dglVertex2i = (glVertex2i_fp)mod_GetSymbol(&OpenGLDLLInst,"glVertex2i",255);
	if(! dglVertex2i) goto dll_error;
	
	dglVertex3f = (glVertex3f_fp)mod_GetSymbol(&OpenGLDLLInst,"glVertex3f",255);
	if(! dglVertex3f) goto dll_error;
	
	dglVertexPointer = (glVertexPointer_fp)mod_GetSymbol(&OpenGLDLLInst,"glVertexPointer",255);
	if(! dglVertexPointer) goto dll_error;
	
	dglViewport = (glViewport_fp)mod_GetSymbol(&OpenGLDLLInst,"glViewport",255);
	if(! dglViewport) goto dll_error;

	dwglCreateContext = (wglCreateContext_fp)mod_GetSymbol(&OpenGLDLLInst,"wglCreateContext",255);
	if(! dwglCreateContext) goto dll_error;

	dwglDeleteContext = (wglDeleteContext_fp)mod_GetSymbol(&OpenGLDLLInst,"wglDeleteContext",255);
	if(! dwglDeleteContext) goto dll_error;

	dwglMakeCurrent = (wglMakeCurrent_fp)mod_GetSymbol(&OpenGLDLLInst,"wglMakeCurrent",255);
	if(! dwglMakeCurrent) goto dll_error;

	dwglGetProcAddress= (wglGetProcAddress_fp)mod_GetSymbol(&OpenGLDLLInst,"wglGetProcAddress",255);
	if(! dwglGetProcAddress) goto dll_error;

	mprintf ((0,"OpenGL dll loading successful.\n"));

	return &OpenGLDLLInst;

	dll_error:
	mprintf ((0,"Error loading opengl dll!\n"));
	mod_FreeModule(&OpenGLDLLInst);
#endif
	return NULL;
}

#else
module *LoadOpenGLDLL(char *dllname);
#endif