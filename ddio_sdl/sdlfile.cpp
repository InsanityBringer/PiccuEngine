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
#include <stdlib.h>
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
#include "mem.h"

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

char* ddio_GetUserDir(const char* extraname)
{
    const char* savepath = SDL_GetUserFolder(SDL_FOLDER_SAVEDGAMES);

    if (savepath == nullptr)
        Error("ddio_GetUserDir: SDL_GetUserFolder failed!");

    int length = strlen(savepath);

    int extrachars = 0;
    if (extraname)
        extrachars = strlen(extraname); //[ISB] one for the path separator

    char* narrowstr = (char*)malloc(length + extrachars + 1);
    if (narrowstr == nullptr)
    {
        Error("ddio_GetUserDir: Can't allocate narrowstr");
    }

    strcpy(narrowstr, savepath);
    if (extraname)
    {
        //The strcpy will null terminate this.
        strcpy(&narrowstr[length + 1], extraname);
    }

    return narrowstr;
}

//	 pass in a pathname (could be from ddio_SplitPath), root_path will have the drive name.
void ddio_GetRootFromPath(const char* srcPath, char* root_path)
{
    assert(root_path);
    assert(srcPath);

    //the first char should be drive letter, second char should be a :
    if ((isalpha(srcPath[0])) && (srcPath[1] == ':')) {
        //everything is ok
        strncpy(root_path, srcPath, 2);
        root_path[2] = '\0';
    }
    else {
        //invalid path (no root)
        root_path[0] = '\0';
    }
}

//	retrieve root names, free up roots array (allocated with malloc) after use
int ddio_GetFileSysRoots(char** roots, int max_roots)
{
    char buffer[100];

    int ret = GetLogicalDriveStrings(100, buffer);
    if (ret == 0) {
        //there was an error
        return 0;
    }

    if (ret > 100) {
        //we didn't have enough space
        return 0;
    }

    int count = 0;
    bool done = false;
    char* strptr = buffer;
    char* string;
    int strsize;
    while ((count < max_roots) && (!done)) {
        if (*strptr != 0) {
            strsize = strlen(strptr);
            string = roots[count] = (char*)mem_malloc(strsize);
            if (!string)
                break;
            //remove the trailing \ from windows
            strncpy(string, strptr, strsize - 1);
            string[strsize - 1] = '\0';

            strptr += (strsize + 1);

            count++;
        }
        else
            done = true;
    }
    return count;
}

//	given a path, it cleans it up (if the path is c:\windows\..\dos it would make it c:\dos)
//	srcPath is the original path
//	dest is the finished cleaned path.
//		dest should be at least _MAX_PATH in size
void ddio_CleanPath(char* dest, const char* srcPath)
{
    strcpy(dest, srcPath);

    //break the path into directories
    char** directories;
    int dirs;
    int path_length;

    //make sure the path ends with a \ for sanity
    path_length = strlen(dest);

    if (dest[path_length - 1] != '\\') {
        dest[path_length] = '\\';
        dest[path_length + 1] = '\0';
        path_length++;
    }

    //now divide the full path into seperate NULL terminated strings,counting the number
    //of directories in the process
    dirs = 0;
    char* strptr = dest;
    while (*strptr != '\0') {
        if (*strptr == '\\') {
            *strptr = '\0';
            dirs++;
        }
        strptr++;
    }

    //check to make sure we have a directory, if we don't then return the original path given
    if (dirs == 0) {
        strcpy(dest, srcPath);
        return;
    }

    //allocate the memory needed for the seperate strings of each directory
    directories = (char**)mem_malloc(sizeof(char*) * dirs);
    if (!directories) {
        strcpy(dest, srcPath);
        return;
    }

    //now get all the directories, and place into the individual strings
    strptr = dest;
    int count = 0;
    while (count < dirs) {
        directories[count] = mem_strdup(strptr);
        strptr += strlen(strptr) + 1;
        count++;
    }

    //now the fun part, figure out the correct order of the directories
    int* dir_order;

    dir_order = (int*)mem_malloc(sizeof(int) * dirs);
    if (!dir_order) {
        strcpy(dest, srcPath);
        return;
    }

    for (count = 0; count < dirs; count++)
        dir_order[count] = -1;	//a -1 means the end of the sequence

    //now build the order based on the indicies
    int curr_index = 0;
    for (count = 0; count < dirs; count++) {

        if (!stricmp(directories[count], "..")) {
            //we have to back up a directory
            curr_index--;				//back up
            if (curr_index < 0)
                curr_index = 0;			//can't go further than root
            dir_order[curr_index] = -1;	//invalidate current slot
        }
        else
            if (stricmp(directories[count], ".")) {
                //we have a normal directory, add it's index
                dir_order[curr_index] = count;
                curr_index++;
            }
    }

    //now rebuild the correct path for use, when we hit -1, we're done
    dest[0] = '\0';
    for (count = 0; count < dirs; count++) {
        if (dir_order[count] == -1)
            break;
        else {
            strcat(dest, directories[dir_order[count]]);
            strcat(dest, "\\");
        }
    }

    //now remove trailing \ char
    path_length = strlen(dest);
    if ((path_length > 0) && (dest[path_length - 1] == '\\'))
        dest[path_length - 1] = '\0';

    //free up all the allocated memory and we're done
    for (count = 0; count < dirs; count++) {
        if (directories[count])
            mem_free(directories[count]);
    }
    if (directories)
        mem_free(directories);
    if (dir_order)
        mem_free(dir_order);
}

