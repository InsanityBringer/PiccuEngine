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

#ifndef AIMAIN_H_
#define AIMAIN_H_

#include "object.h"
#include "player.h"

#define AI_SOUND_SHORT_DIST 60.0f

extern int AI_NumRendered;
extern int AI_RenderedList[MAX_OBJECTS];

extern int AI_NumHostileAlert;		// A rough number of alert/hostile robots 
                                    // that have seen the player recently
#ifdef _DEBUG
extern bool AI_debug_robot_do;
extern int AI_debug_robot_index;
#endif

// Is my buddy in the level? (no if the handle is invalid)
extern int Buddy_handle[MAX_PLAYERS];

bool AINotify(object *obj, ubyte notify_type, void *info = NULL);
void AIDoFrame(object *obj);
void AIFrameAll(void);
bool AIInit(object *obj, ubyte ai_class, ubyte ai_type, ubyte ai_movement);
void AIInitAll(void);
void AIPowerSwitch(object *obj, bool f_on);
void AITurnTowardsDir(object *obj, /*velocity *new_vel,*/ vector *goal_dir/*, bool remain_level*/, float turn_rate);
void AIMoveTowardsDir(object *obj, vector *dir, float scale = 1.0f);
bool AIMoveTowardsPosition(object *obj, /*velocity *new_vel,*/ vector *pos, float scale, bool stop_at_end_point, vector *mdir, bool *f_moved);
void AITurnTowardsPosition(object *obj, /*velocity *new_vel,*/ vector *pos/*, bool remain_level*/);
bool AIFindHidePos(object *hide_obj, object *view_obj, vector *hpos, int *hroom, float max_hide_time = 3.0f);
int  AIFindRoomWithFlag(object *obj, int flag);
object *AIFindObjOfType(object *obj, int type, int id, bool f_ignore_init_room, int parent_handle = OBJECT_HANDLE_NONE);
float AIFindDist(vector *s_pos, int s_roomnum, vector *e_pos, int e_roomnum, int flags);
bool AIStatusCircleFrame(object *obj, object *g_obj, float dist, float c_dist, int *status_reg);
bool AIObjEnemy(object *obj, object *target);
bool AISetTarget(object *obj, int handle);
void AIDestroyObj(object *obj);
bool AIObjFriend(object *obj, object *target);
void AIUpdateAnim(object *obj);
bool AITurnTowardsMatrix(object *obj, float turn_rate, matrix *g_orient);
int AIFindRandomRoom(object *obj, ai_frame *ai_info, goal *goal_ptr, int avoid_room, int min_depth, int max_depth, bool f_check_path, bool f_cur_room_ok, int *depth);
int AIMakeNextRoomList(int roomnum, int *next_rooms, int max_rooms);

#endif
