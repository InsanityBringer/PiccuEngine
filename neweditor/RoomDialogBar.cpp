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
 



#include "stdafx.h"
#include "neweditor.h"
#include "RoomDialogBar.h"
#include "ExtrudeDialog.h"
#include "LatheDialog.h"
#include "BendDialog.h"
#include "RefFrameDialog.h"
#include "ned_Geometry.h"
#include "HRoom.h"
#include "erooms.h"
#include "EditLineDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRoomDialogBar dialog


CRoomDialogBar::CRoomDialogBar()
{
	//{{AFX_DATA_INIT(CRoomDialogBar)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRoomDialogBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRoomDialogBar)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRoomDialogBar, CDialogBar)
	//{{AFX_MSG_MAP(CRoomDialogBar)
	ON_BN_CLICKED(IDC_MODIFY_EXTRUDE, OnModifyExtrude)
	ON_UPDATE_COMMAND_UI(IDC_MODIFY_EXTRUDE,OnControlUpdate)
	ON_BN_CLICKED(IDC_MODIFY_LATHE, OnModifyLathe)
	ON_BN_CLICKED(IDC_MODIFY_BEND, OnModifyBend)
	ON_BN_CLICKED(IDC_FACE_PLANAR_CHECK, OnFacePlanarCheck)
	ON_BN_CLICKED(IDC_FACE_SPLIT_CURRENT_FACE, OnFaceSplitCurrentFace)
	ON_BN_CLICKED(IDC_FACE_TRIANGULATE_NONP, OnFaceTriangulateNonPlanar)
	ON_BN_CLICKED(IDC_FACE_TRIANGULATE_CURRENT, OnFaceTriangulateCurrent)
	ON_BN_CLICKED(IDC_EDGE_CONTRACT, OnEdgeContract)
	ON_BN_CLICKED(IDC_EDGE_EXPAND, OnEdgeExpand)
	ON_BN_CLICKED(IDC_FACE_CONTRACT, OnFaceContract)
	ON_BN_CLICKED(IDC_FACE_EXPAND, OnFaceExpand)
	ON_BN_CLICKED(IDC_FACE_JOIN, OnFaceJoin)
	ON_BN_CLICKED(IDC_ROOM_CONTRACT, OnRoomContract)
	ON_BN_CLICKED(IDC_ROOM_EXPAND, OnRoomExpand)
	ON_BN_CLICKED(IDC_MODIFY_SET_REF_FRAME, OnModifySetRefFrame)
	ON_BN_CLICKED(IDC_VERTEX_REMOVE_EXTRAS, OnVertexRemoveExtras)
	ON_BN_CLICKED(IDC_FACE_FLIP, OnFaceFlip)
	ON_BN_CLICKED(IDC_VERTEX_SNAP_FACE, OnVertexSnapFace)
	ON_BN_CLICKED(IDC_VERTEX_SNAP_EDGE, OnVertexSnapEdge)
	ON_BN_CLICKED(IDC_VERTEX_SNAP_GRID, OnVertexSnapGrid)
	ON_BN_CLICKED(IDC_VERTEX_SNAP_VERT, OnVertexSnapVert)
	ON_BN_CLICKED(IDC_ROOM_MIRROR, OnRoomMirror)
	ON_BN_CLICKED(IDC_FACE_ADD_VERT, OnFaceAddVert)
	ON_BN_CLICKED(IDC_FACE_REMOVE_VERT, OnFaceRemoveVert)
	ON_BN_CLICKED(IDC_FACE_SWAP_VERTS, OnFaceSwapVerts)
	ON_BN_CLICKED(IDC_VERTEX_SNAP_EDGE2, OnVertexSnapEdge2)
	ON_BN_CLICKED(IDC_VERTEX_SNAP_FACE2, OnVertexSnapFace2)
	ON_BN_CLICKED(IDC_VERTEX_SNAP_GRID2, OnVertexSnapGrid2)
	ON_BN_CLICKED(IDC_VERTEX_SNAP_VERT2, OnVertexSnapVert2)
	ON_BN_CLICKED(IDC_FACE_TWIST2, OnFaceTwist)
	ON_UPDATE_COMMAND_UI(IDC_MODIFY_LATHE,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_MODIFY_BEND,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_FACE_PLANAR_CHECK,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_FACE_SPLIT_CURRENT_FACE,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_FACE_TRIANGULATE_NONP,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_FACE_TRIANGULATE_CURRENT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_EDGE_CONTRACT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_EDGE_EXPAND,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_FACE_CONTRACT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_FACE_EXPAND,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_FACE_JOIN,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_ROOM_CONTRACT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_ROOM_EXPAND,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_MODIFY_SET_REF_FRAME,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_VERTEX_REMOVE_EXTRAS,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_FACE_FLIP,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_VERTEX_SNAP_FACE,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_VERTEX_SNAP_EDGE,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_VERTEX_SNAP_GRID,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_VERTEX_SNAP_VERT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_ROOM_MIRROR,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_FACE_ADD_VERT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_FACE_REMOVE_VERT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_FACE_TWIST2,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_VERTEX_SNAP_FACE2,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_VERTEX_SNAP_EDGE2,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_VERTEX_SNAP_GRID2,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_VERTEX_SNAP_VERT2,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_FACE_SWAP_VERTS,OnControlUpdate)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog )
END_MESSAGE_MAP()

