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
 


#if !defined(AFX_TEXTUREPALETTE_H__B8C78461_EEAA_11D2_AB2B_006008BF0B09__INCLUDED_)
#define AFX_TEXTUREPALETTE_H__B8C78461_EEAA_11D2_AB2B_006008BF0B09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TexturePalette.h : header file
//

#include "grListBox.h"
#include "ned_Tablefile.h"

#define SMALL_SIZE		16
#define MEDIUM_SIZE		32
#define LARGE_SIZE		64
#define NUM_SIZES		3

/////////////////////////////////////////////////////////////////////////////
// CTexturePalette dialog
class CTexturePick : public CGrListBox
{
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTexturePick)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CGrListBox)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


public:
	void SetParent(CWnd *wnd);
	void SetShowFilter(int filter);
	void OnCopyToClip(char *string);

protected:
	int m_ShowFilter;
	//	enumerates items maximum in the list box.  this is here to insure that any changes in
	//	the user-managed list.
	int ListEnumerate();

	// Specify the first item to be drawn in the box by DrawItem. returns the item #
	//	return -1 if there is no first item (list is empty, maybe)
	int ListFirstItem(int firstitem);

	// Specify the next/prev item to be drawn in the box by DrawItem. 
	//	Returns the item # following or preceding the passed the item #
	int ListNextItem(int curitem);
	int ListPrevItem(int curitem);

	void DrawItem(int x, int y, grHardwareSurface *itemsurf, int item);

	//	if item was selected from the list box, this function is invoked.
	void OnItemSelected(int item);

private:
	CWnd *ParentWnd;
	COleDropTarget	m_DropTarget;
};

class CTexturePalette : public CDialog
{
// Construction
public:
	CTexturePalette(CWnd* pParent = NULL);   // standard constructor
	BOOL Create();

// Dialog Data
	//{{AFX_DATA(CTexturePalette)
	enum { IDD = IDD_TEXTURE_PALETTE };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTexturePalette)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	CWnd* m_pParent;
	int m_nID;
	int m_ShowFilter;
	bool bCalledClose;
	CTexturePick m_TextureList;
	
	void PrepTextures(int filter);
	char texture_in_mem[MAX_TEXTURES];
	void SetFilterFlag(int flag,bool enabled);
	void DoPageInProgressDialog(int state,int count,int total);
	void EnableControls(bool enable);

	// Generated message map functions
	//{{AFX_MSG(CTexturePalette)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnDHud();
	afx_msg void OnDMine();
	afx_msg void OnDObject();
	afx_msg void OnDTerrain();
	afx_msg void OnDAnimated();
	afx_msg void OnDDestroyable();
	afx_msg void OnDFlythru();
	afx_msg void OnDForcefield();
	afx_msg void OnDLava();
	afx_msg void OnDLight();
	afx_msg void OnDMarble();
	afx_msg void OnDMetal();
	afx_msg void OnDPlastic();
	afx_msg void OnDProcedural();
	afx_msg void OnDRubble();
	afx_msg void OnDWater();
	afx_msg void OnDestroy();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTUREPALETTE_H__B8C78461_EEAA_11D2_AB2B_006008BF0B09__INCLUDED_)
