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
 #if !defined(AFX_NED_GRWND_H__FBB555C2_E754_11D2_A6A1_006097E07445__INCLUDED_)
#define AFX_NED_GRWND_H__FBB555C2_E754_11D2_A6A1_006097E07445__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ned_GrWnd.h : header file
//

#include "RendHandle.h"
#include "grdefs.h"

const DWORD GRWND_STYLE = WS_VISIBLE | WS_CHILD | WS_CAPTION | WS_CLIPSIBLINGS |
							WS_THICKFRAME | WS_SYSMENU;
const DWORD GRWND_STATIC_STYLE = WS_VISIBLE | WS_CHILD | WS_BORDER;

/////////////////////////////////////////////////////////////////////////////
// Cned_GrWnd window

class Cned_GrWnd : public CWnd
{
	DECLARE_DYNCREATE(Cned_GrWnd)
protected:
	Cned_GrWnd();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:
	void SetDCPixelFormat(HDC hDC);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Cned_GrWnd)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	ddgr_color m_BackColor;
	bool m_bInitted;					// ready for rendering

protected:
	virtual ~Cned_GrWnd();

	// Generated message map functions
	//{{AFX_MSG(Cned_GrWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Variables
protected:
	HDC renderer_hDC;
	HGLRC hRC;
	RendHandle m_handle;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NED_GRWND_H__FBB555C2_E754_11D2_A6A1_006097E07445__INCLUDED_)
