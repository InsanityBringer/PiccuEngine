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
 

#include "AppDatabase.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>

#include "mono.h"
#include "pserror.h"

//	Construction and destruction.

oeWin32AppDatabase::oeWin32AppDatabase()
{
	bool res;

	hCurKey = 0;

	hBaseKey = (unsigned)HKEY_CURRENT_USER;

//	create outrage entertainment key
	lstrcpy(m_Basepath, "SOFTWARE\\Outrage");
	res = lookup_record(m_Basepath);
	if (!res) {
		res = create_record(m_Basepath);
		if (!res) {
			mprintf((1, "Unable to create registry directory.\n"));
		}
	}
}


oeWin32AppDatabase::oeWin32AppDatabase(oeWin32AppDatabase *parent)
{
	hCurKey = 0;
	hBaseKey = parent->hCurKey;

}


oeWin32AppDatabase::~oeWin32AppDatabase()
{
	if (hCurKey) {
		RegCloseKey((HKEY)hCurKey);
	}
}


//	Record functions
//		these are actual folders of information

//	creates an empty classification or structure where you can store information
bool oeWin32AppDatabase::create_record(const char *pathname)
{
	LONG lres;
	HKEY hkey;
	DWORD disp;

	assert(hBaseKey);

	lres = RegCreateKeyEx((HKEY)hBaseKey, pathname, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
					&hkey, &disp);
	if (lres != ERROR_SUCCESS) {
		mprintf((1, "Unable to create key %s (%x)\n", pathname, lres));
		return 0;
	}

	hCurKey = (unsigned)hkey;

	return 1;
}


//	set current database focus to a particular record
bool oeWin32AppDatabase::lookup_record(const char *pathname)
{
	LONG lres;
	HKEY hkey;

	assert(hBaseKey);

	if (hCurKey) {
		RegCloseKey((HKEY)hCurKey);
	}

	lres = RegOpenKeyEx((HKEY)hBaseKey, pathname, 0, KEY_READ | KEY_WRITE | KEY_EXECUTE, &hkey);
	if (lres != ERROR_SUCCESS) {
		mprintf((1, "Unable to open key %s (%x)\n", pathname, lres));
		return 0;
	}

	hCurKey = (unsigned)hkey;

	return 1;
}


//	read either a string from the current record
bool oeWin32AppDatabase::read(const char *label, char *entry, int *entrylen)
{
	LONG lres;
	DWORD type;

	assert(hBaseKey);
	assert(label != NULL);
	assert(entry != NULL);
	assert(entrylen != NULL);

#ifdef GAMEGAUGE
	//We don't want to read the registry for game gauge
	return 0;
#endif

	lres = RegQueryValueEx((HKEY)hCurKey, label, NULL, &type, (LPBYTE)entry, (LPDWORD)entrylen);

	assert(type != REG_DWORD);

	if (lres != ERROR_SUCCESS) {
		mprintf((1, "Unable to query str key %s (%x)\n", label, lres));
		return 0;
	}
	return 1;
}

//read a variable-sized integer from the current record
bool oeWin32AppDatabase::read(const char *label, void *entry, int wordsize)
{
	LONG lres;
	DWORD len = 4;
	DWORD type;
	int t;

	assert(hBaseKey);
	assert(label != NULL);
	assert(entry != NULL);

#ifdef GAMEGAUGE
	//We don't want to read the registry for game gauge
	return 0;
#endif

	lres = RegQueryValueEx((HKEY)hCurKey, label, NULL, &type, (LPBYTE) &t, &len);

	assert(len == 4);

	if (lres != ERROR_SUCCESS) {
		mprintf((1, "Unable to query int key %s (%x)\n", label, lres));
		return 0;
	}

	assert(type == REG_DWORD);

	switch (wordsize) {
		case 1: *((char *) entry) = (char) t; break;
		case 2: *((short *) entry) = (short) t; break;
		case 4: *((int *) entry) = (int) t; break;
		default: Int3(); break;		//invalid word size
	}

	return 1;
}

bool oeWin32AppDatabase::read(const char *label, bool *entry)
{
	int t;

#ifdef GAMEGAUGE
	//We don't want to read the registry for game gauge
	return 0;
#endif

	if (read(label,&t,sizeof(t))) {
		*entry = (t != 0);
		return 1;
	}
	else
		return 0;
}

//	write either an integer or string to a record.
bool oeWin32AppDatabase::write(const char *label, const char *entry, int entrylen)
{
	LONG lres;

	assert(hBaseKey);
	assert(label != NULL);
	assert(entry != NULL);
//	assert(entrylen > 0);

	lres = RegSetValueEx((HKEY)hCurKey, label, 0, REG_SZ, (LPBYTE)entry, entrylen);

	if (lres != ERROR_SUCCESS) {
		mprintf((1, "Unable to write str key %s (%x)\n", label, lres));
		return 0;
	}
	return 1;
}


bool oeWin32AppDatabase::write(const char *label, int entry)
{
	LONG lres;

	assert(hBaseKey);
	assert(label != NULL);

	lres = RegSetValueEx((HKEY)hCurKey, label, 0, REG_DWORD, (LPBYTE) &entry, sizeof(int));

	if (lres != ERROR_SUCCESS) {
		mprintf((1, "Unable to write int key %s (%x)\n", label, lres));
		return 0;
	}

	return 1;
}

// get the current user's name from the os
void oeWin32AppDatabase::get_user_name(char* buffer, ulong* size)
{
	GetUserName (buffer,size);
}


/////////////////////////////////////////////////////////////////////////////////
// pass name of dll which contains desired language
// NULL library is the default resource DLL

static HINSTANCE hThisResourceModule = NULL;

bool win32_SetResourceDLL(const char *libname)
{
	if (hThisResourceModule) {
		FreeLibrary(hThisResourceModule);
		hThisResourceModule = NULL;
	}

	if (libname) {
		hThisResourceModule = LoadLibrary(libname);
	}

	return (hThisResourceModule) ? true : false;
}


// returns a string from the current resource
bool win32_GetStringResource(int txt_id, char *buffer, int buflen)
{
	if (LoadString(hThisResourceModule, txt_id, buffer, buflen)) {
		buffer[buflen-1] = 0;
		return true;
	}
	strncpy(buffer, "!!!ERROR MISSING DLL STRING!!!", buflen-1);
	buffer[buflen-1] = 0;
	return false;
}


