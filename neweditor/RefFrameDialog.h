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
 #if !defined(AFX_REFFRAMEDIALOG_H__D2556320_1637_11D3_A6A2_B9E0BFC20211__INCLUDED_)
#define AFX_REFFRAMEDIALOG_H__D2556320_1637_11D3_A6A2_B9E0BFC20211__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RefFrameDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRefFrameDialog dialog

class CRefFrameDialog : public CDialog
{
// Construction
public:
	CRefFrameDialog(char *caption, char *title, vector *initial, CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRefFrameDialog)
	enum { IDD = IDD_REF_FRAME };
	float	m_Xcoord;
	float	m_Ycoord;
	float	m_Zcoord;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRefFrameDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRefFrameDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	char *m_Caption;
	char *m_Title;
};

//Gets a vector from the user
//Parameters:	n - filled in the with return value
//					title - the title for the input window
//					prompt - the prompt for the input box
//Returns:	false if cancel was pressed on the dialog, else true
//				If false returned, n is unchanged
bool InputVector(vector *vec,char *title,char *prompt,CWnd *wnd = NULL);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REFFRAMEDIALOG_H__D2556320_1637_11D3_A6A2_B9E0BFC20211__INCLUDED_)
