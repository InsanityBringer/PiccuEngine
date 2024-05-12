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

#ifndef GAUGES_H
#define GAUGES_H

#include "pstypes.h"
#include "vecmat.h"
#include "hud.h"

//Hack vars for turning off the monitors
extern bool Disable_primary_monitor,Disable_secondary_monitor;

//	number of gauges
#define NUM_GAUGES						16

//	initializes cockpit gauges
void InitGauges(tStatMask gauge_mask);

//	deinitializes cockpit gauges
void CloseGauges();

//	renders gauges
void RenderGauges(vector *cockpit_pos, matrix *cockpit_mat, float *normalized_times, bool moving,bool reset=false);

//	flags certain gauges to be modified next frame.
void FlagGaugesModified(tStatMask mask_modified);

//	sets gauges as functional
void FlagGaugesFunctional(tStatMask mask);

//	sets gauges as gauges nonfunctional
void FlagGaugesNonfunctional(tStatMask mask);

//Returns the coordinates of the specified cockpit monitor
//Parameter:	window - 0 means primary monitor, 1 means secondary
//					x0,y0,x1,y1 - these are filled in with the coordinates of the montiro
//Returns: true if got coords, false if the monitor was animating
bool GetCockpitWindowCoords(int window,int *left,int *top,int *right,int *bot);

#endif