void CRoomDialogBar::InitBar()
{
	// TODO: Add extra initialization here
	HINSTANCE hinst;
	HICON hicon;

	hinst = AfxGetInstanceHandle();
	// Associate bitmaps with the buttons
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_ROOM_EXPAND),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_ROOM_EXPAND))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_ROOM_CONTRACT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_ROOM_CONTRACT))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_FACE_EXPAND),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_FACE_EXPAND))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_FACE_CONTRACT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_FACE_CONTRACT))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_EDGE_EXPAND),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_EDGE_EXPAND))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_EDGE_CONTRACT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_EDGE_CONTRACT))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_MODIFY_SET_REF_FRAME),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_MODIFY_SET_REF_FRAME))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_MODIFY_EXTRUDE),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_MODIFY_EXTRUDE))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_MODIFY_LATHE),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_MODIFY_LATHE))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_MODIFY_BEND),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_MODIFY_BEND))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_FACE_JOIN),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_FACE_JOIN))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_FACE_FLIP),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_FACE_FLIP))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_FACE_PLANAR_CHECK),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_FACE_PLANAR_CHECK))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_FACE_SPLIT_CURRENT_FACE),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_FACE_SPLIT_CURRENT_FACE))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_FACE_TRIANGULATE_CURRENT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_FACE_TRIANGULATE_CURRENT))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_FACE_TRIANGULATE_NONP),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_FACE_TRIANGULATE_NONP))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_VERTEX_REMOVE_EXTRAS),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_VERTEX_REMOVE_EXTRAS))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_VERTEX_SNAP_FACE),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_VERTEX_SNAP_FACE))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_VERTEX_SNAP_EDGE),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_VERTEX_SNAP_EDGE))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_VERTEX_SNAP_VERT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_VERTEX_SNAP_VERT))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_VERTEX_SNAP_GRID),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_VERTEX_SNAP_GRID))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_VERTEX_SNAP_FACE2),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_VERTEX_SNAP_FACE2))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_VERTEX_SNAP_EDGE2),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_VERTEX_SNAP_EDGE2))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_VERTEX_SNAP_VERT2),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_VERTEX_SNAP_VERT2))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_VERTEX_SNAP_GRID2),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_VERTEX_SNAP_GRID2))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_ROOM_MIRROR),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_ROOM_MIRROR))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_EDGE_ADD_VERT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_FACE_ADD_VERT))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_EDGE_REMOVE_VERT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_FACE_REMOVE_VERT))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_FACE_TWIST),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_FACE_TWIST2))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_FACE_SWAP_VERTS),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_FACE_SWAP_VERTS))->SetIcon(hicon);
}

/////////////////////////////////////////////////////////////////////////////
// CRoomDialogBar message handlers

LONG CRoomDialogBar::OnInitDialog ( UINT wParam, LONG lParam)
{
	if ( !HandleInitDialog(wParam, lParam) || !UpdateData(FALSE))
	{
		TRACE0("Warning: UpdateData failed during dialog init.\n");
		return FALSE;
	}

	InitBar();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// enable controls
void CRoomDialogBar::OnControlUpdate(CCmdUI* pCmdUI)
{
	bool bEnable = false;

	CFrameWnd *frmwnd = (CFrameWnd *) theApp.AcquireWnd();
	if(frmwnd != NULL && frmwnd->IsKindOf(RUNTIME_CLASS(CRoomFrm)) && frmwnd == theApp.m_pFocusedRoomFrm)
		bEnable = true;

	pCmdUI->Enable(bEnable);
}

void CRoomDialogBar::OnModifyExtrude() 
{
	// TODO: Add your control notification handler code here
	CExtrudeDialog ex_dlg;
	if ( ex_dlg.DoModal() == IDCANCEL )
		return;
	HandleExtrude(ex_dlg.m_Direction,ex_dlg.m_Distance,ex_dlg.m_DeleteBaseFace,ex_dlg.m_Inward,ex_dlg.m_Faces);
}

void CRoomDialogBar::OnModifyLathe() 
{
	// TODO: Add your control notification handler code here
	CLatheDialog lth_dlg;
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;

	if (rp != NULL)
	{
		// Get list of marked verts
		short marked_list[MAX_VERTS_PER_ROOM];
		int num_m_verts = wnd->GetMarkedVerts(marked_list);
		ASSERT(num_m_verts == wnd->m_Num_marked_verts);

		if (num_m_verts > 1)
		{
			if ( lth_dlg.DoModal() == IDCANCEL )
				return;

			int which = lth_dlg.m_Rotation;
			int num_sides = lth_dlg.m_Num_sides;
			BOOL bCaps = lth_dlg.m_bEndCaps;
			int inward = lth_dlg.m_Inward;

			int num_faces = rp->num_faces;
			if ( LatheVerts(rp,marked_list,num_m_verts,which,num_sides,bCaps,inward,wnd->m_vec_RefPos) )
			{
				for (int i=0; i<num_m_verts; i++)
					wnd->UnMarkVert(marked_list[i]);
				wnd->m_Room_changed = true;
				PrintStatus("Vertices lathed.");
				wnd->SetPrim(prim->roomp,num_faces,prim->roomp->faces[num_faces].portal_num,0,0);
			}
		}
		else
			MessageBox("You must have two or more vertices marked in order to use the lathe function.");
	}
	else
		MessageBox("No current room!");
}


void CRoomDialogBar::OnModifyBend() 
{
	// TODO: Add your control notification handler code here
	CBendDialog bend_dlg;
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;

	if (rp != NULL)
	{
		// Get list of marked verts
		short marked_list[MAX_VERTS_PER_ROOM];
		int num_m_verts = wnd->GetMarkedVerts(marked_list);
		ASSERT(num_m_verts == wnd->m_Num_marked_verts);

		if (num_m_verts > 0)
		{
			if ( bend_dlg.DoModal() == IDCANCEL )
				return;

			int which = bend_dlg.m_Direction;
			float dist = bend_dlg.m_Distance;
			float ang = bend_dlg.m_Rotation;

			if ( BendVerts(rp,marked_list,num_m_verts,which,ang,dist,wnd->m_vec_RefPos) )
			{
				for (int i=0; i<num_m_verts; i++)
					wnd->UnMarkVert(marked_list[i]);
				wnd->m_Room_changed = true;
				PrintStatus("Vertices bended.");
			}
		}
		else
			MessageBox("You must have one or more vertices marked in order to use the bend function.");
	}
	else
		MessageBox("No current room!");
}

/*
void CRoomDialogBar::OnVertexMerge() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;

	if (rp != NULL)
	{
		// Get list of marked verts
		short marked_list[MAX_VERTS_PER_ROOM];
		int num_m_verts = wnd->GetMarkedVerts(marked_list);
		ASSERT(num_m_verts == wnd->m_Num_marked_verts);

		if (num_m_verts > 0)
		{
			if ( wnd->MergeMarkedVerts() )
			{
				wnd->m_Room_changed = true;
				PrintStatus("Vertices merged.");
			}
		}
		else
			MessageBox("You must have two or more vertices marked in order to attempt a merge.");
	}
	else
		MessageBox("No current room!");
}
*/

void CRoomDialogBar::OnFacePlanarCheck() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;

	if (rp != NULL)
	{
		// Get list of marked faces
		short marked_list[MAX_FACES_PER_ROOM];
		int num_m_faces = wnd->GetMarkedFaces(marked_list);
		ASSERT(num_m_faces == wnd->m_Num_marked_faces);

		if (num_m_faces > 0)
		{
			short non_planars[MAX_FACES_PER_ROOM];
			int i, num_non_planars = 0;
			int facenum;

			for (i=0; i<num_m_faces; i++)
			{
				facenum = marked_list[i];
				if ( !FaceIsPlanar(rp,facenum) )
					non_planars[num_non_planars++] = facenum;
			}
			if (num_non_planars > 0)
			{
				int input = MessageBox("Some marked faces have been found to be non-planar. Would you like to triangulate these faces?", "Triangulate non-planars?", MB_YESNO);
				if (input == IDYES)
				{
					int count = 0;
					int old_num_faces,num_new_faces;
					int num_triangulated_faces = 0;

					for (i=0; i<num_non_planars; i++)
					{
						old_num_faces = rp->num_faces;
						TriangulateFace(rp,non_planars[i],0);
						count += num_new_faces = rp->num_faces - old_num_faces + 1;
						if (num_new_faces > 0)
						{
							num_triangulated_faces++;
							wnd->UnMarkFace(non_planars[i]);
						}
					}
					if (num_triangulated_faces > 0)
					{
						wnd->m_Room_changed = true;
						PrintStatus("%d marked face(s) replaced by %d new faces.", num_triangulated_faces, count);
					}
				}
			}
			else
				MessageBox("All marked faces are planar.");
		}
		else
			MessageBox("You must mark one or more faces in order to use this function.");
	}
	else
		MessageBox("No current room!");
}

