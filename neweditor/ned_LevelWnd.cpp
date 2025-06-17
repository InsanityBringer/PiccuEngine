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

#include "ddaccess.h"
#include "renderer.h"
#include "ned_Rend.h"
#include "room_external.h"
#include "gr.h"
#include "selectedroom.h"
#include "SelManager.h"
#include "globals.h"
#include "MainFrm.h"
#include "ned_DrawWorld.h"
#include "ned_LevelWnd.h"
#include "ned_HFile.h"
#include "ned_Geometry.h"
#include "neweditor.h"

#include "render.h"
#include "manage.h"
#include "boa.h"
#include "gameloop.h"
#include "descent.h"
#include "terrain.h"
#include "HTexture.h"
#include "room.h"
#include "roomuvs.h"
#include <string.h>
#include "ObjMoveManager.h"
#include "lightmap_info.h"
#include "lightmap.h"
#include "erooms.h"
#include "hroom.h"
#include "ebnode.h"

#include <assert.h>

#include "mono.h"
#include "3d.h"
#include "ddio.h"
#include "epath.h"
#include "radiosity.h"
#include "hemicube.h"
#include "args.h"
#include "game.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int FindPointRoom(vector *pnt);

// Movement controls
#define PANNING  ( (m_Mouse.left && m_Keys.shift && !m_Keys.ctrl) || (m_Mouse.mid && !m_Keys.ctrl && !m_Keys.shift && !m_Keys.alt) )
#define ZOOMING	 ( (m_Mouse.left && m_Keys.shift && m_Keys.ctrl)  || (m_Mouse.mid && !m_Keys.ctrl && m_Keys.shift && !m_Keys.alt) )
#define ROTATING ( (m_Mouse.left && !m_Keys.shift && m_Keys.ctrl) || (m_Mouse.mid && !m_Keys.shift && m_Keys.ctrl && !m_Keys.alt) )
#define RADDING  ( (m_Mouse.left && m_Keys.alt && m_Keys.shift)   || (m_Mouse.mid && m_Keys.alt && !m_Keys.ctrl && !m_Keys.shift) )

object *Viewer_object = &Objects[0];		//which object we are seeing from
static bool bDoOpenGL = false;
static first_time=1;
HWND save_wnd;
oeWin32Application *app;

extern bool Dont_draw_dots;
extern bool Dont_draw_terrain;

/////////////////////////////////////////////////////////////////////////////
// Cned_LevelWnd

IMPLEMENT_DYNCREATE(Cned_LevelWnd, CWnd)

Cned_LevelWnd::Cned_LevelWnd()
{
	m_nID = 0;
	m_Title = "";
	m_InFocus = false;
	m_View_changed = false;
	m_bTextured = false;
	m_bOutline = false;
	m_bCaptured = false;
	memset(&m_Prim,0,sizeof(m_Prim));
	m_Prim.roomp = NULL;
	m_Prim.portal = -1;
	memset(&m_Mouse,0,sizeof(m_Mouse));
	memset(&m_Keys,0,sizeof(m_Keys));
	m_StartFlip = FALSE;
	m_last_start_room = -1;	
	m_bAutoCenter = true;
	m_bSmoothCenter = true;
	m_Modified = false;
	m_bFlat = false;
	m_BDOWN_Found_type = -1;
	m_BDOWN_Found_roomnum = -1;
	m_BDOWN_Found_facenum = -1;
	m_BDOWN_Found_x = -1;
	m_BDOWN_Found_y = -1;

/*
	vector pos = {0, 0, 0};
	matrix orient = IDENTITY_MATRIX;
	Viewer_object->pos = pos;
	Viewer_object->orient = orient;
*/
}

Cned_LevelWnd::~Cned_LevelWnd()
{
}


BEGIN_MESSAGE_MAP(Cned_LevelWnd, Cned_GrWnd)
	//{{AFX_MSG_MAP(Cned_LevelWnd)
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_KEYUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_SYSCHAR()
	ON_WM_TIMER()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SYSKEYDOWN()
	ON_WM_SYSKEYUP()
	ON_WM_PAINT()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_CENTER_MINE, OnCenterMine)
	ON_COMMAND(ID_CENTER_ROOM, OnCenterRoom)
	ON_COMMAND(ID_TEXTURED, OnTextured)
	ON_COMMAND(ID_WIREFRAME, OnWireframe)
	ON_COMMAND(ID_CENTER_ORIGIN, OnCenterOrigin)
	ON_COMMAND(ID_DISPLAY_CURRENTROOMVIEW, OnDisplayCurrentRoomView)
	ON_COMMAND(ID_VIEW_MOVECAMERATOCURRENTFACE, OnViewMoveCameraToCurrentFace)
	ON_COMMAND(ID_VIEW_MOVECAMERATOCURRENTOBJECT, OnViewMoveCameraToCurrentObject)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MOVECAMERATOCURRENTFACE, OnUpdateViewMoveCameraToCurrentFace)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MOVECAMERATOCURRENTOBJECT, OnUpdateViewMoveCameraToCurrentObject)
	ON_UPDATE_COMMAND_UI(ID_TEXTURED, OnUpdateTextured)
	ON_UPDATE_COMMAND_UI(ID_WIREFRAME, OnUpdateWireframe)
	ON_COMMAND(ID_VIEW_TEXTURED_OUTLINE, OnViewTexturedOutline)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTURED_OUTLINE, OnUpdateViewTexturedOutline)
	ON_COMMAND(ID_VIEW_MOVECAMERATOCURRENTROOM, OnViewMoveCameraToCurrentRoom)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void Cned_LevelWnd::TexGrStartOpenGL()
{
	if (bDoOpenGL)
	{
		app=(oeWin32Application *)g_OuroeApp;
		StateLimited=1;
		save_wnd=(HWND)app->m_hWnd;
		app->m_hWnd=(HWnd)m_hWnd;
		rend_SetOpenGLWindowState (1,g_OuroeApp,NULL);
		rend_ClearScreen(0);
		StateLimited=1;
		UseMultitexture=0;
		NoLightmaps=false;
	}
}

void Cned_LevelWnd::TexGrStopOpenGL ()
{
	if (bDoOpenGL)
	{
		rend_SetOpenGLWindowState (0,g_OuroeApp,NULL);
		app->m_hWnd=(HWnd)save_wnd;
	}
}


