/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * filebrowser.h
 *
 * Generic file routines - reading, writing, browsing
 ****************************************************************************/

#ifndef _TREEBROWSER_H_
#define _TREEBROWSER_H_

#include <unistd.h>
#include <gccore.h>

#define MAXJOLIET 255
#define MAXDISPLAY 45

typedef struct _TreeBrowserEntry
{
    struct _TreeBrowserEntry *parent;
    struct _TreeBrowserEntry *children;
    char numChildren;
    char name[MAXJOLIET + 1]; // full filename
    char displayname[MAXDISPLAY + 1]; // name for browser display
} TreeBrowserNode;

typedef struct
{
    int numEntries; // # of entries in browserList
    int selIndex; // currently selected index of browserList
    int pageIndex; // starting index of browserList page display
} TreeBrowserInfo;

extern TreeBrowserInfo treeBrowser;
extern TreeBrowserNode *treeBrowserList;
extern TreeBrowserNode rootNode;

void ResetTreeBrowser();
int BrowserChangeNode();
int BrowseTree();

#endif
