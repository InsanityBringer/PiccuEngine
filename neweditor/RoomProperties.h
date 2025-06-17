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
 #if !defined(AFX_ROOMPROPERTIES_H__C7EDE200_2F63_11D3_A6A2_444553540000__INCLUDED_)
#define AFX_ROOMPROPERTIES_H__C7EDE200_2F63_11D3_A6A2_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RoomProperties.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRoomProperties dialog

class CRoomProperties : public CDialog
{
// Construction
public:
	CRoomProperties(CWnd* pParent = NULL);   // standard constructor
	void UpdateDialog();

// Dialog Data
	//{{AFX_DATA(CRoomProperties)
	enum { IDD = IDD_ROOM_PROPERTIES };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRoomProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
	prim *m_pPrim;					// the primitives

protected:
	void EnableControls(bool enable);
	void SetWind();
	void GetWind(float *speed,vector *direction);

	// Generated message map functions
	//{{AFX_MSG(CRoomProperties)
	afx_msg void OnRefuelingCheck();
	afx_msg void OnSecretCheck();
	afx_msg void OnRedGoalCheck();
	afx_msg void OnBlueGoalCheck();
	afx_msg void OnGreenGoalCheck();
	afx_msg void OnYellowGoalCheck();
	afx_msg void OnSpecial1Check();
	afx_msg void OnSpecial2Check();
	afx_msg void OnSpecial3Check();
	afx_msg void OnSpecial4Check();
	afx_msg void OnSpecial5Check();
	afx_msg void OnSpecial6Check();
	afx_msg void OnPaint();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnRoomName();
	virtual void OnOK();
	afx_msg void OnKillfocusRoomDamageAmount();
	afx_msg void OnSelchangeRoomDamageType();
	afx_msg void OnFaceGoalCheck();
	afx_msg void OnKillfocusMirrorFace();
	afx_msg void OnRadioNoMirror();
	afx_msg void OnRadioMirrorFace();
	afx_msg void OnSetMirror();
	afx_msg void OnFogEnable();
	afx_msg void OnKillfocusFogDepth();
	afx_msg void OnKillfocusFogRed();
	afx_msg void OnKillfocusFogGreen();
	afx_msg void OnKillfocusFogBlue();
	afx_msg void OnWindMake();
	afx_msg void OnKillfocusWindSpeed();
	afx_msg void OnKillfocusWindX();
	afx_msg void OnKillfocusWindY();
	afx_msg void OnKillfocusWindZ();
	afx_msg void OnSelchangeRoomAmbient();
	afx_msg void OnRoomExternal();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROOMPROPERTIES_H__C7EDE200_2F63_11D3_A6A2_444553540000__INCLUDED_)