void Cned_LevelWnd::Render(bool force_repaint)
{
	int flags = 0;

	FrameCount++;

	//	Don't do a thing if these objects aren't created yet or the window isn't initialized for rendering
	if (!m_grScreen || !m_grViewport || !m_bInitted)
		return;

	if (first_time)
	{
		first_time=0;
		if (FindArg("-WindowGL"))
			bDoOpenGL=true;
	}

	vector view_pos = m_Cam.target - (m_Cam.orient.fvec * m_Cam.dist);

	if (m_bTextured) {
		TexGrStartOpenGL();
		TGWRenderMine(&view_pos,&m_Cam.orient,D3_DEFAULT_ZOOM,ROOMNUM(m_Prim.roomp)); // D3EditState.current_room);
//		TGWRenderMine(&Viewer_object->pos,&Viewer_object->orient,D3_DEFAULT_ZOOM,ROOMNUM(m_Prim.roomp)); // D3EditState.current_room);
		if (!Dont_draw_terrain)
			RenderTerrain(0);
		DrawAllPaths(m_grViewport,&Viewer_object->pos,&Viewer_object->orient,D3_DEFAULT_ZOOM);
		EBNode_Draw(EBDRAW_LEVEL,m_grViewport,&Viewer_object->pos,&Viewer_object->orient,D3_DEFAULT_ZOOM); // EBN_draw_type
		TexGrStopOpenGL();
	} else {
		flags |= DRAW_ALL | DRAW_CUR;
		DrawWorld(m_grViewport,&m_Cam.target,&m_Cam.orient,m_Cam.dist,0,m_Cam.rad,flags,&m_Prim);
	}

	m_StartFlip = TRUE;
//	m_State_changed = true;
	if (force_repaint)
		InvalidateRect(NULL, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// Cned_LevelWnd message handlers

int Cned_LevelWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (Cned_GrWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	theApp.m_pLevelWnd = this;
	g_OuroeApp->m_hWnd = (HWnd)m_hWnd; // why is this done?

	if (m_Title = "World View")
		m_Cam = Level_cam;
	m_Cam.rad = DEFAULT_VIEW_RAD;
	m_Cam.view_changed = true;

	m_TimerHandle = SetTimer(gTimerID++,10,NULL);

	// Reinitialize OpenGL
	if (FindArg("-WindowGL"))
	{
		PrintStatus("Reinitting OpenGL");
		rend_Close();
		rend_Init (RENDERER_SOFTWARE_16BIT,g_OuroeApp,NULL);
	}

	return 0;
}

BOOL Cned_LevelWnd::Create(const RECT& rect, CWnd* pParentWnd, LPCTSTR name, UINT nID) 
{
	// TODO: Add your specialized code here and/or call the base class
	DWORD dwStyle = GRWND_STATIC_STYLE;

	m_Title = "World View";

	return CWnd::Create(NULL, name, dwStyle, rect, pParentWnd, nID);
}


void Cned_LevelWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.x = point.x;
	m_Mouse.y = point.y;

	Cned_GrWnd::OnMouseMove(nFlags, point);
}


void EndSel(editorSelectorManager *esm);
extern CTerrainDialogBar *dlgTerrainDialogBar;


void Cned_LevelWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.left = true;

	if (!m_InFocus)
	{
		SetFocus();
		return;
	}

	m_BDOWN_Found_type = -1;
	m_BDOWN_Found_roomnum=-1;
	m_BDOWN_Found_facenum=-1;
	m_BDOWN_Found_x = -1;
	m_BDOWN_Found_y = -1;

	// Different routines depending on whether textures are up
	if (!m_bTextured)
	{
		int found_segnum,found_facenum;
		int find_mode = FM_CLOSEST;

		//See if we're clicking on the current face
		found_segnum = ROOMNUM(m_Prim.roomp);
		found_facenum = m_Prim.face;
	if (Placed_room == -1)
	{
		// WireframeFindRoomFace requires that the viewport its checking to be the last one that was rendered, so call Render() first.
		Render(false);
		if (WireframeFindRoomFace(m_grViewport,point.x,point.y,&found_segnum,&found_facenum,FM_SPECIFIC,false)) {
			find_mode = FM_NEXT;
		}

find_again:;

		if (WireframeFindRoomFace(m_grViewport,point.x,point.y,&found_segnum,&found_facenum,find_mode,false)) {
			ASSERT(found_segnum!=-1);
			ASSERT(found_facenum!=-1);
			room *rp = &Rooms[found_segnum];
			SetPrim(rp,found_facenum,rp->faces[found_facenum].portal_num,0,0);
			// Update the current face/texture displays
			Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(rp), m_Prim.face);
			if (theApp.m_pRoomFrm != NULL)
				theApp.m_pRoomFrm->CenterRoom();
		}
		else if (find_mode == FM_NEXT) {
			find_mode = FM_CLOSEST;
			goto find_again;
		}
	}

		//	no more dragging, but something is highlighted.   
		//	so we will redraw world if mouse is pressed

		SelManager.StartSelection(this, EndSel, point.x, point.y);
	}
	else // Textures
	{
		CRect rc(m_grViewport->left(),m_grViewport->top(),m_grViewport->right(),m_grViewport->bottom());
		// Make sure the mouse cursor is not outside of the viewport!!!!
		if (rc.PtInRect(point))
		{

		TexGrStartOpenGL();

//		ResetPostrenderList();

		//	mprintf((0,"ButtonDown %d,%d\n",point.x,point.y));
		TSearch_found_type=-1;
		TSearch_seg=-1;
		TSearch_on=1;
		TSearch_x=point.x;
		TSearch_y=point.y;

		vector view_pos = m_Cam.target - (m_Cam.orient.fvec * m_Cam.dist);

		TGWRenderMine(&view_pos,&m_Cam.orient,D3_DEFAULT_ZOOM,ROOMNUM(m_Prim.roomp)); // D3EditState.current_room);
//		TGWRenderMine (&Viewer_object->pos,&Viewer_object->orient,D3_DEFAULT_ZOOM,Viewer_object->roomnum);
		if (!Dont_draw_terrain)
			RenderTerrain(0);
		DrawAllPaths(m_grViewport,&Viewer_object->pos,&Viewer_object->orient,D3_DEFAULT_ZOOM);
		EBNode_Draw(EBDRAW_LEVEL,m_grViewport,&Viewer_object->pos,&Viewer_object->orient,D3_DEFAULT_ZOOM); // EBN_draw_type
		TSearch_on=0;

		// Save what we found, for the button up handler
		m_BDOWN_Found_type = TSearch_found_type;
		m_BDOWN_Found_roomnum=TSearch_seg;
		m_BDOWN_Found_facenum=TSearch_face;
		m_BDOWN_Found_x = TSearch_x;
		m_BDOWN_Found_y = TSearch_y;

		//this looks at the last frame rendered
		if  (TSearch_found_type==TSEARCH_FOUND_OBJECT) 
		{	
			//Found a face or object
			m_StartFlip = FALSE;			// unsignal flip.
			SelectObject(TSearch_seg);
			// Update the current object display
			Editor_state.SetCurrentLevelObject(Cur_object_index);
			// start object moving if mouse down in an object.
//			ObjMoveManager.Start(this, m_grViewport->width(), m_grViewport->height(), &m_ViewPos, &m_ViewMatrix, point.x, point.y);							
		}
		else
		if (TSearch_found_type==TSEARCH_FOUND_NODE)
		{
			// Found a gamepath node!
			int path = TSearch_seg;
			int node = TSearch_face;
			Editor_state.SetCurrentPath(path);
			Editor_state.SetCurrentNode(node);

			if (path != -1)
				PrintStatus("Path %d, Node %d selected. Path Name = %s",path,node+1,GamePaths[path].name);
			State_changed = true;
		}
		else
		if (TSearch_found_type==TSEARCH_FOUND_BNODE)
		{
			// Found a Bnode!
			int room = TSearch_seg;
			int node = TSearch_face;
			Editor_state.SetCurrentBNode(room,node,0);

			PrintStatus("BNode %d in room %d selected.",node+1,room);
			State_changed = true;
		}
		else
		if (TSearch_found_type==TSEARCH_FOUND_SKY_DOME)
		{
			if (nFlags & MK_SHIFT)
			{	
				LevelTexDecrementTexture(Terrain_sky.dome_texture);
				Terrain_sky.dome_texture=Editor_state.GetCurrentTexture();
				LevelTexIncrementTexture(Terrain_sky.dome_texture);
				PrintStatus("Texture %d applied to sky dome\n",Editor_state.GetCurrentTexture());

				World_changed=1;
			}
		}
		else
		if (TSearch_found_type==TSEARCH_FOUND_SATELLITE)
		{
			if (nFlags & MK_SHIFT)
			{	
				LevelTexDecrementTexture(Terrain_sky.satellite_texture[TSearch_seg]);
				Terrain_sky.satellite_texture[TSearch_seg]=Editor_state.GetCurrentTexture();			
				LevelTexIncrementTexture(Terrain_sky.satellite_texture[TSearch_seg]);
				dlgTerrainDialogBar->SetCurrentSat(TSearch_seg);

				World_changed=1;
			}
			else
				dlgTerrainDialogBar->SetCurrentSat(TSearch_seg);
		}

		TexGrStopOpenGL();

		}
	}

	Cned_GrWnd::OnLButtonDown(nFlags, point);
}

