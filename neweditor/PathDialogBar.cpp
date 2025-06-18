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
 // PathDialogBar.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"

#include "gamepath.h"
#include "../editor/EPath.h"
#include "boa.h"
#include "bnode.h"
#include "../editor/ebnode.h"
#include "ned_PathNode.h"
#include "PathDialogBar.h"
#include "ListDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPathDialogBar dialog

CPathDialogBar::CPathDialogBar()
{
	//{{AFX_DATA_INIT(CPathDialogBar)
	//}}AFX_DATA_INIT
}


void CPathDialogBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPathDialogBar)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPathDialogBar, CDialogBar)
	//{{AFX_MSG_MAP(CPathDialogBar)
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(IDC_BNODE_CREATE,OnControlUpdate)
	ON_BN_CLICKED(IDC_BNODE_CREATE, OnBnodeCreate)
	ON_BN_CLICKED(IDC_BNODE_DESTROY, OnBnodeDestroy)
	ON_BN_CLICKED(IDC_BNODE_VERIFY, OnBnodeVerify)
	ON_BN_CLICKED(IDC_PATH_RADIO, OnPathRadio)
	ON_BN_CLICKED(IDC_BNODE_RADIO, OnBnodeRadio)
	ON_BN_CLICKED(IDC_BNODE_AUTO_MAKE_EDGES, OnBnodeAutoMakeEdges)
	ON_BN_CLICKED(IDC_BNODE_AUTO_EDGE_ROOM, OnBnodeAutoEdgeRoom)
	ON_UPDATE_COMMAND_UI(IDC_BNODE_DESTROY,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_BNODE_VERIFY,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_PATH_RADIO,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_BNODE_RADIO,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_BNODE_AUTO_MAKE_EDGES,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_BNODE_AUTO_EDGE_ROOM,OnControlUpdate)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog )
END_MESSAGE_MAP()

CPathDialogBar *dlgPathDialogBar = NULL;


/////////////////////////////////////////////////////////////////////////////
// CPathDialogBar message handlers

