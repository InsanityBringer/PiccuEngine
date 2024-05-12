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

#include "CFILE.H"
#include "room.h"


//Chunk types
#define CHUNK_TEXTURE_NAMES			"TXNM"
#define CHUNK_GENERIC_NAMES 		"GNNM"
#define CHUNK_ROBOT_NAMES			"RBNM"
#define CHUNK_POWERUP_NAMES			"PWNM"
#define CHUNK_DOOR_NAMES			"DRNM"
#define CHUNK_ROOMS					"ROOM"
#define CHUNK_ROOM_WIND				"RWND"
#define CHUNK_OBJECTS				"OBJS"
#define CHUNK_TERRAIN				"TERR"
#define CHUNK_EDITOR_INFO			"EDIT"
#define CHUNK_SCRIPT				"SCPT"
#define CHUNK_TERRAIN_HEIGHT		"TERH"
#define CHUNK_TERRAIN_TMAPS_FLAGS	"TETM"
#define CHUNK_TERRAIN_LINKS			"TLNK"
#define CHUNK_TERRAIN_SKY			"TSKY"
#define CHUNK_TERRAIN_END			"TEND"
#define CHUNK_SCRIPT_CODE			"CODE"
#define CHUNK_TRIGGERS				"TRIG"
#define CHUNK_LIGHTMAPS				"LMAP"
#define CHUNK_BSP					"CBSP"
#define CHUNK_OBJECT_HANDLES		"OHND"
#define CHUNK_GAME_PATHS			"PATH"
#define CHUNK_BOA                   "CBOA"
#define CHUNK_NEW_BSP				"CNBS"
#define CHUNK_LEVEL_INFO			"INFO"
#define CHUNK_PLAYER_STARTS			"PSTR"
#define CHUNK_MATCEN_DATA			"MTCN"
#define CHUNK_LEVEL_GOALS			"LVLG"
#define CHUNK_ROOM_AABB				"AABB"
#define CHUNK_NEW_LIGHTMAPS			"NLMP"
#define CHUNK_ALIFE_DATA            "LIFE"
#define CHUNK_TERRAIN_SOUND			"TSND"
#define CHUNK_BNODES				"NODE"
#define CHUNK_OVERRIDE_SOUNDS       "OSND"
#define CHUNK_FFT_MOD				"FFTM"

#ifndef NEWEDITOR // for compatibility with Descent 3 v1.2 and earlier
	#define LEVEL_FILE_VERSION	132
#else
	#define LEVEL_FILE_VERSION	128
#endif

//This is the oldest version the code will read
#define LEVEL_FILE_OLDEST_COMPATIBLE_VERSION	13

//Version numbers of specific changes
#define LEVEL_FILE_SCRIPTING 18
#define LEVEL_FILE_NEWSCRIPTING 26
#define LEVEL_FILE_SCRIPTNAMES 31
#define LEVEL_FILE_SCRIPTPARMS 46
#define LEVEL_FILE_TRIGPARMS 48
#define LEVEL_FILE_SCRIPTCHECK 70
#define LEVEL_FILE_OSIRIS1DEAD 97

