/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * filebrowser.cpp
 *
 * Generic file routines - reading, writing, browsing
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiiuse/wpad.h>
#include <sys/dir.h>
#include <malloc.h>
#include <iview.h>

#include "treebrowser.h"
#include "menu.h"

TreeBrowserInfo treeBrowser;
TreeBrowserEntry * treeBrowserList = NULL; // list of immediate children in treeBrowser

TreeBrowserEntry rootNode;

/****************************************************************************
 * ResetTreeBrowser()
 * Clears the tree browser memory, and allocates one initial entry
 ***************************************************************************/
void ResetTreeBrowser()
{
	treeBrowser.numEntries = 0;
	treeBrowser.selIndex = 0;
	treeBrowser.pageIndex = 0;

	// Clear any existing values
	if(treeBrowserList != NULL)
	{
		free(treeBrowserList);
		treeBrowserList = NULL;
	}
	// set aside space for 1 entry
	treeBrowserList = (TreeBrowserEntry *)malloc(sizeof(TreeBrowserEntry));
	memset(treeBrowserList, 0, sizeof(TreeBrowserEntry));
}

/****************************************************************************
 * UpdateNodeName()
 * Update curent directory name for file treeBrowser
 ***************************************************************************/
static int UpdateNodeName()
{
    TreeBrowserEntry *tbl = &treeBrowserList[treeBrowser.selIndex];
    // Did we try to go up a menu?
    if(0 == treeBrowser.selIndex) {
        // If we're at the series list there's nothing to do
        if(&rootNode == tbl->parent) {
            return 0;
        }
        // Otherwise jump up a level
        if(NULL != tbl->parent->parent) {
            treeBrowser.numEntries = tbl->parent->parent->numChildren;
            treeBrowserList = tbl->parent->parent->children;
        }
    } else {
        treeBrowser.numEntries = tbl->numChildren;
        treeBrowserList = tbl->children;
    }
    treeBrowser.selIndex = 0;
    treeBrowser.pageIndex = 0;
    return 1;
}

/****************************************************************************
 * BrowserChangeFolder
 *
 * Update current directory and set new entry list if directory has changed
 ***************************************************************************/
int BrowserChangeNode()
{
    return UpdateNodeName() ? treeBrowser.numEntries : -1;
}

/****************************************************************************
 * BrowseTree
 * Displays a list of files on the selected device
 ***************************************************************************/
int BrowseTree()
{
    int return_value = 0;

    // libiview variable initialisation - can't declare these inline as g++
    // barfs about jumping over declarations.

    struct iv_config *iview_config;

    struct iv_series *index;
    int index_len;

    TreeBrowserEntry *r_children;

    // root node value initialisation
    rootNode.parent = NULL;
    rootNode.children = NULL;
    rootNode.numChildren = 0;
    snprintf(rootNode.name, MAXJOLIET, "%s", "ROOT");
    snprintf(rootNode.name, MAXDISPLAY, "%s", "ROOT");

    // start querying ABC iview servers
    if(0 > iv_easy_config(&iview_config)) {
        return_value = -1;
    }
    index_len = iv_easy_index(iview_config, &index);
    if(0 > index_len) {
        return_value = -2;
        goto config_cleanup;
    }

    // Artificial limit - hack for testing
    index_len = 5;

    // populate tree root node with the series index elements
    rootNode.children = (TreeBrowserEntry *)calloc(index_len+1, sizeof(rootNode));
    rootNode.numChildren = index_len+1;
    return_value = index_len+1;
    rootNode.children[0].parent = NULL;
    rootNode.children[0].children = NULL;
    rootNode.children[0].numChildren = 0;
    snprintf(rootNode.children[0].name, MAXJOLIET, "%s", "Up");
    snprintf(rootNode.children[0].displayname, MAXDISPLAY, "%s", "Up");
    r_children = &rootNode.children[1];
    for(int i=0; i<index_len; i++) {
        // Initialise the entry
        TreeBrowserEntry *c = &r_children[i];
        c->parent = &rootNode;
        c->children = NULL;
        c->numChildren = 0;
        snprintf(c->name, MAXJOLIET, "%s", index[i].title);
        snprintf(c->displayname, MAXDISPLAY, "%s", index[i].title);

        // Populate the child nodes with episodes
        struct iv_item *items;
        const int items_len =
            iv_easy_series_items(iview_config, &index[i], &items);
        if(0 > items_len) {
            continue;
        }
        c->children = (TreeBrowserEntry *)calloc(items_len+1, sizeof(TreeBrowserEntry));
        c->numChildren = items_len+1;
        c->children[0].parent = c;
        c->children[0].children = NULL;
        c->children[0].numChildren = 0;
        snprintf(c->children[0].name, MAXJOLIET, "%s", "Up");
        snprintf(c->children[0].displayname, MAXDISPLAY, "%s", "Up");
        TreeBrowserEntry *c_children = &c->children[1];
        for(int j=0; j<items_len; j++) {
            TreeBrowserEntry *c2 = &c_children[j];
            c2->parent = c;
            c2->children = NULL;
            c2->numChildren = 0;
            snprintf(c2->name, MAXJOLIET, "%s", items[j].title);
            snprintf(c2->displayname, MAXDISPLAY, "%s", items[j].title);
        }
        iv_destroy_series_items(items, items_len);
    }
    treeBrowser.numEntries = index_len+1;
    treeBrowserList = rootNode.children;
    // Cleanup
    iv_destroy_index(index, index_len);
config_cleanup:
    iv_destroy_config(iview_config);
    return return_value;
}
