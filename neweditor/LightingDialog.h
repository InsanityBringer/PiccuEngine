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
 #if !defined(AFX_LIGHTINGDIALOG_H__AD3FFCE0_1F46_11D3_A6A2_EB3867557D2F__INCLUDED_)
#define AFX_LIGHTINGDIALOG_H__AD3FFCE0_1F46_11D3_A6A2_EB3867557D2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LightingDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLightingDialog dialog

class CLightingDialog : public CDialog
{
// Construction
public:
	CLightingDialog(CWnd* pParent = NULL);   // standard constructor
	void UpdateDialog();

// Dialog Data
	//{{AFX_DATA(CLightingDialog)
	enum { IDD = IDD_LIGHTINGDIALOG };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLightingDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation

protected:
	void EnableControls(bool enable,int which);

	// Generated message map functions
	//{{AFX_MSG(CLightingDialog)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnPaint();
	afx_msg void OnKillfocusLightmapSpacing();
	afx_msg void OnKillfocusIterations();
	afx_msg void OnKillfocusIgnoreLimit();
	afx_msg void OnCombineFaces();
	afx_msg void OnSuperDetail();
	afx_msg void OnVolumeLights();
	afx_msg void OnLightsInMine();
	afx_msg void OnMineRadiosity();
	virtual void OnOK();
	afx_msg void OnClearLightmaps();
	afx_msg void OnResetMultiples();
	afx_msg void OnKillfocusAmbienceBlue();
	afx_msg void OnKillfocusAmbienceGreen();
	afx_msg void OnKillfocusAmbienceRed();
	afx_msg void OnFillCoronas();
	afx_msg void OnKillfocusMultiplier();
	afx_msg void OnRoomClearLightmaps();
	afx_msg void OnKillfocusRoomAmbienceBlue();
	afx_msg void OnKillfocusRoomAmbienceGreen();
	afx_msg void OnKillfocusRoomAmbienceRed();
	afx_msg void OnRoomFillCoronas();
	afx_msg void OnKillfocusRoomMultiplier();
	afx_msg void OnRoomRadiosity();
	afx_msg void OnRoomResetMultiples();
	afx_msg void OnRoomCountLights();
	afx_msg void OnFaceCorona();
	afx_msg void OnKillfocusFaceMultiplier();
	afx_msg void OnTouchesTerrain();
	afx_msg void OnTerrainRadiosity();
	afx_msg void OnLightsOnTerrain();
	afx_msg void OnDynamicTerrainLight();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIGHTINGDIALOG_H__AD3FFCE0_1F46_11D3_A6A2_EB3867557D2F__INCLUDED_)
