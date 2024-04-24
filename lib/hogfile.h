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

#ifndef HOGFILE_H
#define HOGFILE_H

#include "pstypes.h"

#define HOG_HDR_SIZE			64
#define HOG_TAG_STR			"HOG2"

struct tHogHeader
{
	unsigned int nfiles;			// number of files in header
	unsigned int file_data_offset;	// offset in file to filedata.
};

struct tHogFileEntry
{
	char name[PSFILENAME_LEN + 1];	// file name
	unsigned int flags;				// extra info
	unsigned int len;				// length of file
	unsigned int timestamp;			// time of file.
};

#define HOGMAKER_ERROR			0		// Incorrect number of files passed in
#define HOGMAKER_OK				1		//	Hog file was created successfully
#define HOGMAKER_MEMORY			2		// Could not allocated hog entry table
#define HOGMAKER_OUTFILE		3		// Error occurred writing to output hog file
#define HOGMAKER_INFILE			4		// An input file could not be found (filename is stored in hogerr_filename)
#define HOGMAKER_COPY			5		// An error occurred copying an input file into the hog file
#define HOGMAKER_OPENOUTFILE	6		// The specified hog file could not be opened for output

// Used to return filenames involved in a NewHogFile() error
extern char hogerr_filename[PSPATHNAME_LEN];

int NewHogFile(const char *hogname, int nfiles, const char **filenames);
bool ReadHogHeader(FILE *fp, tHogHeader *header);
bool ReadHogEntry(FILE *fp, tHogFileEntry *entry);
bool WriteHogEntry(FILE *fp, tHogFileEntry *entry);
bool FileCopy(FILE *ofp,FILE *ifp,int length);

int CreateNewHogFile(const char *hogname, int nfiles, const char **filenames,
					 void(* UpdateFunction)(char *));

// returns hog cfile info, using a library handle opened via cf_OpenLibrary.
bool cf_ReadHogFileEntry(int library, const char *filename, tHogFileEntry *entry, int *fileoffset);

#endif
