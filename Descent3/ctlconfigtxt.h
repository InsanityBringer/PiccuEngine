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

#include "stringtable.h"

#define CtlText_FirePrimary		TXI_KB_FIREPRIMARY
#define CtlText_FireSecondary	TXI_KB_FIRESECONDARY
#define CtlText_Accelerate		TXI_KB_ACCELERATE
#define CtlText_Reverse			TXI_KB_REVERSE
#define CtlText_SlideHoriz		TXI_KB_SLIDEHORIZ
#define CtlText_SlideLeft		TXI_KB_SLIDELEFT
#define CtlText_SlideRight		TXI_KB_SLIDERIGHT
#define CtlText_SlideVert		TXI_KB_SLIDEVERT
#define CtlText_SlideUp			TXI_KB_SLIDEUP
#define CtlText_SlideDown		TXI_KB_SLIDEDOWN
#define CtlText_BankLeft		TXI_KB_BANKLEFT
#define CtlText_BankRight		TXI_KB_BANKRIGHT
#define CtlText_PitchUp			TXI_KB_PITCHUP
#define CtlText_PitchDown		TXI_KB_PITCHDOWN
#define CtlText_TurnLeft		TXI_KB_TURNLEFT
#define CtlText_TurnRight		TXI_KB_TURNRIGHT
#define CtlText_FireFlare		TXI_KB_FIREFLARE
#define CtlText_ToggleSlide		TXI_KB_TOGGLESLIDE
#define CtlText_ToggleBank		TXI_KB_TOGGLEBANK
#define CtlText_Heading			TXI_KB_HEADING
#define CtlText_Pitch			TXI_KB_PITCH
#define CtlText_Throttle		TXI_KB_THROTTLE
#define CtlText_Forward			TXI_KB_FORWARD
#define CtlText_Bank			TXI_KB_BANK
#define CtlText_Afterburn		TXI_KB_AFTERBURN
#define CtlText_Automap			TXI_KB_AUTOMAP
#define CtlText_PrevInv			TXI_KB_PREVINV
#define CtlText_NextInv			TXI_KB_NEXTINV
#define CtlText_InvUse			TXI_KB_INVUSE
#define CtlText_PrevCntMs		TXI_KB_PREVCNTMS
#define CtlText_NextCntMs		TXI_KB_NEXTCNTMS
#define CtlText_CntMsUse		TXI_KB_CNTMSUSE
#define CtlText_Headlight		TXI_KB_HEADLIGHT
#define CtlText_WpnCycP			TXI_KB_WPNPCYCLE
#define CtlText_WpnCycS			TXI_KB_WPNSCYCLE
#define CtlText_AudioTaunt1		TXI_KB_AUDIOTAUNT1
#define CtlText_AudioTaunt2		TXI_KB_AUDIOTAUNT2
#define CtlText_Rearview		TXI_KB_REARVIEW
#define CtlText_AudioTaunt3		TXI_KB_AUDIOTAUNT3
#define CtlText_AudioTaunt4		TXI_KB_AUDIOTAUNT4

#define CtlText_WeaponGroup		TXI_KB_WPNGRP
#define CtlText_MiscGroup		TXI_KB_MISCGRP
#define CtlText_ThrustGroup		TXI_KB_THRUSTGRP
#define CtlText_TurningGroup	TXI_KB_TURNINGGRP
