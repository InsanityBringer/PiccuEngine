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

#ifndef __OSIRIS_H_
#define __OSIRIS_H_

#include "pstypes.h"
#include "object_external_struct.h"
#include "osiris_share.h"
#include "module.h"
#include "CFILE.H"

extern uint Osiris_game_checksum;

//	Osiris_InitModuleLoader
//	Purpose:
//		Initializes the OSIRIS module loader and handling system
void Osiris_InitModuleLoader(void);

//	Osiris_FreeModule
//	Purpose:
//		Removes a currently loaded module from the OSIRIS system
void Osiris_FreeModule(int id);

//	Osiris_ShutdownModuleLoader
//	Purpose:
//		Closes down the OSIRIS module loader and handling system
void Osiris_ShutdownModuleLoader(void);

//	Osiris_FindLoadedModule
//	Purpose:
//		Given the name of a module, it returns the id of a loaded OSIRIS module.  -1 if it isn't loaded.
int Osiris_FindLoadedModule(char *filename);

//	Osiris_LoadLevelModule
//	Purpose:
//		Given a module name, it will attempt to load the module as a level module.  If it succeeds
//	it will return the id of the module where it has been loaded.  If the module was already loaded
//	before calling this function, it will return the id to where the module is, and will not reload 
//	the module.  Returns -1 if the module does not exist.  Returns -2 if the module couldn't initialize.
//	Returns -3 if the module is not a level module. Returns -4 if no module slots are available.
int Osiris_LoadLevelModule(char *module_name);

//	Osiris_UnloadLevelModule
//	Purpose:
//		Instead of saving the handle returned from Osiris_LoadLevelModule() and calling
//	Osiris_UnloadModule() with that handle, you can just call this, instead of the call
//	to Osiris_UnloadModule().
void Osiris_UnloadLevelModule(void);

//	Osiris_LoadGameModule
//	Purpose:
//		Given a module name, it will attempt to load the module as a game module.  If it succeeds
//	it will return the id of the module where it has been loaded.  If the module was already loaded
//	before calling this function, it will return the id to where the module is, and will not reload 
//	the module.  Returns -1 if the module does not exist.  Returns -2 if the module couldn't initialize.
//	Returns -3 if the module is not a game module. Returns -4 if no module slots are available.
int Osiris_LoadGameModule(char *module_name);

//	Osiris_UnloadModule
//	Purpose:
//		Given a module id, it will decrement the reference count to that module by one.  If the reference
//	count becomes zero, the module will be unloaded from memory.
void Osiris_UnloadModule(int module_id);

//	Osiris_LoadMissionModule
//	Purpose:
//		It will attempt to load the module as a game module.  If it succeeds
//	it will return the id of the module where it has been loaded.  If the module was already loaded
//	before calling this function, it will return the id to where the module is, and will not reload 
//	the module.  Returns -1 if the module does not exist.  Returns -2 if the module couldn't initialize.
//	Returns -3 if the module is not a game module. Returns -4 if no module slots are available.
//	This technically doesn't load a mission module, as it should already be loaded by
//	Descent 3 prior.
int Osiris_LoadMissionModule(module *module_handle,char *filename);

//	Osiris_UnloadMissionModule
//	Purpose:
//		Instead of saving the handle returned from Osiris_LoadMissionModule() and calling
//	Osiris_UnloadModule() with that handle, you can just call this, instead of the call
//	to Osiris_UnloadModule().
void Osiris_UnloadMissionModule(void);

//	Osiris_BindScriptsToObject
//	Purpose:
//		Call this function after an object has been created to bind all the scripts associated
//	with it to the object.  This function must be called near the end of it's initialization,
//	to make sure that all fields have been filled in.  This function does not call any events.
//	This function will also load any dll's needed for it's game script.
//	returns false if nothing was bound.
bool Osiris_BindScriptsToObject(object *obj);

//	Osiris_DetachScriptsFromObject
//	Purpose:
//		Call this function when an object is about to be destroyed.  This will unbind and remove
//	all scripts associated with that object.  This function does not call any events.
void Osiris_DetachScriptsFromObject(object *obj);

//	Osiris_CallEvent
//	Purpose:
//		Triggers an event for an object.  Pass in the event number and the associated
//	structure of data.  All events will be chained through the associated scripts of the
//	object (as long as they are available) in the order: custom script, level script,
//	mission script, and finally it's default script.  The chain breaks if one of the scripts
//	returns false on the call to their CallInstanceEvent().
bool Osiris_CallEvent(object *obj, int event, tOSIRISEventInfo *data );

//	Osiris_CallLevelEvent
//	Purpose:
//		Triggers an event for a level script.  Returns true if the default action should continue
//	to process.
bool Osiris_CallLevelEvent(int event, tOSIRISEventInfo *data );

