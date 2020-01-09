
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>

#include "util.h"
#include "linesrc.h"

using namespace std;

bool haxpp_linesource::lineresize(const size_t newsz) {
    if (newsz != 0u) {
        if (newsz < line_alloc_minimum || newsz > line_alloc_maximum)
            return false;

        if (line != NULL) {
            /* already allocated, resize it. do nothing if the same size. */
            if (line_alloc == newsz)
                return true;

            /* NTS: GNU GLIBC is said to support realloc(NULL,newsz) as malloc(), but other platforms might not do that */
            char *np = (char*)realloc(line,newsz);
            if (np == NULL)
                return false;

            line = np;
            line_alloc = newsz;
        }
        else {
            /* not allocated yet */
            line = (char*)malloc(newsz);
            if (line == NULL) return false;
            line_alloc = newsz;
        }
    }
    else { /* newsz == 0 which is a command to free the buffer */
        if (line != NULL) {
            free((void*)line);
            line = NULL;
        }
        line_alloc = 0;
    }

    return true;
}

haxpp_linesource::haxpp_linesource() {
}

haxpp_linesource::haxpp_linesource(FILE *_fp) {
    fp = _fp;
    fp_owner = false;
}

haxpp_linesource::haxpp_linesource(const string &path) {
    sourcepath = path;
}

haxpp_linesource::~haxpp_linesource() {
    lineresize(0);
    close();
}

void haxpp_linesource::close() {
    if (fp != nullptr) {
        if (fp_owner) fclose(fp);
        fp = nullptr;
    }
    fp_owner = false;
}

bool haxpp_linesource::is_open() const {
    return (fp != NULL);
}

bool haxpp_linesource::eof() const {
    if (is_open())
        return (feof(fp) != 0);

    return true;
}

bool haxpp_linesource::error() const {
    if (is_open())
        return (ferror(fp) != 0);

    return true;
}

bool haxpp_linesource::open() {
    if (!is_open()) {
        if (line == NULL) {
            if (!lineresize(line_alloc_default)) {
                errno = ENOMEM;
                return false;
            }
        }

        if (!fp_owner)
            return true;

        if (sourcepath.empty() || line_alloc < line_alloc_minimum) {
            errno = EINVAL;
            return false;
        }

        if (!is_file(sourcepath)) {
            errno = EINVAL;
            return false;
        }

        fp = fopen(sourcepath.c_str(),"r");
        if (fp == NULL) {
            /* fopen set errno */
            return false;
        }

        fp_owner = true;
        lineno = 1;
    }

    return true;
}

char *haxpp_linesource::readline() {
    errno = 0;
    if (is_open()) {
        const size_t s = linesize(); /* must be >= 2 */
        if (!ferror(fp) && !feof(fp) && s >= line_alloc_minimum) {
            /* write a NUL to the last two bytes of the buffer.
             * if those bytes are no longer NUL it means fgets() read a line that's too long.
             * This is faster than using strlen() on the returned string. */
            line[s-2] = line[s-1] = 0;

            /* fgets() will read up to s-1 bytes, and store a NUL where it stops (which is why we test line[s-2] instead).
             * the returned string will contain the entire line including the '\n' newline. */
            if (fgets(line,s,fp) == NULL)
                return NULL;

            /* check for line too long */
            if (line[s-2] != 0) {
                errno = E2BIG;
                return NULL;
            }

            return line;
        }
    }

    return NULL;
}

