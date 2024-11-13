#include "version.h"

// Make sure to rebuild "version.obj" every time you compile your project!
const char* BulbToys::GetBuildDateTime()
{
	return __DATE__ " " __TIME__;
}