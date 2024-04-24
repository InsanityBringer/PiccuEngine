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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>	
#ifndef __LINUX__
//Non-Linux Builds
#include <io.h>
#else
//Linux Builds
#include "linux/linux_fix.h"
#endif
#include "byteswap.h"
#include "hogfile.h"
#include "pstypes.h"
#include "Macros.h"
#include "mem.h"
#include "ddio.h"
/*	HOG FILE FORMAT v2.0 
		
		HOG_TAG_STR			[strlen()]
		NFILES				[int32]
		HDRINFO				[HOG_HDR_SIZE]
		FILE_TABLE			[sizeof(FILE_ENTRY) * NFILES]
		FILE 0				[filelen(FILE 0)]
		FILE 1				[filelen(FILE 1)]
		.
		.
		.
		FILE NFILES-1		[filelen(NFILES -1)]
*/
char hogerr_filename[PSPATHNAME_LEN];  // Used by NewHogFile() to return errors
////////////////////////////////////////////////////////////////////////
/*	FileCopy
		used to copy one file to another.
*/
bool FileCopy(FILE *ofp,FILE *ifp,int length)
{
	#define BUFFER_SIZE			(1024*1024)			//	1 meg
	char *buffer;
	buffer = (char *)mem_malloc(BUFFER_SIZE);
	if (!buffer) 
		return false;

	while (length) 
	{
		size_t n,read_len;
		read_len = min(length,(int)BUFFER_SIZE);
		n = fread( buffer, 1, read_len, ifp );
		if ( n != read_len )	
		{
			mem_free(buffer);
			return false;
		}

		if (fwrite( buffer, 1, read_len, ofp) != read_len )	
		{
			mem_free(buffer);
			return false;
		}
		length -= read_len;
	}
	mem_free(buffer);
	return true;
}

//[ISB] reads a 32-bit LE int raw
static bool cf_ReadInt32Raw(FILE* fp, int& val)
{
	ubyte b[4];
	if (fread(b, 1, 4, fp) != 4)
		return false;
	val = b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
	return true;
}

//[ISB] reads a 32-bit LE uint raw
static bool cf_ReadUInt32Raw(FILE* fp, unsigned int& val)
{
	ubyte b[4];
	if (fread(b, 1, 4, fp) != 4)
		return false;
	val = b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
	return true;
}

bool ReadHogHeader(FILE *fp, tHogHeader *header)
{
	bool res=false;
	cf_ReadUInt32Raw(fp, header->nfiles);
	res = cf_ReadUInt32Raw(fp, header->file_data_offset);
	
	if (res)
		return true;
	else
		return false;
}

bool ReadHogEntry(FILE *fp, tHogFileEntry *entry)
{
	bool res=false;
	fread(entry->name, sizeof(char), PSFILENAME_LEN + 1, fp);
	cf_ReadUInt32Raw(fp, entry->flags);
	cf_ReadUInt32Raw(fp, entry->len);
	res = cf_ReadUInt32Raw(fp, entry->timestamp);
	
	if (res) 
		return true;
	else
		return false;
}

static bool cf_WriteUInt32Raw(FILE* fp, unsigned int value)
{
	ubyte b[4];
	b[0] = value & 255;
	b[1] = (value >> 8) & 255;
	b[2] = (value >> 16) & 255;
	b[3] = (value >> 24) & 255;

	if (fwrite(b, 1, 4, fp) != 4)
		return false;
	return true;
}

bool WriteHogEntry(FILE *fp, tHogFileEntry *entry)
{
	bool res = false;
	fwrite(entry->name, sizeof(char), PSFILENAME_LEN+1, fp);
	res = cf_WriteUInt32Raw(fp, entry->flags);
	res = cf_WriteUInt32Raw(fp, entry->len);
	res = cf_WriteUInt32Raw(fp, entry->timestamp);
	
	if (res) 
		return true;
	else
		return false;
}

