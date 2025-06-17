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
 
#ifndef NED_UTIL_HEADER_INCLUDED
#define NED_UTIL_HEADER_INCLUDED

//Returns a pointer to enough spaces to right-justify a number that's printed after the spaces
char *IntSpacing(int i);

//Strips leading and trailing spaces from a string
//Returns true if spaces were stripped
bool StripLeadingTrailingSpaces(char *s);

// Split a pathname into its component parts
void SplitPath(const char* srcPath, char* path, char* filename, char* ext);

#define MAX_ARGS			15
#define MAX_CHARS_PER_ARG	100

extern char GameArgs[MAX_ARGS][MAX_CHARS_PER_ARG];

// Gathers all arguments
void GatherArgs (char *str);

// Returns index of argument sought, or 0 if not found
int FindArg (char *which);

void EnableTabControls(CWnd *parentwnd, bool enable, int first_id, int last_id = -1);
void EnableTabControls(HWND hDlg, bool enable, int first_id, int last_id);

#endif