extern int Current_cell;

void Cned_LevelWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.left = false;

	// Different routines depending on whether textures are up
	if (!m_bTextured)
	{
	}
	else
	{
		int roomnum,facenum,type;

		CRect rc(m_grViewport->left(),m_grViewport->top(),m_grViewport->right(),m_grViewport->bottom());
		// Make sure the mouse cursor is not outside of the viewport!!!!
		if (rc.PtInRect(point))
		{
			//	mprintf((0,"ButtonDown %d,%d\n",point.x,point.y));
			TSearch_found_type=-1;
			TSearch_seg=-1;
			TSearch_on=1;
			TSearch_x=point.x;
			TSearch_y=point.y;

			vector view_pos = m_Cam.target - (m_Cam.orient.fvec * m_Cam.dist);

			TexGrStartOpenGL();
			TGWRenderMine(&view_pos,&m_Cam.orient,D3_DEFAULT_ZOOM,ROOMNUM(m_Prim.roomp)); // D3EditState.current_room);
			if (!Dont_draw_terrain)
				RenderTerrain(0);
			TexGrStopOpenGL();

			TSearch_on=0;

			if (TSearch_found_type==TSEARCH_FOUND_MINE)
			{
				type = TSearch_found_type;
				roomnum=TSearch_seg;
				facenum=TSearch_face;

				// Now, if what we found was the same as what we found in lbuttondown, do the various actions
				// (This is done so that ctrl+click and shift+click only move the view when that is what is desired)
				if (m_BDOWN_Found_x != -1 && m_BDOWN_Found_y != -1 && 
					type == m_BDOWN_Found_type && 
					roomnum == m_BDOWN_Found_roomnum && facenum == m_BDOWN_Found_facenum)
				{

				if ((nFlags & MK_SHIFT) && !(nFlags & MK_CONTROL)) 
				{	//apply current texture to face
					HTextureApplyToRoomFace(&Rooms[TSearch_seg], TSearch_face, Editor_state.GetCurrentTexture());
					PrintStatus("Texture %d applied to room %d face %d\n",Editor_state.GetCurrentTexture(),TSearch_seg,TSearch_face);
					World_changed=1;
					State_changed = true;
					// Update the current face/texture displays
					Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(&Rooms[TSearch_seg]), TSearch_face);
				}
				else if ((nFlags & MK_CONTROL) && !(nFlags & MK_SHIFT)) 
				{	
					// Propagate texture alignment from current face
					if ((TSearch_seg == ROOMNUM(m_Prim.roomp)) && (TSearch_face == m_Prim.face))
						PrintStatus("Cannot propagate from a face to itself.");
					else if (HTexturePropagateToFace(&Rooms[roomnum],facenum,m_Prim.roomp,m_Prim.face,false)) 
					{
						PrintStatus("Texture propagated from room %d face %d to room %d face %d\n",ROOMNUM(m_Prim.roomp),m_Prim.face,roomnum,facenum);
						World_changed = 1;
					}
					else 
					{
						OutrageMessageBox("Room %d face %d is not adjacent to current room and face)",roomnum,facenum);
						return;
					}
				}
				else if ((nFlags & MK_CONTROL) && (nFlags & MK_SHIFT)) 
				{
					// Copy uv values from marked face
					static int last_copy_marked_room=-1,last_copy_marked_face=-1;
					static int last_copy_room=-1,last_copy_face=-1;
					static count;

					if (Markedroomp == NULL) 
					{
						OutrageMessageBox("You must have a marked face for this operation.");
						return;
					}

					if ((roomnum == ROOMNUM(Markedroomp)) && (facenum == Markedface))
						PrintStatus("Cannot copy from a face to itself.");
					else 
					{
						if ((last_copy_marked_room == ROOMNUM(Markedroomp)) && (last_copy_marked_face == Markedface) && (last_copy_room == roomnum) && (last_copy_face == facenum))
							 count++;
						else 
						{
							count=0;
							last_copy_marked_room = ROOMNUM(Markedroomp);
							last_copy_marked_face = Markedface;
							last_copy_room = roomnum;
							last_copy_face = facenum;
						}

						if (HTextureCopyUVsToFace(&Rooms[roomnum],facenum,Markedroomp,Markedface,count)) 
						{
							Rooms[roomnum].faces[facenum].tmap = Markedroomp->faces[Markedface].tmap;
							PrintStatus("Texture & UVs copied from room %d face %d to room %d face %d (rotation = %d/%d)\n",ROOMNUM(Markedroomp),Markedface,roomnum,facenum,count%Markedroomp->faces[Markedface].num_verts,Markedroomp->faces[Markedface].num_verts);
							World_changed = 1;
						}
						else
							PrintStatus("Can't copy UVs: faces don't have the same number of vertices.");
					}
				}
	/*
				else if (KEY_STATE(KEY_N))
				{
					vector vec=Rooms[roomnum].verts[Rooms[roomnum].faces[facenum].face_verts[0]];
					vector norm=Rooms[roomnum].faces[facenum].normal;
					float plane_dist=-(vec.x*norm.x+vec.y*norm.y+vec.z*norm.z);
					int i;
					for (i=0;i<Rooms[roomnum].faces[facenum].num_verts;i++)
					{
						vec=Rooms[roomnum].verts[Rooms[roomnum].faces[facenum].face_verts[i]];
						float dist = vec.x*norm.x+vec.y*norm.y+vec.z*norm.z+plane_dist;
						mprintf ((0,"Vertex %d distance from plane=%f\n",i,dist));
					}
				}
	*/
				else 
				{	//Just change curface
					State_changed = 1;
					// Update the current face/texture displays
					Editor_state.SetCurrentRoomFaceTexture(roomnum, facenum);
				}

				//All clicks on face change the current face
				SetPrim(&Rooms[roomnum],facenum,-1,0,0);
				// If this room is a door room, set the current doorway
				Editor_state.SetCurrentDoorway(m_Prim.roomp);

				}
			}
			else
			if (TSearch_found_type==TSEARCH_FOUND_TERRAIN)
			{
				if (nFlags & MK_SHIFT) 
				{
					LevelTexDecrementTexture(Terrain_tex_seg[Terrain_seg[TSearch_seg].texseg_index].tex_index);
					Terrain_tex_seg[Terrain_seg[TSearch_seg].texseg_index].tex_index=Editor_state.GetCurrentTexture();
					LevelTexIncrementTexture(Terrain_tex_seg[Terrain_seg[TSearch_seg].texseg_index].tex_index);
					World_changed = 1;
				}
				else
				{
					memset (TerrainSelected,0,TERRAIN_WIDTH*TERRAIN_DEPTH);
					EditorStatus("Cell %d selected. Light=%d Height=%.2f (%d).  Texture=%s",TSearch_seg,Terrain_seg[TSearch_seg].l,Terrain_seg[TSearch_seg].y,Terrain_seg[TSearch_seg].ypos,GameTextures[Terrain_tex_seg[Terrain_seg[TSearch_seg].texseg_index].tex_index].name);
					TerrainSelected[TSearch_seg]++;
					Current_cell = TSearch_seg;
					Num_terrain_selected=1;
					State_changed = 1;
				}
			}
		}
	}

	Cned_GrWnd::OnLButtonUp(nFlags, point);
}