LONG CPathDialogBar::OnInitDialog ( UINT wParam, LONG lParam)
{
	dlgPathDialogBar = this;

	if ( !HandleInitDialog(wParam, lParam) || !UpdateData(FALSE))
	{
		TRACE0("Warning: UpdateData failed during dialog init.\n");
		return FALSE;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPathDialogBar::PreTranslateMessage(MSG* pMsg) 
{
	return CDialogBar::PreTranslateMessage(pMsg);
}

//this is what enables your controls:
void CPathDialogBar::OnControlUpdate(CCmdUI* pCmdUI)
{
	bool bEnable = false;

	if (theApp.m_pLevelWnd != NULL)
	{
		switch (pCmdUI->m_nID)
		{
		case IDC_BNODE_CREATE:
		case IDC_BNODE_DESTROY:
		case IDC_BNODE_VERIFY:
		case IDC_BNODE_AUTO_MAKE_EDGES:
		case IDC_BNODE_AUTO_EDGE_ROOM:
			if (Editor_state.GetCurrentNodeType() == NODETYPE_AI)
				bEnable = true;
			break;

		case IDC_PATH_RADIO:
		case IDC_BNODE_RADIO:
			bEnable = true;
			break;
		}
	}

	pCmdUI->Enable(bEnable);
}

void CPathDialogBar::OnDestroy() 
{
	CDialogBar::OnDestroy();

	dlgPathDialogBar = NULL;
}

void CPathDialogBar::OnBnodeCreate() 
{
	// TODO: Add your control notification handler code here
	MakeBOA();
	EBNode_MakeFirstPass();	
	World_changed = 1;
}

void CPathDialogBar::OnBnodeDestroy() 
{
	// TODO: Add your control notification handler code here
	EBNode_ClearLevel();
	World_changed = 1;
}

void CPathDialogBar::OnBnodeVerify() 
{
	// TODO: Add your control notification handler code here
	ned_EBNode_VerifyGraph();
}

void CPathDialogBar::OnPathRadio() 
{
	// TODO: Add your control notification handler code here
	Editor_state.SetCurrentNodeType(NODETYPE_GAME);
}

void CPathDialogBar::OnBnodeRadio() 
{
	// TODO: Add your control notification handler code here
	Editor_state.SetCurrentNodeType(NODETYPE_AI);
}

void CPathDialogBar::OnBnodeAutoMakeEdges() 
{
	// TODO: Add your control notification handler code here
	int room,node,edge;
	Editor_state.GetCurrentBNode(&room,&node,&edge);

	EBNode_AutoEdgeNode(node, room);
	ned_EBNode_VerifyGraph();	
	World_changed = 1;
}

void CPathDialogBar::OnBnodeAutoEdgeRoom() 
{
	// TODO: Add your control notification handler code here
	int room,node,edge;
	Editor_state.GetCurrentBNode(&room,&node,&edge);

	bn_list *bnlist = BNode_GetBNListPtr(room);
	if(!bnlist)
		return;

	for(int i = 0; i < bnlist->num_nodes; i++)
	{
		EBNode_AutoEdgeNode(i, room);
	}

	ned_EBNode_VerifyGraph();	
	World_changed = 1;
}


CListDialog *bn_verify_dlg = NULL;

// copy of EBNode_VerifyGraph from main\editor\ebnode.cpp, with output to a list dialog
bool ned_EBNode_VerifyGraph()
{
	char str[256];

	if (bn_verify_dlg == NULL)
	{
		bn_verify_dlg = new CListDialog;
		bn_verify_dlg->Create(IDD_LISTDIALOG);
	}
	else
		bn_verify_dlg->SetActiveWindow();
	bn_verify_dlg->SetWindowText("Verify BNodes");
	CListBox *lb = (CListBox *) bn_verify_dlg->GetDlgItem(IDC_LIST1);
	// Delete any strings that may be in the listbox
	while ( lb->DeleteString(0) != LB_ERR );
	CStatic *hdr = (CStatic *) bn_verify_dlg->GetDlgItem(IDC_HEADER_TEXT);
	hdr->SetWindowText("BNode check results:");
	CStatic *status = (CStatic *) bn_verify_dlg->GetDlgItem(IDC_STATUS_TEXT);
	status->SetWindowText("Verifying bnodes...");

	bool f_verified = true;
	int i;
	int j;
	int k;

	if(!BNode_allocated)
	{
		lb->InsertString(-1,"EBNode Verify: No BNodes for this level");
		return false;
	}

	MakeBOA();

	for(i = Highest_room_index + 1; i <= Highest_room_index + BOA_num_terrain_regions; i++)
	{
		bn_list *nlist;
		nlist = BNode_GetBNListPtr(i);
		int cur_region = i - Highest_room_index - 1;

		for(j = nlist->num_nodes - 1; j >= 0; j--)
		{
			int cell = GetTerrainRoomFromPos(&nlist->nodes[j].pos);
			if(cur_region != TERRAIN_REGION(cell))
			{
				for(k = 0; k < nlist->nodes[j].num_edges; k++)
				{
					if(BOA_INDEX(nlist->nodes[j].edges[k].end_room) >= 0 && BOA_INDEX(nlist->nodes[j].edges[k].end_room) <= Highest_room_index)
					{
						int r = nlist->nodes[j].edges[k].end_room;
						int p = nlist->nodes[j].edges[k].end_index;

						int x;
						for(x = 0; x < Rooms[r].num_portals; x++)
						{
							if(Rooms[r].portals[x].bnode_index == p)
							{
								int cr = Rooms[r].portals[x].croom;
								int cp = Rooms[r].portals[x].cportal;

								Rooms[cr].portals[cp].bnode_index = -1;
							}
						}
					}
				}

				EBNode_RemoveNode(i, j);
			}
		}
	}

	for(i = 0; i <= Highest_room_index + BOA_num_terrain_regions; i++)
	{
		bn_list *nlist;
		int j;
		int k;

		if(i >= 0 && i <= Highest_room_index && !Rooms[i].used)
			continue;

		if(i <= Highest_room_index && (Rooms[i].flags & RF_EXTERNAL))
			continue;

		nlist = BNode_GetBNListPtr(i);

		for(j = 0; j < nlist->num_nodes; j++)
		{
			for(k = 0; k < nlist->nodes[j].num_edges; k++)
			{
				if(nlist->nodes[j].edges[k].max_rad < 5.0f)
				{
					lb->InsertString(-1,"EBNode Verify: Removed a skinny edge.");
					EBNode_RemoveEdge(j, i, nlist->nodes[j].edges[k].end_index, nlist->nodes[j].edges[k].end_room);
					k--;
				}
			}
		}
	}

	// Bash invalid nodes
	for(i = 0; i <= Highest_room_index; i++)
	{
		if(Rooms[i].used)
		{
			room *rp = &Rooms[i];

			if(i <= Highest_room_index && (Rooms[i].flags & RF_EXTERNAL))
				continue;

			for(j = 0; j < Rooms[i].num_portals; j++)
			{
				if(Rooms[i].portals[j].bnode_index >= 0 && Rooms[i].portals[j].bnode_index >= Rooms[i].bn_info.num_nodes)
				{
					lb->InsertString(-1,"EBNode: Bashed an invalid node");
					Rooms[i].portals[j].bnode_index = -1;
				}
				else if(Rooms[i].portals[j].bnode_index < 0)
				{
					room *rp = &Rooms[i];
					bool f_add = true;

					if(!((rp->portals[j].flags & PF_BLOCK) && !(rp->portals[j].flags & PF_BLOCK_REMOVABLE)))
					{
						f_add = false;
					}
					
					if((rp->portals[j].flags & PF_RENDER_FACES) && !(rp->portals[j].flags & PF_RENDERED_FLYTHROUGH))
					{
						if(!(GameTextures[rp->faces[rp->portals[j].portal_face].tmap].flags & (TF_BREAKABLE | TF_FORCEFIELD)))
						{
							f_add = false;
						}
					}
					
					if(rp->portals[j].flags & PF_TOO_SMALL_FOR_ROBOT)
					{
						f_add = false;
					}

					if(f_add)
					{
						vector pos;
						pos = rp->portals[j].path_pnt + rp->faces[rp->portals[j].portal_face].normal * 0.75f;
						rp->portals[j].bnode_index = EBNode_AddNode(i, &pos, false, false);
						lb->InsertString(-1,"EBNode Verify: Added a portal node");
					}
				}
			}
		}
	}

	int region;
	for(region = 0; region < BOA_num_terrain_regions; region++)
	{
		for(i = 0; i < BOA_num_connect[region]; i++)
		{
			int end_room = BOA_connect[region][i].roomnum;
			room *rp = &Rooms[end_room];
			int p = BOA_connect[region][i].portal;

			vector pos;
			pos = rp->portals[p].path_pnt - rp->faces[rp->portals[p].portal_face].normal * 0.75f;

			int external_room = rp->portals[p].croom;
			int external_portal = rp->portals[p].cportal;
			ASSERT(Rooms[external_room].flags & RF_EXTERNAL);
	
			if(Rooms[external_room].portals[external_portal].bnode_index < 0)
			{
				Rooms[external_room].portals[external_portal].bnode_index = EBNode_AddNode(Highest_room_index + region + 1, &pos, false, false);
				if(Rooms[end_room].portals[p].bnode_index >= 0)
					EBNode_AddEdge(Rooms[external_room].portals[external_portal].bnode_index, Highest_room_index + region + 1, Rooms[end_room].portals[p].bnode_index, end_room);
			}
		}
	}

	for(i = 0; i <= Highest_room_index; i++)
	{
		if(Rooms[i].used)
		{
			room *rp = &Rooms[i];
			if(i <= Highest_room_index && (Rooms[i].flags & RF_EXTERNAL))
				continue;

			for(j = 0; j < Rooms[i].num_portals; j++)
			{
				if((Rooms[i].portals[j].flags & PF_BLOCK) && !(Rooms[i].portals[j].flags & PF_BLOCK_REMOVABLE))
				{
					if(Rooms[i].portals[j].bnode_index >= 0)
					{
						lb->InsertString(-1,"EBNode Verify: Removed a node.");
						EBNode_RemoveNode(i, Rooms[i].portals[j].bnode_index);
					}
					continue;
				}
				
				if((Rooms[i].portals[j].flags & PF_RENDER_FACES) && !(Rooms[i].portals[j].flags & PF_RENDERED_FLYTHROUGH))
				{
					if(!(GameTextures[Rooms[i].faces[Rooms[i].portals[j].portal_face].tmap].flags & (TF_BREAKABLE | TF_FORCEFIELD)))
					{
						if(Rooms[i].portals[j].bnode_index >= 0)
						{
							lb->InsertString(-1,"EBNode Verify: Removed a node.");
							EBNode_RemoveNode(i, Rooms[i].portals[j].bnode_index);
						}
						continue;
					}
				}
				
				if(Rooms[i].portals[j].flags & PF_TOO_SMALL_FOR_ROBOT)
				{
					if(Rooms[i].portals[j].bnode_index >= 0)
					{
						lb->InsertString(-1,"EBNode Verify: Removed a node.");
						EBNode_RemoveNode(i, Rooms[i].portals[j].bnode_index);
					}
					continue;
				}
				
				if(Rooms[i].portals[j].bnode_index < 0)
				{
					if(Rooms[i].flags & RF_EXTERNAL)
					{
						int cr = rp->portals[j].croom;
						int ci = Rooms[cr].portals[rp->portals[j].cportal].bnode_index;

						if(Rooms[cr].flags & RF_EXTERNAL)
						{
							continue;
						}
						
						vector pos;
						pos = rp->portals[j].path_pnt + rp->faces[rp->portals[j].portal_face].normal * 0.75f;
						int roomnum = BOA_INDEX(GetTerrainRoomFromPos(&pos));
						
						int xxx;
						for(xxx = 0; xxx < BOA_num_connect[TERRAIN_REGION(roomnum)]; xxx++)
						{
							if(BOA_connect[TERRAIN_REGION(roomnum)][xxx].roomnum == cr && BOA_connect[TERRAIN_REGION(roomnum)][xxx].portal == rp->portals[j].cportal)
							{
								break;
							}
						}
						
						if(xxx >= BOA_num_connect[TERRAIN_REGION(roomnum)])
						{
							lb->InsertString(-1,"EBNode Verify:  External room isn't in terrain region list");
							f_verified = false;
							continue;
						}
						
						rp->portals[j].bnode_index = EBNode_AddNode(roomnum, &pos, false, false);
						ASSERT(rp->portals[j].bnode_index >= 0);
						EBNode_AutoEdgeNode(rp->portals[j].bnode_index, roomnum);
						lb->InsertString(-1,"EBNode Verify: Added a node and autoedged it.");

						if(ci >= 0)
						{
							EBNode_AddEdge(rp->portals[j].bnode_index, roomnum, ci, cr);
						}
					}
					else
					{
						vector pos;
						pos = rp->portals[j].path_pnt + rp->faces[rp->portals[j].portal_face].normal * 0.75f;
						rp->portals[j].bnode_index = EBNode_AddNode(i, &pos, false, false);
						ASSERT(rp->portals[j].bnode_index >= 0);
						EBNode_AutoEdgeNode(rp->portals[j].bnode_index, i);
						lb->InsertString(-1,"EBNode Verify: Added a node and autoedged it.");

						int cr = rp->portals[j].croom;
						int ci = Rooms[cr].portals[rp->portals[j].cportal].bnode_index;

						if(ci < 0)
						{
							continue;
						}

						if(Rooms[cr].flags & RF_EXTERNAL)
						{
							vector pos;
							pos = rp->portals[j].path_pnt - rp->faces[rp->portals[j].portal_face].normal * 0.75f;
							int roomnum = BOA_INDEX(GetTerrainRoomFromPos(&pos));
				
							EBNode_AddEdge(rp->portals[j].bnode_index, i, ci, roomnum);
						}
						else
						{
							if(ci >= 0)
							{
								EBNode_AddEdge(rp->portals[j].bnode_index, i, ci, cr);
							}
						}
					}
				}
			}
		}
	}

	for(i = 0; i <= Highest_room_index + BOA_num_terrain_regions; i++)
	{
		bn_list *nlist;
		int j;
		int k;

		if(i >= 0 && i <= Highest_room_index && !Rooms[i].used)
			continue;

		if(i >= 0 && i <= Highest_room_index && (Rooms[i].flags & RF_EXTERNAL))
			continue;

		nlist = BNode_GetBNListPtr(i);

		for(j = 0; j < nlist->num_nodes; j++)
		{
			for(k = 0; k < nlist->nodes[j].num_edges; k++)
			{
				if(nlist->nodes[j].edges[k].max_rad < 5.0f)
				{
					lb->InsertString(-1,"EBNode Verify: Removed a skinny edge.");
					EBNode_RemoveEdge(j, i, nlist->nodes[j].edges[k].end_index, nlist->nodes[j].edges[k].end_room);
					k--;
				}
			}
		}
	}

	for(i = 0; i <= Highest_room_index + BOA_num_terrain_regions; i++)
	{
		bn_list *nlist;
		int j;
		int k;

		if(i >= 0 && i <= Highest_room_index && !Rooms[i].used)
			continue;

		if(i >= 0 && i <= Highest_room_index && (Rooms[i].flags & RF_EXTERNAL))
			continue;

		nlist = BNode_GetBNListPtr(i);

		for(j = 0; j < nlist->num_nodes; j++)
		{
			for(k = 0; k < nlist->nodes[j].num_edges; k++)
			{
				if(nlist->nodes[j].edges[k].max_rad < 5.0f)
				{
					sprintf(str,"Skinny Edge - from r%d n%d to r%d n%d", i, j, nlist->nodes[j].edges[k].end_room, nlist->nodes[j].edges[k].end_index);
					lb->InsertString(-1,str);
					f_verified = false;
				}
			}
		}
	}


	for(i = 0; i <= Highest_room_index + BOA_num_terrain_regions; i++)
	{
		if(i >= 0 && i <= Highest_room_index && !Rooms[i].used)
			continue;

		if(i >= 0 && i <= Highest_room_index && (Rooms[i].flags & RF_EXTERNAL))
			continue;

		bn_list *nlist = BNode_GetBNListPtr(i);
		ASSERT(nlist);

		for(j = 0; j < nlist->num_nodes; j++)
		{
			for(k = j + 1; k < nlist->num_nodes; k++)
			{
				if(!BNode_FindPath(i, j, k, 0.0f))
				{
					sprintf(str,"BNODE ERROR: No path from %d to %d in room %d", j + 1, k + 1, i);
					lb->InsertString(-1,str);
					f_verified = false;
				}
			}
		}
	}

	BNode_verified = f_verified;
	
	if(f_verified)
	{
		lb->InsertString(-1,"EBNode:  VERIFY OK!");
		status->SetWindowText("Verify complete.");
	}
	else
	{
		lb->InsertString(-1,"EBNode:  VERIFY FAILED!");
		status->SetWindowText("Verify complete.\nNOTE: You should manually fix errors by adding nodes and edges as needed.");
	}

	return f_verified;
}
