/* 
* Descent 3 
* Copyright (C) 2024 Parallax Software
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

#ifndef CFILE_H
#define CFILE_H

#include <stdio.h>

#include "pstypes.h"

struct library;

//The structure for a CFILE
struct CFILE 
{
	char	*name;			//pointer to filename
	FILE 	*file;			//the file itself (on disk) or the HOG
	int	lib_handle;			//the handle of the library, or -1
	int	size;				//length of this file
	int	lib_offset;			//offset into HOG of start of file, or 0 if on disk
	int	position;			//current position in file
	int	flags;				//see values below
};

//Defines for cfile_error
#define CFE_READING		1
#define CFE_WRITING		2

//The structure thrown by a cfile error
struct cfile_error
{
	int	read_write;		//reading or writing?  See defines.
	char	*msg;		//the error message
	CFILE	*file;		//the file that got the error
};

//Flags for CFILE struct
#define CF_TEXT		1		//if this bit set, file is text
#define CF_WRITING	2		//if bit set, file opened for writing

//See if a file is in a hog
bool cf_IsFileInHog(char *filename, char *hogname);

//Opens a HOG file.  Future calls to cfopen(), etc. will look in this HOG.
//Parameters:  libname - the path & filename of the HOG file 
//NOTE:	libname must be valid for the entire execution of the program.  Therefore, it should either
//			be a fully-specified path name, or the current directory must not change.
//Returns: 0 if error, else library handle that can be used to close the library
int cf_OpenLibrary(const char *libname);

//Closes a library file.
//Parameters:  handle: the handle returned by cf_OpenLibrary()
void cf_CloseLibrary(int handle);

//Specify a directory to look in for files
//if ext==NULL, look in this directory for all files.  If ext is non-null,
//it is a NULL-terminated list of file extensions.  If extensions are
//specifed, the directory will only be searched for files that match
//one of the listed extensions.
int cf_SetSearchPath(const char *path,char *ext,...);

//Removes all search paths that have been added by cf_SetSearchPath
void cf_ClearAllSearchPaths(void);

//Opens a file for reading or writing
//If a path is specified, will try to open the file only in that path.
//If no path is specified, will look through search directories and library files.
//Parameters:	filename - the name if the file, with or without a path
//					mode - the standard C mode string
//Returns:		the CFile handle, or NULL if file not opened
CFILE *cfopen(const char *filename, const char *mode);

// Opens a file for reading in a library, given the library id.
// Works just like cfopen, except it assumes "rb" mode and forces the file to be 
// opened from the given library.  Returns the CFILE handle or NULL if file
// couldn't be found or open.
CFILE *cf_OpenFileInLibrary(const char *filename,int libhandle);

//Returns the length of the specified file
//Parameters: cfp - the file pointer returned by cfopen()
int cfilelength( CFILE *cfp );

//Closes an open CFILE.
//Parameters:  cfile - the file pointer returned by cfopen()
void cfclose( CFILE * cfp );

//Just like stdio fgetc(), except works on a CFILE
//Returns a char or EOF
int cfgetc( CFILE * cfp );

//Just like stdio fseek(), except works on a CFILE
int cfseek( CFILE *cfp, long int offset, int where );

//Just like stdio ftell(), except works on a CFILE
int cftell( CFILE * cfp );

//Returns true if at EOF
int cfeof(CFILE *cfp);

//return values for cfexist()
#define CF_NOT_FOUND		0
#define CF_ON_DISK		1
#define CF_IN_LIBRARY	2

// Tells if the file exists
// Returns non-zero if file exists.  Also tells if the file is on disk
//	or in a hog -  See return values in cfile.h
int cfexist( const char * filename );

//Reads the specified number of bytes from a file into the buffer
//DO NOT USE THIS TO READ STRUCTURES.  This function is for byte
//data, such as a string or a bitmap of 8-bit pixels.
//Returns the number of bytes read.
//Throws an exception of type (cfile_error *) if the OS returns an error on read
int cf_ReadBytes(ubyte *buf, int count, CFILE *cfp);

// The following functions read numeric vales from a CFILE.  All values are 
// stored in the file in Intel (little-endian) format.  These functions
// will convert to big-endian if required.
// These funtions will throw an exception of if the value cannot be read, 
// so do not call these if you don't require the data to be present.   

//Read and return an integer (32 bits)
//Throws an exception of type (cfile_error *) if the OS returns an error on read
int cf_ReadInt(CFILE *cfp);
int cf_ReadIntBE(CFILE *cfp);

//Read and return a short (16 bits)
//Throws an exception of type (cfile_error *) if the OS returns an error on read
short cf_ReadShort(CFILE *cfp);
short cf_ReadShortBE(CFILE *cfp);

//Read and return a byte (8 bits)
//Throws an exception of type (cfile_error *) if the OS returns an error on read
sbyte cf_ReadByte(CFILE *cfp);

//Read and return a float (32 bits)
//Throws an exception of type (cfile_error *) if the OS returns an error on read
float cf_ReadFloat(CFILE *cfp);

//Read and return a double (64 bits)
//Throws an exception of type (cfile_error *) if the OS returns an error on read
double cf_ReadDouble(CFILE *cfp);

//Reads a string from a CFILE.  If the file is type binary, this 
//function reads until a NULL or EOF is found.  If the file is text,
//the function reads until a newline or EOF is found.  The string is always
//written to the destination buffer null-terminated, without the newline.
//Parameters:  buf - where the string is written
//					n - the maximum string length, including the terminating 0
//					cfp - the CFILE pointer
//Returns the number of bytes in the string, before the terminator
//Does not generate an exception on EOF
int cf_ReadString(char * buf, size_t n, CFILE * cfp );

//Writes the specified number of bytes from a file into the buffer
//DO NOT USE THIS TO WRITE STRUCTURES.  This function is for byte
//data, such as a string or a bitmap of 8-bit pixels.
//Returns the number of bytes written.
//Throws an exception of type (cfile_error *) if the OS returns an error on write
int cf_WriteBytes(const ubyte *buf, int count, CFILE *cfp);

//Writes a null-terminated string to a file.  If the file is type binary, 
//the string is terminated in the file with a null.  If the file is type 
//text, the string is terminated with a newline.
//Parameters:	buf - pointer to the string
//					cfp = the CFILE pointer
//Returns the number of bytes written
//Throws an exception of type (cfile_error *) if the OS returns an error on write
int cf_WriteString(CFILE *cfp, const char *buf);

//Just like stdio fprintf(), except works on a CFILE
int cfprintf( CFILE *cfp, const char *format, ... );

// The following functions write numeric vales to a CFILE.  All values are 
// stored to the file in Intel (little-endian) format. 
// All these throw an exception if there's an error on write.


//Write an integer (32 bits)
//Throws an exception of type (cfile_error *) if the OS returns an error on write
void cf_WriteInt(CFILE *cfp,int i);

//Write a short (16 bits)
//Throws an exception of type (cfile_error *) if the OS returns an error on write
void cf_WriteShort(CFILE *cfp,short s);

//Write a byte (8 bits).  If the byte is a newline & the file is a text file, writes a CR/LF pair.
//Throws an exception of type (cfile_error *) if the OS returns an error on write
void cf_WriteByte(CFILE *cfp,sbyte b);

//Write a float (32 bits)
//Throws an exception of type (cfile_error *) if the OS returns an error on write
void cf_WriteFloat(CFILE *cfp,float f);

//Write a double (64 bits)
//Throws an exception of type (cfile_error *) if the OS returns an error on write
void cf_WriteDouble(CFILE *cfp,double d);

//Copies a file.  Returns TRUE if copied ok.  Returns FALSE if error opening either file.
//Throws an exception of type (cfile_error *) if the OS returns an error on read or write
// If copytime is nonzero, copies the filetime info as well
bool cf_CopyFile (char *dest,const char *src,int copytime=0);

//Checks to see if two files are different.
//Returns TRUE if the files are different, or FALSE if they are the same.
bool cf_Diff (const char *a,const char *b);

// Copies the file time from one file to another
void cf_CopyFileTime (char *dest,const char *src);


// Changes a files attributes (ie read/write only)
void cf_ChangeFileAttributes (const char *name,int attr);

//	rewinds cfile position
void cf_Rewind(CFILE *fp);

// Calculates a 32 bit CRC
unsigned int cf_GetfileCRC (char *src);
unsigned int cf_CalculateFileCRC (CFILE *fp);//same as cf_GetfileCRC, except works with CFILE pointers

//	the following cf_LibraryFind function are similar to the ddio_Find functions as they look
//	for files that match the wildcard passed in, however, this is to be used for hog files.
bool cf_LibraryFindFirst(int handle,const char *wildcard,char *buffer);
bool cf_LibraryFindNext(char *buffer);
void cf_LibraryFindClose(void);

#endif
