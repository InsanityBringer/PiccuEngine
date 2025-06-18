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
#include "room.h"
#include "object.h"
#include "../editor/ERooms.h"
#include "ned_Trigger.h"
#include "../editor/selectedroom.h"
#include "mission.h"
#include "lighting.h"
#include "lightmap.h"
#include "lightmap_info.h"
#include "special_face.h"
#include "polymodel.h"
#include "globals.h"
#include "gamepath.h"
#include "boa.h"
#include "levelgoal.h"
#include "findintersection.h"
#include "matcen.h"
#include "game.h"

#include "neweditor.h"
#include "ned_HFile.h"
#include "ned_Util.h"
#include "ListDialog.h"

//vertices for the default room created by CreateNewMine()
vector default_room_verts[] = {	{ -10,  8, 20 },
											{  -5, 10, 20 },
											{   5, 10, 20 },
											{  10,  8, 20 },
											{  10, -8, 20 },
											{   5,-10, 20 },
											{  -5,-10, 20 },
											{ -10, -8, 20 },
											{ -10,  8,-20 },
											{  -5, 10,-20 },
											{   5, 10,-20 },
											{  10,  8,-20 },
											{  10, -8,-20 },
											{   5,-10,-20 },
											{  -5,-10,-20 },
											{ -10, -8,-20 } };

extern void AssignDefaultUVsToRoom(room *rp);
extern int sound_override_force_field;
extern int sound_override_glass_breaking;

//Where the center of the (mine's) world is
vector Mine_origin = {TERRAIN_WIDTH*(TERRAIN_SIZE/2), -100, TERRAIN_DEPTH*(TERRAIN_SIZE/2)};


//Create a default room for a new mine
room *CreateDefaultRoom()
{
	room *rp;
	int i;

	//Get a pointer to our room
	rp = CreateNewRoom(16,10,0);		//16 verts, 10 faces, normal room
	ASSERT(rp != NULL);

	//Set the vertices for the room
	for (i=0;i<16;i++)
		rp->verts[i] = default_room_verts[i] + Mine_origin;

	//Set the faces for the room
	InitRoomFace(&rp->faces[0],8);
	for (i=0;i<8;i++)
		rp->faces[0].face_verts[i] = i;

	InitRoomFace(&rp->faces[1],8);
	for (i=0;i<8;i++)
		rp->faces[1].face_verts[i] = 15-i;

	for (i=0;i<8;i++) {
		InitRoomFace(&rp->faces[i+2],4);
		rp->faces[i+2].face_verts[0] = i;
		rp->faces[i+2].face_verts[1] = i+8;
		rp->faces[i+2].face_verts[2] = ((i+1)%8)+8;
		rp->faces[i+2].face_verts[3] = (i+1)%8;
	}

	//Set normals, textures and UVLs for face
	for (i=0;i<10;i++) {
		if (! ComputeFaceNormal(rp,i))
			Int3();	//this is odd.  Get Matt!
		// HACK: No need to mark textures in use, because it's done by LevelTexInitLevel
		rp->faces[i].tmap = i+1;
		AssignDefaultUVsToRoomFace(rp,i);
	}

	return rp;
}


bool MoveVerts(room *rp, short *list, int num_verts, vector vec);

//Create a new mine with one segment, specified by filename
bool CreateNewMineFromRoom(char *filename)
{
	char name[_MAX_PATH];
	char ext[10];
	char path[_MAX_PATH*2];
	int i=0,n=0;
	object *objp;
	vector room_center;
	vector newpos;
	short vert_list[MAX_VERTS_PER_ROOM];

	//Get rid of old mine
	FreeAllRooms();
	FreeAllObjects();

	SplitPath(filename,path,name,ext);

	// Load the room, but not into the paletted rooms part of the room array
	n = AllocLoadRoom(filename,false,0);
	if(n<0)
	{
		OutrageMessageBox("Unable to load room!");
		return false;
	}
	ASSERT(Rooms[n].name == NULL);

	Curroomp = &Rooms[n];
	CreateNewMine(false);

	// Move the room to the mine's center (IMPORTANT!!)
	ComputeRoomBoundingSphere(&room_center,Curroomp); // ComputeRoomCenter(&room_center,Curroomp);
	for (i=0; i<Curroomp->num_verts; i++)
		vert_list[i] = i;
	MoveVerts(Curroomp,vert_list,Curroomp->num_verts,Mine_origin-room_center);
	for (i=0; i<=Highest_object_index; i++)
	{
		objp = &Objects[i];
		if (objp != NULL && objp->roomnum == 0)
		{
			newpos = objp->pos+Mine_origin-room_center;
			ObjSetPos(objp,&newpos,0,&objp->orient,false);
		}
	}
	World_changed = 1;

	return true;
}