//	given a path (with no filename), it will return the parent path
//	srcPath is the source given path
//	dest is where the parent path will be placed
//	returns true on success
//		dest should be at least _MAX_PATH in length
bool ddio_GetParentPath(char* dest, const char* srcPath)
{
    assert(srcPath);
    assert(dest);

#define PARENT_DELIM ".."
    int spath_len = strlen(srcPath);
    char* temp;

    temp = (char*)mem_malloc(spath_len + strlen(PARENT_DELIM) + 3);
    if (!temp)
        return false;

    ddio_MakePath(temp, srcPath, PARENT_DELIM, NULL);
    ddio_CleanPath(dest, temp);
    mem_free(temp);
    return true;
}

//Finds a full path from a relative path
//Parameters:	full_path - filled in with the fully-specified path.  Buffer must be at least _MAX_PATH bytes long
//					rel_path - a path specification, either relative or absolute
//Returns TRUE if successful, FALSE if an error
bool ddio_GetFullPath(char* full_path, const char* rel_path)
{
    char old_path[_MAX_PATH];

    ddio_GetWorkingDir(old_path, sizeof(old_path));	//save old directory

    if (!ddio_SetWorkingDir(rel_path))					//try switching to new directory
        return 0;												//couldn't switch, so return error

    ddio_GetWorkingDir(full_path, _MAX_PATH);		 	//get path from the OS

    ddio_SetWorkingDir(old_path);							//now restore old path

    return 1;
}

//Renames file
//Returns true on success or false on an error
bool ddio_RenameFile(char* oldfile, char* newfile)
{
    int rcode = rename(oldfile, newfile);
    if (!rcode)
        return true;
    else
        return false;
}

bool ddio_CopyFile(char* srcfile, char* destfile)
{
#ifdef WIN32
    if (CopyFile(srcfile, destfile, TRUE))
        return true;

    return false;
#else
#error "ddio_CopyFile: Not implemented for this platform!"
#endif
}

char* ddio_GetCDDrive(char* vol)
{
    return nullptr;
}

