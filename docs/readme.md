# Descent 3: Piccu Engine
Piccu Engine is a new engine for playing Descent 3 with enhanced quality of life features. It is licensed under the terms of the GNU GPL, version 3. 

# Features
Piccu Engine provides the following features:
- Centered window mode
- Scaling in window and fullscreen mode
- Fullscreen which does not change desktop settings (multi adapter support forthcoming)
- New mouse code
- Adjustable FOV, with automatic expansion for widescreen
- Missing features in OpenGL restored, including specular highlights and the ability to control mipmapping
- Cockpit actually works in widescreen, unlike InjectD3
- Faster iteration when many missions are present.
- Smoother UI (FPS limit raised to 60)
- OpenAL sound system with no crackling and environment reverb support.
- text scaling for the HUD. 
- Many bugfixes

# Installation
Piccu Engine can be dropped directly into an existing Descent 3 directory, but this may not be desired if you wish to run the 1.4 release.
In addition, Piccu Engine cannot currently be run from Program Files without issues. This will change in a future release.

When first running Piccu Engine, the game will save user generated content, namely savegames, demos, and pilot files into the user's Saved Games directory.
If you do not wish to do this, put an empty file named "piccu_portable" in the directory with Piccu Engine's executable, and the old behavior will be retained.
If there are any pilot files in the current directory, and the Piccu Engine saved games directory doesn't exist, it will ask if you want to make the install portable. 
Select "Yes" to make the install portable. Otherwise, Piccu Engine will copy all of the demos, pilots, and saves into the user's save game directory. 

If you want to install Piccu Engine into another directory, the following files should be copied over from a patched 1.4 installation.
- d3.hog
- extra.hog
- extra13.hog
- merc.hog (if you own the Descent 3: Mercenary expansion pack)
- missions/d3.mn3
- missions/d3_2.mn3
- missions/d3voice1.hog
- missions/d3voice2.hog
- missions/training.mn3
- missions/merc.mn3 (if you own the Descent 3: Mercenary expansion pack)

The following directories should be copied over if you want to play Piccu Engine in multiplayer
- netgames
- missions/Fury.mn3 (stock Anarchy levels)
- missions/bedlam.mn3 (stock Capture the Flag levels)

The following directories may be copied over, but are not critical
- movies

# Known Issues
The following issues are known about the current release
- Terrain rendering limits were raised, but the rendering code was not improved, making it significantly laggy even on modern machines. Adjust the draw distance and level of detail to make it run faster.

# Future Plans
The following features are intended for future releases of Piccu Engine
- Better versioning in game
- Significantly faster rendering, using GPU shaders to reimplement missing features like bump mapping. This will fix the terrain rendering slowness. 
- Ability to run either in "portable" mode (save config locally instead of in registry, and save locally) or to save data in user directories, so it can be placed in Program Files on Windows.
- Fix long-standing resource management issues, including assets being loaded by custom missions persisiting across level changes and game saves and loads.
- Automatic detection and loading of Descent 3 installs on the Steam and GOG platforms.

# Credits
- btb: MVE source from D2X
- GZDoom: Revision checking CMake script
- OpenAL Soft: Primary sound library