void SplitFace(room *rp,int facenum,int v0,int v1);

void CRoomDialogBar::OnFaceSplitCurrentFace() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;

	if (rp != NULL && prim->face != -1)
	{
		// Get list of marked verts
		short marked_list[MAX_VERTS_PER_ROOM];
		int num_m_verts = wnd->GetMarkedVerts(marked_list);
		ASSERT(num_m_verts == wnd->m_Num_marked_verts);

		if (num_m_verts > 0)
		{
			int facenum = prim->face;
			int v0 = -1; int v1 = -1;
			int too_many = 0;

			for (int i=0; i<num_m_verts; i++)
			{
				int index = VertInFace(rp,facenum,marked_list[i]);
				if ( index != -1 )
				{
					if (v0 == -1)
						v0 = index;
					else if (v1 == -1)
						v1 = index;
					else
					{
						too_many = 1; // only two verts should be marked; we've got too many now
						continue;
					}
				}
			}

			if ( !too_many )
			{
				if (v0 != -1 && v1 != -1)
				{
					SplitFace(rp,facenum,v0,v1);
					// Unmark the verts
					wnd->UnMarkVert(marked_list[0]);
					wnd->UnMarkVert(marked_list[1]);
					wnd->m_Room_changed = true;
				}
				else
					MessageBox("You need to mark two verts in the current face in order to split the face.");
			}
			else
				MessageBox("Only two verts may be marked in the current face for splitting to occur.");
		}
		else
			MessageBox("You need to mark two verts in the current face in order to split the face.");
	}
	else
		MessageBox("No current face!");
}

int VertInFace(room *rp, int facenum, int vertnum)
{
	if (facenum != -1)
	{
		face *fp = &rp->faces[facenum];

		for (int i=0; i<fp->num_verts; i++)
			if (fp->face_verts[i] == vertnum)
				return i;
	}

	return -1;
}

void CRoomDialogBar::OnFaceTriangulateNonPlanar() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;

	if (rp != NULL)
	{
		// Get list of marked faces
		short marked_list[MAX_FACES_PER_ROOM];
		int num_m_faces = wnd->GetMarkedFaces(marked_list);
		ASSERT(num_m_faces == wnd->m_Num_marked_faces);

		if (num_m_faces > 0)
		{
			int count = 0;
			int old_num_faces,num_new_faces;
			int num_triangulated_faces = 0;

			for (int i=0; i<num_m_faces; i++)
			{
				int facenum = marked_list[i];
				int vertnum = 0;

				if ( !FaceIsPlanar(rp,facenum) )
				{
					old_num_faces = rp->num_faces;
					TriangulateFace(rp,facenum,vertnum);
					count += num_new_faces = rp->num_faces - old_num_faces;
					if (num_new_faces > 0)
					{
						num_triangulated_faces++;
						wnd->UnMarkFace(facenum);
					}
				}
			}

			if (num_triangulated_faces > 0)
			{
				wnd->m_Room_changed = true;
				PrintStatus("%d marked faces replaced by %d new faces.", num_triangulated_faces, count);
			}
			else
				MessageBox("All marked faces are already planar. No triangulation performed.");
		}
		else
			MessageBox("You must mark one or more faces in order to use this function.");
	}
	else
		MessageBox("No current room!");
}

void CRoomDialogBar::OnFaceTriangulateCurrent() 
{
	// TODO: Add your control notification handler code here
	CLatheDialog lth_dlg;
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;
	int facenum = prim->face;
	int vertnum = prim->vert;

	if (rp != NULL && facenum != -1 && vertnum != -1)
	{
		int old_num_faces = rp->num_faces;
		TriangulateFace(rp,facenum,vertnum);
		int num_new_faces = rp->num_faces - old_num_faces + 1;

		if (num_new_faces > 0)
		{
			wnd->m_Room_changed = true;
			// Update current primitives
			int portnum;
			prim->face = rp->num_faces - num_new_faces;
			prim->face != -1 ? portnum = prim->roomp->faces[prim->face].portal_num : portnum = -1;
			wnd->SetPrim(prim->roomp,prim->face,portnum,0,0);

			// Update the current face/texture displays
			Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(prim->roomp), prim->face);

			PrintStatus("Face %d replaced by %d new faces.", facenum, num_new_faces);
		}
		else
			PrintStatus("Face %d is already a triangle!", facenum);
	}
	else
		MessageBox("No current face vert!");
}