void Cned_LevelWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.right = true;

	Cned_GrWnd::OnRButtonDown(nFlags, point);
}

void Cned_LevelWnd::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.right = false;

	Cned_GrWnd::OnRButtonUp(nFlags, point);
}

void Cned_LevelWnd::OnMButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.mid = true;

	Cned_GrWnd::OnMButtonDown(nFlags, point);
}

void Cned_LevelWnd::OnMButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.mid = false;

	Cned_GrWnd::OnMButtonUp(nFlags, point);
}

void Cned_LevelWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar)
	{
	case VK_CONTROL:
		m_Keys.ctrl = false;
		break;

	case VK_SHIFT:
		m_Keys.shift = false;
		break;
	}
	
	Cned_GrWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

BOOL Cned_LevelWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	m_Cam.dist += -((zDelta/WHEEL_DELTA) * ZOOM_SCALE);
//	if (m_Cam.dist < 0)
//		m_Cam.dist = 0;
	m_Cam.view_changed = 1;
	
	return true;//Cned_GrWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void DoPathPointCheck()
{
	char message[10000];
	message[0] = '\0';

	BOA_ComputePathPoints(message, 10000);
	OutrageMessageBox("Done computing bad center points. Check the system clipboard for the list.");
	DumpTextToClipboard(message);
}

void ned_DeleteRoom();

void Cned_LevelWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	static int vert_idx = 0;
	room *rp = m_Prim.roomp;
	int facenum,portnum;
	int not_generic_count = 0;
	int i=0,j=0;
	int next;
	int path,node;
	float x,y,z;

	switch (nChar)
	{
	case VK_CONTROL:
		m_Keys.ctrl = true;
		break;

	case VK_SHIFT:
		m_Keys.shift = true;
		break;

	case VK_SPACE:
		int newstate;
		newstate = ToggleRoomSelectedState(ROOMNUM(rp));
		State_changed = 1;
		PrintStatus("Room %d %s",ROOMNUM(rp),newstate?"selected":"deselected");
		break;

	case VK_NUMPAD2:
		AdjustCamera(ROT_DOWN);
		break;

	case VK_UP:
		if ( HIWORD(::GetKeyState(VK_CONTROL)) )
		{
			// Go through portal to next room
			if (m_Prim.portal != -1)
			{
				rp = &Rooms[rp->portals[m_Prim.portal].croom];
				// Pick a portal in this room
				ASSERT(rp->num_portals);
				SetPrim(rp,rp->portals[0].portal_face,0,0,0);
				// Update the current face/texture displays
				Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(rp), m_Prim.face);
				// If this room is a door room, set the current doorway
				Editor_state.SetCurrentDoorway(rp);
				if (m_bAutoCenter)
					CenterRoom(rp);
			}
		}
		else
			AdjustCamera(PAN_UP);
		break;

	case VK_NUMPAD8:
		AdjustCamera(ROT_UP);
		break;

	case VK_DOWN:
		AdjustCamera(PAN_DOWN);
		break;

	case VK_NUMPAD1:
		AdjustCamera(BANK_LEFT);
		break;

	case VK_NUMPAD3:
		AdjustCamera(BANK_RIGHT);
		break;

	case VK_INSERT:
		// TODO : insert objects in 3D view?
		if (m_Prim.face != -1)
			AddRoom();
		else
			OutrageMessageBox("You must have a face selected!");
		break;

	case VK_DELETE:
		// TODO : deletion of objects, rooms, and/or faces?
		ned_DeleteRoom();
		break;

	case VK_HOME:
		CenterMine();
		break;

	case 0x43:				// 'C'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			OnViewMoveCameraToCurrentFace();
		else
			CenterRoom(rp);
		break;

	case 0x47:				// 'G'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			OnViewMoveCameraToCurrentObject();
		break;

	case 0x57:				// 'W'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			AdjustCamera(PAN_UP);
		else
			AdjustCamera(ZOOM_INWARD);
		break;

	case 0x53:				// 'S'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			AdjustCamera(PAN_DOWN);
		else
			AdjustCamera(ZOOM_OUTWARD);
		break;

	case VK_NUMPAD4:
		AdjustCamera(ROT_LEFT);
		break;

	case VK_LEFT:
	case 0x41:			// 'A'
		AdjustCamera(PAN_LEFT);
		break;

	case VK_NUMPAD6:
		AdjustCamera(ROT_RIGHT);
		break;

	case VK_RIGHT:
	case 0x44:			// 'D'
		AdjustCamera(PAN_RIGHT);
		break;

	case 0x54:				// 'T'
		if ( HIWORD(::GetKeyState(VK_CONTROL)) )
		{
			Dont_draw_dots = !Dont_draw_dots;
			Dont_draw_terrain = Dont_draw_dots;
		}
		else
			m_bTextured = !m_bTextured;
		State_changed = true;
		Render();
		break;

	case 0x4C:				// 'L'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
		{
			m_bFlat = !m_bFlat;
			State_changed = true;
			Render();
		}
		else
		{
			if (m_bTextured)
			{
				m_bOutline = !m_bOutline;
				State_changed = true;
				Render();
			}
		}
		break;

	case 0x48:				// 'H'
		if (Num_game_paths<1)
			break;
		path = Editor_state.GetCurrentPath();
		node = 0;
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			path = GetPrevPath(path);
		else
			path = GetNextPath(path);
		Editor_state.SetCurrentPath(path);
		Editor_state.SetCurrentNode(0);

		if (path != -1)
			PrintStatus("Path %d, Node %d selected. Path Name = %s",path,node+1,GamePaths[path].name);
		State_changed = true;
		break;

	case 0x4E:				// 'N'
		if (Num_game_paths<1)
			break;
		path = Editor_state.GetCurrentPath();
		node = Editor_state.GetCurrentNode();
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
		{
			node--;
			if (node<0)
				node=GamePaths[path].num_nodes-1;
		}
		else
		{
			node++;
			node%=GamePaths[path].num_nodes;
		}
		Editor_state.SetCurrentNode(node);

		if (path != -1)
			PrintStatus("Path %d, Node %d selected. Path Name = %s",path,node+1,GamePaths[path].name);
		State_changed = true;
		break;

	case 0x4D:				// 'M'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			OnViewMoveCameraToCurrentRoom();
		else
		{
			if (rp == Markedroomp)
			{
				int roomnum = ROOMNUM(Markedroomp);
				int face = Markedface;
				int edge = Markededge;
				int vert = Markedvert;
				Markedroomp = NULL;
				Markedface = -1;
				Markededge = -1;
				Markedvert = -1;
				PrintStatus("Room:face:edge:vertex %d:%d:%d:%d unmarked.",roomnum,face,edge,vert);
				State_changed = 1;
			}
			else if (m_Prim.face != -1)
			{
				Markedroomp = rp;
				Markedface = m_Prim.face;
				Markededge = m_Prim.edge;
				Markedvert = m_Prim.vert;
				PrintStatus("Room:face:edge:vertex %d:%d:%d:%d marked.",ROOMNUM(Markedroomp),Markedface,Markededge,Markedvert);
				State_changed = 1;
			}
		}

		break;

	case 0x45:				// 'E'
		if (m_Prim.face == -1 || m_Prim.edge == -1)
			break;
		m_Prim.vert = 0;
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			(m_Prim.edge <= 0) ? (m_Prim.edge = rp->faces[m_Prim.face].num_verts-1) : (m_Prim.edge--);
		else
			(m_Prim.edge == rp->faces[m_Prim.face].num_verts-1) ? (m_Prim.edge = 0) : (m_Prim.edge++);
		State_changed = true;
		break;

	case 0x46:				// 'F'
		facenum = m_Prim.face;
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			(facenum <= 0) ? (facenum = rp->num_faces-1) : (facenum--);
		else
			(facenum == rp->num_faces-1) ? (facenum = 0) : (facenum++);
		SetPrim(rp,facenum,rp->faces[facenum].portal_num,0,0);
		// Update the current face/texture displays
		Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(rp), m_Prim.face);
		break;

	case 0x50:				// 'P'
		if (rp->num_portals > 0)
		{
			portnum = m_Prim.portal;
			if ( HIWORD(::GetKeyState(VK_SHIFT)) )
				(portnum <= 0) ? (portnum = rp->num_portals-1) : (portnum--);
			else
				(portnum == rp->num_portals-1) ? (portnum = 0) : (portnum++);
			SetPrim(rp,rp->portals[portnum].portal_face,portnum,0,0);
			// Update the current face/texture displays
			Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(rp), m_Prim.face);
		}
		else
			PrintStatus("There are no portals in the current room");
		break;

	case 0x52:				// 'R'
