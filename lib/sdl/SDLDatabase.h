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

#pragma once

class SDLRegistry;

class oeSDLAppDatabase : public oeAppDatabase
{

protected:
	SDLRegistry* database;
public:
	oeSDLAppDatabase();
	oeSDLAppDatabase(oeSDLAppDatabase* parent);
	virtual ~oeSDLAppDatabase();
	SDLRegistry* GetSystemRegistry();

	//	creates an empty classification or structure where you can store information
	virtual bool create_record(const char* pathname);

	//	set current database focus to a particular record
	virtual bool lookup_record(const char* pathname);

	//	read either an integer or string from the current record
	virtual bool read(const char* label, char* entry, int* entrylen);
	virtual bool read(const char* label, void* entry, int wordsize);	 //read a variable-size int
	virtual bool read(const char* label, bool* entry);

	//	write either an integer or string to a record.
	virtual bool write(const char* label, const char* entry, int entrylen);
	virtual bool write(const char* label, int entry);

	// get the current user's name.
	virtual void get_user_name(char* buffer, ulong* size);
};

//Handy macro to read an int without having to specify the wordsize
#define read_int(label,varp) read(label,varp,sizeof(*varp))

//Macro to write a string
#define write_string(label,varp) write(label,varp,strlen(varp))