void CRoomDialogBar::OnEdgeContract() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;
	int facenum = prim->face;
	int edgenum = prim->edge;

	if (rp != NULL && facenum != -1 && edgenum != -1)
	{
		SizeFaceEdge(rp,facenum,edgenum,0.95f);
		wnd->m_Room_changed = true;
	}
	else
		MessageBox("No current edge!");
}

void CRoomDialogBar::OnEdgeExpand() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;
	int facenum = prim->face;
	int edgenum = prim->edge;

	if (rp != NULL && facenum != -1 && edgenum != -1)
	{
		SizeFaceEdge(rp,facenum,edgenum,1.05f);
		wnd->m_Room_changed = true;
	}
	else
		MessageBox("No current edge!");
}

void CRoomDialogBar::OnFaceContract() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;
	int facenum = prim->face;

	if (rp != NULL && facenum != -1)
	{
		SizeFaceVertices(rp,facenum,0.95f);
		wnd->m_Room_changed = true;
	}
	else
		MessageBox("No current face!");
}

void CRoomDialogBar::OnFaceExpand() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;
	int facenum = prim->face;

	if (rp != NULL && facenum != -1)
	{
		SizeFaceVertices(rp,facenum,1.05f);
		wnd->m_Room_changed = true;
	}
	else
		MessageBox("No current face!");
}

void CRoomDialogBar::OnFaceJoin() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;
	int facenum = prim->face;

	if (rp != NULL && facenum != -1)
	{
		// Get list of marked faces
		short marked_list[MAX_FACES_PER_ROOM];
		int num_m_faces = wnd->GetMarkedFaces(marked_list);
		ASSERT(num_m_faces == wnd->m_Num_marked_faces);

		if (num_m_faces == 1)
		{
			if (marked_list[0] != facenum)
			{
				//Combine the faces
				if (! CombineFaces(rp,marked_list[0],facenum))
					OutrageMessageBox("Cannot combine faces: %s",GetErrorMessage());
				else
				{
					wnd->m_Room_changed = true;
					if (facenum < marked_list[0])
					{
						wnd->UnMarkFace(marked_list[0]);
						wnd->MarkFace(marked_list[0]-1);
					}

					// Update current primitives
					int portnum;
					prim->face = marked_list[0]-1;
					prim->face != -1 ? portnum = prim->roomp->faces[prim->face].portal_num : portnum = -1;
					wnd->SetPrim(prim->roomp,prim->face,portnum,0,0);
					// Update the current face/texture displays
					Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(prim->roomp), prim->face);
				}
			}
			else
				OutrageMessageBox("The marked face and current face cannot be the same for this operation.");
		}
		else if (num_m_faces > 1)
			OutrageMessageBox("You must have only one face marked for this operation.");
		else if (num_m_faces == 0)
			OutrageMessageBox("You must have a marked face for this operation.");
	}
	else
		MessageBox("No current face!");
}

void CRoomDialogBar::OnRoomContract() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	if (prim->roomp != NULL)
	{
		SizeRoomVertices (prim->roomp,0.95f);
		wnd->m_Room_changed = true;
	}
	else
		MessageBox("No current room!");
}

void CRoomDialogBar::OnRoomExpand() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	if (prim->roomp != NULL)
	{
		SizeRoomVertices (prim->roomp,1.05f);
		wnd->m_Room_changed = true;
	}
	else
		MessageBox("No current room!");
}


/////////////////////////////////////////////////////////////////////////////
// helper functions


// Goes through all the vertices in the room and scales them by scale_factor
void SizeRoomVertices (room *rp,float scale_factor)
{
	vector room_center;
	vector vec;
	int portal_list[MAX_VERTS_PER_ROOM];
	int i,t,ok_to_scale;
	int num_portal_verts;

	ASSERT (rp->used>0);
	
	num_portal_verts=BuildListOfPortalVerts(rp,portal_list); 

	ComputeRoomCenter (&room_center,rp);

	for (i=0;i<rp->num_verts;i++)
	{
		for (ok_to_scale=1,t=0;t<num_portal_verts;t++)
		{
			if (portal_list[t]==i)
			{
				ok_to_scale=0;
				break;
			}
			
		}

		if (ok_to_scale)
		{
			vec=rp->verts[i]-room_center;
			vec*=scale_factor;
			rp->verts[i]=vec+room_center;
		}
	
	}
	
}


// Goes through all the vertices in a face and scales them by scale_factor
void SizeFaceVertices (room *rp,int facenum,float scale_factor)
{
	vector face_center;
	vector vec;
	int portal_list[MAX_VERTS_PER_ROOM];
	int face_list[MAX_FACES_PER_ROOM];
	int i,t,ok_to_scale,k;
	int num_portal_verts;
	int facecount=0;
	face *fp=&rp->faces[facenum];

	ASSERT (rp->used>0);
	
	num_portal_verts=BuildListOfPortalVerts(rp,portal_list); 

	ComputeCenterPointOnFace(&face_center,rp,facenum);

	for (i=0;i<fp->num_verts;i++)
	{
		int vertnum=fp->face_verts[i];

		for (ok_to_scale=1,t=0;t<num_portal_verts;t++)
		{
			if (portal_list[t]==vertnum)
			{
				ok_to_scale=0;
				break;
			}
			
		}

		if (ok_to_scale)
		{
			vec=rp->verts[vertnum]-face_center;
			vec*=scale_factor;
			rp->verts[vertnum]=vec+face_center;
		}
	}

	// Build a list of faces that share vertices with this face
	for (i=0;i<rp->num_faces;i++)
	{
		for (t=0;t<rp->faces[i].num_verts;t++)
		{
			for (k=0;k<fp->num_verts;k++)
			{
				if (fp->face_verts[k]==rp->faces[i].face_verts[t])
				{
					face_list[facecount]=i;
					facecount++;
				}
			}
		}
	}

	// Now fixup any concave/nonplanar faces
	FixConcaveFaces (rp,face_list,facecount);
	
}

