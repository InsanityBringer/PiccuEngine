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
 

#include "room.h"

struct object;

//Set the viewer in the specified room facing the specified face
//If room_center is true, put viewer at the center of the room facing the face
//If room_center is false, put the viewer directly in front of the selected face
//If the room is external, put the viewer a distance away from the room, 
//facing either the center (if room_center is true) or the specified face
void SetViewerFromRoomFace(room *roomp,int facenum,bool room_center);

//Returns the number (not the id) of the current viewer, in the range 0..MAX_VIEWERS
int GetViewerNum();

//Creates a new viewer object.  Copies position & orientation from the current viewer
void CreateNewViewer();

//Select next viewer
void SelectNextViewer();

//Deletes the current viewer object
void DeleteViewer();

//Sets the viewer object for the editor, creating if not already in mine
void SetEditorViewer();

//Finds a specific viewer object it one exists
//Parameters:	id - which viewer id
//Returns:		object number of a viewer object, or -1 if none
int FindViewerObject(int id);

//Finds a viewer object if one exists.  
//Starts looking at the specified id and searches through all possible ids
//Parameters:	id - which viewer id
//					view_mode - if -1, find any viewer, else find one that matches view mode
//Returns:		object number of a viewer object, or -1 if none
int FindNextViewerObject(int id,int view_mode);

//Moves the room viewer to the origin, if there is a room viewer
void ResetRoomViewer();
