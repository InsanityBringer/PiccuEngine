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

#include <sys/stat.h>
#include <SDL3/SDL_filesystem.h>
#include <stdio.h>
#include <string>
#if defined(WIN32)
#include <Windows.h>
#include <io.h>
#include <sys/utime.h>
#elif defined(UNIX)
#include <unistd.h>
#include <utime.h>
#endif

#include "pserror.h"
#include "ddio.h"

//	---------------------------------------------------------------------------
//	File operations

//	creates a directory or folder on disk
bool ddio_CreateDir(const char* path)
{
	return SDL_CreateDirectory(path);
}

//	destroys a directory
bool ddio_RemoveDir(const char* path)
{
	return SDL_RemovePath(path);
}

void ddio_GetWorkingDir(char* path, int len)
{
	char* workingdir = SDL_GetCurrentDirectory();
	strncpy(path, workingdir, len);
	path[len - 1] = '\0';
    SDL_free(workingdir);
}

bool ddio_SetWorkingDir(const char* path)
{
#ifdef WIN32
	return (SetCurrentDirectory(path)) ? true : false;
#else
#error "ddio_SetWorkingDir: Not implemented for current platform"
#endif
}

bool ddio_FileDiff(const char* path1, const char* path2)
{
	struct _stat abuf, bbuf;

	if (_stat(path1, &abuf))
		Int3();		//error getting stat info

	if (_stat(path2, &bbuf))
		Int3();		//error getting stat info

	if ((abuf.st_size != bbuf.st_size) || (abuf.st_mtime != bbuf.st_mtime))
		return true;

	return false;
}

// get a file's length
int	ddio_GetFileLength(FILE* filePtr)
{
	struct stat fstats;
	if (fstat(fileno(filePtr), &fstats) == -1)
	{
		Error("ddio_GetFileLength: fstat failed!"); //should try to figure out when this would happen and handle accordingly
	}

	return fstats.st_size;
}

// Split a pathname into its component parts
void ddio_SplitPath(const char* srcPath, char* path, char* filename, char* ext)
{
    int pathStart = -1;
    int pathEnd = -1;
    int fileStart = -1;
    int fileEnd = -1;
    int extStart = -1;
    int	extEnd = -1;

    int totalLen = strlen(srcPath);

    // Check for an extension
    ///////////////////////////////////////
    int t = totalLen - 1;
    while ((srcPath[t] != '.') && (srcPath[t] != '/') && (t >= 0)) t--;
    //see if we are at an extension
    if ((t >= 0) && (srcPath[t] == '.')) 
    {
        //we have an extension
        extStart = t;
        extEnd = totalLen - 1;
        if (ext)
        {
            strncpy(ext, &(srcPath[extStart]), extEnd - extStart + 1);
            ext[extEnd - extStart + 1] = '\0';
        }
    }
    else 
    {
        //no extension
        if (ext)
            ext[0] = '\0';
    }

    // Check for file name
    ////////////////////////////////////
    int temp = (extStart != -1) ? (extStart) : (totalLen - 1);
    while ((temp >= 0) && (srcPath[temp] != '/')) temp--;
    if (temp < 0)
        temp = 0;
    if (srcPath[temp] == '/') 
    {
        //we have a file
        fileStart = temp + 1;
        if (extStart != -1)
            fileEnd = extStart - 1;
        else
            fileEnd = totalLen - 1;
        if (filename)
        {
            strncpy(filename, &(srcPath[fileStart]), fileEnd - fileStart + 1);
            filename[fileEnd - fileStart + 1] = '\0';
        }
        pathStart = 0;
        pathEnd = fileStart - 2;
        //Copy the rest into the path name
        if (path)
        {
            strncpy(path, &(srcPath[pathStart]), pathEnd - pathStart + 1);
            path[pathEnd - pathStart + 1] = 0;
        }
    }
    else 
    {
        //only file, no path
        fileStart = 0;
        if (extStart != -1)
            fileEnd = extStart - 1;
        else
            fileEnd = totalLen - 1;

        if (filename)
        {
            strncpy(filename, &(srcPath[fileStart]), fileEnd - fileStart + 1);
            filename[fileEnd - fileStart + 1] = 0;
        }

        // Only file no path
        if (path)
        {
            path[0] = 0;
        }
    }
}