void SizeFaceEdge (room *rp,int facenum,int edge,float scale_factor)
{
	vector line_center;
	vector vec;
	int portal_list[MAX_VERTS_PER_ROOM];
	int face_list[MAX_FACES_PER_ROOM];
	int i,t,ok_to_scale;
	int num_portal_verts;
	int facecount=0;
	face *fp=&rp->faces[facenum];
	int nextedge,vert,nextvert;

	nextedge=(edge+1)%fp->num_verts;
	vert=fp->face_verts[edge];
	nextvert=fp->face_verts[nextedge];

	ASSERT (rp->used>0);
	
	num_portal_verts=BuildListOfPortalVerts(rp,portal_list); 

	line_center=((rp->verts[nextvert]-rp->verts[vert])/2)+rp->verts[vert];
	
	for (ok_to_scale=1,t=0;t<num_portal_verts;t++)
		if (portal_list[t]==vert)
			ok_to_scale=0;

	if (ok_to_scale)
	{
		vec=rp->verts[vert]-line_center;
		vec*=scale_factor;
		rp->verts[vert]=vec+line_center;
	}

	for (ok_to_scale=1,t=0;t<num_portal_verts;t++)
		if (portal_list[t]==nextvert)
			ok_to_scale=0;

	if (ok_to_scale)
	{
		vec=rp->verts[nextvert]-line_center;
		vec*=scale_factor;
		rp->verts[nextvert]=vec+line_center;
	}


	// Build a list of faces that share vertices with this edge
	for (i=0;i<rp->num_faces;i++)
	{
		for (t=0;t<rp->faces[i].num_verts;t++)
		{
			if (rp->faces[i].face_verts[t]==vert || rp->faces[i].face_verts[t]==nextvert)
			{
				face_list[facecount]=i;
				facecount++;
			}
		}
	}

	// Now fixup any concave/nonplanar faces
	FixConcaveFaces (rp,face_list,facecount);
	
}

void CRoomDialogBar::OnModifySetRefFrame() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();
	vector pos;

	if (prim->roomp != NULL)
	{
		pos = wnd->m_vec_RefPos;
		if (InputVector(&pos,"Set Reference Frame","Set the position of the reference frame",this))
			wnd->SetRefFrame(pos);
	}
	else
		MessageBox("No current room!");
}

void CRoomDialogBar::OnVertexRemoveExtras() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;

	if (wnd != NULL)
		wnd->RemoveExtraPoints();
}

void CRoomDialogBar::OnFaceFlip() 
{
	// TODO: Add your control notification handler code here
	// Get list of marked faces
	short marked_face_list[MAX_FACES_PER_ROOM];
	int num_m_faces;
	int i;
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();
	bool change = false;

	num_m_faces = wnd->GetMarkedFaces(marked_face_list);
	ASSERT(num_m_faces == wnd->m_Num_marked_faces);
	if (!num_m_faces)
		return;
	// Flip the normals
	for (i=0; i<num_m_faces; i++)
	{
		if (prim->roomp->faces[marked_face_list[i]].portal_num == -1)
		{
			FlipFace(prim->roomp,marked_face_list[i]);
			change = true;
		}
		else
			MessageBox("Can't flip a portal face.");
	}
	if (change)
	{
		wnd->m_Room_changed = true;
		MessageBox("Marked faces flipped.");
	}
}

void CRoomDialogBar::OnVertexSnapFace() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;
	int facenum = prim->face;
	int edgenum = prim->edge;
	int vertnum = -1;

	if (rp != NULL && facenum != -1 && edgenum != -1)
	{
		// Get list of marked verts
		short marked_list[MAX_VERTS_PER_ROOM];
		int num_m_verts = wnd->GetMarkedVerts(marked_list);
		ASSERT(num_m_verts == wnd->m_Num_marked_verts);

		if (num_m_verts > 0)
		{
			vector v0 = rp->verts[rp->faces[facenum].face_verts[edgenum]];
			vector normal = rp->faces[facenum].normal;
			SnapVertsToFace(rp,marked_list,num_m_verts,&v0,&normal);
			wnd->m_Room_changed = true;
			PrintStatus("Vertices snapped to plane of current face.");
		}
		else
			MessageBox("You must have one or more vertices marked in order to use this function.");
	}
	else
		MessageBox("No current edge!");
}

void CRoomDialogBar::OnVertexSnapEdge() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;
	int facenum = prim->face;
	int edgenum = prim->edge;
	int vertnum = -1;

	if (rp != NULL && facenum != -1 && edgenum != -1)
	{
		// Get list of marked verts
		short marked_list[MAX_VERTS_PER_ROOM];
		int num_m_verts = wnd->GetMarkedVerts(marked_list);
		ASSERT(num_m_verts == wnd->m_Num_marked_verts);

		if (num_m_verts > 0)
		{
			vector v0 = rp->verts[rp->faces[facenum].face_verts[edgenum]];
			vector v1 = rp->verts[rp->faces[facenum].face_verts[(edgenum+1)%rp->faces[facenum].num_verts]];
			SnapVertsToEdge(rp,marked_list,num_m_verts,&v0,&v1);
			wnd->m_Room_changed = true;
			PrintStatus("Vertices snapped to line of current edge.");
		}
		else
			MessageBox("You must have one or more vertices marked in order to use this function.");
	}
	else
		MessageBox("No current edge!");
}