//Create a new mine with one segment
void CreateNewMine(bool new_room)
{
	if (new_room)
	{
		//Get rid of old mine
		FreeAllRooms();
		FreeAllObjects();

		//Create a new room
		Curroomp = CreateDefaultRoom();
  		ASSERT(Curroomp != NULL);
	}

	//Reset misc. vars
	Curface = Curedge = Curvert = 0;
	Curportal = -1;

	//Say that this is a new mine
	World_changed = 1;		

	//Reinitialize the objects
	ResetObjectList();

	//Create the player and put him in the center of the fa
	CreatePlayerObject(ROOMNUM(Curroomp));

	//Look for player objects & set player starts
	FindPlayerStarts();

  	//Clear the marked room
	Markedroomp = NULL;

	//Clear the selected segments
	ClearRoomSelectedList();

	//Clear the placed room & group
	Placed_room = -1;
	Placed_group = NULL;

	//Reset the triggers
	Num_triggers = 0;
	Current_trigger = -1;

	// Reset the terrain
	ResetTerrain(1);

	// Reset terrain sound
	ClearTerrainSound();

	int i;
	BOA_AABB_checksum = BOA_mine_checksum = 0;
	for(i = 0; i < MAX_ROOMS; i++)
	{
		BOA_AABB_ROOM_checksum[i] = 0;
	}

	InitGamePaths();

	DestroyAllMatcens();

	Level_goals.CleanupAfterLevel();

	sound_override_force_field = -1;
	sound_override_glass_breaking = -1;

	for(i = 0; i < MAX_FORCE_FIELD_BOUNCE_TEXTURES; i++)
	{
		force_field_bounce_texture[i] = -1;
		force_field_bounce_multiplier[i] = 1.0f;
	}

	Level_powerups_ignore_wind = false;

	BNode_ClearBNodeInfo();
	FVI_always_check_ceiling = false;
	Ceiling_height = MAX_TERRAIN_HEIGHT;

	//Init level info
	strcpy(Level_info.name,"Unnamed");
	strcpy(Level_info.designer,"Anonymous");
	strcpy(Level_info.copyright,"Copyright (c) <year> <author>");
	strcpy(Level_info.notes,"");
}


CListDialog *lstats_dlg = NULL;

