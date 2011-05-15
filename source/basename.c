/*
 * basename implementation stolen from libiso9660.
 * Copyright (C) 2008-2010
 * tipoloski, clava, shagkur, Tantric, joedj
 */

#include <string.h>
#include <ogcsys.h>

#define DIR_SEPARATOR       '/'

char* basename(char *path)
{
    s32 i;

    for (i = strlen(path) - 1; i >= 0; i--)
    {
        if (path[i] == DIR_SEPARATOR)
            return path + i + 1;
    }
    return path;
}
