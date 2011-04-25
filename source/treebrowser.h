/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * filebrowser.h
 *
 * Generic file routines - reading, writing, browsing
 ****************************************************************************/

#ifndef _FILEBROWSER_H_
#define _FILEBROWSER_H_

#include <unistd.h>
#include <gccore.h>

#define MAXJOLIET 255
#define MAXDISPLAY 45

typedef struct
{
	int numEntries; // # of entries in browserList
	int selIndex; // currently selected index of browserList
	int pageIndex; // starting index of browserList page display
} TREEBROWSERINFO;

typedef struct _TREEBROWSERENTRY
{
        struct _TREEBROWSERENTRY *parent;
        struct _TREEBROWSERENTRY *children;
	char numChildren;
	char name[MAXJOLIET + 1]; // full filename
	char displayname[MAXDISPLAY + 1]; // name for browser display
} TREEBROWSERENTRY;

extern TREEBROWSERINFO treeBrowser;
extern TREEBROWSERENTRY * treeBrowserList;
extern TREEBROWSERENTRY * rootNode;

int UpdateNodeName(int method);
void ResetTreeBrowser();
int BrowserChangeNode();

#endif
