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

#ifndef AIGOAL_H_
#define AIGOAL_H_

#include "object.h"

// Clears and removes all goals for a robot
void GoalClearAll(object *obj); 

// Adds a new goal
int GoalAddGoal(object *obj, unsigned int goal_type, void *args, int level = 0, float influence = 1.0f, int f_goal = 0, int guid = -1, char subtype = 0);

// Adds a ending condition to a goal
int GoalAddDisabler(object *obj, int goal_index, ubyte ender_type, void *args, float percent = 1.0f, float interval = 0.0f);

// Adds a enabler condition to a goal
int GoalAddEnabler(object *obj, int goal_index, ubyte enabler_type, void *arg_struct, float percent, float interval);

// Clears one goal
void GoalClearGoal(object *obj, goal *goal_ptr, int notify_reason = AI_INVALID_INDEX);

// Init's an AI to no goals
void GoalInitTypeGoals(object *obj, int ai_type);

// Does the goal related stuff for an object per frame
void GoalDoFrame(object *obj);

// Goal's path is complete.  What next?
void GoalPathComplete(object *obj);

// Is a used goal currently enabled?
bool GoalIsGoalEnabled(object *obj, int goal_index);

// Determines minimum distance to look for nearby objects
float GoalDetermineTrackDist(object *obj);

// Returns a pointer to the highest priority non-blended goal
goal *GoalGetCurrentGoal(object *obj);

#endif