void CRoomDialogBar::OnVertexSnapGrid() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();
	int vertnum = -1;

	room *rp = prim->roomp;

	if (rp != NULL)
	{
		// Get list of marked verts
		short marked_list[MAX_VERTS_PER_ROOM];
		int num_m_verts = wnd->GetMarkedVerts(marked_list);
		ASSERT(num_m_verts == wnd->m_Num_marked_verts);

		if (num_m_verts > 0)
		{
			SnapVertsToGrid(wnd,rp,marked_list,num_m_verts);
			wnd->m_Room_changed = true;
			PrintStatus("Vertices snapped to grid.");
		}
		else
			MessageBox("You must have one or more vertices marked in order to use this function.");
	}
	else
		MessageBox("No current room!");
}

void CRoomDialogBar::OnVertexSnapVert() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;
	int vertnum = -1;
	if (prim->face != -1)
		vertnum = rp->faces[prim->face].face_verts[prim->vert];
	else
		vertnum = wnd->m_Current_nif_vert;

	if (rp != NULL && vertnum != -1)
	{
		// Get list of marked verts
		short marked_list[MAX_VERTS_PER_ROOM];
		int num_m_verts = wnd->GetMarkedVerts(marked_list);
		ASSERT(num_m_verts == wnd->m_Num_marked_verts);

		if (num_m_verts > 0)
		{
			SnapVertsToVert(rp,marked_list,num_m_verts,vertnum);
			wnd->m_Room_changed = true;
			PrintStatus("Vertices snapped to current vert.");
		}
		else
			MessageBox("You must have one or more vertices marked in order to use this function.");
	}
	else
		MessageBox("No current vert!");
}


float CRoomDialogBar::SnapSingleVertToGrid(CRoomFrm *wnd,room *rp,short *list,int num_verts,int snap_vert)
{
	Cned_OrthoWnd *pane;
	vector *vp;
	vec2D pos;
	ASSERT (snap_vert != -1);

	switch (wnd->m_Focused_pane)
	{
	case VIEW_XY:
		pane = (Cned_OrthoWnd *)wnd->GetPane(0,0);
		break;
	case VIEW_XZ:
		pane = (Cned_OrthoWnd *)wnd->GetPane(0,1);
		break;
	case VIEW_ZY:
		pane = (Cned_OrthoWnd *)wnd->GetPane(1,0);
		break;
	default:
		Int3();
	}

	vp = &rp->verts[snap_vert];

	// Get position of snapped vert
	switch (wnd->m_Focused_pane)
	{
	case VIEW_XY:
		pos.x = vp->x;
		pos.y = vp->y;
		break;
	case VIEW_XZ:
		pos.x = vp->x;
		pos.y = vp->z;
		break;
	case VIEW_ZY:
		pos.x = vp->z;
		pos.y = vp->y;
		break;
	default:
		Int3();
	}

	pane->SnapPoint(&pos);

	// Save old vert position
	vector oldvec = *vp;

	// Snap the vert to the 2D grid
	switch (wnd->m_Focused_pane)
	{
	case VIEW_XY:
		vp->x = pos.x;
		vp->y = pos.y;
		break;
	case VIEW_XZ:
		vp->x = pos.x;
		vp->z = pos.y;
		break;
	case VIEW_ZY:
		vp->z = pos.x;
		vp->y = pos.y;
		break;
	default:
		Int3();
	}

	// Move all the verts by the snap vector
	vector snap_vec = *vp-oldvec;
	MoveVerts(rp,list,num_verts,snap_vec);

	return vm_GetMagnitudeFast(&snap_vec);
}


void CRoomDialogBar::SnapVertsToGrid(CRoomFrm *wnd,room *rp,short *list,int num_verts)
{
	Cned_OrthoWnd *pane;
	vector *vp;
	vec2D pos;

	switch (wnd->m_Focused_pane)
	{
	case VIEW_XY:
		pane = (Cned_OrthoWnd *)wnd->GetPane(0,0);
		break;
	case VIEW_XZ:
		pane = (Cned_OrthoWnd *)wnd->GetPane(0,1);
		break;
	case VIEW_ZY:
		pane = (Cned_OrthoWnd *)wnd->GetPane(1,0);
		break;
	default:
		return;
	}

	for (int i=0; i<num_verts; i++)
	{
		vp = &rp->verts[list[i]];

		// Get position of snapped vert
		switch (wnd->m_Focused_pane)
		{
		case VIEW_XY:
			pos.x = vp->x;
			pos.y = vp->y;
			break;
		case VIEW_XZ:
			pos.x = vp->x;
			pos.y = vp->z;
			break;
		case VIEW_ZY:
			pos.x = vp->z;
			pos.y = vp->y;
			break;
		default:
			return;
		}

		pane->SnapPoint(&pos);

		// Snap the vert to the 2D grid
		switch (wnd->m_Focused_pane)
		{
		case VIEW_XY:
			vp->x = pos.x;
			vp->y = pos.y;
			break;
		case VIEW_XZ:
			vp->x = pos.x;
			vp->z = pos.y;
			break;
		case VIEW_ZY:
			vp->z = pos.x;
			vp->y = pos.y;
			break;
		default:
			return;
		}
	}
}

void CRoomDialogBar::OnRoomMirror() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;

	if (rp != NULL)
	{
		if (ROOMNUM(rp) >= MAX_ROOMS)
		{
			CMirrorDialog mir_dlg;
			if ( mir_dlg.DoModal() == IDCANCEL )
				return;

			int which = mir_dlg.m_Along;

			MirrorRoom(rp,which);
			wnd->m_Room_changed = true;
			PrintStatus("Room mirrored.");
		}
		else
			MessageBox("You cannot mirror a room that's already in a level!");
	}
	else
		MessageBox("No current room!");
}

int FindConnectedFace(room *rp,int facenum,int edgenum,int startface);
void AddVertToAllEdges(room *rp,int v0,int v1,int new_v);

