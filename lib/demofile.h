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

#ifndef _DEMO_FILE_HEADER_
#define _DEMO_FILE_HEADER_

extern char Demo_fname[_MAX_PATH*2];

extern unsigned int Demo_flags;
extern bool Demo_paused;
extern bool Demo_do_one_frame;
extern bool Demo_restart;
extern bool Demo_auto_play;
extern float Demo_frame_ofs;

#define DF_NONE			0
#define DF_RECORDING	1
#define DF_PLAYBACK		2

#define D3_DEMO_SIG	"D3DEM"
#define D3_DEMO_SIG_NEW	"D3DM1"

#define DT_OBJ				1			//Object data
#define DT_NEW_FRAME		2			//Start of a new frame
#define DT_WEAPON_FIRE		3			//Someone fired a weapon
#define DT_HUD_MESSAGE		4			//Display a hud message
#define DT_3D_SOUND			5			//Play a 3d sound (associated with a weapon firing usually)
#define DT_OBJ_CREATE		6			//A new object was created...
#define DT_OBJ_ANIM			7			//Object animation has changed
#define DT_OBJ_TURRET		8			//Object's turrets have changed
#define DT_OBJ_EXPLODE		9			//Explode object
#define DT_PLAYER_DEATH		10			//Player died
#define DT_COLLIDE_PLR		11			//Player collided with a weapon
#define DT_COLLIDE_GEN		12			//generic collided with a weapon
#define DT_ATTACH			13			//Attach some objects
#define DT_ATTACH_RAD		14			//Attach some objects with a radius?
#define DT_UNATTACH			15			//Unattach some stuff
#define DT_WEAP_FIRE_FLAG	16			//flags like spraying and on/off
#define DT_PLAYER_INFO		17			//Player 1's info like energy/sheilds, etc.
#define DT_MSAFE			18			//MSAFE data (ie script stuff)
#define DT_POWERUP			19			//Powerups data
#define DT_CINEMATICS		20			//Cinematic info
#define DT_PERSISTANT_HUD	21			//Persistant hud message
#define DT_SETOBJDEAD		22			//Mark an object as dead
#define DT_PLAYERBALLS		23			//Rotating balls around player ship
#define DT_PLAYERTYPECHNG	24			//Player type is changing
#define DT_SETOBJLIFELEFT	25			//Object is getting OF_LIFELEFT flag changed
#define DT_2D_SOUND			26			//Play a 2d sound

//If not recording prompts user for filename and starts recording if successfull
//If recording, close the demo file
void DemoToggleRecording();

void DemoWriteChangedObj(object *op);

void DemoWriteHeader();

void DemoStartNewFrame();

void DemoWriteHudMessage(unsigned int color,bool blink,char *msg);

void DemoWriteChangedObjects();

void DemoWriteWeaponFire(unsigned short objectnum,vector *pos,vector *dir,unsigned short weaponnum,unsigned short weapobjnum,short gunnum);

void DemoWriteObjCreate(ubyte type, ushort id, int roomnum, vector *pos, const matrix *orient, int parent_handle,object * obj);

void DemoWriteTurretChanged(unsigned short objnum);

int DemoReadHeader();

int DemoPlaybackFile(char *filename);

bool LoadDemoDialog();

void DemoFrame();

void DemoAbort(bool deletefile=false);

void DemoWriteKillObject(object *hit_obj,object *killer,float damage,int death_flags,float delay,int seed);

void DemoWritePlayerDeath(object *player,bool melee,int fate=-1);

void DemoWrite3DSound(short soundidx,ushort objnum,int priority,float volume= 0.5f);

void DemoWriteCollidePlayerWeapon(object * playerobj, object * weapon, vector *collision_point, vector *collision_normal, bool f_reverse_normal, void *hit_info );

void DemoWriteCollideGenericWeapon( object * robotobj, object * weapon, vector *collision_point, vector *collision_normal, bool f_reverse_normal, void *hit_info );

void DemoReadCollidePlayerWeapon(void);

void DemoReadCollideGenericWeapon(void);

void DemoReadNewFrame(void);

void DemoWriteObjAnimChanged(unsigned short objnum);

void DemoWriteAttachObjRad(object *parent, char parent_ap, object *child, float rad);

void DemoReadAttachObjRad(void);

void DemoWriteAttachObj(object *parent, char parent_ap, object *child, char child_ap, bool f_aligned);

void DemoReadAttachObj(void);

void DemoWriteUnattachObj(object *child);

void DemoReadUnattachObj(void);

void DemoPostPlaybackMenu(void);

void DemoReadObjWeapFireFlagChanged(void);

void DemoWriteObjWeapFireFlagChanged(short objnum);

void DemoWritePlayerInfo(void);

void DemoReadPlayerInfo(void);

void DemoPlayAutoDemo(void);

void DemoWriteMSafe(ubyte *data,unsigned short len);

void DemoReadMSafe();

void DemoWritePowerup(ubyte *data,unsigned short len);

void DemoReadPowerups();

void DemoReadCinematics();

void DemoWriteCinematics(ubyte *data,unsigned short len);

void DemoWritePersistantHUDMessage(ddgr_color color,int x, int y, float time, int flags, int sound_index,char *msg);

void DemoReadPersistantHUDMessage();

void DemoWriteSetObjDead(object *obj);
void DemoReadSetObjDead();

void DemoWritePlayerBalls(int pnum);
void DemoReadPlayerBalls(void);

void DemoWritePlayerTypeChange(int slot,bool stop_observing=false,int observer_mode=-1,int piggy_objnum=-1);
void DemoReadPlayerTypeChange(void);

void DemoWriteObjLifeLeft(object *obj);
void DemoReadObjLifeLeft(void);

void DemoWrite2DSound(short soundidx,float volume= 0.5f);
void DemoRead2DSound(void);

#endif
