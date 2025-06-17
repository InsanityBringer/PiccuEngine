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
 #if !defined(AFX_TEXALIGNDIALOG_H__F7AFF382_0055_11D3_A6A1_006097E07445__INCLUDED_)
#define AFX_TEXALIGNDIALOG_H__F7AFF382_0055_11D3_A6A1_006097E07445__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TexAlignDialog.h : header file
//

#define TIMER_TEXALIGN		4289

/////////////////////////////////////////////////////////////////////////////
// CTexAlignDialog dialog

class CTexAlignDialog : public CDialog
{
// Construction
public:
	CTexAlignDialog(CWnd* pParent = NULL);   // standard constructor
	CTexAlignDialog(prim *pprim, CWnd* pParent = NULL);
	void UpdateDialog();

// Dialog Data
	//{{AFX_DATA(CTexAlignDialog)
	enum { IDD = IDD_TEXTURE_ALIGN };
	float	m_U;
	float	m_V;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTexAlignDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
	prim *m_pPrim;					// the primitives
//	bool m_bTimerSet;				// is a timer set?
	CRoomFrm *m_pRoomFrm;
	int m_Faces;

protected:
//	void SetTexTimer();				// set a timer to enable continuous texture movement 
									// when certain dialog buttons are highlighted
	void EnableControls(bool enable);

	// Generated message map functions
	//{{AFX_MSG(CTexAlignDialog)
	afx_msg void OnUVApply();
	afx_msg void OnUVDefault();
	virtual void OnCancel();
	afx_msg void OnPaint();
	virtual void OnOK();
	afx_msg void OnUVFlipX();
	afx_msg void OnUVFlipY();
	afx_msg void OnUVRotate90();
	afx_msg void OnKillfocusUVEditStep();
	afx_msg void OnUVAlignMarked();
	afx_msg void OnUVFaceMap();
	afx_msg void OnUVAlignEdge();
	afx_msg void OnUVDown();
	afx_msg void OnUVExpandU();
	afx_msg void OnUVExpandV();
	afx_msg void OnUVLeft();
	afx_msg void OnUVRight();
	afx_msg void OnUVRotLeft();
	afx_msg void OnUVRotRight();
	afx_msg void OnUVShrinkU();
	afx_msg void OnUVShrinkV();
	afx_msg void OnUVStretchLess();
	afx_msg void OnUVStretchMore();
	afx_msg void OnUVUp();
	afx_msg void OnCurrentFace();
	afx_msg void OnMarkedFaces();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void FlipTextureX(int facenum);
	void FlipTextureY(int facenum);
	void ExpandTextureU(int facenum,bool expand);
	void ExpandTextureV(int facenum,bool expand);
	void FaceMapTexture(int facenum);
};

void UVApply(room *rp, int facenum, int vertnum, float u, float v);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXALIGNDIALOG_H__F7AFF382_0055_11D3_A6A1_006097E07445__INCLUDED_)
