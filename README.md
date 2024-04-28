# Descent 3: Piccu Engine

This is a new engine for Descent 3, from the initial 1.5 source code release. It incorporates almost every change from InjectD3, among many more.

At the moment, this includes the following features:
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
- Many bugfixes

## Building
At the moment the build environment is only set up for Windows, but I hope to change this shortly. Building on Windows is done with CMake. 
