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
#include "resource.h"

#include "globals.h"
#include "gamepath.h"
#include "../editor/epath.h"
#include "EditLineDialog.h"
#include "ned_PathNode.h"
#include "../editor/ebnode.h"

int InsertNodeIntoPath(int pathnum,int nodenum,int flags,int roomnum,vector pos,matrix orient);

// Inserts a new gamepath with a single node at the specified position
void ned_InsertPath(int roomnum,vector pos,matrix orient)
{
	char *namestr;
	int path,node;
	int dup_test;

	path = AllocGamePath();
	if (path < 0)
		return;

	CEditLineDialog dlg("Enter path name",NULL);
	if (dlg.DoModal() == IDCANCEL)
	{
		FreeGamePath(path);
		return;
	}

	namestr = (char *)dlg.GetText();
	dup_test = FindGamePathName(namestr);
	if(dup_test != -1)
	{
		OutrageMessageBox("PATH NOT CREATED: A path named %s already exists.", namestr);
		FreeGamePath(path);
		return;
	}
	strcpy(GamePaths[path].name,namestr);

	node = InsertNodeIntoPath(path,-1,0,roomnum,pos,orient);
	if(node < 0)
		return;

	Editor_state.SetCurrentPath(path);
	Editor_state.SetCurrentNode(node);
	World_changed = 1;
}

// Inserts a gamepath node at the specified position, and makes it current
void ned_InsertNode(int roomnum,vector pos,matrix orient)
{
	int path = Editor_state.GetCurrentPath();
	int node = Editor_state.GetCurrentNode();

	if (path == -1)
	{
		OutrageMessageBox("No current path.");
		return;
	}

	if (Num_game_paths < 1)
	{
		OutrageMessageBox("There are no paths to operate on.");
		return;
	}

	node = InsertNodeIntoPath(path,node,0,roomnum,pos,orient);
	if (node < 0)
		return;

	Editor_state.SetCurrentNode(node);
	World_changed = 1;
}

// Deletes the current path
void ned_DeletePath(bool force)
{
	int path = Editor_state.GetCurrentPath();

	if (Num_game_paths < 1)
		return;

	if (!force)
	{
		if (MessageBox(NULL,"Are you sure you want to delete this path?",GamePaths[path].name,MB_YESNO) == IDNO)
			return;
	}

	FreeGamePath(path);

	Editor_state.SetCurrentPath(GetNextPath(path));
	World_changed = 1;
}

// Deletes the current gamepath node
void ned_DeleteNode()
{
	int path = Editor_state.GetCurrentPath();
	int node = Editor_state.GetCurrentNode();

	if (Num_game_paths < 1)
	{
		OutrageMessageBox("There are no paths to operate on.");
		return;
	}

	if (GamePaths[path].num_nodes == 1)
	{
		if (MessageBox(NULL,"This is the only node in this path. Would you like to delete this path?.",GamePaths[path].name,MB_YESNO) == IDYES)
			ned_DeletePath(true);
		return;
	}

	DeleteNodeFromPath(path,node);

	// If this was the ending node, move it back one
	if (node == GamePaths[path].num_nodes)
		Editor_state.SetCurrentNode(node-1);

	World_changed = 1;
}

// Moves the specified gamepath node by the specified amount
bool ned_MoveNode(int path,int node,float x,float y,float z)
{
	if (Num_game_paths < 1)
	{
		OutrageMessageBox("There are no paths to operate on.");
		return false;
	}

//	vector delta_movement = Viewer_object->orient.rvec * -D3EditState.node_movement_inc;
	vector delta_movement = {x,y,z};

	int ret = MovePathNode(path,node,&delta_movement);

	if (ret != -1)
	{
		World_changed = 1;
		return true;
	}

	return false;
}



// Moves the specified BNode by the specified amount
bool ned_MoveBNode(int room,int node,float x,float y,float z)
{
	vector diff = {x,y,z};

	EBNode_Move(true, room, node, &diff);
	World_changed = 1;

	return true;
}

// Deletes the current BNode
void ned_DeleteBNode()
{
	int room,node,edge;
	Editor_state.GetCurrentBNode(&room,&node,&edge);

	EBNode_RemoveNode(room, node);
	World_changed = 1;

	EBNode_VerifyGraph();	
}

// Deletes an edge between two BNodes
void ned_DeleteEdge()
{
	int room1,node1,room2,node2,edge;

	Editor_state.GetCurrentBNode(&room1,&node1,&edge);
	int num_edges = Rooms[room1].bn_info.nodes[node1].num_edges;
	if (num_edges)
	{
		room2 = Rooms[room1].bn_info.nodes[node1].edges[edge].end_room;
		node2 = Rooms[room1].bn_info.nodes[node1].edges[edge].end_index;

		EBNode_RemoveEdge(node1, room1, node2, room2);	
		World_changed = 1;

		EBNode_VerifyGraph();	
	}
}

// Inserts a new BNode on the current edge or apart from all edges, depending on user input
void ned_InsertBNode(int roomnum,vector pos)
{
	int room1,node1,room2,node2,edge;
	int new_node;

	Editor_state.GetCurrentBNode(&room1,&node1,&edge);
	int num_edges = Rooms[room1].bn_info.nodes[node1].num_edges;
	if (num_edges)
	{
		room2 = Rooms[room1].bn_info.nodes[node1].edges[edge].end_room;
		node2 = Rooms[room1].bn_info.nodes[node1].edges[edge].end_index;

		if (AfxMessageBox("Would you like to create a node on the current edge? Answering NO will create a node with no edges.",MB_YESNO) == IDYES)
		{
			new_node = EBNode_InsertNodeOnEdge(node1, room1, node2, room2);	
			if (new_node != -1)
			{
				EBNode_VerifyGraph();	
				World_changed = 1;
			}
		}
		else
		{
			new_node = EBNode_AddNode(roomnum, &pos, true, true);
			if (new_node != -1)
			{
//				EBNode_VerifyGraph();	
				World_changed = 1;
			}
		}
	}
	else
	{
		new_node = EBNode_AddNode(roomnum, &pos, true, true);
		if (new_node != -1)
		{
//			EBNode_VerifyGraph();	
			World_changed = 1;
		}
	}
}

// Inserts an edge between the given BNodes
void ned_InsertEdge()
{
	int room1,node1,room2,node2,edge;

	Editor_state.GetCurrentBNode(&room1,&node1,&edge);
	int num_edges = Rooms[room1].bn_info.nodes[node1].num_edges;
	if (num_edges)
	{
		room2 = Rooms[room1].bn_info.nodes[node1].edges[edge].end_room;
		node2 = Rooms[room1].bn_info.nodes[node1].edges[edge].end_index;

		EBNode_AddEdge(node1, room1, node2, room2);	
		EBNode_VerifyGraph();	
		World_changed = 1;
	}
}