//	Osiris_CallTriggerEvent
//	Purpose:
//		Triggers an event for a trigger script.  Returns true if the default action should continue
//	to process.
bool Osiris_CallTriggerEvent(int trignum,int event,tOSIRISEventInfo *ei);

//	Osiris_ProcessTimers
//	Purpose:
//		This function checks all timers currently running, if any need to be signaled it signals them.
void Osiris_ProcessTimers(void);

//	Osiris_CreateTimer
//	Pupose:
//		Adds a timer to the list to be processed.  You'll receive a EVT_TIMER when the timer is signaled.
//	Returns an id to the timer, which can be used to cancel a timer. -1 on error.
int Osiris_CreateTimer(tOSIRISTIMER *ot);

//	Osiris_ResetAllTimers
//	Purpose:
//		Flushes all the timers
void Osiris_ResetAllTimers(void);

//	Osiris_CancelTimer
//	Purpose:
//		Cancels a timer thats in use, given it's ID
void Osiris_CancelTimer(int handle);

//	Osiris_TimerExists
//	Purpose:
//		Returns true if the timer is valid
ubyte Osiris_TimerExists(int handle);

//	Osiris_SaveSystemState
//	Purpose:
//		Saves the current state of the system (not the scripts!) to file
void Osiris_SaveSystemState(CFILE *file);

//	Osiris_RestoreSystemState
//	Purpose:
//		Restore the state of the system from file (does not restore scripts!)
bool Osiris_RestoreSystemState(CFILE *file);

//	Osiris_InitMemoryManager
//	Purpose:
//		Initializes the memory manager for the scripts, for buffers that the scripts want
//	automatically restored/save.
void Osiris_InitMemoryManager(void);

//	Osiris_CloseMemoryManager
//	Purpose:
//		Shuts down the Osiris memory manager, freeing any unfreed memory
void Osiris_CloseMemoryManager(void);

//	Osiris_AllocateMemory
//	Purpose:
//		Allocates a chunk of memory to be associated with a script.  It will automatically
//	save this memory to disk on game save, and will pass the pointer to this memory on EVT_RESTORE
void *Osiris_AllocateMemory(tOSIRISMEMCHUNK *mc);

//	Osiris_FreeMemory
//	Purpose:
//		Frees a chunk of memory that was allocated by Osiris_AllocateMemory().
void Osiris_FreeMemory(void *mem_ptr);

//	Osiris_FreeMemoryForScript
//	Purpose:
//		Frees all memory allocated for a given script
void Osiris_FreeMemoryForScript(tOSIRISSCRIPTID *sid);

//	Osiris_RestoreMemoryChunks
//	Purpose:
//		Restores the 'auto manage' from file, calls the EVT_MEMRESTORE
void Osiris_RestoreMemoryChunks(CFILE *file);

//	Osiris_SaveMemoryChunks
//	Purpose:
//		Saves out the 'auto manage' script memory to file
void Osiris_SaveMemoryChunks(CFILE *file);

//	Osiris_ExtractScriptsFromHog
//	Given the handle of a loaded hog file, this extracts all the scripts out to a temp directory
//	Pass false for the second parameter if it's a game hog (d3.hog for example)
int Osiris_ExtractScriptsFromHog(int library_handle,bool is_mission_hog=true);

//	Osiris_ClearExtractedScripts
//	Deletes the temp files created when the scripts where extracted from the hog
//	Pass false if you want it to remove _all_ extracted hogs...else only mission related ones
void Osiris_ClearExtractedScripts(bool misson_only=true);

// Initializes the Osiris Mission Memory System
void Osiris_InitOMMS(void);
// Shutsdown the Osiris Mission Memory System (frees all memory associated, call at end of mission)
void Osiris_CloseOMMS(void);

extern bool Show_osiris_debug;

#define OEM_OBJECTS		0x01
#define OEM_TRIGGERS	0x02
#define OEM_LEVELS		0x04
//	Osiris_EnableEvents
//	Purpose:
//		Enables the passed in mask of event types to be called
void Osiris_EnableEvents(ubyte mask);
//	Osiris_DisableEvents
//	Purpose:
//		Disables the passed in mask of event types
void Osiris_DisableEvents(ubyte mask);

//	Osiris_DisableCreateEvents
//	Purpose:
//		Disables any events involved when an object is created.  This is to be used for
//	Loading games/viewing demos, as so not to re-initialize good data.
void Osiris_DisableCreateEvents(void);
//	Osiris_EnablesCreateEvents
//	Purpose:
//		Enables any events involved when an object is created.  This is to be used for
//	Loading games/viewing demos, as so not to re-initialize good data. (call when done with Osiris_DisableCreateEvents())
void Osiris_EnableCreateEvents(void);

#endif
