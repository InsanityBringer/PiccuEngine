#pragma once

#include <stdint.h>

//Abstract renderer handle, replacing the grSurface, grHardwareSurface, and grViewport classes used in the software renderer.
//This should allow this file to be passed around anywhere without much issue. 

struct RendViewport
{
	int x, y;
	int width, height;
};

//One of these is associated with each window.
struct RendHandle
{
	uint32_t handle;
	RendViewport default_viewport;

	RendHandle()
	{
		handle = 0;
		default_viewport = {};
	}

	explicit operator bool() const noexcept
	{
		return handle != 0;
	}
};