#ifdef WIN32
// Checks to see if a lock file is located in the specified directory.
//	Parameters:
//		dir		Directory for which the lock file should be checked
//	Returns:
//		1		Lock file doesn't exist
//		2		Lock file was in a directory, but it belonged to a process that no longer
//				exists, so a lock file _can_ be made in the directory.
//		3		Lock file for this process already exists
//		0		Lock file currently exists in directory
//		-1		Illegal directory
//		-2		There is a lock file in the directory, but it is in an illegal format
int ddio_CheckLockFile(const char* dir)
{
    DWORD curr_pid = GetCurrentProcessId();
    char old_directory[_MAX_PATH];
    ddio_GetWorkingDir(old_directory, _MAX_PATH);
    if (!ddio_SetWorkingDir(dir))
        return -1;

    bool found_lock_file_in_dir = false;
    FILE* file;

    chmod(".lock", _S_IREAD | _S_IWRITE);
    file = fopen(".lock", "rb");

    if (!file)
    {
        //File exists, but couldn't open it
        if (errno == EACCES)
        {
            ddio_SetWorkingDir(old_directory);
            return -2;
        }

        found_lock_file_in_dir = false;
    }
    else
    {
        found_lock_file_in_dir = true;

        //check the file, see if it is a lock file
        char c;
        c = fgetc(file); if (c != 'L') { fclose(file); ddio_SetWorkingDir(old_directory); return -2; }
        c = fgetc(file); if (c != 'O') { fclose(file); ddio_SetWorkingDir(old_directory); return -2; }
        c = fgetc(file); if (c != 'C') { fclose(file); ddio_SetWorkingDir(old_directory); return -2; }
        c = fgetc(file); if (c != 'K') { fclose(file); ddio_SetWorkingDir(old_directory); return -2; }

        //it appears to be a lock file, check the pid
        DWORD f_pid;
        fread(&f_pid, sizeof(DWORD), 1, file);
        fclose(file);

        //check the file id in the file, compared to our pid
        if (f_pid == curr_pid)
        {
            //lock file already exists for the current process
            ddio_SetWorkingDir(old_directory);
            return 3;
        }

        HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, f_pid);
        if (proc)
        {
            DWORD obj_ret = WaitForSingleObject(proc, 0);
            CloseHandle(proc);
            if (obj_ret == WAIT_TIMEOUT)
            {
                //this process exists still
                ddio_SetWorkingDir(old_directory);
                return 0;
            }
        }

        //the process no longer exists, we can create a lock file if needed
        //we'll delete the useless one now		
        ddio_DeleteFile(".lock");
    }

    ddio_SetWorkingDir(old_directory);
    return (found_lock_file_in_dir) ? 2 : 1;
}

// Creates a lock file in the specified directory
//	Parameters:
//		dir		Directory for which the lock file should be created in
//	Returns:
//		1		Lock file created
//		2		Lock file created (there was a lock file in that directory, but it belonged
//				to a process that no longer exists)
//		3		Lock file for this process already exists
//		0		Lock file not created, a lock file currently exists in the directory
//		-1		Illegal directory
//		-2		There is a lock file in the directory, but it is in an illegal format
//		-3		Unable to create lock file
int ddio_CreateLockFile(const char* dir)
{
    int result = ddio_CheckLockFile(dir);
    switch (result)
    {
    case 0:
    case -1:
    case -2:
    case 3:
        return result;
    };

    DWORD curr_pid = GetCurrentProcessId();
    char old_directory[_MAX_PATH];
    ddio_GetWorkingDir(old_directory, _MAX_PATH);
    if (!ddio_SetWorkingDir(dir))
        return -1;

    FILE* file;
    file = fopen(".lock", "wb");
    if (!file)
    {
        ddio_SetWorkingDir(old_directory);
        return -3;
    }

    fputc('L', file);
    fputc('O', file);
    fputc('C', file);
    fputc('K', file);
    fwrite(&curr_pid, sizeof(DWORD), 1, file);

    fclose(file);
    ddio_SetWorkingDir(old_directory);

    // at this point result will either be 1 or 2 from checking the lock file
    // either way, a lock file has been created
    return result;
}