try_again:
		// Force unmark everything
		if (theApp.m_pRoomFrm != NULL)
			theApp.m_pRoomFrm->ForceUnmarkAll();

		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			(ROOMNUM(rp) == 0) ? (rp = &Rooms[Highest_room_index]) : (rp = --m_Prim.roomp);
		else
			(ROOMNUM(rp) == Highest_room_index) ? (rp = &Rooms[0]) : (rp = ++m_Prim.roomp);
		if (!rp->used)
			goto try_again;
		SetPrim(rp,0,rp->faces[0].portal_num,0,0);
		// Update the current face/texture displays
		Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(rp), m_Prim.face);
		// If this room is a door room, set the current doorway
		Editor_state.SetCurrentDoorway(rp);
		if (m_bAutoCenter)
			CenterRoom(rp);
		if (theApp.m_pRoomFrm != NULL)
			theApp.m_pRoomFrm->CenterRoom();
		break;

	case 0x56:				// 'V'
		if (m_Prim.face == -1)
		{
			OutrageMessageBox("You must have a current face to use this function.");
			break;
		}
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			(m_Prim.vert <= 0) ? (m_Prim.vert = rp->faces[m_Prim.face].num_verts-1) : (m_Prim.vert--);
		else
			(m_Prim.vert == rp->faces[m_Prim.face].num_verts-1) ? (m_Prim.vert = 0) : (m_Prim.vert++);
		State_changed = true;
		vert_idx = rp->faces[m_Prim.face].face_verts[m_Prim.vert];
		x = rp->verts[vert_idx].x; y = rp->verts[vert_idx].y; z = rp->verts[vert_idx].z;
		PrintStatus("Face %d vert %d selected. Position: %.2f, %.2f, %.2f",m_Prim.face,m_Prim.vert,x,y,z);
		break;

	case 0x4F:				// 'O'
		next = Cur_object_index;
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
		{
prev_object:
			(next == 0) ? (next = Highest_object_index) : (next--);
			// Skip doors and unlinked objects
			if (Objects[next].type == OBJ_DOOR || Objects[next].roomnum == -1)
				goto prev_object;
			// Select object
			SelectObject(next);
			// Update the current object display
			Editor_state.SetCurrentLevelObject(Cur_object_index);
		}
		else
		{
next_object:
			(next == Highest_object_index) ? (next = 0) : (next++);
			// Skip doors or unlinked objects
			if (Objects[next].type == OBJ_DOOR || Objects[next].roomnum == -1)
				goto next_object;
			// Select object
			SelectObject(next);
			// Update the current object display
			Editor_state.SetCurrentLevelObject(Cur_object_index);
		}
		break;

	case VK_F8:
		// TODO : put this in the UI
		if ( HIWORD(::GetKeyState(VK_CONTROL)) && HIWORD(::GetKeyState(VK_SHIFT)) )
			DoPathPointCheck();
		break;
	}
	
	Cned_GrWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void Cned_LevelWnd::OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	Cned_GrWnd::OnSysChar(nChar, nRepCnt, nFlags);
}

