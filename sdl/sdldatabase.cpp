/*
* Descent 3: Piccu Engine
* Copyright (C) 2024 SaladBadger
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "appdatabase.h"

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <SDL3/SDL_filesystem.h>
#include <string>

#include "appdatabase.h"
#include "linux/lnxdatabase.h"
#include "pserror.h"
#include "mono.h"
#include "pserror.h"
#include "registry.h"
#include "enginebrand.h"
#include "ddio.h"

#ifdef WIN32
#define REGISTRY_FILENAME ENGINE_NAME_NO_SPACE".registry"
#else
#define REGISTRY_FILENAME "."ENGINE_NAME_NO_SPACE"Registry"
#endif

//Construction and destruction.

oeSDLAppDatabase::oeSDLAppDatabase()
{
    //Open up the database file, for reading, read in all data and keep it in memory
    //then close the database 

#ifdef WIN32
    const char* prefPath = (char*)SDL_GetUserFolder(SDL_FOLDER_SAVEDGAMES);
    std::string filename = prefPath;
    filename += '\\';
    filename += ENGINE_NAME;
    filename += '\\';

    ddio_CreateDir(filename.c_str());

    filename += REGISTRY_FILENAME;

#else
#error "oeSDLAppDatabase::oeSDLAppDatabase: Implment more suitable path for non-windows platforms here"
#endif

    //sprintf(fileName, "%s/%s", prefPath, REGISTRY_FILENAME);

    database = new SDLRegistry(filename.c_str());
    database->Import();
    create_record("Version");
}

oeSDLAppDatabase::oeSDLAppDatabase(oeSDLAppDatabase* parent)
{
    char name[256];
    SDLRegistry* db = parent->GetSystemRegistry();
    db->Export();
    database = new SDLRegistry("");
    db->GetSystemName(name);
    database->SetSystemName(name);
    database->Import();
}

oeSDLAppDatabase::~oeSDLAppDatabase()
{
    if (database) {
        database->Export();
        delete database;
        return;
    }

    mprintf((0, "Can't Export Database Since It's Not There!\n"));
}

SDLRegistry* oeSDLAppDatabase::GetSystemRegistry()
{
    return database;
}

//Record functions
//these are actual folders of information

//creates an empty classification or structure where you can store information
bool oeSDLAppDatabase::create_record(const char* pathname)
{
    ASSERT(pathname != NULL);
    if (database) {
        database->CreateKey((char*)pathname);
        return true;
    }
    mprintf((0, "Can't CreateKey because database NULL\n"));
    return false;
}


//set current database focus to a particular record
bool oeSDLAppDatabase::lookup_record(const char* pathname)
{
    ASSERT(pathname);
    if (database) {
        return database->LookupKey((char*)pathname);
    }
    mprintf((0, "Can't lookup key because database NULL\n"));
    return false;
}


//read either a string from the current record
bool oeSDLAppDatabase::read(const char* label, char* entry, int* entrylen)
{
    ASSERT(label);
    ASSERT(entry);
    ASSERT(entrylen);
    if (!database) {
        mprintf((0, "Can't read record because database NULL\n"));
        return false;
    }

    //See if it exists
    int size = database->GetDataSize((char*)label);
    if (size > 0)
        *entrylen = size - 1;//-1 because of NULL
    else
        return false;

    //ok it exists, no look it up
    database->LookupRecord((char*)label, entry);
    return true;
}

//read a variable-sized integer from the current record
bool oeSDLAppDatabase::read(const char* label, void* entry, int wordsize)
{
    ASSERT(label);
    ASSERT(entry);
    if (!database) {
        mprintf((0, "Can't read record because Database NULL\n"));
        return false;
    }

    int size = database->GetDataSize((char*)label);
    if (size == 0)
        return false;

    //ok so it does exist
    int data;
    database->LookupRecord((char*)label, &data);

    switch (wordsize) {
    case 1:
        *((unsigned char*)entry) = (unsigned char)data;
        break;
    case 2:
        *((unsigned short*)entry) = (unsigned short)data;
        break;
    case 4:
        *((unsigned int*)entry) = (unsigned int)data;
        break;
    default:
        mprintf((0, "Unable to read key %s, unsupported size", label));
        return false;
        break;
    }
    return true;
}

bool oeSDLAppDatabase::read(const char* label, bool* entry)
{
    bool data;
    if (!read(label, &data, sizeof(bool)))
        return false;

    *entry = (data != 0) ? true : false;
    return true;
}

//write either an integer or string to a record.
bool oeSDLAppDatabase::write(const char* label, const char* entry, int entrylen)
{
    ASSERT(label);
    ASSERT(entry);
    if (!database) {
        mprintf((0, "Can't write record because database NULL\n"));
        return false;
    }

    return database->CreateRecord((char*)label, REGT_STRING, (void*)entry);
}


bool oeSDLAppDatabase::write(const char* label, int entry)
{
    ASSERT(label);
    if (!database) {
        mprintf((0, "Can't write record because database NULL\n"));
        return false;
    }
    return database->CreateRecord((char*)label, REGT_DWORD, &entry);
}

// get the current user's name from the os
//[ISB] This is only used for the tablefile dev sync stuff and maybe editor?
//it basically doesn't need to exist

static const char bogusstr[] = "bogususer";
void oeSDLAppDatabase::get_user_name(char* buffer, ulong* size)
{
    if (*size <= strlen(bogusstr))
        strcpy(buffer, bogusstr);
}