void ddio_CopyFileTime(char* destname, const char* srcname)
{
    struct stat abuf;

    if (stat(srcname, &abuf))
        Int3();

    struct utimbuf bbuf;
    bbuf.actime = abuf.st_atime;
    bbuf.modtime = abuf.st_mtime;

    if (utime(destname, &bbuf))
        Int3();
}

// deletes a file.  Returns 1 if successful, 0 on failure
int ddio_DeleteFile(char* name)
{
    return (!unlink(name));
}

// Save/Restore the current working directory

static char SavedWorkingDir[_MAX_DIR];

void ddio_SaveWorkingDir(void)
{
    ddio_GetWorkingDir(SavedWorkingDir, _MAX_DIR);
}


void ddio_RestoreWorkingDir(void)
{
    ddio_SetWorkingDir(SavedWorkingDir);
}

// Checks if a directory exists (returns 1 if it does, 0 if not)
// This pathname is *RELATIVE* not fully qualified
bool ddio_DirExists(const char* path)
{
    bool res;

    ddio_SaveWorkingDir();
    res = ddio_SetWorkingDir(path);
    ddio_RestoreWorkingDir();

    return (res) ? true : false;
}

// Constructs a path in the local file system's syntax
// 	newPath: stores the constructed path
//  absolutePathHeader: absolute path on which the sub directories will be appended
//						(specified in local file system syntax)
//  takes a variable number of subdirectories which will be concatenated on to the path
//		the last argument in the list of sub dirs *MUST* be NULL to terminate the list
void ddio_MakePath(char* newPath, const char* absolutePathHeader, const char* subDir, ...)
{
    const char	delimiter = '\\';
    va_list		args;
    char* currentDir = NULL;
    int			pathLength = 0;

    assert(newPath);
    assert(absolutePathHeader);
    assert(subDir);

    if (newPath != absolutePathHeader)
    {
        strcpy(newPath, absolutePathHeader);
    }

    // Add the first sub directory
    pathLength = strlen(newPath);
    if (newPath[pathLength - 1] != delimiter)
    {
        newPath[pathLength] = delimiter;		// add the delimiter
        newPath[pathLength + 1] = 0;				// terminate the string
    }
    strcat(newPath, subDir);

    // Add the additional subdirectories
    va_start(args, subDir);
    while ((currentDir = va_arg(args, char*)) != NULL)
    {
        pathLength = strlen(newPath);
        if (newPath[pathLength - 1] != delimiter)
        {
            newPath[pathLength] = delimiter;		// add the delimiter
            newPath[pathLength + 1] = 0;				// terminate the string
        }
        strcat(newPath, currentDir);
    }
    va_end(args);
}

//Generates a temporary filename based on the prefix, and basedir
//Parameters: 
//		basedir - directory to put the files
//		prefix - prefix for the temp filename
//		filename - buffer to hold generated filename (must be at least _MAX_PATH in length)
//					
//Returns TRUE if successful, FALSE if an error
bool ddio_GetTempFileName(char* basedir, char* prefix, char* filename)
{
    char old_workdir[_MAX_PATH];
    bool success = false;

    if (strlen(prefix) > 64)
        return false;

    ddio_GetWorkingDir(old_workdir, _MAX_PATH);

    if (!ddio_SetWorkingDir(basedir))
    {
        return false;	//invalid base directory
    }

    char randname[10];
    int index;
    int tries = 0;
    char rc;

    bool created = false;

    index = 0;

    while (!success && tries < 20)
    {
        //generate a bunch of random characters
        rc = (rand() % 128);
        if ((rc >= 'a' && rc <= 'z') || (rc >= 'A' && rc <= 'Z') || (rc >= '0' && rc <= '9'))
        {
            //valid character
            randname[index] = rc;
            index++;

            if (index == 10)
            {
                //we hit the size of our max, see if we generated a unique filename
                char t[_MAX_PATH];
                randname[9] = '\0';
                sprintf(t, "%s%s.tmp", prefix, randname);

                //see if we can find this file
                FILE* fd = fopen(t, "rb");
                if (!fd)
                {
                    //we found a good file!
                    ddio_MakePath(filename, basedir, t, NULL);
                    success = true;
                    created = true;
                }
                else
                {
                    //already taken
                    fclose(fd);
                    tries++;
                    index = 0;
                }
            }
        }
        else
        {
            continue;	//try again
        }
    }

    ddio_SetWorkingDir(old_workdir);
    return created;
}