void Cned_LevelWnd::OnTimer(UINT nIDEvent) 
{
	//Do our thing here
	int dx,dy;

	if (m_Cam.view_changed) 
	{
		m_Cam.view_changed = false;
		Render();
	}

	//Only process input when we have the focus
	if(m_InFocus)
	{
		dx = m_Mouse.x-m_Mouse.oldx;
		dy = m_Mouse.y-m_Mouse.oldy;

		if ( (dx!=0) || (dy!=0) ) {
			if (PANNING || ZOOMING || ROTATING || RADDING || (Placed_room != -1) || (Placed_group != NULL))
			{
				if (!m_bCaptured && (!((Placed_room != -1) || (Placed_group != NULL))))
				{
					SetCapture();
					m_bCaptured = true;
				}
				if ( ROTATING ) {
					matrix rotmat,tempm;
					GetMouseRotation( dx, dy, &rotmat );
					tempm = m_Cam.orient * rotmat;
					m_Cam.orient = tempm;
					m_View_changed = true;
				}
				else if ( PANNING ) {
					m_Cam.target += m_Cam.orient.rvec * -dx * MOVE_SCALE; 
					m_Cam.target += m_Cam.orient.uvec *  dy * MOVE_SCALE; 
					m_View_changed = true;
				}
				else if ( ZOOMING ) {
					m_Cam.dist += dy * ZOOM_SCALE;
//					if (m_Cam.dist < 0)
//						m_Cam.dist = 0;
					m_View_changed = true;
				}
				else if ( RADDING ) {
					m_Cam.rad += dy * RAD_SCALE;
					if (m_Cam.rad < 0)
						m_Cam.rad = 0;
					m_View_changed = true;
				}
			//	attaching rooms.
			else if ((Placed_room != -1) || (Placed_group != NULL)) {
				if (m_Keys.alt) {
					if (m_Keys.ctrl)
						Placed_room_attachpoint += (Placed_room_orient.rvec * (float) dx / 20.0) + (Placed_room_orient.uvec * (float) dy / 20.0); 
					else if ((Placed_baseroomp == NULL) && m_Keys.shift)	//If on terrain, slide up/down
						Placed_room_attachpoint += (Placed_room_orient.fvec * (float) dx / 20.0); 
					else {
						Placed_room_angle += (float) dx * 20.0;
						ComputePlacedRoomMatrix();
					}

					State_changed = 1;
				}
			}
			}
			else
			{
				if (m_bCaptured)
				{
					ReleaseCapture();
					m_bCaptured = false;
				}
			}
		}

		// If there was mouse input, update the view immediately
		if (m_View_changed) {
			m_View_changed = false;
			Render();
		}
	}

	// Save mouse position
	m_Mouse.oldx = m_Mouse.x;
	m_Mouse.oldy = m_Mouse.y;

	// If the mouse position changes an appreciable amount, invalidate the saved button down coordinates
	if (m_BDOWN_Found_x != -1 && m_BDOWN_Found_y != -1)
		if ( abs(m_BDOWN_Found_x-m_Mouse.x) > 5 || abs(m_BDOWN_Found_y-m_Mouse.y) > 5 )
			m_BDOWN_Found_x = m_BDOWN_Found_y = -1;

	if (m_Cam.moving)
	{
		vector target = {m_Cam.target.x+m_Cam.step.x, m_Cam.target.y+m_Cam.step.y, m_Cam.target.z+m_Cam.step.z};
		PlaceCamera(target,m_Cam.orient,m_Cam.dist);
		if ( abs(m_Cam.target.x-m_Cam.desttarget.x) <= abs(m_Cam.step.x) )
			m_Cam.step.x = 0;
		if ( abs(m_Cam.target.y-m_Cam.desttarget.y) <= abs(m_Cam.step.y) )
			m_Cam.step.y = 0;
		if ( abs(m_Cam.target.z-m_Cam.desttarget.z) <= abs(m_Cam.step.z) )
			m_Cam.step.z = 0;
		if (m_Cam.step.x == 0 && m_Cam.step.y == 0 && m_Cam.step.z == 0)
		{
			PlaceCamera(m_Cam.desttarget,m_Cam.orient,m_Cam.dist);
			m_Cam.moving = false;
		}
	}

	Cned_GrWnd::OnTimer(nIDEvent);
}

void Cned_LevelWnd::AdjustCamera(int code)
{
	matrix tempm,rotmat;
	float scale = 0.5f;
	angle p = 0, h = 0, b = 0;

	tempm = m_Cam.orient;

	switch (code)
	{
	case PAN_LEFT:
		m_Cam.target += m_Cam.orient.rvec * (-(100*scale));
		break;
	case PAN_RIGHT:
		m_Cam.target += m_Cam.orient.rvec * (100*scale);
		break;
	case PAN_UP:
		m_Cam.target += m_Cam.orient.uvec * (100*scale);
		break;
	case PAN_DOWN:
		m_Cam.target += m_Cam.orient.uvec * (-(100*scale));
		break;
	case ROT_LEFT:
	case ROT_RIGHT:
	case ROT_UP:
	case ROT_DOWN:
	case BANK_LEFT:
	case BANK_RIGHT:
		switch (code)
		{
		case ROT_LEFT:
			h = 65535 - 0x800;
			break;
		case ROT_RIGHT:
			h = 0x800;
			break;
		case ROT_UP:
			p = 65535 - 0x800;
			break;
		case ROT_DOWN:
			p = 0x800;
			break;
		case BANK_LEFT:
			b = 65535 - 0x800;
			break;
		case BANK_RIGHT:
			b = 0x800;
			break;
		}
		vm_AnglesToMatrix(&rotmat, p, h, b);
		m_Cam.orient = tempm * rotmat;
		vm_Orthogonalize(&m_Cam.orient);
		break;
	case ZOOM_INWARD:
		m_Cam.dist -= (100*scale);
		break;
	case ZOOM_OUTWARD:
		m_Cam.dist += (100*scale);
		break;
	case MOVE_FORWARD:
		m_Cam.target += m_Cam.orient.fvec * (100*scale);
		break;
	case MOVE_BACKWARD:
		m_Cam.target += m_Cam.orient.fvec * (-(100*scale));
		break;
	default:
		Int3();
		break;
	}

	m_View_changed = 1;
}

void Cned_LevelWnd::PlaceCamera(vector pos,matrix orient,float dist)
{
	m_Cam.dist = dist;
	m_Cam.rad = DEFAULT_VIEW_RAD;
	m_Cam.target = pos;
	m_Cam.orient = orient;

	m_Cam.view_changed = true;
}

extern CTextureDialogBar *dlgTextureDialogBar;

void Cned_LevelWnd::OnSetFocus(CWnd* pOldWnd) 
{
	Cned_GrWnd::OnSetFocus(pOldWnd);
	if(gCameraSlewer)
		gCameraSlewer->AttachCamera(&m_Cam,(char *)LPCSTR(m_Title));
	m_InFocus = true;

	if (m_bInitted)
	{
		dlgTextureDialogBar->SetCurrentFace(ROOMNUM(m_Prim.roomp),m_Prim.face);
//		dlgTextureDialogBar->SetCurrentTexture(0);
	}
}

void Cned_LevelWnd::OnKillFocus(CWnd* pNewWnd) 
{
	Cned_GrWnd::OnKillFocus(pNewWnd);
	memset(&m_Mouse,0,sizeof(m_Mouse));
	memset(&m_Keys,0,sizeof(m_Keys));

//	dlgTextureDialogBar->SetCurrentTexture(-1);

	m_InFocus = false;
	
}

void Cned_LevelWnd::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch(nChar)
	{
	case VK_MENU:
		m_Keys.alt = true;
		break;
	default:
		CWnd ::OnSysKeyDown(nChar, nRepCnt, nFlags);
	}
	
	//Cned_GrWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

void Cned_LevelWnd::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	
	switch(nChar)
	{
	case VK_MENU:
		m_Keys.alt = false;
		break;
	default:
		CWnd ::OnSysKeyUp(nChar, nRepCnt, nFlags);
	}
	
	//Cned_GrWnd::OnSysKeyUp(nChar, nRepCnt, nFlags);
}


//Draws the mine, starting at a the specified room
//The rendering surface must be set up, and g3_StartFrame() must have been called
//Parameters:	viewer_roomnum - what room the viewer is in
//					flag_automap - if true, flag segments as visited when rendered
//					called_from_terrain - set if calling this routine from the terrain renderer
void RenderMine(int viewer_roomnum,int flag_automap,int called_from_terrain,bool render_all,bool outline,bool flat,prim *prim);