// Deletes a lock file (for the current process) in the specified directory
//	Parameters:
//		dir		Directory for which the lock file should be deleted from
//	Returns:
//		1		Lock file deleted
//		0		Lock file not deleted, the lock file in the directory does not belong to our
//				process
//		-1		Illegal directory
//		-2		A lock file exists in the directory, but wasn't deleted...illegal format
//		-3		Unable to delete file
int ddio_DeleteLockFile(const char* dir)
{
    DWORD curr_pid = GetCurrentProcessId();
    char old_directory[_MAX_PATH];
    ddio_GetWorkingDir(old_directory, _MAX_PATH);
    if (!ddio_SetWorkingDir(dir))
        return -1;

    FILE* file;

    chmod(".lock", _S_IREAD | _S_IWRITE);
    file = fopen(".lock", "rb");

    if (!file)
        return 1;

    //check the file, see if it is a lock file
    char c;
    c = fgetc(file); if (c != 'L') { fclose(file); ddio_SetWorkingDir(old_directory); return -2; }
    c = fgetc(file); if (c != 'O') { fclose(file); ddio_SetWorkingDir(old_directory); return -2; }
    c = fgetc(file); if (c != 'C') { fclose(file); ddio_SetWorkingDir(old_directory); return -2; }
    c = fgetc(file); if (c != 'K') { fclose(file); ddio_SetWorkingDir(old_directory); return -2; }

    //it appears to be a lock file, check the pid
    DWORD f_pid;
    fread(&f_pid, sizeof(DWORD), 1, file);
    fclose(file);

    //check the file id in the file, compared to our pid
    if (f_pid != curr_pid)
    {
        //it doesn't belong to 		
        ddio_SetWorkingDir(old_directory);
        return 0;
    }

    //the lock file in the directory belongs to us!
    if (!ddio_DeleteFile(".lock"))
    {
        ddio_SetWorkingDir(old_directory);
        return -3;
    }

    ddio_SetWorkingDir(old_directory);

    return 1;
}
#else
// Checks to see if a lock file is located in the specified directory.
//	Parameters:
//		dir		Directory for which the lock file should be checked
//	Returns:
//		1		Lock file doesn't exist
//		2		Lock file was in a directory, but it belonged to a process that no longer
//				exists, so a lock file _can_ be made in the directory.
//		3		Lock file for this process already exists
//		0		Lock file currently exists in directory
//		-1		Illegal directory
//		-2		There is a lock file in the directory, but it is in an illegal format
int ddio_CheckLockFile(const char* dir)
{
    pid_t curr_pid = getpid();

    //  rcg 06092000 what's all this working dir shite?
    //	char old_directory[_MAX_PATH];
    //	ddio_GetWorkingDir(old_directory,_MAX_PATH);
    //	if(!ddio_SetWorkingDir(dir))
    //		return -1;

            // rcg 06092000 The buffer stuff throughout is my add.
    char buffer[strlen(dir) + strlen(".lock") + 2];
    bool found_lock_file_in_dir = false;
    FILE* file;

    sprintf(buffer, "%s/%s", dir, ".lock");
    mprintf((0, "LockFile: Checking [%s]...", buffer));
    chmod(buffer, S_IREAD | S_IWRITE);
    file = fopen(buffer, "rb");

    if (!file)
    {
        //File exists, but couldn't open it
        if (errno == EACCES)
        {
            //			ddio_SetWorkingDir(old_directory);
            return -2;
        }

        found_lock_file_in_dir = false;
    }
    else
    {
        found_lock_file_in_dir = true;

        //check the file, see if it is a lock file
        char c;
        //		c = fgetc(file); if(c!='L')	{fclose(file); ddio_SetWorkingDir(old_directory); return -2;}
        //		c = fgetc(file); if(c!='O')	{fclose(file); ddio_SetWorkingDir(old_directory); return -2;}
        //		c = fgetc(file); if(c!='C')	{fclose(file); ddio_SetWorkingDir(old_directory); return -2;}
        //		c = fgetc(file); if(c!='K')	{fclose(file); ddio_SetWorkingDir(old_directory); return -2;}
        c = fgetc(file); if (c != 'L') { fclose(file); return -2; }
        c = fgetc(file); if (c != 'O') { fclose(file); return -2; }
        c = fgetc(file); if (c != 'C') { fclose(file); return -2; }
        c = fgetc(file); if (c != 'K') { fclose(file); return -2; }

        //it appears to be a lock file, check the pid
        pid_t f_pid;
        fread(&f_pid, sizeof(pid_t), 1, file);
        fclose(file);

        //check the file id in the file, compared to our pid
        if (f_pid == curr_pid)
        {
            //lock file already exists for the current process
//			ddio_SetWorkingDir(old_directory);
            return 3;
        }

        if (kill(f_pid, 0) == -1)
        {
            if (errno == ESRCH)
            {
                /* pid does not exist */

            }
            else
            {
                /* some other error */
                //technically this shouldn't happen, but I get it
                //when the pid no longer exists...so I'm going to
                //pretend it doesn't
                mprintf((0, "Error sending signal to pid for lock check (%d)\n", f_pid));
                //perror ("Error sending signal to pid for lock check, maybe remove lock file in temp directory");
                //ddio_SetWorkingDir(old_directory);
                //return 0;
            }

        }
        else
        {
            /* pid exists */
//			ddio_SetWorkingDir(old_directory);
            return 0;
        }

        //the process no longer exists, we can create a lock file if needed
        //we'll delete the useless one now		
        ddio_DeleteFile(".lock");
    }

    //	ddio_SetWorkingDir(old_directory);
    return (found_lock_file_in_dir) ? 2 : 1;
}

