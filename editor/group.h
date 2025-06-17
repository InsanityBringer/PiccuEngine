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
 

#ifndef _GROUP_H
#define _GROUP_H

//Define room, so we don't have to include room.h
struct room;
struct object;
struct trigger;

//A group.  Used for cut, paste, copy, etc.
typedef struct group {
	int  		nrooms;			//number of rooms in this group
	room 		*rooms;			//pointer to list of rooms
	int  		attachroom;		//which room is attached when pasting
	int  		attachface;		//which face is attached when pasting
	int		nobjects;		//how many objects
	int		ndoors;			//how many doors
	object	*objects;		//pointer to list of objects
	int		ntriggers;		//how many triggers
	trigger	*triggers;		//pointer to list of triggers
} group;


//Free a group.
//Parameters:	g - the group to be freed 
void FreeGroup(group *g);

//Copy the given list of rooms to a group
//Parameters:	nrooms - the number of rooms in list
//					roomnums - pointer to list of room numbers
//					attachroom, attachface - where group attaches when pasted 
//Returns:		pointer to group
group *CopyGroup(int nrooms,int *roomnums,int attachroom,int attachface);

//Delete the given list of rooms
//Parameters:	nrooms - the number of rooms in list
//					roomnums - pointer to list of room numbers
void DeleteGroup(int nrooms,int *roomnums);

//Place the given group at the specified room face
//The function merely causes the group to be drawn in the editor, allowing the user to line it up
//before attaching it.  The function AttachGroup() must be called to do the actual attachment.
//Parameters:	destroomp, destside - where to place the group
//					g - the group to place
void PlaceGroup(room *destroomp,int destface,group *g);

//Place the given group at the specified terrain cell
//The function merely causes the group to be drawn in the editor, allowing the user to line it up
//before attaching it.  The function AttachGroup() must be called to do the actual attachment.
//Parameters:	cellnum - where to place the group
//					g - the group to place
void PlaceGroupTerrain(int cellnum,group *g,bool align_to_terrain);

//Attach the already-placed group
void AttachGroup();

//Saves a group to disk in the given filename
void SaveGroup(char *filename,group *g);

//Loads a group from disk
//Returns:	pointer to the group loaded
group *LoadGroup(char *filename);

#endif	//ifdef _GROUP_H

