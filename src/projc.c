/*
 * A utility for creating C projects on Windows or Linux and
 *      generates a Makefile for both Windows (nmake compatible) 
 *      and Linux (man(1) make)
 *
 *      Creates three directories to be used in the following way 
 *          lib  -| Header files and associated items for preprocessing
 *                | includes
 *          src  -| The source C file for main compilation
 *          test -| A testing directory for tests to run against src
 *                | and library functions
 *
 *   Copyright (C) 2017 John Andersen
 *      Email: johnandersen185@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

/* All of the constant arrays are rounded up to the nearest
 * byte to fit into a page better
 */
static const char GCC_MAKE_MACROS[128] = "\
IDIR =./include\n\
CC=gcc\n\
CFLAGS=-I$(IDIR)\n\
ODIR=obj\n\
LDIR =./lib\n\
LIBS=\n\n\
_DEPS = ";

/* insert header file right here */
static const char GCC_MAKE_DEPS[64] = ".h\n\
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))\n\n\
_OBJ = ";

/* insert header source  here */
static const char GCC_MAKE_OBJ[4] = ".o ";

/* insert testing source here */
static const char GCC_MAKE_OBJ_BUILD[128] = ".o\n\
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))\n\n\
$(ODIR)/%.o: %.c $(DEPS)\n\
	$(CC) -c -o $@ $< $(CFLAGS)\n\n";

/* insert project name here */
static const char GCC_MAKE_PHONY[128] = ": $(OBJ)\n\
	gcc -o $@ $^ $(CFLAGS) $(LIBS)\n\n\
.PHONY: clean\n\n\
clean:";


/* Disable security warnings for string functions */
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#if (defined (_WIN32) || defined (_WIN64))


#include <windows.h>
#include <sys/stat.h>

#define PATH_MAX MAX_PATH

static const char sep = '\\';

static void mk_dir(const char *name) {
    struct _stat st = {0};
    SECURITY_ATTRIBUTES sec = {0};
    if (_stat(name, &st) == -1) {
        CreateDirectory(name, &sec);
    }
}

static int exists(const char *name) {
    struct _stat st = {0};
    return (_stat(name, &st) + 1);
}

static char *abspath(char *dest, const char *name) {
    return _fullpath(dest, name, PATH_MAX);
}


#elif (defined (LINUX) || defined (__linux__))

#include <sys/unistd.h>
#include <linux/limits.h>
#include <unistd.h>

static const char sep = '/';

static void mk_dir(const char *name) {
    struct stat st = {0};
    if (stat(name, &st) == -1) {
        mkdir(name);
    }
}

static int exists(const char *name) {
    struct stat st = {0};
    return (stat(name, &st) + 1);
}

static char *abspath(char *dest, const char *name) {
    return realpath(dest, name, PATH_MAX);
}

#endif

/* Zero the contents of a string */
static char *strclr(char *str, size_t strt, size_t ssize) {
    for (int i = strt; i < ssize; i++) {
        str[i] = 0x00;
    }
    return str;
}

/* Searches backwards to seperator and then copies O(m); sep also 
 * of one character length, because I don't need to cover the general
 * case in this instance
 */
static char *strslice(char *dest, const char *str, const char *spr) {
    size_t stsize = strlen(str);
    int stop = stsize;
    for (stop; stop >= 0; stop--) {
        if (str[stop] == spr[0]) {
            break;
        }
    }
    if (stop == -1) {
        dest = NULL;
    } else {
        for (int i = 0; i < (stsize - stop); i++) {
            dest[i] = str[stop + 1 + i];
        }
    }
    return dest;
}


static char *strupper(char *dest, const char *str) {
    for (int i = 0; i < strlen(str); i++) {
        if (0x60 < (unsigned char) str[i] < 0x7b) {
            dest[i] = (unsigned char) str[i] - (unsigned char) 0x20;
        } else {
            dest[i] = str[i];
        }
    }
    return dest;
}


/* 1 indicates failure to create and 0 indicates success
 * Syntatically correct: !makefile_create = makefile not created */
static int makefile_create(const char *makename, const char *project, const char *dirname) {
    char make[PATH_MAX];
    char make_contents[432];
    int ret = 1;
    sprintf(make, "%s%c%s", dirname, sep, makename);
    if (!exists(make)) {
        FILE *mkfile = fopen(make, "w");
        if (mkfile == NULL) {
            ret = 0;
        } else {
            sprintf(make_contents, "%s%s%s%s%s%s_test%s%s_app%s",
                    GCC_MAKE_MACROS, project, GCC_MAKE_DEPS, 
                    project, GCC_MAKE_OBJ, project, 
                    GCC_MAKE_OBJ_BUILD, project, GCC_MAKE_PHONY);
            fprintf(mkfile, "%s", make_contents);
        }

        fclose(mkfile);
    } else {
        ret = 0;
    }
    return ret;
}

