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
    return 0;
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

	//return treeBrowser.numEntries;
	return 0;
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
    char *config_buf;
    ssize_t config_buf_len;

    struct iv_config *iview_config;

    char *index_buf;
    ssize_t index_buf_len;

    struct iv_series *index;
    int index_len;

    char *items_buf;

    // root node value initialisation
    rootNode.parent = NULL;
    rootNode.children = NULL;
    rootNode.numChildren = 0;
    snprintf(rootNode.name, MAXJOLIET, "%s", "ROOT");
    snprintf(rootNode.name, MAXDISPLAY, "%s", "ROOT");

    // start querying ABC iview servers
    config_buf_len = iv_get_xml_buffer(IV_CONFIG_URI, &config_buf);
    if(0 > config_buf_len) {
        return -1;
    }
    if(0 > iv_get_config(config_buf, config_buf_len, &iview_config)) {
        return_value = -2;
        goto config_cleanup;
    }
    index_buf_len = iv_get_index(iview_config, &index_buf);
    if(0 > index_buf_len) {
        return_value = -3;
        goto config_cleanup;
    }
    index_len = iv_parse_index(index_buf, &index);
    if(0 > index_len) {
        return_value = -4;
        goto index_buf_cleanup;
    }

    // populate tree root node with the series index elements
    rootNode.children = (TREEBROWSERENTRY *)calloc(index_len, sizeof(rootNode));
    rootNode.numChildren = index_len;
    return_value = index_len;
    for(int i=0; i<index_len; i++) {
        TREEBROWSERENTRY *c = &rootNode.children[i];
        c->parent = &rootNode;
        c->children = NULL;
        c->numChildren = 0;
        snprintf(c->name, MAXJOLIET, "%s", index[i].title);
        snprintf(c->displayname, MAXDISPLAY, "%s", index[i].title);
        /*
        const ssize_t items_buf_len =
            iv_get_series_items(iview_config, index[i], &items_buf);
        if(0 > items_buf_len) {
            continue;
        }
        struct iv_item *items;
        const int items_len =
            iv_parse_series_items(series_buf, series_buf_len, &items);
        if(0 > items_len) {
            goto series_buf_cleanup;
        }
series_buf_cleanup:
        iv_destroy_series_items(items, items_len);
        */
    }
    treeBrowserList = rootNode.children;
index_buf_cleanup:
    iv_destroy_xml_buffer(index_buf);
config_cleanup:
    iv_destroy_config(iview_config);
config_buf_cleanup:
    iv_destroy_xml_buffer(config_buf);
    //sprintf(treeBrowser.dir, "/");
    //sprintf(rootdir, "sd:/");
    //_ParseDirectory(); // Parse root directory
    //return treeBrowser.numEntries;
    return return_value;
}
