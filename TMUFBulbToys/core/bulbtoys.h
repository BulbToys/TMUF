#pragma once

/*
	Welcome to the user-friendly BulbToys header file.

	Everything you will ever need is right here; there is no reason to touch any of the CPP nor any of the other headers.
	Feel free to include whichever individual header files you need, or all of them (ie. this entire file).
*/

// Contains setup procedures necessary for initialization
#include "bulbtoys/main.h"

// Contains the module framework - the main way to make your trainer do stuff
#include "bulbtoys/module.h"

// Contains initialization file (INI) reading functionality
#include "bulbtoys/settings.h"

// Contains ImGui functionality, in-house widgets, and a custom window framework
#include "bulbtoys/my_imgui.h"

// Contains basic function hooking functionality
#include "bulbtoys/hook.h"

// Contains various utility functions, such as Read<T> and Write<T> for reading and writing memory, etc...
#include "bulbtoys/utils.h"