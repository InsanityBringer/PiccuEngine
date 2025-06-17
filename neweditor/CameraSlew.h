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
 #if !defined(AFX_CAMERASLEW_H__417272AA_EB56_11D2_8F49_00104B27BFF0__INCLUDED_)
#define AFX_CAMERASLEW_H__417272AA_EB56_11D2_8F49_00104B27BFF0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CameraSlew.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCameraSlew dialog

#include "resource.h"       // main symbols
#include "globals.h"
#include "ned_OrthoWnd.h"

class CCameraSlew : public CDialogBar
{
// Construction
public:
	float m_MovementScaler;
	CString m_Title;
	void InitBar();
	void AttachCamera(camera *cam,char *title);
	void AttachOrthoCamera(o_camera *cam,char *title);
	camera * m_AttachedCamera;
	o_camera * m_AttachedOrthoCamera;
	bool m_bOrtho;	// is the camera for the currently focused view orthogonal?
	unsigned int m_LastSlideUp;
	unsigned int m_LastSlideLeft;
	unsigned int m_LastSlideRight;
	unsigned int m_LastSlideDown;
	unsigned int m_LastSlideForward;
	unsigned int m_LastSlideReverse;

	unsigned int m_LastRotateUp;
	unsigned int m_LastRotateDown;
	unsigned int m_LastRotateLeft;
	unsigned int m_LastRotateRight;
	unsigned int m_LastZoomIn;
	unsigned int m_LastZoomOut;


	CCameraSlew();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCameraSlew)
	enum { IDD = IDD_CAMERASLEW };
	CSliderCtrl	m_SensSlider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCameraSlew)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCameraSlew)
	afx_msg void OnOrigindown();
	afx_msg void OnOriginforward();
	afx_msg void OnOriginleft();
	afx_msg void OnOriginreverse();
	afx_msg void OnOriginright();
	afx_msg void OnOriginup();
	afx_msg void OnRotateleft();
	afx_msg void OnRotateright();
	afx_msg void OnRotateup();
	afx_msg void OnRotatedown();
	afx_msg void OnReleasedcaptureSensslider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnZoomin();
	afx_msg void OnZoomout();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnOutofmemorySensslider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnEngineerUpdStyle(CCmdUI*);
	afx_msg LONG OnInitDialog ( UINT, LONG );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAMERASLEW_H__417272AA_EB56_11D2_8F49_00104B27BFF0__INCLUDED_)
