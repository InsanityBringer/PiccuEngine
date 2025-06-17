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
 
#ifndef SELMANAGER_H
#define SELMANAGER_H

#include "pstypes.h"

class CWnd;
class editorSelectorManager;

class editorSelectorManager
{
	int m_l, m_t, m_r, m_b;				// selection rectangle in owner window coords
	int m_DragX, m_DragY;				// dragging x,y in desktop coords
	int m_DragState;					// 0 if not dragging, 1 if dragging. 2 if no drag, but sel.

	void (*m_EndFunc)(editorSelectorManager *);

public:

	CWnd *m_OwnerWnd;					// owner window of selection box

	editorSelectorManager();

//	sets the owner window of selection
//	starts a selection at x,y
	void StartSelection(CWnd *wnd, void (*func)(editorSelectorManager *), int x, int y);

//	removes currently drawn rectangle
	void EndSelection();

//	returns the selected rectangle.
	void GetSelectedRect(int *l, int *t, int *r, int *b);

//	defers control to the selector manager
	void Defer();

//	tell whether we are drawing a selection box (if drag state == 1)
	bool IsSelecting() const { return (m_DragState == 1); };
};

extern editorSelectorManager SelManager;

#endif





		