void Cned_LevelWnd::TGWRenderMine(vector *pos,matrix *orient,float zoom,int start_roomnum)
{
	if (start_roomnum<0 || start_roomnum>MAX_ROOMS+MAX_PALETTE_ROOMS)
		return;

	//save these vars for possible use by FindRoomFace()
	m_last_viewer_eye = *pos;
	m_last_viewer_orient = *orient;
	m_last_zoom = zoom;
	m_last_start_room = start_roomnum;
	
	StartEditorFrame(m_grViewport,pos,orient,zoom);
	FrameCount++;

	RenderMine(start_roomnum,0,0,true,m_bOutline,m_bFlat,&m_Prim);
//	PostRender(Viewer_object->roomnum);

	//	we need to save these for object movement.
	g3_GetViewPosition(&m_ViewPos);
	g3_GetViewMatrix(&m_ViewMatrix);

	EndEditorFrame();
}



void Cned_LevelWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
//	Draw what's on the screen back page to the desktop
	if (m_bTextured && m_StartFlip) {	// do if a flip was signaled
		m_StartFlip = FALSE;

		if (bDoOpenGL)
		{
			HWND save_wnd;

			app=(oeWin32Application *)g_OuroeApp;

			save_wnd=(HWND)app->m_hWnd;
			app->m_hWnd=(HWnd)m_hWnd;
			rend_SetOpenGLWindowState (1,g_OuroeApp,NULL);

			rend_Flip();
//			Cned_GrWnd::OnPaint();

			rend_SetOpenGLWindowState (0,g_OuroeApp,NULL);
			app->m_hWnd=(HWnd)save_wnd;
		}
		else
		{
	  		m_grScreen->flip();
			m_grViewport->clear();
		}

		
	} else
		Cned_GrWnd::OnPaint();
}


void Cned_LevelWnd::OnSize(UINT nType, int cx, int cy) 
{
	Cned_GrWnd::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	Render();
}

void Cned_LevelWnd::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	//Load top-level menu from resource
	CMenu mnuTop;
	mnuTop.LoadMenu(IDR_LEVELWND_POPUP);

	//get popup menu from first submenu
	CMenu *pPopup = mnuTop.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	//Checked state for popup menu items is automatically managed by MFC
	//UPDATE_COMMAND_UI 

	//Display popup menu
	pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON,point.x,point.y,this,NULL);

//	CWnd::OnContextMenu(pWnd, point);
}

void Cned_LevelWnd::OnCenterMine() 
{
	// TODO: Add your command handler code here
	CenterMine();
}

void Cned_LevelWnd::OnCenterRoom() 
{
	// TODO: Add your command handler code here
	CenterRoom(m_Prim.roomp);
}

void Cned_LevelWnd::OnTextured() 
{
	// TODO: Add your command handler code here
	m_bTextured = true;
	m_bOutline = false;
	State_changed = true;
	Render();
}

void Cned_LevelWnd::OnUpdateTextured(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bTextured && !m_bOutline);
}

void Cned_LevelWnd::OnViewTexturedOutline() 
{
	// TODO: Add your command handler code here
	m_bTextured = true;
	m_bOutline = true;
	State_changed = true;
	Render();
}

void Cned_LevelWnd::OnUpdateViewTexturedOutline(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bTextured && m_bOutline);
}

void Cned_LevelWnd::OnWireframe() 
{
	// TODO: Add your command handler code here
	if (m_bTextured)
	{
		m_bTextured = false;
		m_bOutline = false;
		State_changed = true;
		Render();
	}
}

void Cned_LevelWnd::OnUpdateWireframe(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(!m_bTextured);
}


void Cned_LevelWnd::InitCamera()
{
	// This is the only time to use camera Level_cam; it contains the initial camera view that is loaded from level
	m_Cam.orient = Level_cam.orient;
	m_Cam.target = Level_cam.target;
	m_Cam.dist = Level_cam.dist;
	m_Cam.rad = DEFAULT_VIEW_RAD;
	m_Cam.moving = false;

	m_Cam.view_changed = true;
}


void Cned_LevelWnd::CenterRoom(room *rp)
{
	vector pos;

	ComputeRoomBoundingSphere(&pos,rp); // ComputeRoomCenter(&pos,rp);
	if (!m_bSmoothCenter)
		m_Cam.target = pos;
	else
	{
		m_Cam.desttarget = pos;
		// Start the camera moving
		m_Cam.step.x = (m_Cam.desttarget.x-m_Cam.target.x)/10;
		m_Cam.step.y = (m_Cam.desttarget.y-m_Cam.target.y)/10;
		m_Cam.step.z = (m_Cam.desttarget.z-m_Cam.target.z)/10;
		m_Cam.moving = true;
	}

	m_Cam.view_changed = true;
}


void Cned_LevelWnd::CenterOrigin()
{
	vector pos = {0,0,0};

	m_Cam.target = pos;

	m_Cam.view_changed = true;
}


void Cned_LevelWnd::CenterMine()
{
	vector center = {TERRAIN_WIDTH*(TERRAIN_SIZE/2), -100, TERRAIN_DEPTH*(TERRAIN_SIZE/2)};

	m_Cam.dist = DEFAULT_VIEW_DIST;
	m_Cam.rad = DEFAULT_VIEW_RAD;
	vm_MakeIdentity(&m_Cam.orient);

	m_Cam.target = center;

	m_Cam.view_changed = true;
}


void Cned_LevelWnd::OnCenterOrigin() 
{
	// TODO: Add your command handler code here
	CenterOrigin();
}

void Cned_LevelWnd::OnDisplayCurrentRoomView() 
{
	// TODO: Add your command handler code here
	char title[_MAX_PATH];

	if (theApp.m_pRoomFrm == NULL)
	{
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

		// create a new MDI child window for the room(s)
		CRoomFrm * rFrame = (CRoomFrm *) pFrame->CreateNewChild(
			RUNTIME_CLASS(CRoomFrm), IDR_ORFTYPE, theApp.m_hMDI_ORF_Menu, theApp.m_hMDI_ORF_Accel);

		theApp.m_pRoomFrm = rFrame;
		strcpy(title,"Current Room View");
		rFrame->SetWindowText(title);
		rFrame->SetTitle(title);
		rFrame->SetPrim(m_Prim.roomp, m_Prim.face, m_Prim.portal, m_Prim.edge, m_Prim.vert);
		rFrame->InitTex();
		rFrame->InitPanes();
	}
}

//Set the current object
void SelectObject(int objnum)
{
	char *type_name,*obj_name;
	char *id_name = NULL;

	ASSERT(objnum != -1);

	Cur_object_index = objnum;

	object *objp = &Objects[objnum];
	type_name = Object_type_names[objp->type];
	obj_name = objp->name ? objp->name : "<none>";

	if (IS_GENERIC(objp->type))
		id_name = Object_info[objp->id].name;
	else if (objp->type == OBJ_DOOR)
		id_name = Doors[objp->id].name;

	if (id_name)
		if (objp->type != OBJ_PLAYER)
			PrintStatus("Object %d selected. Type = %s:%s, Name = %s",objnum,type_name,id_name,obj_name);
		else
			PrintStatus("Object %d selected. Type = %s:%s, Start = %d",objnum,type_name,id_name,objp->id);
	else
		if (objp->type != OBJ_PLAYER)
			PrintStatus("Object %d selected. Type = %s, Name = %s",objnum,type_name,obj_name);
		else
			PrintStatus("Object %d selected. Type = %s, Start = %d",objnum,type_name,objp->id);

	State_changed = 1;
}