static int touch(const char *path, const char *project, const char *ext) {
    char fullpath[PATH_MAX];
    int ret = 1;
    if (strlen(path) + strlen(project) + strlen(ext) > PATH_MAX) {
        ret = 0;
    } else {
        sprintf(fullpath, "%s%c%s%s", path, sep, project, ext);
    }

    if (!exists(fullpath) && ret) {
        FILE *fp = fopen(fullpath, "w");
        if (fp == NULL) {
            ret = 0;
        } else {
            if (ext == ".h") {
                char tmp[PATH_MAX]; /* length of project limited by PATH by default */
                strupper(tmp, project);
                fprintf(fp,
                    "#ifndef %s_H\n#define %s_H\n/* Code goes here */\n\n#endif",
                    tmp, tmp);
            } else if (ext == ".c") {
                fprintf(fp,
                    "#include \"%s.h\"\n\n/* Code goes here */\n\n",
                    project);
            } else {
                fprintf(fp, "/* Project %s */", project);
            }
        }
        fclose(fp);
    } else {
        ret = 0;
    }

    return ret;
}


static void touch_wrap(const char *path, const char *name, const char *dir, const char *ext) {
    printf("Creating file %s%s in %s directory...\n", name, ext, dir);
    if (!touch(path, name, ext)) {
        printf("Failed to create %s%s in %s\n", name, ext, dir);
    } else {
        printf("%s%s created in %s\n", name, ext, dir);
    }
}


static int create_dir(const char *dirname, const char *destname) {
    int ret = 1;
    if (strlen(dirname) + strlen(destname) > PATH_MAX) {
        ret = 0;
    } else {
        char destdir[PATH_MAX];
        sprintf(destdir, "%s%c%s", dirname, sep, destname);

        if (!exists(destdir)) {
            mk_dir(destdir);
        } else {
            ret = 0;
        }
    }
    return ret;
}


static void create_files(const char *dirname, const char *project) {
    const char *file_ext[2] = { ".h", ".c" };
    const char *dirs[3] = {"lib", "src", "test"};
    char tmp[PATH_MAX];
    sprintf(tmp, "%s%c%s", dirname, sep, dirs[0]);
    touch_wrap(tmp, project, dirs[0], file_ext[0]);
    for (int i = 0; i < 3; i++) {
        strclr(tmp, strlen(dirname) + 1,  strlen(tmp));
        strcat(tmp, dirs[i]);
        switch (i) {
            char tmp2[PATH_MAX];
            case 1:
                strcpy(tmp2, project);
                touch_wrap(tmp, strcat(tmp2, "_app"), dirs[i], file_ext[1]);
                break;
            case 2:
                strcpy(tmp2, project);
                touch_wrap(tmp, strcat(tmp2, "_test"), dirs[i], file_ext[1]);
                break;
            default:
                touch_wrap(tmp, project, dirs[i], file_ext[1]);
                break;
        }
    }
}


static void create_makes(const char *dirname, const char *project) {
    const char *mks[2] = {"Makefile", "Makefile.win"};

    for (int i = 0; i < 2; i++) {
        printf("Creating %s...", mks[i]);
        if (!makefile_create(mks[i], project, dirname)) {
            printf("Failed to create %s; %s may already exist.",
                   mks[i], mks[i]);
        } else {
            printf("%s was created.", mks[i]);
        }
    }
}


static void create_tree(char *dirname) {
    const char *dirs[4] = {"lib", "src", "test", "include"};

    for (int i = 0; i < 4; i++) {
        char tmp[256] = "Creating ";
        sprintf(tmp, "%s directory...", dirs[i]);
        puts(tmp);
        if (!create_dir(dirname, dirs[i])) {
            printf("Failed to create %s directory."
                   " Directory already exists or path"
                   " exceeds MAX_PATH.\n", dirs[i]);
            strclr(tmp, 0, 256);
        } else {
            printf("Directory %s created.\n", dirs[i]);
            strclr(tmp, 0, 256);
        }
    }
}


int main(int argc, char *argv[]) {
    char dirname[PATH_MAX];
    char project[PATH_MAX];

    if (argc == 2) {
        abspath(dirname, argv[1]);
        strcpy(project, argv[1]);
        if (dirname == NULL) {
            goto ERRORQUIT;
        }
    } else if (argc == 1) {
        dirname[0] = '.';
        /* Can the path overflow if it is formed by os? */
        abspath(dirname, dirname);
        /* Takes the previous directory as the project if none given */
        strslice(project, dirname, &sep);
    } else {
        goto ERRORQUIT;
    }

    create_tree(dirname);
    create_files(dirname, project);
    create_makes(dirname, project);

    return 0;

ERRORQUIT:
    //print_help();
    return 1;
}
