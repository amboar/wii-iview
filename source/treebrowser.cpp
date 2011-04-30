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

TREEBROWSERINFO treeBrowser;
TREEBROWSERENTRY * treeBrowserList = NULL; // list of immediate children in treeBrowser

TREEBROWSERENTRY rootNode;

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
	treeBrowserList = (TREEBROWSERENTRY *)malloc(sizeof(TREEBROWSERENTRY));
	memset(treeBrowserList, 0, sizeof(TREEBROWSERENTRY));
}

/****************************************************************************
 * UpdateNodeName()
 * Update curent directory name for file treeBrowser
 ***************************************************************************/
int UpdateNodeName()
{
    /*
	int size=0;
	char * test;
	char temp[1024];

	// current directory doesn't change
	if (strcmp(treeBrowserList[treeBrowser.selIndex].name,".") == 0)
	{
		return 0;
	}
	// go up to parent directory
	else if (strcmp(treeBrowserList[treeBrowser.selIndex].name,"..") == 0)
	{
		// determine last subdirectory namelength
		sprintf(temp,"%s",treeBrowser.dir);
		test = strtok(temp,"/");
		while (test != NULL)
		{
			size = strlen(test);
			test = strtok(NULL,"/");
		}

		// remove last subdirectory name
		size = strlen(treeBrowser.dir) - size - 1;
		treeBrowser.dir[size] = 0;

		return 1;
	}
	// Open a directory
	else
	{
		// test new directory namelength
		if ((strlen(treeBrowser.dir)+1+strlen(treeBrowserList[treeBrowser.selIndex].name)) < MAXPATHLEN)
		{
			// update current directory name
			sprintf(treeBrowser.dir, "%s/%s",treeBrowser.dir, treeBrowserList[treeBrowser.selIndex].name);
			return 1;
		}
		else
		{
			return -1;
		}
	}
        */
    TREEBROWSERENTRY *tbl = &treeBrowserList[treeBrowser.selIndex];
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
 * NodeSortCallback
 *
 * Quick sort callback to sort file entries with the following order:
 *   .
 *   ..
 *   <dirs>
 *   <files>
 ***************************************************************************/
static int
NodeSortCallback(const void *f1, const void *f2)
{
	/* Special case for implicit directories */
	if(((TREEBROWSERENTRY *)f1)->name[0] == '.' || ((TREEBROWSERENTRY *)f2)->name[0] == '.')
	{
		if(strcmp(((TREEBROWSERENTRY *)f1)->name, ".") == 0) { return -1; }
		if(strcmp(((TREEBROWSERENTRY *)f2)->name, ".") == 0) { return 1; }
		if(strcmp(((TREEBROWSERENTRY *)f1)->name, "..") == 0) { return -1; }
		if(strcmp(((TREEBROWSERENTRY *)f2)->name, "..") == 0) { return 1; }
	}

	/* If one is a file and one is a directory the directory is first. */
	if(0 == ((TREEBROWSERENTRY *)f1)->numChildren && (0 < ((TREEBROWSERENTRY *)f2)->numChildren)) return -1;
	if((0 < ((TREEBROWSERENTRY *)f1)->numChildren) && 0 == ((TREEBROWSERENTRY *)f2)->numChildren) return 1;

	return stricmp(((TREEBROWSERENTRY *)f1)->name, ((TREEBROWSERENTRY *)f2)->name);
}

/***************************************************************************
 * Browse subdirectories
 **************************************************************************/
static int
ParseDirectory()
{
    /*
	DIR_ITER *dir = NULL;
	char fulldir[MAXPATHLEN];
	char filename[MAXPATHLEN];
	struct stat filestat;

	// reset treeBrowser
	ResetTreeBrowser();

	// if we can't open the dir, try opening the root dir
	if (dir == NULL)
	{
		sprintf(treeBrowser.dir,"/");
		dir = diropen(rootdir);
		if (dir == NULL)
		{
			return -1;
		}
	}

	// index files/folders
	int entryNum = 0;

	while(dirnext(dir,filename,&filestat) == 0)
	{
		if(strcmp(filename,".") != 0)
		{
			TREEBROWSERENTRY *newBrowserList =
                                (TREEBROWSERENTRY *)realloc(browserList, (entryNum+1) * sizeof(TREEBROWSERENTRY));

			if(!newBrowserList) // failed to allocate required memory
			{
				ResetTreeBrowser();
				entryNum = -1;
				break;
			}
			else
			{
				treeBrowserList = newBrowserList;
			}
			memset(&(treeBrowserList[entryNum]), 0, sizeof(TREEBROWSERENTRY)); // clear the new entry

			strncpy(treeBrowserList[entryNum].name, filename, MAXJOLIET);

			if(strcmp(filename,"..") == 0)
			{
				sprintf(treeBrowserList[entryNum].displayname, "Up One Level");
			}
			else
			{
				strncpy(treeBrowserList[entryNum].displayname, filename, MAXDISPLAY);	// crop name for display
			}

			//treeBrowserList[entryNum].length = filestat.st_size;
			//treeBrowserList[entryNum].isdir = (filestat.st_mode & _IFDIR) == 0 ? 0 : 1; // flag this as a dir

			entryNum++;
		}
	}

	// close directory
	dirclose(dir);

	// Sort the file list
	qsort(treeBrowserList, entryNum, sizeof(TREEBROWSERENTRY), NodeSortCallback);

	treeBrowser.numEntries = entryNum;
	return entryNum;
        */
    return 0;
}

/****************************************************************************
 * BrowserChangeFolder
 *
 * Update current directory and set new entry list if directory has changed
 ***************************************************************************/
int BrowserChangeNode()
{
	if(!UpdateNodeName())
		return -1;

	//_ParseDirectory();

	return treeBrowser.numEntries;
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

    TREEBROWSERENTRY *r_children;

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
    rootNode.children = (TREEBROWSERENTRY *)calloc(index_len+1, sizeof(rootNode));
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
        TREEBROWSERENTRY *c = &r_children[i];
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
        c->children = (TREEBROWSERENTRY *)calloc(items_len+1, sizeof(TREEBROWSERENTRY));
        c->numChildren = items_len+1;
        c->children[0].parent = c;
        c->children[0].children = NULL;
        c->children[0].numChildren = 0;
        snprintf(c->children[0].name, MAXJOLIET, "%s", "Up");
        snprintf(c->children[0].displayname, MAXDISPLAY, "%s", "Up");
        TREEBROWSERENTRY *c_children = &c->children[1];
        for(int j=0; j<items_len; j++) {
            TREEBROWSERENTRY *c2 = &c_children[j];
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