// Creates a lock file in the specified directory
//	Parameters:
//		dir		Directory for which the lock file should be created in
//	Returns:
//		1		Lock file created
//		2		Lock file created (there was a lock file in that directory, but it belonged
//				to a process that no longer exists)
//		3		Lock file for this process already exists
//		0		Lock file not created, a lock file currently exists in the directory
//		-1		Illegal directory
//		-2		There is a lock file in the directory, but it is in an illegal format
//		-3		Unable to create lock file
int ddio_CreateLockFile(const char* dir)
{
    int result = ddio_CheckLockFile(dir);
    switch (result)
    {
    case 0:
    case -1:
    case -2:
    case 3:
        return result;
    };

    pid_t curr_pid = getpid();
    char old_directory[_MAX_PATH];
    ddio_GetWorkingDir(old_directory, _MAX_PATH);
    if (!ddio_SetWorkingDir(dir))
        return -1;

    FILE* file;
    file = fopen(".lock", "wb");
    if (!file)
    {
        ddio_SetWorkingDir(old_directory);
        return -3;
    }

    fputc('L', file);
    fputc('O', file);
    fputc('C', file);
    fputc('K', file);
    fwrite(&curr_pid, sizeof(pid_t), 1, file);

    fclose(file);
    ddio_SetWorkingDir(old_directory);

    // at this point result will either be 1 or 2 from checking the lock file
    // either way, a lock file has been created
    return result;
}

// Deletes a lock file (for the current process) in the specified directory
//	Parameters:
//		dir		Directory for which the lock file should be deleted from
//	Returns:
//		1		Lock file deleted
//		0		Lock file not deleted, the lock file in the directory does not belong to our
//				process
//		-1		Illegal directory
//		-2		A lock file exists in the directory, but wasn't deleted...illegal format
//		-3		Unable to delete file
int ddio_DeleteLockFile(const char* dir)
{
    pid_t curr_pid = getpid();
    char old_directory[_MAX_PATH];
    ddio_GetWorkingDir(old_directory, _MAX_PATH);
    if (!ddio_SetWorkingDir(dir))
        return -1;

    FILE* file;

    chmod(".lock", S_IREAD | S_IWRITE);
    file = fopen(".lock", "rb");

    if (!file)
        return 1;

    //check the file, see if it is a lock file
    char c;
    c = fgetc(file); if (c != 'L') { fclose(file); ddio_SetWorkingDir(old_directory); return -2; }
    c = fgetc(file); if (c != 'O') { fclose(file); ddio_SetWorkingDir(old_directory); return -2; }
    c = fgetc(file); if (c != 'C') { fclose(file); ddio_SetWorkingDir(old_directory); return -2; }
    c = fgetc(file); if (c != 'K') { fclose(file); ddio_SetWorkingDir(old_directory); return -2; }

    //it appears to be a lock file, check the pid
    pid_t f_pid;
    fread(&f_pid, sizeof(pid_t), 1, file);
    fclose(file);

    //check the file id in the file, compared to our pid
    if (f_pid != curr_pid)
    {
        //it doesn't belong to 		
        ddio_SetWorkingDir(old_directory);
        return 0;
    }

    //the lock file in the directory belongs to us!
    if (!ddio_DeleteFile(".lock"))
    {
        ddio_SetWorkingDir(old_directory);
        return -3;
    }

    ddio_SetWorkingDir(old_directory);

    return 1;
}

#endif