//	These functions allow one to find a file
//		You use FindFileStart by giving it a wildcard (like *.*, *.txt, u??.*, whatever).  It returns
//		a filename in namebuf.
//		Use FindNextFile to get the next file found with the wildcard given in FindFileStart.
//		Use FindFileClose to end your search.
//  The SDL implementation just uses SDL3's glob func for laziness
static int DDIO_glob_count;
static char** DDIO_glob;
static int DDIO_next_glob;
static std::string DDIO_glob_dir;

//hey sdl 3 guys is there any particular reason why you felt the need to make the pattern and directory separate? just curious.
//windows doesn't do it that way.
//posix doesn't do it that way.
//so who does?

bool ddio_IsSeparator(char c)
{
    if (c == '/')
        return true;
#ifdef WIN32
    else if (c == '\\')
        return true;
#endif
    return false; 
}

void ddio_StupidPathSplit(std::string& path, std::string& directory, std::string& pattern)
{
    if (path.size() == 0)
        return; //something very stupid happened

    bool foundsep = false;
    size_t i;

    //look for a path separator
    for (i = 0; i < path.size(); i++)
    {
        if (ddio_IsSeparator(path[path.size() - i - 1]))
        {
            foundsep = true;
            break;
        }
    }

    if (foundsep)
    {
        size_t split = path.size() - i;
        directory.assign(path.c_str(), split);
        if (split < path.size() - 1)
        {
            pattern.assign(path.c_str() + split, path.size() - split);
        }
        else //did I get a string in the form "blarg/"?
        {
            pattern = "";
        }
    }
    else
    {
        char* aaaaa = SDL_GetCurrentDirectory();
        directory = aaaaa;
        SDL_free(aaaaa);
        pattern = path;
    }
}

void ddio_FindFileFetchResult(int num, char* buffer)
{
    if (num < 0 || num >= DDIO_glob_count)
        Error("ddio_FindFileFetchResult: result out of range!");

    snprintf(buffer, _MAX_PATH, "%s%s", DDIO_glob_dir.c_str(), DDIO_glob[num]);
}

bool ddio_FindFileStart(const char* wildcard, char* namebuf)
{
    std::string wildcardstr = wildcard;
    std::string pattern;

    ddio_StupidPathSplit(wildcardstr, DDIO_glob_dir, pattern);

#ifdef WIN32
    SDL_GlobFlags flags = SDL_GLOB_CASEINSENSITIVE;
#else
    SDL_GlobFlags flags = 0;
#endif

    DDIO_next_glob = 0;

    DDIO_glob = SDL_GlobDirectory(DDIO_glob_dir.c_str(), pattern.c_str(), flags, &DDIO_glob_count);
    if (DDIO_glob_count == 0)
    {
        namebuf[0] = '\0';
        return false;
    }
    else
    {
        ddio_FindFileFetchResult(DDIO_next_glob++, namebuf);
        return true;
    }
}

bool ddio_FindNextFile(char* namebuf)
{
    if (DDIO_glob == nullptr || DDIO_next_glob >= DDIO_glob_count)
    {
        namebuf[0] = '\0';
        return false;
    }

    ddio_FindFileFetchResult(DDIO_next_glob++, namebuf);
}

void ddio_FindFileClose()
{
    SDL_free(DDIO_glob);
    DDIO_glob = nullptr;
}
