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

struct IviewGlobalMetadata {
    struct iv_config *config;
    struct iv_series *index;
    int index_len;
};

struct IviewSeriesMetadata {
    struct iv_item *items;
    int items_len;
};

int
WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label);

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

TreeBrowserNode *GetSelectedNode(TreeBrowserInfo *info)
{
    return &info->currentNode->children[info->selIndex];
}

int IsLeafNode(TreeBrowserNode *node)
{
    return (0 == node->numChildren) && (NULL == node->children);
}

int TriggerSelectedAction(TreeBrowserNode *node)
{
    if(NULL == node->selectedEvent) {
        return 0;
    }
    return node->selectedEvent(node);
}

/****************************************************************************
 * BrowserChangeFolder
 *
 * Update current directory and set new entry list if directory has changed
 *
 * Returns 0 If the current node was changed as requested, -1 if the current
 * node is unchanged.
 ***************************************************************************/
int BrowserChangeNode(TreeBrowserInfo *info)
{
    TreeBrowserNode *chosenNode = GetSelectedNode(info);
    if(NULL == chosenNode || 0 == chosenNode->numChildren) {
        return -1;
    }
    // Did we try to go up a menu?
    if(0 == info->selIndex) {
        // If we're at the series list there's nothing to do
        if(rootNode == chosenNode->parent) {
            return -1;
        }
        // Otherwise jump up a level
        info->currentNode = info->currentNode->parent;
    } else {
        info->currentNode = chosenNode;
    }
    info->selIndex = 0;
    info->pageIndex = 0;
    return 0;
}

static int DownloadEp(TreeBrowserNode *node)
{
    struct IviewGlobalMetadata *globalMetadata = (struct IviewGlobalMetadata *)rootNode->data;
    struct iv_auth *auth;
    int result = iv_get_auth(globalMetadata->config, &auth);
    if(IV_OK != result) {
       return result;
    }
    xmlChar *path = xmlStrdup(((struct iv_item *)node->data)->url);
    result = iv_fetch_video(auth, (struct iv_item *)node->data, "bazinga.flv");
    if(IV_OK != result) {
        iv_destroy_auth(auth);
        return result;
    }
    iv_destroy_auth(auth);
    free(path);
    return 0;
}

static void DestroyTree(TreeBrowserNode *node)
{
    // PRE: Destroy children first
    if(0 < node->numChildren && NULL != node->children) {
        for(int i=0; i<node->numChildren; i++) {
            DestroyTree(&node->children[i]);
        }
    }

    // IN: Destroy node in current node
    if(NULL != node->destroyedEvent) {
        node->destroyedEvent(node);
    }
    free(node->name);
    free(node->displayname);
    free(node->children);
    node->numChildren = 0;
}

/****************************************************************************
 * DestroyTreeBrowser
 * Triggers destroyEvent callback to free user data and frees child nodes
 ***************************************************************************/
void DestroyTreeBrowser(TreeBrowserInfo *info)
{
    DestroyTree(rootNode);
    free(rootNode);
}

static int DestroyIviewConfig(TreeBrowserNode *node)
{
    struct IviewGlobalMetadata *metadata = (struct IviewGlobalMetadata *)node->data;
    if(NULL != metadata->config) {
        iv_destroy_config(metadata->config);
    }
    if(NULL != metadata->index) {
        iv_destroy_index(metadata->index, metadata->index_len);
    }
    return 0;
}

static int DestroyIviewItems(TreeBrowserNode *node)
{
    struct IviewSeriesMetadata *metadata = (struct IviewSeriesMetadata *)node->data;
    if(NULL != metadata->items) {
        iv_destroy_series_items(metadata->items, metadata->items_len);
    }
    return 0;
}

static void
populateNode(
        TreeBrowserNode *node,
        int (*selectedEvent)(TreeBrowserNode *node),
        int (*destroyedEvent)(TreeBrowserNode *node),
        void *data,
        TreeBrowserNode *parent,
        TreeBrowserNode *children,
        int numChildren,
        char *name)
{
    node->selectedEvent = selectedEvent;
    node->destroyedEvent = destroyedEvent;
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
    struct IviewGlobalMetadata *globalMetadata = (struct IviewGlobalMetadata *)
        calloc(1, sizeof(struct IviewGlobalMetadata));
    if(NULL == globalMetadata) {
        return -1;
    }

    if(NULL == info->currentNode) {
        return -2;
    }

    // root node value initialisation
    info->currentNode = rootNode;
    populateNode(info->currentNode, NULL, &DestroyIviewConfig, globalMetadata,
            NULL, NULL, 0, (char *)"ROOT");

    // start querying ABC iview servers
    if(0 > iv_easy_config(&globalMetadata->config)) {
        return -3;
    }

    // Fetch series index
    globalMetadata->index_len = iv_easy_index(globalMetadata->config,
            &globalMetadata->index);
    if(0 > globalMetadata->index_len) {
        return -4;
    }

    // Artificial limit - hack for testing
    globalMetadata->index_len = 5;

    // populate tree root node with the series index elements
    info->currentNode->children =
        (TreeBrowserNode *)calloc(globalMetadata->index_len + 1,
                sizeof(TreeBrowserNode));
    info->currentNode->numChildren = globalMetadata->index_len + 1;
    populateNode(&info->currentNode->children[0], NULL, NULL, NULL, NULL, NULL,
            0, (char *)"Up");

    TreeBrowserNode *r_children = &info->currentNode->children[1];
    for(int i=0; i<globalMetadata->index_len; i++) {
        // Initialise the entry
        TreeBrowserNode *c = &r_children[i];
        struct IviewSeriesMetadata *seriesMetadata =
            (struct IviewSeriesMetadata *)calloc(1, sizeof(IviewSeriesMetadata));

        // Store series struct in data element
        populateNode(c, NULL, &DestroyIviewItems, seriesMetadata, rootNode,
                NULL, 0, (char *)(globalMetadata->index[i].title));

        // Populate children with mandatory "Up" entry
        c->children = (TreeBrowserNode *)calloc(1, sizeof(TreeBrowserNode));
        c->numChildren = 1;
        populateNode(&c->children[0], NULL, NULL, NULL, c, NULL, 0,
                (char *)"Up");

        // Fetch episodes
        seriesMetadata->items_len = iv_easy_series_items(globalMetadata->config,
                &globalMetadata->index[i], &seriesMetadata->items);
        if(0 > seriesMetadata->items_len) {
            continue;
        }

        // Make space in ->children for the children
        TreeBrowserNode *tmpChildren = (TreeBrowserNode *)realloc(c->children,
                (seriesMetadata->items_len+1)*sizeof(TreeBrowserNode));
        if(NULL == tmpChildren) {
            return -6;
        }
        c->children = tmpChildren;
        c->numChildren = seriesMetadata->items_len+1;
        TreeBrowserNode *c_children = &c->children[1];
        for(int j=0; j<seriesMetadata->items_len; j++) {
            TreeBrowserNode *c2 = &c_children[j];

            // Store item struct in data element
            populateNode(c2, &DownloadEp, NULL, &seriesMetadata->items[j], c,
                    NULL, 0, (char *)(seriesMetadata->items[j].title));
        }
    }
    return globalMetadata->index_len + 1;
}
