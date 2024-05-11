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

//
//  Object death info flags
//

//Not a bit flag, but an override value
#define DF_DEFAULT					-1				//use the default death for this object

//Unused flag
#define DF_UNUSED					0x0000001

//Delay options
#define DF_DELAY_FROM_ANIM			0x0000002		//delay time from death animation
#define DF_DELAY_SPARKS				0x0000004		//delay with sparks
#define DF_DELAY_LOSES_ANTIGRAV		0x0000008		//object gets gravity during delay
#define DF_DELAY_SMOKES				0x0000010		//delay with smoke
#define DF_DELAY_FLYING				0x0100000		//delay with object flying up into the air
#define DF_DELAY_FIREBALL			0x0200000		//delay with fireballs
#define DF_DELAY_FADE_AWAY			0x0400000		//fade away
#define DF_DELAY_NO_TUMBLE_FLY		0x2000000		//don't tumble while flying up in the air

//Options for what happens on death
#define DF_FIREBALL					0x0000020		//there are fireballs when the object dies
#define DF_BREAKS_APART				0x0000040		//the object breaks into pieces when it dies
#define DF_BLAST_RING				0x0000080		//a blast ring is created when the object dies
#define DF_REMAINS					0x0000100		//the object does not go away when it does
#define DF_LOSES_ANTIGRAV			0x0000200		//object gets gravity on death
#define DF_FADE_AWAY				0x0800000		//fades away

//Explosion size if there is a death fireball
#define DF_EXPL_SMALL				0x0000000		//use a small explosion
#define DF_EXPL_MEDIUM				0x0000400		//use a medium explosion
#define DF_EXPL_LARGE				0x0000800		//use a large explosion
#define DF_EXPL_SIZE_MASK			0x0000c00
#define DF_EXPL_SIZE_SHIFT			10

//What happens when this object hits something during delay
#define DF_CONTACT_FIREBALL			0x0001000		//creates fireballs
#define DF_CONTACT_BREAKS_APART		0x0002000		//break apart
#define DF_CONTACT_BLAST_RING		0x0004000		//blast ring
#define DF_CONTACT_REMAINS			0x0008000		//stays around

//Whether the debris puts off smoke
#define DF_DEBRIS_SMOKES			0x0010000		//the debris that's created smokes

//What happens to the debris when it times out or hits something
#define DF_DEBRIS_FIREBALL			0x0020000		//creates fireballs
#define DF_DEBRIS_BLAST_RING		0x0040000		//blast ring
#define DF_DEBRIS_REMAINS			0x0080000		//stays around

//Sound option
#define DF_DELAY_SOUND				0x1000000		//play sound at start of fade