//Version number changes:
//0 -> 1	Save curseg & markedseg to file
//1 -> 2 Save texture names to file & remap when load
//2 -> 3	Save selected list
//3 -> 4	Save triggers and doorways
//4 -> 5 Save segnums as ints, not shorts.  Also, save terrain height array
//5 -> 6 Save & xlate names for robots, powerups, & doors
//6 -> 7 Instead of saving a bunch of type-specific data, read it from the data page
//7 -> 8 Now saves terrain sky data
//8 -> 9 Now saves the mine/terrain links
//9 ->10 Changes for floating segments
//10->11 Added tmap2 textures to terrain
//11->12 save u,v coords for terrain
//12->13 New chunk-based file format to save room data
//13->14 Save some more data in editor chunk
//14->15 Saves terrain info in chunk format
//15->16 Changes UV terrain chunk format
//16->17 Generic objects replace robots & powerups
//17->18 Custom script handle read and written for objects 8-11-97
//18->19 Now saves lightmap info with room faces
//19->20 Now saves mine/terrain links and sky info
//20->21 Now saves alpha component per vertex
//21->22 Now saves upper left vertex
//22->23 Save portal num as byte
//23->24 Saves RGB lighting for terrain
//24->25 Saves terrain dynamic lighting table
//25->26 If level isn't version 26 or above, then we need to ignore any script chunk, create a new one.
//26->27 Save face flags as short
//27->28 Store doorway information
//28->29 Killed static light field in room struct
//29->30 Store new lightmap_info information
//30->31 Read in script names instead of handles for all objects.
//31->32	Do RLE compression for terrain data
//32->33	Save more info about doorways
//33->34 Save object id as short
//34->35 Save info about object lightmaps
//35->36 Save "keys_needed" field for doorways
//36->37 Do tricks to restore OF_POLYGON_OBJECT flag
//37->38 Do RLE compression for lightmaps
//38->39 Store lightmap spacing as floats,not ubytes
//39->40 Save/load shadow rooms/faces
//40->41 Save horizon texture info
//41->42	Save BSP info for the mine
//42->43 Save extra texture pieces for terrain sky
//43->44	Save terrain satellite flags
//44->45 Objects no longer compressed, so save objnum (handle,actually) with each object
//45->46 Objects and triggers have optional script parameters.  Also, compressed script info in object.
//46->47 Only save one viewer id, not two
//47->48 Read in trigger parameters too.
//48->49	Objects now store terrain flag as part of roomnum
//49->50 Store light multiplier per face
//50->51 Store fvec/uvec of path nodes
//51->52 Now saves/loads wall pulsing data
//52->53 Face light multiple now works in quarter steps
//54->55 Add wireframe view info to editor chunk
//55->56 Don't read/write useless face info such as rgba
//56->57 Don't read/write pixel error or terrain distance
//57->58 Read uvec,rvec for lightmap infos
//58->59 Store lightmap normals
//59->60 Don't store lightmap normals for dynamic objects
//60->61 Trimmed some fat from rooms,faces structures
//61->62 BOA now saves out the portal cost array
//62->63 We now save path_pnts with rooms and portals
//62->64 Translate old 565 lightmaps into 1555 format
//64->65 Added object parent_handles
//65->66 Store lightmap spacing as bytes, not floats
//66->67 Save volume lighting for rooms
//67->68	Save specular lighting for rooms
//68->69 Save terrain sky radius
//69->70 Read in whether attached script is default or custom.
//70->71 Threw out vertex based specularity and went with lightmap based specularity
//71->72 Added terrain occlusion data
//72->73 Now store volumetric fog info with room
//73->74 Support for new banded-dome skies
//74->75 Now supports satellite lighting values
//75->76	Saves BOA_vis_checksum to file
//76->77 Saves multiple specular values
//77->78 Added ambient sound field to room
//78->79	Added mirrored faces
//79->80	New single-face portals
//80->81	Save marked face & vert
//81->82	Door name translation
//82->83 Added gravity to level info
//83->84 Added level goal information
//84->85 Save the amount of memory needed by the rooms
//85->86 Added room AABB information to save
//86->87 Added damage per second for terrain sky
//87->88 Add fog scalar for adjustable fog distance
//88->89 Took out dynamic light data saving
//89->90 Save out more than one lightmap_info per lightmap
//90->91 Save out lightmap_info x1,y1 origin in megalightmap
//91->92 Reduced boa save size
//92->93 Ambeint Life
//93->94 Changed object handle bit allocation
//94->95 Added object names
//95->96 Added room names, & took out room compression (so saved list can have holes)
//96->97 Ripped out OSIRIS v1.0
//97->98 ???
//98->99 Added trigger names
//99->100 Add combinable portals
//100->101 Changed object flags from short to int, and only read certain flags
//101->102 Changed terrain textures system
//102->103 Removed portal triggers
//103->104 Changed terrain sky system
//104->105 Added custom_default_script_name and custom_default_module_name to objects
//105->106 Rewrote doorway system
//106->107 Save out BOA terrain info (temp, for vis and multiplay only)
//107->108 Added room damage
//108->109 Added blastable doors
//109->110 Update BOA to its final form
//109->111 Don't save an object's parent handle
//111->112 Added BOA_connect information
//112->113 Save room multipliers out with editor chunk
//113->114 Took out band textures
//114->115 Added soundsource objects
//115->116 Save satellite sizes
//116->117 added smooth specular mapping
//117->118 added room ambience settings
//118->119 Save sound names, not indices
//119->120 Save the number of player starts to the file
//120->121 Added the BOA Node chunk
//121->122 Added the ability for a level to always have the ceiling checked
//122->123 Added the bnode_index to portals...
//122->124 Added the bnode verify flag
//124->125 Removed the portal field from BNode edges as the room portals now contain bnode info (so it isn't necessary)
//125->126 Save lightmap spacing info
//126->127 Added ceiling value
//127->128 Save rest of lighting parameters
//128->129 Add the Override sound chunk
//129->130 Added the modified force field bounce textures
//130->131 Added the powerups ignore gravity checkbox
//131->132 Added another ff bounce texture for Dan (his last day wish)

