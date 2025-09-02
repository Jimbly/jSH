/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __JSH_H__
#define __JSH_H__

#if WINDOWS == 1
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  // These are in allegro headers on DOjS
  #include <io.h>
  #include <fcntl.h>
  #include <direct.h>
  #include <malloc.h>
  #include <errno.h>
  #include <limits.h>
  #include <stdarg.h>
  #include <stddef.h>
  #include <stdlib.h>
  #include <time.h>
  #include <string.h>
#endif

#include <mujs.h>
#include <stdbool.h>
#include <stdio.h>

/************
** defines **
************/

#define SYSINFO ">>> "  //!< logfile line prefix for system messages

#define JSH_VERSION 0.96         //!< version number
#define JSH_VERSION_STR "V0.96"  //!< version number as string

#define JSBOOT_DIR "JSBOOT/"     //!< directory with boot files.
#define JSBOOT_ZIP "JSBOOT.ZIP"  //!< filename for ZIP of JSBOOT
#define JSBOOT_VAR "JSBOOTPATH"  //!< global variable containing the prefix for JSBOOT

#define LOGFILE "JSLOG.TXT"  //!< filename for logfile
#define LOGSTREAM logfile  //!< output stream for logging on DOS

#if LINUX == 1
#define JS_ELINUX(j) js_error(j, "Not supported on Linux")  //!< use always the same message when not supported
#endif
#define JS_ENOMEM(j) js_error(j, "Out of memory")                     //!< use always the same message when memory runs out
#define JS_ENOARR(j) js_error(j, "Array expected")                    //!< use always the same message when array expected
#define JS_EIDX(j, idx) js_error(j, "Index out of bound (%ld)", idx)  //!< use always the same message when array index out of bound

#define SYSTICK_RESOLUTION 1  //!< 1ms resolution
//! check if parameter has a certain usertype
#define JS_CHECKTYPE(j, idx, type)            \
    {                                         \
        if (!js_isuserdata(j, idx, type)) {   \
            js_error(j, "%s expected", type); \
            return;                           \
        }                                     \
    }

//! check if a number is positive
#define JS_CHECKPOS(j, num)                                                 \
    {                                                                       \
        if (num < 0) {                                                      \
            js_error(j, "Non negative number expected: %ld", (int32_t)num); \
            return;                                                         \
        }                                                                   \
    }

/***********
** macros **
***********/
//! define a new constructor
#define CTORDEF(j, f, t, p)                \
    {                                      \
        js_newcconstructor(J, f, f, t, p); \
        js_defglobal(J, t, JS_DONTENUM);   \
    }

//! define a global function (new version)
#define NFUNCDEF(j, n, p)                 \
    {                                     \
        js_newcfunction(j, f_##n, #n, p); \
        js_setglobal(j, #n);              \
    }

//! define a method in a class (new version)
#define NPROTDEF(j, t, n, p)                                                \
    {                                                                       \
        js_newcfunction(j, t##_##n, #t ".prototype." #n, p);                \
        js_defproperty(j, -2, #n, JS_READONLY | JS_DONTENUM | JS_DONTCONF); \
    }

//! define a global property of type number
#define PROPDEF_N(j, i, n)  \
    {                       \
        js_newnumber(j, i); \
        js_setglobal(j, n); \
    }

//! define a global property of type boolean
#define PROPDEF_B(j, i, n)   \
    {                        \
        js_newboolean(j, i); \
        js_setglobal(j, n);  \
    }

//! define a global property of type string
#define PROPDEF_S(j, i, n)  \
    {                       \
        js_newstring(j, i); \
        js_setglobal(j, n); \
    }

//! printf-style write info to logfile/console
#define LOGF(str, ...)                                  \
    if (LOGSTREAM) {                                    \
        fprintf(LOGSTREAM, SYSINFO str, ##__VA_ARGS__); \
        fflush(LOGSTREAM);                              \
    }

//! write info to logfile/console
#define LOG(str)                       \
    if (LOGSTREAM) {                   \
        fputs(SYSINFO str, LOGSTREAM); \
        fflush(LOGSTREAM);             \
    }

#ifdef DEBUG_ENABLED
//! printf-style debug message to logfile/console
#define DEBUGF(str, ...)                                   \
    if (LOGSTREAM) {                                       \
        fprintf(LOGSTREAM, "[DEBUG] " str, ##__VA_ARGS__); \
        printf("[DEBUG] " str, ##__VA_ARGS__);             \
        fflush(LOGSTREAM);                                 \
        fflush(stdout);                                    \
    }

//! print debug message to logfile/console
#define DEBUG(str)                        \
    if (LOGSTREAM) {                      \
        fputs("[DEBUG] " str, LOGSTREAM); \
        puts("[DEBUG] " str);             \
        fflush(LOGSTREAM);                \
        fflush(stdout);                   \
    }
#else
#define DEBUGF(str, ...)
#define DEBUG(str)
#endif

#ifdef GC_BEFORE_MALLOC
#define NEW_OBJECT_PREP(j) js_gc(j, false)
#else
#define NEW_OBJECT_PREP(j)
#endif


/************
** structs **
************/
#if LINUX != 1
typedef struct __library_t {
    struct __library_t *next;
    const char *name;
    void *handle;
    void (*shutdown)(void);
} library_t;
#endif

/*********************
** global variables **
*********************/
extern FILE *logfile;       //!< file for log output.
extern char *logfile_name;  //!< name of the logfile
#if LINUX != 1
extern library_t *jsh_loaded_libraries;
#endif
extern bool no_tcpip;  //!< command line option

/***********************
** exported functions **
***********************/
#if LINUX != 1
extern bool jsh_register_library(const char *name, void *handle, void (*shutdown)(void));
extern bool jsh_check_library(const char *name);
#endif
extern int jsh_do_file(js_State *J, const char *fname);
extern void jsh_logflush(void);
extern bool jsh_file_exists(const char *filename);

#endif  // __JSH_H__
