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
 #if !defined(AFX_EXTRUDEDIALOG_H__A0D283E1_FE2D_11D2_A6A1_006097E07445__INCLUDED_)
#define AFX_EXTRUDEDIALOG_H__A0D283E1_FE2D_11D2_A6A1_006097E07445__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExtrudeDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExtrudeDialog dialog

class CExtrudeDialog : public CDialog
{
// Construction
public:
	CExtrudeDialog(CWnd* pParent = NULL);   // standard constructor

	BOOL m_Inward;

// Dialog Data
	//{{AFX_DATA(CExtrudeDialog)
	enum { IDD = IDD_EXTRUDE_FACE };
	int		m_Direction;
	BOOL	m_DeleteBaseFace;
	float	m_Distance;
	BOOL	m_Default;
	int		m_Faces;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExtrudeDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExtrudeDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnXAxis();
	afx_msg void OnYAxis();
	afx_msg void OnZAxis();
	afx_msg void OnNormal();
	afx_msg void OnInward();
	afx_msg void OnOutward();
	afx_msg void OnCurrentFace();
	afx_msg void OnMarkedFaces();
	virtual void OnOK();
	afx_msg void OnDeleteFace();
	afx_msg void OnDefaultSettings();
	afx_msg void OnKillfocusDistance();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

void HandleExtrude(int which,float dist,BOOL delete_base_face,int inward,int faces);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXTRUDEDIALOG_H__A0D283E1_FE2D_11D2_A6A1_006097E07445__INCLUDED_)
