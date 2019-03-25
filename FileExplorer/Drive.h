#pragma once
#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#define IDI_OS 135
#define IDI_DATA 136

#define STRING_SIZE 256
class Drive
{	
public:
	Drive();
	~Drive();
	int _numberOfDrive;
	TCHAR** driveLetter;
	TCHAR** volumeName;
	TCHAR** driveType;
	int *icon;

	void getSystemDrives();
};

