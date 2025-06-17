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
 

#ifndef _OBJ_MOVE_MANAGER_H
#define _OBJ_MOVE_MANAGER_H

#include "vecmat.h"

struct object;

const int OBJMOVEAXIS_X = 1,
			OBJMOVEAXIS_Y = 2,
			OBJMOVEAXIS_Z = 3,
			OBJMOVEAXIS_XY = 4,
			OBJMOVEAXIS_P = 5,
			OBJMOVEAXIS_H = 6,
			OBJMOVEAXIS_B = 7,
			OBJMOVEAXIS_PH = 8;

class ObjectMoveManager
{
	int m_DragState;					// 0 if not dragging, 1 if dragging. 2 if no drag.
	int m_MoveAxis;					// defined to a value from above.
	int m_ObjNum;						// object index to be moved.
	float m_WindowW2, m_WindowH2;	// window x and y centers, from which we translate mouse coords to 3d coords

	CWnd *m_DragWnd;					// window where we can drag the mouse
	RECT m_DragRect;

	matrix m_ViewMat;
	vector m_ViewPos;

//	dx and dy are the return values projected from dsx and dsy, given the object's position
	void GetObjectDeltas(float *dx, float *dy, object *obj, int dsx, int dsy);

public:
	ObjectMoveManager();

//	starts motion
	void Start(CWnd *wnd, int view_width, int view_height, vector *view_pos, matrix *view_mat, int x, int y);

//	ends current object movement
	void End();

//	defers control to the move manager
	void Defer();

//	tell whether we are drawing a selection box (if drag state == 1)
	bool IsMoving() const { return (m_DragState == 1); };

//	sets the moving axis
	void SetMoveAxis(int axis) { m_MoveAxis = axis; };
};

extern ObjectMoveManager ObjMoveManager;

#endif

