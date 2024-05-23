/*
* Piccu Engine
* Copyright (C) 2024 SaladBadger
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

const char* blitVertexSrc =
"#version 330 core\n"
"\n"
"layout(location = 0) in vec2 position;\n"
"layout(location = 1) in vec2 uv;\n"
"\n"
"out vec2 outuv;\n"
"\n"
"void main()\n"
"{\n"
"gl_Position = vec4(position, 0.0, 1.0);\n"
"outuv = uv;"
"}\n"
"";

const char* blitFragmentSrc =
"#version 330 core\n"
"\n"
"in vec2 outuv;\n"
"\n"
"out vec4 color;\n"
"\n "
"uniform sampler2D heh;\n"
"uniform float gamma;\n"
"\n"
"void main()\n"
"{\n"
"vec4 sourcecolor = texture(heh, outuv);\n"
"color = vec4(pow(sourcecolor.rgb, vec3(gamma)), sourcecolor.a);\n"
"}\n"
"";

const char* testVertexSrc = 
"#version 330 core\n"
"\n"
"layout(std140) uniform CommonBlock\n"
"{\n"
"	mat4 projection;\n"
"	mat4 modelview;\n"
"} commons;\n"
"\n"
"layout(location = 0) in vec3 position;\n"
"layout(location = 1) in float x1;\n"
"layout(location = 2) in vec2 uv;\n"
"\n"
"out vec2 outuv;\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position = commons.projection * commons.modelview * vec4(position, 1.0);\n"
"	outuv = uv;\n"
"}\n"
"";

const char* testFragmentSrc = 
"#version 330 core\n"
"\n"
"uniform sampler2D colortexture;\n"
"\n"
"in vec2 outuv;\n"
"\n"
"out vec4 color;\n"
"\n"
"void main()\n"
"{\n"
"	color = texture(colortexture, outuv);\n"
"}\n"
"";
