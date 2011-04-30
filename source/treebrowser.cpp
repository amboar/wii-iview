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
TreeBrowserNode * treeBrowserList = NULL; // list of immediate children in treeBrowser

TreeBrowserNode *rootNode;

/****************************************************************************
 * ResetTreeBrowser()
 * Clears the tree browser memory, and allocates one initial entry
 ***************************************************************************/
void ResetTreeBrowser(TreeBrowserInfo *info)
{
	info->numEntries = 0;
	info->selIndex = 0;
	info->pageIndex = 0;

	// Clear any existing values
	if(info->currentNode != NULL)
	{
		free(info->currentNode);
                info->currentNode = NULL;
	}

	// set aside space for 1 entry
        rootNode = (TreeBrowserNode *)calloc(1, sizeof(TreeBrowserNode));
	info->currentNode = rootNode;
}

/****************************************************************************
 * UpdateNodeEntries()
 * Update curent directory name for file treeBrowser
 ***************************************************************************/
static void UpdateNodeEntries(TreeBrowserInfo *info)
{
    TreeBrowserNode *chosenNode = &info->currentNode[info->selIndex];
    if(NULL == chosenNode || NULL == chosenNode->parent) {
        return;
    }
    // Did we try to go up a menu?
    if(0 == info->selIndex) {
        // If we're at the series list there's nothing to do
        if(rootNode == chosenNode->parent) {
            return;
        }
        // Otherwise jump up a level
        if(NULL != chosenNode->parent->parent) {
            info->numEntries = chosenNode->parent->parent->numChildren;
            info->currentNode = chosenNode->parent->parent->children;
        }
    } else {
        info->numEntries = chosenNode->numChildren;
        info->currentNode = chosenNode->children;
    }
    info->selIndex = 0;
    info->pageIndex = 0;
}

/****************************************************************************
 * BrowserChangeFolder
 *
 * Update current directory and set new entry list if directory has changed
 ***************************************************************************/
void BrowserChangeNode(TreeBrowserInfo *info)
{
    UpdateNodeEntries(info);
    return;
}

/****************************************************************************
 * BrowseTree
 * Displays a list of files on the selected device
 ***************************************************************************/
int BrowseTree(TreeBrowserInfo *info)
{
    int return_value = 0;

    // libiview variable initialisation - can't declare these inline as g++
    // barfs about jumping over declarations.

    struct iv_config *iview_config;

    struct iv_series *index;
    int index_len;

    if(NULL == info->currentNode) {
        return -1;
    }

    TreeBrowserNode *r_children;

    // root node value initialisation
    info->currentNode = rootNode;
    info->currentNode->parent = NULL;
    info->currentNode->children = NULL;
    info->currentNode->numChildren = 0;
    snprintf(info->currentNode->name, MAXJOLIET, "%s", "ROOT");
    snprintf(info->currentNode->name, MAXDISPLAY, "%s", "ROOT");

    // start querying ABC iview servers
    if(0 > iv_easy_config(&iview_config)) {
        return_value = -2;
    }
    index_len = iv_easy_index(iview_config, &index);
    if(0 > index_len) {
        return_value = -3;
        goto config_cleanup;
    }

    // Artificial limit - hack for testing
    index_len = 5;

    // populate tree root node with the series index elements
    info->currentNode->children = (TreeBrowserNode *)calloc(index_len+1, sizeof(TreeBrowserNode));
    info->currentNode->numChildren = index_len+1;
    return_value = index_len+1;
    info->currentNode->children[0].parent = NULL;
    info->currentNode->children[0].children = NULL;
    info->currentNode->children[0].numChildren = 0;
    snprintf(info->currentNode->children[0].name, MAXJOLIET, "%s", "Up");
    snprintf(info->currentNode->children[0].displayname, MAXDISPLAY, "%s", "Up");
    r_children = &info->currentNode->children[1];
    for(int i=0; i<index_len; i++) {
        // Initialise the entry
        TreeBrowserNode *c = &r_children[i];
        c->parent = rootNode;
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
        c->children = (TreeBrowserNode *)calloc(items_len+1, sizeof(TreeBrowserNode));
        c->numChildren = items_len+1;
        c->children[0].parent = c;
        c->children[0].children = NULL;
        c->children[0].numChildren = 0;
        snprintf(c->children[0].name, MAXJOLIET, "%s", "Up");
        snprintf(c->children[0].displayname, MAXDISPLAY, "%s", "Up");
        TreeBrowserNode *c_children = &c->children[1];
        for(int j=0; j<items_len; j++) {
            TreeBrowserNode *c2 = &c_children[j];
            c2->parent = c;
            c2->children = NULL;
            c2->numChildren = 0;
            snprintf(c2->name, MAXJOLIET, "%s", items[j].title);
            snprintf(c2->displayname, MAXDISPLAY, "%s", items[j].title);
        }
        iv_destroy_series_items(items, items_len);
    }
    info->numEntries = index_len+1;
    info->currentNode = info->currentNode->children;
    // Cleanup
    iv_destroy_index(index, index_len);
config_cleanup:
    iv_destroy_config(iview_config);
    return return_value;
}
