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

#include "treebrowser.h"
#include "menu.h"

TREEBROWSERINFO treeBrowser;
TREEBROWSERENTRY * treeBrowserList = NULL; // list of immediate children in treeBrowser

TREEBROWSERENTRY * rootNode;

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
	//sprintf(treeBrowser.dir, "/");
	//sprintf(rootdir, "sd:/");
	//_ParseDirectory(); // Parse root directory
	//return treeBrowser.numEntries;
	return 0;
}
