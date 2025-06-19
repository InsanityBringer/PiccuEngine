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
#include "vecmat.h"
#include "globals.h"
#include "ned_Util.h"

//Returns a pointer to enough spaces to right-justify a number that's printed after the spaces
char *IntSpacing(int i)
{
	static char *spaces = "               ";

	i = abs(i);

	int n;
	for (n=1;i >= 10;n++)
		i /= 10;

	ASSERT(n <= 20);

	return spaces+n*2+n/2;
}


//Strips leading and trailing spaces from a string
//Returns true if spaces were stripped
bool StripLeadingTrailingSpaces(char *s)
{
	char *t;
	bool stripped = 0;

	//Look for leading spaces
	t = s;
	while (*t == ' ')
		t++;

	//Leading spaces found, so copy string over spaces
	if (t != s) {
		strcpy(s,t);
		stripped = 1;
	}

	//Strip any trailing spaces
	for (t = s + strlen(s) - 1;(t >= s) && (*t == ' ');t--) {
		*t = 0;
		stripped = 1;
	}

	return stripped;
}



// Split a pathname into its component parts
void SplitPath(const char* srcPath, char* path, char* filename, char* ext)
{
	char drivename[_MAX_DRIVE], dirname[_MAX_DIR];

	_splitpath(srcPath, drivename, dirname, filename, ext);

	if (path) 
		sprintf(path, "%s%s", drivename, dirname);
}




static int TotalArgs=0;
char GameArgs[MAX_ARGS][MAX_CHARS_PER_ARG];

// Gathers all arguments
void GatherArgs (char *str)
{
	int i,t,curarg=1;
	int len=strlen(str);
	bool gotquote = false;

	for (t=0,i=0;i<len;i++)
	{
		if(t==0)
		{
			if(str[i]=='\"')
			{
				gotquote = true;
				continue;
			}
		}
		if(gotquote)
		{
			if (str[i]!='\"' && str[i]!=0)
				GameArgs[curarg][t++]=str[i];
			else
			{
				GameArgs[curarg][t]=0;
				t=0;
				mprintf ((0,"Arg %d is %s\n",curarg,GameArgs[curarg]));
				gotquote = false;
				curarg++;
			}
		}
		else
		{
			if (str[i]!=' ' && str[i]!=0)
				GameArgs[curarg][t++]=str[i];
			else
			{
				GameArgs[curarg][t]=0;
				t=0;
				mprintf ((0,"Arg %d is %s\n",curarg,GameArgs[curarg]));
				curarg++;
			}
		}
	}

	GameArgs[curarg][t]=0;
	curarg++;
	TotalArgs=curarg;
}
 
// Returns index of argument sought, or 0 if not found
int FindArg (char *which)
 {
  int i;
  
  for (i=1;i<=TotalArgs;i++)
   if (stricmp (which,GameArgs[i])==0)
     return (i);

  return (0);
 }



void EnableTabControls(CWnd *parentwnd, bool enable, int first_id, int last_id)
{
	int old_id, id = 0;
	CWnd *pwnd = parentwnd->GetDlgItem(first_id);

	while (id != last_id)
	{
		if (last_id != -1)
			id = pwnd->GetDlgCtrlID();
		else
		{
			old_id = id;
			id = pwnd->GetDlgCtrlID();
			if (id == old_id)
				break; // we've reached the last dialog item
		}

		pwnd->EnableWindow(enable);
		pwnd = parentwnd->GetNextDlgTabItem(pwnd);
	}
}


void EnableTabControls(HWND hDlg, bool enable, int first_id, int last_id)
{
	int old_id, id = 0;
	HWND hwnd = ::GetDlgItem(hDlg,first_id);

	while (id != last_id)
	{
		if (last_id != -1)
			id = ::GetDlgCtrlID(hwnd);
		else
		{
			old_id = id;
			id = ::GetDlgCtrlID(hwnd);
			if (id == old_id)
				break; // we've reached the last dialog item
		}

		::EnableWindow(hwnd,enable);
		hwnd = ::GetNextDlgTabItem(hDlg,hwnd,FALSE);
	}
}