void CRoomDialogBar::OnFaceAddVert() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;

	if (rp != NULL && prim->face != -1 && prim->edge != -1)
	{
		// Get list of marked verts
		short marked_list[MAX_VERTS_PER_ROOM];
		int num_m_verts = wnd->GetMarkedVerts(marked_list);
		ASSERT(num_m_verts == wnd->m_Num_marked_verts);

		if (num_m_verts == 1)
		{
			int vertnum = marked_list[0];
/*	TODO: add later
			if (FindConnectedFace(rp,prim->face,prim->edge,0) != -1)
			{
				int input = MessageBox("Do you want to add the vertex to the edges of the adjacent faces as well?","Add to all adjacent faces?",MB_YESNO);
				if (input == IDYES)
				{
					int nextvert;
					(prim->edge = rp->num_verts-1) ? (nextvert = 0) : nextvert = prim->edge+1;
					AddVertToAllEdges(rp,prim->edge,nextvert,vertnum);
					wnd->m_Room_changed = true;
					return;
				}
			}
			// No connected face or user chose IDNO, so just add the vert to the current face edge
*/
			AddVertToFace(rp,prim->face,vertnum,prim->edge);
			wnd->m_Room_changed = true;
		}
		else if (num_m_verts == 0)
			OutrageMessageBox("You must have a marked vert for this operation.");
		else
			MessageBox("You must have only one vert marked for this operation.");
	}
	else
		MessageBox("No current edge!");
}

void DeleteVert(room *rp,int facenum,int vertnum);

void CRoomDialogBar::OnFaceRemoveVert() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;

	if (rp != NULL && prim->face != -1 && prim->vert != -1)
	{
		if (rp->faces[prim->face].num_verts <= 3) {
			OutrageMessageBox("You cannot remove a vertex from a face with three or fewer vertices.");
			return;
		}

		if (OutrageMessageBox(MBOX_YESNO,"Are you sure you want to remove vertex %d (%d) from face %d?",prim->vert,rp->faces[prim->face].face_verts[prim->vert],prim->face) == IDYES) {
//			DeleteVert(rp,prim->face,prim->vert);
			DeletePointFromFace(rp,prim->face,prim->vert);
			prim->edge = 0;
			prim->vert = 0;
			wnd->m_Room_changed = true;
		}
	}
	else
		MessageBox("No current face vert!");

}

/////////////////////////////////////////////////////////////////////////////
// CMirrorDialog dialog


CMirrorDialog::CMirrorDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMirrorDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMirrorDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_Along = X_AXIS;
}


void CMirrorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMirrorDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMirrorDialog, CDialog)
	//{{AFX_MSG_MAP(CMirrorDialog)
	ON_BN_CLICKED(IDC_XAXIS, OnXaxis)
	ON_BN_CLICKED(IDC_YAXIS, OnYaxis)
	ON_BN_CLICKED(IDC_ZAXIS, OnZaxis)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMirrorDialog message handlers

void CMirrorDialog::OnXaxis() 
{
	// TODO: Add your control notification handler code here
	m_Along = X_AXIS;
}

void CMirrorDialog::OnYaxis() 
{
	// TODO: Add your control notification handler code here
	m_Along = Y_AXIS;
}

void CMirrorDialog::OnZaxis() 
{
	// TODO: Add your control notification handler code here
	m_Along = Z_AXIS;
}

void CRoomDialogBar::OnFaceTwist() 
{
	// TODO: Add your control notification handler code here
 	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();
	short temp_v[3];
	int i;

	room *rp = prim->roomp;
	int facenum = prim->face;

	if (rp != NULL && facenum != -1)
	{
		int num_verts = rp->faces[facenum].num_verts;
		if (num_verts == 4)
		{
			for (i=0; i<num_verts-1; i++)
				temp_v[i] = rp->faces[facenum].face_verts[i+1];
			for (i=0; i<num_verts-1; i++)
				rp->faces[facenum].face_verts[i+1] = temp_v[(i+1)%(num_verts-1)];
			ComputeFaceNormal(rp,facenum);
			wnd->m_Room_changed = true;
		}
		else
			MessageBox("You can only twist 4-sided faces. Use the Swap Face Verts command for faces of 5 or more sides.");
	}
	else
		MessageBox("No current face!");
}

void CRoomDialogBar::OnFaceSwapVerts() 
{
	// TODO: Add your control notification handler code here
 	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;
	int facenum = prim->face;

	if (rp != NULL && facenum != -1)
	{
		// Get list of marked verts
		short marked_list[MAX_VERTS_PER_ROOM];
		int num_m_verts = wnd->GetMarkedVerts(marked_list);
		ASSERT(num_m_verts == wnd->m_Num_marked_verts);

		if (num_m_verts == 2)
		{
			// Make sure they're in the current face
			short v0 = -1, v1 = -1;
			for (int i=0; i<rp->faces[facenum].num_verts; i++)
			{
				if (rp->faces[facenum].face_verts[i] == marked_list[0])
					v0 = i;
				else
				if (rp->faces[facenum].face_verts[i] == marked_list[1])
					v1 = i;
			}

			if (v0 != -1 && v1 != -1)
			{
				// Swap the two marked verts
				short temp_v = rp->faces[facenum].face_verts[v0];
				rp->faces[facenum].face_verts[v0] = rp->faces[facenum].face_verts[v1];
				rp->faces[facenum].face_verts[v1] = temp_v;
				wnd->m_Room_changed = true;
			}
			else
				MessageBox("You must have two vertices marked in the current face in order to use this function.");
		}
		else
			MessageBox("You must have exactly two vertices marked in order to use this function.");
	}
	else
		MessageBox("No current face!");
}

