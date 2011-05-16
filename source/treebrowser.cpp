/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * filebrowser.cpp
 *
 * Generic file routines - reading, writing, browsing
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gccore.h>
#include <string.h>
#include <wiiuse/wpad.h>
#include <sys/dir.h>

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
    struct iv_series *series;
    struct iv_episode *episodes;
    int episodes_len;
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
    info->currentNode = chosenNode;
    info->selIndex = 0;
    info->pageIndex = 0;
    return 0;
}

static int DownloadEp(TreeBrowserNode *node)
{
    struct IviewGlobalMetadata *globalMetadata = (struct IviewGlobalMetadata *)rootNode->data;
    /* TODO(joel): Is ->url trusted to be null terminated? */
    char *path = strdup(((struct iv_episode *)node->data)->url);
    if (path == NULL) {
        /* TODO(joel): Bail appropriately. */
    }
    const int fd = open(basename(path), O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (0 > fd) {
        /* TODO(joel): Fetch errno, and bail appropriately. */
    }
    const int result = iv_easy_fetch_episode(globalMetadata->config,
            (struct iv_episode *)node->data, fd);
    close(fd);
    return result;
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

static int GetSeries(TreeBrowserNode *node)
{
    struct IviewGlobalMetadata *globalMetadata =
        (struct IviewGlobalMetadata *)rootNode->data;
    struct IviewSeriesMetadata *seriesMetadata =
        (struct IviewSeriesMetadata *)node->data;

    // Populate children with mandatory "Up" entry
    node->children = (TreeBrowserNode *)calloc(1, sizeof(TreeBrowserNode));
    node->numChildren = 1;
    populateNode(&node->children[0], NULL, NULL, NULL, rootNode,
            rootNode->children, rootNode->numChildren, (char *)"Up");

    // Fetch episodes
    seriesMetadata->episodes_len = iv_easy_series(globalMetadata->config,
            seriesMetadata->series, &seriesMetadata->episodes);
    if(0 > seriesMetadata->episodes_len) {
        return -1;
    }

    // Make space in ->children for the children
    TreeBrowserNode *tmpChildren = (TreeBrowserNode *)realloc(node->children,
            (seriesMetadata->episodes_len+1)*sizeof(TreeBrowserNode));
    if(NULL == tmpChildren) {
        return -6;
    }
    node->children = tmpChildren;
    node->numChildren = seriesMetadata->episodes_len+1;
    TreeBrowserNode *children = &node->children[1];
    for(int j=0; j<seriesMetadata->episodes_len; j++) {
        // Store item struct in data element
        populateNode(&children[j], &DownloadEp, NULL,
                &seriesMetadata->episodes[j], node, NULL, 0,
                (char *)(seriesMetadata->episodes[j].title));
    }
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
    if(NULL != metadata->episodes) {
        iv_destroy_series(metadata->episodes, metadata->episodes_len);
    }
    return 0;
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

    // populate tree root node with the series index elements
    info->currentNode->children =
        (TreeBrowserNode *)calloc(globalMetadata->index_len,
                sizeof(TreeBrowserNode));
    info->currentNode->numChildren = globalMetadata->index_len;

    for(int i=0; i<globalMetadata->index_len; i++) {
        // Initialise the entry
        TreeBrowserNode *c = &info->currentNode->children[i];
        struct IviewSeriesMetadata *seriesMetadata =
            (struct IviewSeriesMetadata *)calloc(1, sizeof(IviewSeriesMetadata));
        seriesMetadata->series = &globalMetadata->index[i];

        // Store series struct in data element
        populateNode(c, &GetSeries, &DestroyIviewItems, seriesMetadata, rootNode,
                NULL, 0, (char *)(globalMetadata->index[i].title));
    }
    return globalMetadata->index_len;
}