bool Cned_LevelWnd::GetModifiedStatus()
{
	return m_Modified;
}

void Cned_LevelWnd::SetModifiedFlag(bool modified)
{
	//theApp.m_ThisLevelFrame->SetWindowText();

	//theApp.m_ThisLevelFrame->SetModifiedFlag(true);
	if(modified!=m_Modified)
	{
		CString title;
		theApp.m_ThisLevelFrame->GetWindowText(title);
		if(modified && (title[title.GetLength()-1] != '*') )
		{
			title += " *";
			
		}
		else if((!modified) && (title[title.GetLength()-1] == '*') )
		{
			title = title.Left(title.GetLength()-2);
		}
		theApp.m_ThisLevelFrame->SetWindowText(title);
		theApp.m_ThisLevelFrame->SetTitle(title);
		m_Modified = modified;
	}
}


void Cned_LevelWnd::OnViewMoveCameraToCurrentFace() 
{
	// TODO: Add your command handler code here
	if (m_Prim.face != -1)
		PlaceCameraAtRoomFace(m_Prim.roomp,m_Prim.face,false);
}

void Cned_LevelWnd::OnViewMoveCameraToCurrentObject() 
{
	// TODO: Add your command handler code here
	object *objp = &Objects[Cur_object_index];
	matrix orient;

	//Turn the camera around so facing the object
	orient.fvec = -objp->orient.fvec;
	orient.rvec = -objp->orient.rvec;
	orient.uvec = objp->orient.uvec;

	//Move the camera to the object
	PlaceCamera(objp->pos,orient,2*objp->size);
}

//Set the camera in the specified room facing the specified face
//If room_center is true, put camera at the center of the room facing the face
//If room_center is false, put the camera directly in front of the selected face
//If the room is external, put the camera a distance away from the room, 
//facing either the center (if room_center is true) or the specified face
void Cned_LevelWnd::PlaceCameraAtRoomFace(room *roomp,int facenum,bool room_center)
{
	vector	vp,newpos;
	matrix	orient;
	int roomnum;
	bool outside_mine = 0;
	float dist = 0.0f;
	float face_rad = 0.0f;

	if (facenum != -1)
		face_rad = ComputeFaceBoundingCircle(&vp,roomp,facenum); // ComputeCenterPointOnFace(&vp,roomp,facenum);
	roomnum = ROOMNUM(roomp);

	if (room_center) {

		//Get position
		ComputeRoomBoundingSphere(&newpos,roomp); // ComputeRoomCenter(&newpos,roomp);

		if (roomp->flags & RF_EXTERNAL) {
/*
			vector t;
			float rad = ComputeRoomBoundingSphere(&t,roomp);

			newpos.z -= rad * 1.5;

			if(newpos.x < 1.0)
				newpos.x = 1.0;
			if(newpos.x > TERRAIN_WIDTH * TERRAIN_SIZE - 1.0)
				newpos.x = TERRAIN_WIDTH * TERRAIN_SIZE - 1.0;
			if(newpos.z < 1.0)
				newpos.z = 1.0;
			if(newpos.z > TERRAIN_DEPTH * TERRAIN_SIZE - 1.0) 
				newpos.z = TERRAIN_WIDTH * TERRAIN_SIZE - 1.0;

			orient = Identity_matrix;

			roomnum = GetTerrainRoomFromPos(&newpos);
*/
		}
		else {
			//Get orientation
			if (facenum != -1)
				vp -= newpos;		//vector from center of room to face
			else
				vp = newpos;
			vm_VectorToMatrix(&orient,&vp,NULL,NULL);
		}
		dist = 0.0f;
	}
	else {
		ASSERT(facenum != -1);
		face *fp = &roomp->faces[facenum];

		dist = face_rad;
		newpos = vp + fp->normal * dist;

		vector t = -fp->normal;
		vm_VectorToMatrix(&orient,&t,NULL,NULL);

		if (roomp->flags & RF_EXTERNAL) {
/*
			if(newpos.x < 1.0)
				newpos.x = 1.0;
			if(newpos.x > TERRAIN_WIDTH * TERRAIN_SIZE - 1.0)
				newpos.x = TERRAIN_WIDTH * TERRAIN_SIZE - 1.0;
			if(newpos.z < 1.0)
				newpos.z = 1.0;
			if(newpos.z > TERRAIN_DEPTH * TERRAIN_SIZE - 1.0) 
				newpos.z = TERRAIN_WIDTH * TERRAIN_SIZE - 1.0;
			roomnum = GetTerrainRoomFromPos(&newpos);
*/
		}
		else {
			int new_roomnum = FindPointRoom(&newpos);
			if (new_roomnum == -1)
				outside_mine = 1;
			else
				roomnum = new_roomnum;
		}
	}

	PlaceCamera(newpos,orient,5.0); // dist
/*
	//Reset viewer
	if (Editor_view_mode == VM_ROOM) {
		Viewer_object->pos = newpos; 
		Viewer_object->orient = orient;
	}
	else
		MoveViewer(&newpos,roomnum,&orient);

	if (outside_mine)
		Viewer_object->flags |= OF_OUTSIDE_MINE;

	Viewer_moved = 1;
*/
}

void Cned_LevelWnd::OnUpdateViewMoveCameraToCurrentFace(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	bool enable;

	m_Prim.face != -1 ? enable = true : enable = false;
	pCmdUI->Enable(enable);
}

void Cned_LevelWnd::OnUpdateViewMoveCameraToCurrentObject(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	bool enable = false;

	if ( Cur_object_index != -1 && Highest_object_index >= 0 )
		enable = true;

	pCmdUI->Enable(enable);
}

void MatchFacePortalPair(prim *prim);

void Cned_LevelWnd::SetPrim(prim *prim)
{
	room *oldrp = m_Prim.roomp;
	m_Prim = *prim;
	// If the room changes, unmark everything in the current room view
	if (theApp.m_pRoomFrm != NULL && m_Prim.roomp != oldrp)
		theApp.m_pRoomFrm->ForceUnmarkAll();

	MatchFacePortalPair(&m_Prim);

	State_changed = 1;
}


void Cned_LevelWnd::SetPrim(room *rp, int face, int portal, int edge, int vert)
{
	room *oldrp = m_Prim.roomp;
	m_Prim.roomp = rp;
	// If the room changes, unmark everything in the current room view
	if (theApp.m_pRoomFrm != NULL && rp != oldrp)
		theApp.m_pRoomFrm->ForceUnmarkAll();

	m_Prim.face = face;
	m_Prim.portal = portal;
	m_Prim.edge = edge;
	m_Prim.vert = vert;

	MatchFacePortalPair(&m_Prim);

	State_changed = 1;
}


void Cned_LevelWnd::OnViewMoveCameraToCurrentRoom() 
{
	// TODO: Add your command handler code here
	PlaceCameraAtRoomFace(m_Prim.roomp,m_Prim.face,true);
}