void CRoomDialogBar::OnVertexSnapEdge2() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;
	int facenum = 0;
	int edgenum = 0;
	int vertnum = -1;

	if (InputNumber(&facenum,"Face Number","Enter face number",this))
		if (facenum < 0 || facenum > rp->num_faces-1)
			return;

	if (InputNumber(&edgenum,"Edge Number","Enter edge number",this))
		if (edgenum < 0 || edgenum > rp->faces[facenum].num_verts-1)
			return;

	if (rp != NULL)
	{
		// HACK: Unmark the current vert so that it doesn't go in the list
		if (prim->face != -1)
			vertnum = rp->faces[prim->face].face_verts[prim->vert];
		else
			vertnum = wnd->m_Current_nif_vert;
		if (!wnd->IsVertMarked(vertnum))
		{
			MessageBox("The current vert must be marked in order to use this function.");
			return;
		}
		wnd->UnMarkVert(vertnum);

		// Get list of marked verts
		short marked_list[MAX_VERTS_PER_ROOM];
		int num_m_verts = wnd->GetMarkedVerts(marked_list);
		ASSERT(num_m_verts == wnd->m_Num_marked_verts);

		if (num_m_verts > 0)
		{
			vector v0 = rp->verts[rp->faces[facenum].face_verts[edgenum]];
			vector v1 = rp->verts[rp->faces[facenum].face_verts[(edgenum+1)%rp->faces[facenum].num_verts]];
			if (vertnum != -1)
			{
				float dist = SnapSingleVertToEdge(rp,marked_list,num_m_verts,&v0,&v1,vertnum);
				wnd->m_Room_changed = true;
				PrintStatus("Current vertex snapped to edge line, marked verts moved %.2f units.",dist);
				// HACK: Remark the vert so the user doesn't get mad and hit their computer with a hammer
				wnd->MarkVert(vertnum);
			}
			else
				MessageBox("No current vert!");
		}
		else
			MessageBox("You must have one or more vertices marked in order to use this function.");
	}
	else
		MessageBox("No current room!");
}

void CRoomDialogBar::OnVertexSnapFace2() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;
	int facenum = 0;
	int edgenum = 0;
	int vertnum = -1;

	if (InputNumber(&facenum,"Face Number","Enter face number",this))
		if (facenum < 0 || facenum > rp->num_faces-1)
			return;

	if (InputNumber(&edgenum,"Edge Number","Enter edge number",this))
		if (edgenum < 0 || edgenum > rp->faces[facenum].num_verts-1)
			return;

	if (rp != NULL)
	{
		// HACK: Unmark the current vert so that it doesn't go in the list
		if (prim->face != -1)
			vertnum = rp->faces[prim->face].face_verts[prim->vert];
		else
			vertnum = wnd->m_Current_nif_vert;
		if (!wnd->IsVertMarked(vertnum))
		{
			MessageBox("The current vert must be marked in order to use this function.");
			return;
		}
		wnd->UnMarkVert(vertnum);

		// Get list of marked verts
		short marked_list[MAX_VERTS_PER_ROOM];
		int num_m_verts = wnd->GetMarkedVerts(marked_list);
		ASSERT(num_m_verts == wnd->m_Num_marked_verts);

		if (num_m_verts > 0)
		{
			vector v0 = rp->verts[rp->faces[facenum].face_verts[edgenum]];
			vector normal = rp->faces[facenum].normal;
			if (vertnum != -1)
			{
				float dist = SnapSingleVertToFace(rp,marked_list,num_m_verts,&v0,&normal,vertnum);
				wnd->m_Room_changed = true;
				PrintStatus("Current vertex snapped to face plane, marked verts moved %.2f units.",dist);
				// HACK: Remark the vert so the user doesn't get mad and hit their computer with a hammer
				wnd->MarkVert(vertnum);
			}
			else
				MessageBox("No current vert!");
		}
		else
			MessageBox("You must have one or more vertices marked in order to use this function.");
	}
	else
		MessageBox("No current room!");
}

void CRoomDialogBar::OnVertexSnapGrid2() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();
	int vertnum = -1;

	room *rp = prim->roomp;

	if (rp != NULL)
	{
		// HACK: Unmark the current vert so that it doesn't go in the list
		if (prim->face != -1)
			vertnum = rp->faces[prim->face].face_verts[prim->vert];
		else
			vertnum = wnd->m_Current_nif_vert;
		if (!wnd->IsVertMarked(vertnum))
		{
			MessageBox("The current vert must be marked in order to use this function.");
			return;
		}
		wnd->UnMarkVert(vertnum);

		// Get list of marked verts
		short marked_list[MAX_VERTS_PER_ROOM];
		int num_m_verts = wnd->GetMarkedVerts(marked_list);
		ASSERT(num_m_verts == wnd->m_Num_marked_verts);

		if (num_m_verts > 0)
		{
			if (vertnum != -1)
			{
				float dist = SnapSingleVertToGrid(wnd,rp,marked_list,num_m_verts,vertnum);
				wnd->m_Room_changed = true;
				PrintStatus("Current vertex snapped to grid, marked verts moved %.2f units.",dist);
				// HACK: Remark the vert so the user doesn't get mad and hit their computer with a hammer
				wnd->MarkVert(vertnum);
			}
			else
				MessageBox("No current vert!");
		}
		else
			MessageBox("You must have one or more vertices marked in order to use this function.");
	}
	else
		MessageBox("No current room!");
}

void CRoomDialogBar::OnVertexSnapVert2() 
{
	// TODO: Add your control notification handler code here
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();

	room *rp = prim->roomp;
	int vertnum = -1;
	int snaptovert = 0;

	if (InputNumber(&snaptovert,"Vert Number","Enter vert number",this))
		if (snaptovert < 0 || snaptovert > rp->num_verts-1)
			return;

	// HACK: Unmark the current vert so that it doesn't go in the list
	if (prim->face != -1)
		vertnum = rp->faces[prim->face].face_verts[prim->vert];
	else
		vertnum = wnd->m_Current_nif_vert;
	if (!wnd->IsVertMarked(vertnum))
	{
		MessageBox("The current vert must be marked in order to use this function.");
		return;
	}
	wnd->UnMarkVert(vertnum);

	if (rp != NULL && vertnum != -1)
	{
		// Get list of marked verts
		short marked_list[MAX_VERTS_PER_ROOM];
		int num_m_verts = wnd->GetMarkedVerts(marked_list);
		ASSERT(num_m_verts == wnd->m_Num_marked_verts);

		if (num_m_verts > 0)
		{
			float dist = SnapSingleVertToVert(rp,marked_list,num_m_verts,snaptovert,vertnum);
			wnd->m_Room_changed = true;
			PrintStatus("Current vertex snapped to vert, marked verts moved %.2f units.",dist);
		}
		else
			MessageBox("You must have one or more vertices marked in order to use this function.");
	}
	else
		MessageBox("No current vert!");
}
