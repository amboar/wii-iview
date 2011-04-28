/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * demo.cpp
 * Basic template/demonstration of libwiigui capabilities. For a
 * full-featured app using many more extensions, check out Snes9x GX.
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <network.h>

#include "FreeTypeGX.h"
#include "video.h"
#include "audio.h"
#include "menu.h"
#include "input.h"
#include "filelist.h"
#include "demo.h"

struct SSettings Settings;
int ExitRequested = 0;

void ExitApp()
{
	ShutoffRumble();
	StopGX();
	exit(0);
}

void
DefaultSettings()
{
	Settings.LoadMethod = METHOD_AUTO;
	Settings.SaveMethod = METHOD_AUTO;
	sprintf (Settings.Folder1,"libwiigui/first folder");
	sprintf (Settings.Folder2,"libwiigui/second folder");
	sprintf (Settings.Folder3,"libwiigui/third folder");
	Settings.AutoLoad = 1;
	Settings.AutoSave = 1;
}

static
void InitNetwork()
{
    /* Bring up network interface. */
    char localip[16] = {0};
    char gateway[16] = {0};
    char netmask[16] = {0};
    if_config(localip, netmask, gateway, TRUE);
}

int
main(int argc, char *argv[])
{
	InitVideo(); // Initialize video
	SetupPads(); // Initialize input
	InitAudio(); // Initialize audio
        InitNetwork(); // Initialize network
	fatInitDefault(); // Initialize file system
	InitFreeType((u8*)font_ttf, font_ttf_size); // Initialize font system
	InitGUIThreads(); // Initialize GUI

	DefaultSettings();
	MainMenu(MENU_SETTINGS);
}
