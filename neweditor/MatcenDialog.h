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
 #if !defined(AFX_MATCENDIALOG_H__6D6F1340_4F6A_11D3_A6A2_444553540000__INCLUDED_)
#define AFX_MATCENDIALOG_H__6D6F1340_4F6A_11D3_A6A2_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MatcenDialog.h : header file
//

enum {	TAB_MATCEN_ATTACH,
		TAB_MATCEN_PRODUCE,
		TAB_MATCEN_DISPLAY};

class CMatcenDialog;

/////////////////////////////////////////////////////////////////////////////
// CMcAttach dialog

class CMcAttach : public CDialog
{
	friend CMatcenDialog;

// Construction
public:
	CMcAttach(CWnd* pParent = NULL);   // standard constructor
	CMcAttach(UINT nIDTemplate,CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CMcAttach)
	enum { IDD = IDD_MATCEN_ATTACH };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMcAttach)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CObjectListCombo	m_object_combo;
	CRoomListCombo	m_room_combo;

	// Generated message map functions
	//{{AFX_MSG(CMcAttach)
	afx_msg void OnMatcenRoomAttach();
	afx_msg void OnMatcenObjectAttach();
	afx_msg void OnMatcenUnassignedAttach();
	afx_msg void OnKillfocusMatcenSpawn0();
	afx_msg void OnKillfocusMatcenSpawn1();
	afx_msg void OnKillfocusMatcenSpawn2();
	afx_msg void OnKillfocusMatcenSpawn3();
	afx_msg void OnKillfocusMatcenSpawnNum();
	afx_msg void OnSelendokMatcenRoomObjectCombo();
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CMcProduce dialog

class CMcProduce : public CDialog
{
// Construction
public:
	CMcProduce(CWnd* pParent = NULL);   // standard constructor
	CMcProduce(UINT nIDTemplate,CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CMcProduce)
	enum { IDD = IDD_MATCEN_PRODUCE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMcProduce)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMcProduce)
	afx_msg void OnMatcenApnRadio();
	afx_msg void OnMatcenApvRadio();
	afx_msg void OnMatcenScRadio();
	afx_msg void OnMatcenWpnRadio();
	afx_msg void OnMatcenWpvRadio();
	afx_msg void OnKillfocusMatcenObjtypeNum();
	afx_msg void OnSelendokMatcenProd0List();
	afx_msg void OnSelendokMatcenProd1List();
	afx_msg void OnSelendokMatcenProd2List();
	afx_msg void OnSelendokMatcenProd3List();
	afx_msg void OnSelendokMatcenProd4List();
	afx_msg void OnSelendokMatcenProd5List();
	afx_msg void OnSelendokMatcenProd6List();
	afx_msg void OnSelendokMatcenProd7List();
	afx_msg void OnKillfocusMatcenProd0Quantity();
	afx_msg void OnKillfocusMatcenProd1Quantity();
	afx_msg void OnKillfocusMatcenProd2Quantity();
	afx_msg void OnKillfocusMatcenProd3Quantity();
	afx_msg void OnKillfocusMatcenProd4Quantity();
	afx_msg void OnKillfocusMatcenProd5Quantity();
	afx_msg void OnKillfocusMatcenProd6Quantity();
	afx_msg void OnKillfocusMatcenProd7Quantity();
	afx_msg void OnKillfocusMatcenProd0Seconds();
	afx_msg void OnKillfocusMatcenProd1Seconds();
	afx_msg void OnKillfocusMatcenProd2Seconds();
	afx_msg void OnKillfocusMatcenProd3Seconds();
	afx_msg void OnKillfocusMatcenProd4Seconds();
	afx_msg void OnKillfocusMatcenProd5Seconds();
	afx_msg void OnKillfocusMatcenProd6Seconds();
	afx_msg void OnKillfocusMatcenProd7Seconds();
	afx_msg void OnKillfocusMatcenProd0Priority();
	afx_msg void OnKillfocusMatcenProd1Priority();
	afx_msg void OnKillfocusMatcenProd2Priority();
	afx_msg void OnKillfocusMatcenProd3Priority();
	afx_msg void OnKillfocusMatcenProd4Priority();
	afx_msg void OnKillfocusMatcenProd5Priority();
	afx_msg void OnKillfocusMatcenProd6Priority();
	afx_msg void OnKillfocusMatcenProd7Priority();
	afx_msg void OnMatcenProduceEnabled();
	afx_msg void OnMatcenProduceHurt();
	afx_msg void OnMatcenProduceRandom();
	afx_msg void OnMatcenProduceOrdered();
	afx_msg void OnKillfocusMatcenProduceMax();
	afx_msg void OnKillfocusMatcenProduceMult();
	afx_msg void OnKillfocusMatcenProduceMaxalive();
	afx_msg void OnKillfocusMatcenProducePretime();
	afx_msg void OnKillfocusMatcenProducePosttime();
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CMcDisplay dialog

class CMcDisplay : public CDialog
{
// Construction
public:
	CMcDisplay(CWnd* pParent = NULL);   // standard constructor
	CMcDisplay(UINT nIDTemplate,CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CMcDisplay)
	enum { IDD = IDD_MATCEN_DISPLAY };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMcDisplay)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMcDisplay)
	afx_msg void OnSelendokDisplayActiveSound();
	afx_msg void OnSelendokDisplayProdEffect();
	afx_msg void OnSelendokDisplayProdSound();
	afx_msg void OnSelendokDisplayProdTexture();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CMatcenDialog dialog

class CMatcenDialog : public CDialog
{
// Construction
public:
	CMatcenDialog(CWnd* pParent = NULL);   // standard constructor
	void UpdateDialog();

// Dialog Data
	//{{AFX_DATA(CMatcenDialog)
	enum { IDD = IDD_MATCENS };
	CTabCtrl	m_TabCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMatcenDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void EnableFields(bool f_enable);
	void InitTabs();
	void DoMatcenTabNotify(NMHDR *nmhdr);
	void ShowCurrentMatcenTab();
	void EnableSpawnFields(int num_enabled);
	void EnableProdFields(int num_enabled);

	// Tabs
	int m_CurrentMatcenTab;
	CDialog *m_CurrentDlg;
	CMcAttach *m_Adlg;
	CMcProduce *m_Pdlg;
	CMcDisplay *m_Ddlg;

	// Generated message map functions
	//{{AFX_MSG(CMatcenDialog)
	afx_msg void OnMatcenNew();
	afx_msg void OnMatcenDelete();
	afx_msg void OnMatcenPrev();
	afx_msg void OnMatcenNext();
	afx_msg void OnMatcenName();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MATCENDIALOG_H__6D6F1340_4F6A_11D3_A6A2_444553540000__INCLUDED_)