//Shows the stats for a level in a messagebox
void ShowLevelStats()
{
	char str[256];
	int n_rooms,n_rooms_external,n_faces,n_verts,n_objects,n_portals,n_doors,n_objects_outside;
	int n_object_faces,n_object_lightmap_faces;
	int i;
	room *rp;
	int bytes_wasted=0,spec_faces=0,lm_bytes=0;
	int total_volume_bytes=0;
	int num_redgoals=0,num_bluegoals=0,num_greengoals=0,num_yellowgoals=0,num_refueling=0;
	int num_sp1=0,num_sp2=0,num_sp3=0,num_sp4=0,num_sp5=0,num_sp6=0;

	ubyte lightmaps_used[MAX_LIGHTMAPS];

	object *objp;

	if (lstats_dlg == NULL)
	{
		lstats_dlg = new CListDialog;
		lstats_dlg->Create(IDD_LISTDIALOG);
	}
	else
		lstats_dlg->SetActiveWindow();
	lstats_dlg->SetWindowText("Level Stats");
	CListBox *lb = (CListBox *) lstats_dlg->GetDlgItem(IDC_LIST1);
	// Delete any strings that may be in the listbox
	while ( lb->DeleteString(0) != LB_ERR );
	CStatic *hdr = (CStatic *) lstats_dlg->GetDlgItem(IDC_HEADER_TEXT);
	hdr->SetWindowText("Level statistics:");
	CStatic *status = (CStatic *) lstats_dlg->GetDlgItem(IDC_STATUS_TEXT);
	status->SetWindowText("Gathering statistics...");

	//Count the number of rooms, and the number of faces & points per room
	n_rooms = n_rooms_external = n_faces = n_verts = n_doors = n_portals = 0;
	for (i=0,rp=Rooms;i<=Highest_room_index;i++,rp++)
	{
		if (rp->used) {
			n_rooms++;
			n_verts += rp->num_verts;
			n_faces += rp->num_faces;
			n_portals += rp->num_portals;
			if (rp->flags & RF_EXTERNAL)
				n_rooms_external++;
			else
			{
				// Get volume size of room
				total_volume_bytes+=GetVolumeSizeOfRoom (rp);
			}

			for (int t=0;t<rp->num_faces;t++)
			{
				face *fp=&rp->faces[t];
				if (fp->special_handle!=BAD_SPECIAL_FACE_INDEX && GameTextures[fp->tmap].flags & TF_SPECULAR && fp->lmi_handle!=BAD_LMI_INDEX)
					spec_faces++;
			}

			if (rp->flags & RF_DOOR)
				n_doors++;

			if (rp->flags & RF_SPECIAL1)
				num_sp1++;

			if (rp->flags & RF_SPECIAL2)
				num_sp2++;

			if (rp->flags & RF_SPECIAL3)
				num_sp3++;
			
			if (rp->flags & RF_SPECIAL4)
				num_sp4++;
			
			if (rp->flags & RF_SPECIAL5)
				num_sp5++;
			
			if (rp->flags & RF_SPECIAL6)
				num_sp6++;

			if (rp->flags & RF_GOAL1)
				num_redgoals++;

			if (rp->flags & RF_GOAL2)
				num_bluegoals++;

			if (rp->flags & RF_GOAL3)
				num_greengoals++;
			
			if (rp->flags & RF_GOAL4)
				num_yellowgoals++;
			
			if (rp->flags & RF_FUELCEN)
				num_refueling++;
			
		}
	}

	//Count the number of objects
	n_objects = n_object_faces = n_object_lightmap_faces = n_objects_outside = 0;
	for (i=0,objp=Objects;i<=Highest_object_index;i++,objp++)
	{
		if ((objp->type != OBJ_NONE) && (objp->type != OBJ_ROOM) && (objp->render_type == RT_POLYOBJ)) {
			n_objects++;
			if (OBJECT_OUTSIDE(objp))
				n_objects_outside++;
			//Count the number of faces in this object
			poly_model *pm;
			pm = GetPolymodelPointer(objp->rtype.pobj_info.model_num);
			for (int m=0;m<pm->n_models;m++) {
				n_object_faces += pm->submodel[m].num_faces;
				if (objp->lighting_render_type == LRT_LIGHTMAPS)
					n_object_lightmap_faces += pm->submodel[m].num_faces;
			}
		}
	}

	memset (lightmaps_used,0,MAX_LIGHTMAPS);
	for (i=0;i<MAX_LIGHTMAP_INFOS;i++)
	{
		if (!LightmapInfo[i].used)
			continue;
		if (LightmapInfo[i].type==LMI_DYNAMIC || LightmapInfo[i].type==LMI_TERRAIN)
			continue;

		lightmaps_used[LightmapInfo[i].lm_handle]=1;
	}

	for (i=0;i<MAX_LIGHTMAPS;i++)
	{
		if (lightmaps_used[i])
		{
			ushort *data=lm_data (i);
			int w=lm_w(i);
			int h=lm_h(i);

			for (int j=0;j<w*h;j++)
			{
				if (!(data[j] & OPAQUE_FLAG))
				{
					bytes_wasted+=2;
				}
				else
				{	
					lm_bytes+=2;
				}
			}
		}
	}

	// Interestingly, CListBox::AddString does not add the strings in the correct order, so we use CListBox::InsertString instead.
	sprintf(str,"%d Rooms (%d external)",n_rooms,n_rooms_external);
	lb->InsertString(-1,str);
	sprintf(str,"%d Faces",n_faces);
	lb->InsertString(-1,str);
	sprintf(str,"%d Vertices",n_verts);
	lb->InsertString(-1,str);
	sprintf(str,"%d Portals",n_portals/2);
	lb->InsertString(-1,str);
	sprintf(str,"%d Doors",n_doors);
	lb->InsertString(-1,str);
	sprintf(str,"%d Polygon Objects (%d inside, %d outside)",n_objects,n_objects-n_objects_outside,n_objects_outside);
	lb->InsertString(-1,str);
	sprintf(str,"%d Object Faces (%d with lightmaps)",n_object_faces,n_object_lightmap_faces);
	lb->InsertString(-1,str);
	sprintf(str,"%d Total lightmap faces",n_faces+n_object_lightmap_faces);
	lb->InsertString(-1,str);
	sprintf(str,"%d	Total volume bytes",total_volume_bytes);
	lb->InsertString(-1,str);
	sprintf(str,"%d Total bytes in lightmaps",lm_bytes);
	lb->InsertString(-1,str);
	sprintf(str,"%d Total specular faces",spec_faces);
	lb->InsertString(-1,str);
	sprintf(str,"%d Bytes wasted in lightmaps",bytes_wasted);
	lb->InsertString(-1,str);
	sprintf(str,"%d Energy Centers",num_refueling);
	lb->InsertString(-1,str);
	sprintf(str,"%d Red Goals",num_redgoals);
	lb->InsertString(-1,str);
	sprintf(str,"%d Blue Goals",num_bluegoals);
	lb->InsertString(-1,str);
	sprintf(str,"%d Green Goals",num_greengoals);
	lb->InsertString(-1,str);
	sprintf(str,"%d Yellow Goals",num_yellowgoals);
	lb->InsertString(-1,str);
	sprintf(str,"%d Special 1 Rooms",num_sp1);
	lb->InsertString(-1,str);
	sprintf(str,"%d Special 2 Rooms",num_sp2);
	lb->InsertString(-1,str);
	sprintf(str,"%d Special 3 Rooms",num_sp3);
	lb->InsertString(-1,str);
	sprintf(str,"%d Special 4 Rooms",num_sp4);
	lb->InsertString(-1,str);
	sprintf(str,"%d Special 5 Rooms",num_sp5);
	lb->InsertString(-1,str);
	sprintf(str,"%d Special 6 Rooms",num_sp6);
	lb->InsertString(-1,str);

	//Show message
	status->SetWindowText("Statistics gathered.");
}