////////////////////////////////////////////////////////////////////////
//	create new hog file
int NewHogFile(const char *hogname, int nfiles, const char **filenames)
{
	unsigned i;
	int table_pos;
	FILE *hog_fp;
	tHogHeader header;
	tHogFileEntry *table;
	char ext[_MAX_EXT];
	hogerr_filename[0]='\0';

	//allocate file table
	if (nfiles <= 0) 
		return HOGMAKER_ERROR;

	table = new tHogFileEntry[nfiles];
	if (!table) 
		return HOGMAKER_MEMORY;
	//create new file
	hog_fp = fopen( hogname, "wb" );
	if ( hog_fp == NULL )		
	{
		delete[] table;
		strcpy(hogerr_filename,hogname);
		return HOGMAKER_OPENOUTFILE;
	}

	//write the tag
	if (!fwrite(HOG_TAG_STR, strlen(HOG_TAG_STR), 1, hog_fp ))	
	{
		delete[] table;
		fclose(hog_fp);
		strcpy(hogerr_filename,hogname);
		return HOGMAKER_OUTFILE;
	}

	//write number of files
	ubyte filler = 0xff;
	header.nfiles = (unsigned)nfiles;
	header.file_data_offset = strlen(HOG_TAG_STR) + HOG_HDR_SIZE + (sizeof(tHogFileEntry) * header.nfiles);

	if (!cf_WriteUInt32Raw(hog_fp, header.nfiles))
	{
		delete[] table;
		fclose(hog_fp);
		strcpy(hogerr_filename,hogname);
		return HOGMAKER_OUTFILE;
	}

	if (!cf_WriteUInt32Raw(hog_fp, header.file_data_offset))
	{
		delete[] table;
		fclose(hog_fp);
		strcpy(hogerr_filename,hogname);
		return HOGMAKER_OUTFILE;
	}

	//write out filler
	for(i = 0; i < HOG_HDR_SIZE-sizeof(tHogHeader); i++)
		if (!fwrite(&filler,sizeof(ubyte),1,hog_fp))
		{
			delete[] table;
			fclose(hog_fp);
			strcpy(hogerr_filename,hogname);
			return HOGMAKER_OUTFILE;
		}

	//save file position of index table and write out dummy table
	table_pos = strlen(HOG_TAG_STR) + HOG_HDR_SIZE;
	memset(&table[0], 0, sizeof(table[0]));
	for (i = 0; i < header.nfiles; i++)
	{
		if (!WriteHogEntry(hog_fp, &table[0])) 
		{
			delete[] table;
			fclose(hog_fp);
			strcpy(hogerr_filename,hogname);
			return HOGMAKER_OUTFILE;
		}
	}

	//write files (& build index)
	for (i=0;i<header.nfiles;i++) 
	{
		FILE *ifp;
		#if defined(__LINUX__)
		struct stat mystat;
		#else
		struct _stat32 mystat;
		#endif
		
		ifp = fopen(filenames[i],"rb");
		if ( ifp == NULL )
		{
			delete[] table;
			fclose(hog_fp);
			strcpy(hogerr_filename,filenames[i]);
			return HOGMAKER_INFILE;
		}

		ddio_SplitPath(filenames[i],NULL,table[i].name,ext);
		_fstat32(fileno(ifp), &mystat);
	
		strcat(table[i].name,ext);
		table[i].flags = 0;
		table[i].len = _filelength(fileno(ifp));
		table[i].timestamp = mystat.st_mtime;
		if (!FileCopy(hog_fp,ifp,table[i].len)) 
		{
			delete[] table;
			fclose(hog_fp);
			strcpy(hogerr_filename,filenames[i]);
			return HOGMAKER_COPY;
		}
		fclose(ifp);
	}

	//now write the real index
	fseek(hog_fp,table_pos,SEEK_SET);
	for (i = 0; i < header.nfiles; i++)
	{
		if (!WriteHogEntry(hog_fp, &table[i])) 
		{
			delete[] table;
			fclose(hog_fp);
			strcpy(hogerr_filename,hogname);
			return HOGMAKER_OUTFILE;
		}
	}
//	cleanup
	fclose( hog_fp );	
	delete[] table;
	return HOGMAKER_OK;
}

