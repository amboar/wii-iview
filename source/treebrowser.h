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
    int (*selectedEvent)(struct _TreeBrowserEntry *node);
    int (*destroyedEvent)(struct _TreeBrowserEntry *node);
    void *data; // User data for use by callbacks
    struct _TreeBrowserEntry *parent;
    struct _TreeBrowserEntry *children;
    int numChildren;
    char name[MAXJOLIET + 1]; // full filename
    char displayname[MAXDISPLAY + 1]; // name for browser display
} TreeBrowserNode;

typedef struct
{
    int selIndex; // currently selected index of browserList
    int pageIndex; // starting index of browserList page display
    TreeBrowserNode *currentNode;
} TreeBrowserInfo;

extern TreeBrowserInfo treeBrowser;
extern TreeBrowserNode *rootNode;

void ResetTreeBrowser(TreeBrowserInfo *info);
TreeBrowserNode *GetSelectedNode(TreeBrowserInfo *info);
int IsLeafNode(TreeBrowserNode *node);
int TriggerSelectedAction(TreeBrowserNode *node);
int BrowserChangeNode(TreeBrowserInfo *info);
int BrowseTree(TreeBrowserInfo *info);
void DestroyTreeBrowser(TreeBrowserInfo *info);

#endif