//Load a level file
//Returns 1 if file read ok, else 0
// cb_fn returns current chunk, parm1 = # bytes in chunk, parm2 = number of bytes in file
//	parm1 = -1, for 1st just opened level
//	parm2 = -2, for done loading.
int LoadLevel(char *filename, void (*cb_fn)(const char *,int, int) = NULL);

//Save a level file
//Returns 1 if file saved ok, else 0
int SaveLevel(char *filename, bool f_save_room_AABB = true);

//Reads a room from a disk file
//Parameters:	ifile - file to read from
//					rp - room to read
//					version - version number of file
//Returns:		1 if read ok, else 0
int ReadRoom(CFILE *ifile,room *rp,int version);

//Writes a room to a disk file
//Parameters:	ofile - file to write to
//					rp - room to write
//Returns:		1 if written ok, else 0
int WriteRoom(CFILE *ofile,room *rp);

//Write the texture names for remapping when level is loaded
void WriteTextureList(CFILE *ofile);

//Read the texture names & build the xlate table
void ReadTextureList(CFILE *ifile);

// Primarily for multiplayer, makes sure the client and server levels are the same

#include "../md5/md5.h"
extern MD5 *Level_md5;
inline void RestartLevelMD5()
{
  if(Level_md5)
    delete Level_md5;
  Level_md5 = new MD5();
  Level_md5->MD5Init();
}

inline void AppendToLevelChecksum(int val)
{
  if(!Level_md5)
    {
      return;
    }
  Level_md5->MD5Update(val);
}

inline void AppendToLevelChecksum(unsigned int val)
{
  if(!Level_md5)
    {
      return;
    }
  Level_md5->MD5Update(val);
}

inline void AppendToLevelChecksum(unsigned short val)
{
  if(!Level_md5)
    {
      return;
    }
  Level_md5->MD5Update(val);
}

inline void AppendToLevelChecksum(short val)
{
  if(!Level_md5)
    {
      return;
    }
  Level_md5->MD5Update(val);
}

inline void AppendToLevelChecksum(float val)
{
  if(!Level_md5)
    {
      return;
    }
  Level_md5->MD5Update(val);
}

inline void AppendToLevelChecksum(vector val)
{
  if(!Level_md5)
    {
      return;
    }
  Level_md5->MD5Update(val.x);
  Level_md5->MD5Update(val.y);
  Level_md5->MD5Update(val.z);
}

inline void AppendToLevelChecksum(unsigned char val)
{
  if(!Level_md5)
    {
      return;
    }
  Level_md5->MD5Update(val);
}

inline void GetLevelMD5Sum(unsigned char digest[16])
{
  if(!Level_md5)
    {
      for(int i=0;i<16;i++)
		digest[i] = 0;
      return;
    }
  Level_md5->MD5Final(digest);
}
#include <string.h>
inline char * GetCurrentSumString()
{
	static char output_buf[100];
	output_buf[0] = '\0';
	// Make a copy of the context so we don't mess
	// up an in progress md5 sum.
	MD5 *checksum = Level_md5->Clone();

	unsigned char digest[16];
	checksum->MD5Final(digest);
	char bytestr[10] = "";
	// Do level checksum	
	for(int i=0;i<16;i++)
	{
		sprintf(bytestr,"%.2x",digest[i]);
		strcat(output_buf,bytestr);
	}
	MD5::Destroy(checksum);
	return output_buf;
}