// A modifed version of NewHogFile()
// This one also takes a pointer to a function that will perform
// progress updates (for the user)
int CreateNewHogFile(const char *hogname, int nfiles, const char **filenames, void(* UpdateFunction)(char *))
{
	unsigned i;
	int table_pos;
	FILE *hog_fp;
	tHogHeader header;
	tHogFileEntry *table;
	char ext[_MAX_EXT];
	hogerr_filename[0]='\0';

	//allocate file table
	if (nfiles <= 0) 
		return HOGMAKER_ERROR;
	table = new tHogFileEntry[nfiles];
	if (!table) 
		return HOGMAKER_MEMORY;
	//create new file
	hog_fp = fopen( hogname, "wb" );
	if ( hog_fp == NULL )
	{
		delete[] table;
		strcpy(hogerr_filename,hogname);
		return HOGMAKER_OPENOUTFILE;
	}

	//write the tag
	if (!fwrite(HOG_TAG_STR, strlen(HOG_TAG_STR), 1, hog_fp ))
	{
		delete[] table;
		fclose(hog_fp);
		strcpy(hogerr_filename,hogname);
		return HOGMAKER_OUTFILE;
	}

	//write number of files
	ubyte filler = 0xff;
	header.nfiles = (unsigned)nfiles;
	header.file_data_offset = strlen(HOG_TAG_STR) + HOG_HDR_SIZE + (sizeof(tHogFileEntry) * header.nfiles);
	if (!cf_WriteUInt32Raw(hog_fp, header.nfiles))
	{
		delete[] table;
		fclose(hog_fp);
		strcpy(hogerr_filename,hogname);
		return HOGMAKER_OUTFILE;
	}

	if (!cf_WriteUInt32Raw(hog_fp, header.file_data_offset))
	{
		delete[] table;
		fclose(hog_fp);
		strcpy(hogerr_filename,hogname);
		return HOGMAKER_OUTFILE;
	}

	//write out filler
	for(i=0; i < HOG_HDR_SIZE-sizeof(tHogHeader); i++)
		if (!fwrite(&filler,sizeof(ubyte),1,hog_fp))	
		{
			delete[] table;
			fclose(hog_fp);
			strcpy(hogerr_filename,hogname);
			return HOGMAKER_OUTFILE;
		}

	//save file position of index table and write out dummy table
	table_pos = strlen(HOG_TAG_STR) + HOG_HDR_SIZE;
	memset(&table[0], 0, sizeof(table[0]));
	for (i = 0; i < header.nfiles; i++)
	{
		if (!WriteHogEntry(hog_fp, &table[0])) 
		{
			delete[] table;
			fclose(hog_fp);
			strcpy(hogerr_filename,hogname);
			return HOGMAKER_OUTFILE;
		}
	}

	//write files (& build index)
	for (i=0;i<header.nfiles;i++) 
	{
		FILE *ifp;
		#if defined(__LINUX__)
		struct stat mystat;
		#else
		struct _stat32 mystat;
		#endif

		ifp = fopen(filenames[i],"rb");
		if ( ifp == NULL )	
		{
			delete[] table;
			fclose(hog_fp);
			strcpy(hogerr_filename,filenames[i]);
			return HOGMAKER_INFILE;
		}

		ddio_SplitPath(filenames[i],NULL,table[i].name,ext);
		_fstat32(fileno(ifp), &mystat);
		strcat(table[i].name,ext);
		table[i].flags = 0;
		table[i].len = _filelength(fileno(ifp));
		table[i].timestamp = mystat.st_mtime;

		if (!FileCopy(hog_fp,ifp,table[i].len)) 
		{
			delete[] table;
			fclose(hog_fp);
			strcpy(hogerr_filename,filenames[i]);
			return HOGMAKER_COPY;
		}
		fclose(ifp);
		// Setup the update message and send it
		char msg[256];
		int ipct = int(100.0*(double(i)/double(header.nfiles)));
		sprintf(msg,"Creating Hog File... (%d%% done)",ipct);
		if(UpdateFunction!=NULL) UpdateFunction(msg);
	}
	//now write the real index
	fseek(hog_fp,table_pos,SEEK_SET);
	for (i = 0; i < header.nfiles; i++)
	{
		if (!WriteHogEntry(hog_fp, &table[i])) 
		{
			delete[] table;
			fclose(hog_fp);
			strcpy(hogerr_filename,hogname);
			return HOGMAKER_OUTFILE;
		}
	}

	//cleanup
	fclose( hog_fp );	
	delete[] table;
	// Setup the update message and send it
	char msg[256];
	sprintf(msg,"Done Creating Hog File.");
	if(UpdateFunction!=NULL) 
		UpdateFunction(msg);

	return HOGMAKER_OK;
}
