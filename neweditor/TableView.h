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
 

#if !defined(AFX_TABLEVIEW_H__CA620B22_EDCC_11D2_AB2B_006008BF0B09__INCLUDED_)
#define AFX_TABLEVIEW_H__CA620B22_EDCC_11D2_AB2B_006008BF0B09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TableView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTableObjectView dialog

class CTableObjectView : public CPropertyPage
{
	DECLARE_DYNCREATE(CTableObjectView)

// Construction
public:
	CTableObjectView();
	~CTableObjectView();
	void SetFilter(int filter);

// Dialog Data
	//{{AFX_DATA(CTableObjectView)
	enum { IDD = IDD_TABLED_OBJECTS };
	CListCtrl	m_List;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTableObjectView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_Filter;
	// Generated message map functions
	//{{AFX_MSG(CTableObjectView)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CTableTextureView dialog

class CTableTextureView : public CPropertyPage
{
	DECLARE_DYNCREATE(CTableTextureView)

// Construction
public:
	CTableTextureView();
	~CTableTextureView();
	void SetFilter(int filter);

// Dialog Data
	//{{AFX_DATA(CTableTextureView)
	enum { IDD = IDD_TABLED_TEXTURES };
	CListCtrl	m_List;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTableTextureView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_Filter;
	// Generated message map functions
	//{{AFX_MSG(CTableTextureView)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CTableView

class CTableView : public CPropertySheet
{
	DECLARE_DYNAMIC(CTableView)

// Construction
public:
	CTableView(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CTableView(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTableView)
	public:
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTableView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTableView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CTableDoorView dialog

class CTableDoorView : public CPropertyPage
{
	DECLARE_DYNCREATE(CTableDoorView)

// Construction
public:
	CTableDoorView();
	~CTableDoorView();
	void SetFilter(int filter);

// Dialog Data
	//{{AFX_DATA(CTableDoorView)
	enum { IDD = IDD_TABLED_DOORS };
	CListCtrl	m_List;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTableDoorView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_Filter;
	// Generated message map functions
	//{{AFX_MSG(CTableDoorView)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CTableSoundsView dialog

class CTableSoundsView : public CPropertyPage
{
	DECLARE_DYNCREATE(CTableSoundsView)

// Construction
public:
	CTableSoundsView();
	~CTableSoundsView();
	void SetFilter(int filter);

// Dialog Data
	//{{AFX_DATA(CTableSoundsView)
	enum { IDD = IDD_TABLED_SOUNDS };
	CListCtrl	m_List;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTableSoundsView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_Filter;
	// Generated message map functions
	//{{AFX_MSG(CTableSoundsView)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABLEVIEW_H__CA620B22_EDCC_11D2_AB2B_006008BF0B09__INCLUDED_)
