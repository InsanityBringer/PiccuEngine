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

#ifndef MEM_H
#define MEM_H


//#define MEM_USE_RTL	1

#ifdef MEM_USE_RTL
#define mem_malloc(d)	malloc(d)								//Use this if your going to run BoundsChecker
#define mem_free(d)	free(d)
#define mem_strdup(d) strdup(d)
#define mem_size(d) _msize(d)
#define mem_realloc(d,e) realloc(d,e)
#else
//Use this if your going to NOT run BoundsChecker
#define mem_malloc(d)	mem_malloc_sub(d, __FILE__, __LINE__)	
#define mem_free(d)	mem_free_sub(d)
#define mem_strdup(d) mem_strdup_sub(d, __FILE__, __LINE__)
#define mem_size(d) mem_size_sub(d)
#define mem_realloc(d,e) mem_realloc_sub(d,e)
#endif

extern bool Mem_low_memory_mode;
extern bool Mem_superlow_memory_mode; //DAJ

//	use if you want to manually print out a memory error
#define mem_error() mem_error_msg(__FILE__, __LINE__)		

//	initializes memory library.
void mem_Init();

// shutsdown memory
void mem_shutdown(void);

// Returns the number of dynamically allocated bytes
int mem_GetTotalMemoryUsed ();

// Allocates a block of memory and returns a pointer to it
void *mem_malloc_sub (int size, const char *file, int line);

// Frees a previously allocated block of memory
void mem_free_sub (void *memblock);

//	prints out a memory error message
void mem_error_msg(const char *file, int line, int size=-1);

char * mem_strdup_sub(const char *src,char *file,int line);

void * mem_realloc_sub(void * memblock,int size);

int mem_size_sub(void *memblock);

bool mem_dumpmallocstofile(char *filename);

void mem_heapcheck(void);

#endif