int GetNumObjectsInRoom(room *rp)
{
	int num_objects = 0;

	if (rp->objects != -1)
	{
		object *objp = &Objects[rp->objects];
		if (IS_GENERIC(objp->type) || (objp->type == OBJ_SOUNDSOURCE) || objp->type == OBJ_PLAYER)
			num_objects++;

		while (objp->next != -1)
		{
			objp = &Objects[objp->next];
			if (IS_GENERIC(objp->type) || (objp->type == OBJ_SOUNDSOURCE) || objp->type == OBJ_PLAYER)
				num_objects++;
		}
	}

	return num_objects;
}


CListDialog *rstats_dlg = NULL;

//Shows the stats for a room in a messagebox
void ShowRoomStats(room *rp)
{
	char str[256];
	int n_faces,n_verts,n_objects,n_portals;
	int n_object_faces,n_object_lightmap_faces;
	int i;
	int bytes_wasted=0,spec_faces=0,lm_bytes=0;
	int total_volume_bytes=0;

//	ubyte lightmaps_used[MAX_LIGHTMAPS];

	object *objp;

	if (rstats_dlg == NULL)
	{
		rstats_dlg = new CListDialog;
		rstats_dlg->Create(IDD_LISTDIALOG);
	}
	else
		rstats_dlg->SetActiveWindow();
	rstats_dlg->SetWindowText("Room Stats");
	CListBox *lb = (CListBox *) rstats_dlg->GetDlgItem(IDC_LIST1);
	// Delete any strings that may be in the listbox
	while ( lb->DeleteString(0) != LB_ERR );
	CStatic *hdr = (CStatic *) rstats_dlg->GetDlgItem(IDC_HEADER_TEXT);
	hdr->SetWindowText("Room statistics:");
	CStatic *status = (CStatic *) rstats_dlg->GetDlgItem(IDC_STATUS_TEXT);
	status->SetWindowText("Gathering statistics...");

	//Count the number of faces & points in the room
	n_faces = n_verts = n_portals = 0;
	ASSERT(rp->used);
	n_verts = rp->num_verts;
	n_faces = rp->num_faces;
	n_portals = rp->num_portals;

	// Get volume size of room
//	total_volume_bytes=GetVolumeSizeOfRoom (rp);

	for (int t=0;t<rp->num_faces;t++)
	{
		face *fp=&rp->faces[t];
		if (fp->special_handle!=BAD_SPECIAL_FACE_INDEX && GameTextures[fp->tmap].flags & TF_SPECULAR && fp->lmi_handle!=BAD_LMI_INDEX)
			spec_faces++;
	}


	//Count the number of objects
	n_objects = n_object_faces = n_object_lightmap_faces = 0;
	n_objects = GetNumObjectsInRoom(rp);
	if (rp->objects != -1)
	{
		objp = &Objects[rp->objects];
		for (i=0; i<=n_objects; i++,objp=&Objects[objp->next])
		{
			ASSERT(objp->type != OBJ_ROOM);
			if ((objp->type != OBJ_NONE) && (objp->render_type == RT_POLYOBJ)) {
				//Count the number of faces in this object
				poly_model *pm;
				pm = GetPolymodelPointer(objp->rtype.pobj_info.model_num);
				for (int m=0;m<pm->n_models;m++) {
					n_object_faces += pm->submodel[m].num_faces;
					if (objp->lighting_render_type == LRT_LIGHTMAPS)
						n_object_lightmap_faces += pm->submodel[m].num_faces;
				}
			}
		}
	}
	else
		ASSERT(n_objects == 0);

/*
	memset (lightmaps_used,0,MAX_LIGHTMAPS);
	for (i=0;i<rp->num_faces;i++)
	{
		ASSERT(LightmapInfo[i].type!=LMI_TERRAIN);
		if (!LightmapInfo[rp->faces[i].lmi_handle].used)
			continue;
		if (LightmapInfo[rp->faces[i].lmi_handle].type==LMI_DYNAMIC)
			continue;

		lightmaps_used[LightmapInfo[rp->faces[i].lmi_handle].lm_handle]=1;
	}

	for (i=0;i<MAX_LIGHTMAPS;i++)
	{
		if (lightmaps_used[i])
		{
			ushort *data=lm_data (i);
			int w=lm_w(i);
			int h=lm_h(i);

			for (int j=0;j<w*h;j++)
			{
				if (!(data[j] & OPAQUE_FLAG))
				{
					bytes_wasted+=2;
				}
				else
				{	
					lm_bytes+=2;
				}
			}
		}
	}
*/

	// Interestingly, CListBox::AddString does not add the strings in the correct order, so we use CListBox::InsertString instead.
	sprintf(str,"%d Faces",n_faces);
	lb->InsertString(-1,str);
	sprintf(str,"%d Vertices",n_verts);
	lb->InsertString(-1,str);
	sprintf(str,"%d Portals",n_portals);
	lb->InsertString(-1,str);
	sprintf(str,"%d Polygon Objects",n_objects);
	lb->InsertString(-1,str);
	sprintf(str,"%d Object Faces (%d with lightmaps)",n_object_faces,n_object_lightmap_faces);
	lb->InsertString(-1,str);
	sprintf(str,"%d Total lightmap faces",n_faces+n_object_lightmap_faces);
	lb->InsertString(-1,str);
//	sprintf(str,"%d	Total volume bytes",total_volume_bytes);
//	lb->InsertString(-1,str);
//	sprintf(str,"%d Total bytes in lightmaps",lm_bytes);
//	lb->InsertString(-1,str);
	sprintf(str,"%d Total specular faces",spec_faces);
	lb->InsertString(-1,str);
//	sprintf(str,"%d Bytes wasted in lightmaps",bytes_wasted);
//	lb->InsertString(-1,str);

	//Show message
	status->SetWindowText("Statistics gathered.");
}
