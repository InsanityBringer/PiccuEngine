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
 #if !defined(AFX_TERRAINDIALOGBAR_H__BFB2B902_F749_11D2_A6A1_006097E07445__INCLUDED_)
#define AFX_TERRAINDIALOGBAR_H__BFB2B902_F749_11D2_A6A1_006097E07445__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TerrainDialogBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTerrainDialogBar dialog

class CTerrainDialogBar : public CDialogBar
{
// Construction
public:
	CTerrainDialogBar();   // standard constructor
	void InitBar();
	void SetCurrentSat(int);
	void MoveSat (int,int);
	void UpdateDialog();

// Dialog Data
	//{{AFX_DATA(CTerrainDialogBar)
	enum { IDD = IDD_TERRAINBAR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTerrainDialogBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
private:
	int Current_satellite;

protected:

	// Generated message map functions
	//{{AFX_MSG(CTerrainDialogBar)
	afx_msg void OnControlUpdate(CCmdUI*);
	afx_msg LONG OnInitDialog ( UINT, LONG );
	afx_msg void OnTerrainOcclusion();
	afx_msg void OnSkyContract();
	afx_msg void OnSkyExpand();
	afx_msg void OnSkyStars();
	afx_msg void OnSkySatellites();
	afx_msg void OnSkyPrevSat();
	afx_msg void OnSkyNextSat();
	afx_msg void OnSkyFog();
	afx_msg void OnKillfocusSkyDamage();
	afx_msg void OnImportPcxMap();
	afx_msg void OnExportPcxMap();
	afx_msg void OnSkyTextured();
	afx_msg void OnSkyRotateSky();
	afx_msg void OnSkyRotateStars();
	afx_msg void OnKillfocusSkyRotDeg();
	afx_msg void OnKillfocusSkyFogScalar();
	afx_msg void OnSkyMoreSats();
	afx_msg void OnSkyLessSats();
	afx_msg void OnSkyHalo();
	afx_msg void OnSkyAtmosphere();
	afx_msg void OnSkySatMoveUp();
	afx_msg void OnSkySatMoveRight();
	afx_msg void OnSkySatMoveLeft();
	afx_msg void OnSkySatMoveDown();
	afx_msg void OnToggleCellVisible();
	afx_msg void OnDestroy();
	afx_msg void OnSkyMoveSatCloser();
	afx_msg void OnSkyMoveSatFarther();
	afx_msg void OnTerrainSelect();
	afx_msg void OnForceCellVisible();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TERRAINDIALOGBAR_H__BFB2B902_F749_11D2_A6A1_006097E07445__INCLUDED_)
