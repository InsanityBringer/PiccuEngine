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
 

//Returns true if a room is in the selected list
int IsRoomSelected(int roomnum);

//Adds a room to the selected list, if it's not already there
void AddRoomToSelectedList(int roomnum);

//Removes a room from the selected list, if it's there
void RemoveRoomFromSelectedList(int roomnum);

//Empties the selected list
void ClearRoomSelectedList();

//Adds to selected list if not already in there, else removes from list
//Returns:	1 if room was selected, 0 if un-selected
int ToggleRoomSelectedState(int roomnum);

//Add all the connected room to the selected list
//Parameters:	roomnum - the starting room
//Returns:		the number of rooms added to the list
int SelectConnectedRooms(int roomnum);

//Save the (user's) selected list so that an internal function can use it
//You must call RestoreSelectedList() when you're done with the selected list
void SaveRoomSelectedList();

//Restore a previously-saved selected list.  You must have previously called SaveSelectedList()
void RestoreRoomSelectedList();

