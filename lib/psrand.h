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

#undef RAND_MAX

#define RAND_MAX	0x7fff

void ps_srand(unsigned int seed);
int ps_rand(void);

//[ISB] I wanted to use a std::linear_congruential_engine here but it doesn't allow a shift to be specialized.
//Class form of ps_rand to allow isolating various random sources to avoid changing the seed every frame.. 
//Eventually I'd like to make the game use a mersenne twister or different algorithm for better randomization.
class PSRand
{
	unsigned int state;
public:
	PSRand()
	{
		state = 1;
	}

	PSRand(unsigned int newseed)
	{
		state = newseed;
	}

	void seed(unsigned int newseed)
	{
		state = newseed;
	}

	int operator()()
	{
		state = state * 214013 + 2531011;
		return (state >> 16) & RAND_MAX;
	}
};
