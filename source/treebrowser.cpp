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
 * BrowserChangeFolder
 *
 * Update current directory and set new entry list if directory has changed
 ***************************************************************************/
void BrowserChangeNode(TreeBrowserInfo *info)
{
    TreeBrowserNode *chosenNode = &info->currentNode->children[info->selIndex];
    if(NULL == chosenNode) {
        return;
    }
    if(NULL != chosenNode->selectedEvent) {
        if(chosenNode->selectedEvent(chosenNode)) {
            return;
        }
    }
    // Did we try to go up a menu?
    if(0 == info->selIndex) {
        // If we're at the series list there's nothing to do
        if(rootNode == chosenNode->parent) {
            return;
        }
        // Otherwise jump up a level
        info->currentNode = info->currentNode->parent;
    } else {
        info->currentNode = chosenNode;
    }
    info->selIndex = 0;
    info->pageIndex = 0;
    return;
}

static void
populateNode(
        TreeBrowserNode *node,
        int (*selectedEvent)(TreeBrowserNode *node),
        void *data,
        TreeBrowserNode *parent,
        TreeBrowserNode *children,
        int numChildren,
        char *name)
{
    node->selectedEvent = selectedEvent;
    node->data = data;
    node->parent = parent;
    node->children = children;
    node->numChildren = numChildren;
    snprintf(node->name, MAXJOLIET, "%s", name);
    snprintf(node->displayname, MAXDISPLAY, "%s", name);
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
    populateNode(info->currentNode, NULL, NULL, NULL, NULL, 0, (char *)"ROOT");

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
    return_value = index_len+1;

    // populate tree root node with the series index elements
    info->currentNode->children = (TreeBrowserNode *)calloc(index_len+1, sizeof(TreeBrowserNode));
    info->currentNode->numChildren = index_len+1;
    populateNode(&info->currentNode->children[0], NULL, NULL, NULL, NULL, 0, (char *)"Up");
    r_children = &info->currentNode->children[1];
    for(int i=0; i<index_len; i++) {
        // Initialise the entry
        TreeBrowserNode *c = &r_children[i];
        populateNode(c, NULL, &index[i], rootNode, NULL, 0, (char *)index[i].title);

        // Populate children with mandatory "Up" entry
        c->children = (TreeBrowserNode *)calloc(1, sizeof(TreeBrowserNode));
        c->numChildren = 1;
        populateNode(&c->children[0], NULL, NULL, c, NULL, 0, (char *)"Up");

        // Fetch episodes
        struct iv_item *items;
        const int items_len =
            iv_easy_series_items(iview_config, &index[i], &items);
        if(0 > items_len) {
            continue;
        }

        // Make space in ->children for the children
        TreeBrowserNode *tmpChildren =
            (TreeBrowserNode *)realloc(c->children, (items_len+1)*sizeof(TreeBrowserNode));
        if(NULL == tmpChildren) {
            iv_destroy_series_items(items, items_len);
            break;
        }
        c->children = tmpChildren;
        c->numChildren = items_len+1;
        TreeBrowserNode *c_children = &c->children[1];
        for(int j=0; j<items_len; j++) {
            TreeBrowserNode *c2 = &c_children[j];
            populateNode(c2, NULL, &items[j], c, NULL, 0, (char *)items[j].title);
        }
        iv_destroy_series_items(items, items_len);
    }
    // Cleanup
    iv_destroy_index(index, index_len);
config_cleanup:
    iv_destroy_config(iview_config);
    return return_value;
}
