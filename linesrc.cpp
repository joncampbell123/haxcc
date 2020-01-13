
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

#include <stdexcept>

using namespace std;

bool haxpp_linesource::lineresize(const size_t newsz) {
    if (newsz != 0u) {
        if (newsz < line_alloc_minimum || newsz > line_alloc_maximum)
            return false;

        if (line != nullptr) {
            /* already allocated, resize it. do nothing if the same size. */
            if (line_alloc == newsz)
                return true;

            /* NTS: GNU GLIBC is said to support realloc(nullptr,newsz) as malloc(), but other platforms might not do that */
            char *np = (char*)realloc(line,newsz);
            if (np == nullptr)
                return false;

            line = np;
            line_alloc = newsz;
        }
        else {
            /* not allocated yet */
            line = (char*)malloc(newsz);
            if (line == nullptr) return false;
            line_alloc = newsz;
        }
    }
    else { /* newsz == 0 which is a command to free the buffer */
        if (line != nullptr) {
            free((void*)line);
            line = nullptr;
        }
        line_alloc = 0;
    }

    return true;
}

haxpp_linesource::haxpp_linesource() {
}

void haxpp_linesource::setsource() {
    close();
}

void haxpp_linesource::setsource(FILE *_fp) {
    close();
    fp = _fp;
    fp_owner = false;
}

void haxpp_linesource::setsource(const std::string &path) {
    close();
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
    return (fp != nullptr);
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
        if (sourcepath.empty()) {
            errno = EINVAL;
            return false;
        }

        if (!is_file(sourcepath)) {
            errno = EINVAL;
            return false;
        }

        fp = fopen(sourcepath.c_str(),"r");
        if (fp == nullptr) {
            /* fopen set errno */
            return false;
        }

        fp_owner = true;
        lineno = 0;
    }

    return true;
}

char *haxpp_linesource::readline() {
    errno = 0;
    if (is_open()) {
        if (line == nullptr) {
            if (!lineresize(line_alloc_default)) {
                errno = ENOMEM;
                return NULL;
            }
        }

        const size_t s = linesize(); /* must be >= 2 */

        if (s < line_alloc_minimum) {
            errno = ENOMEM;
            return NULL;
        }

        if (!ferror(fp) && !feof(fp)) {
            int c_comment = 0;
            size_t col = 0;
            size_t l = 0;
            int c;

            do {
                if (l >= (s - size_t(1))) {
                    /* line is too long! */
                    errno = E2BIG;
                    return nullptr;
                }

                c = fgetc(fp);
                if (c == EOF) {
                    if (l != 0)
                        break;
                    else
                        return nullptr;
                }
                else if (c == '\r')
                    continue; /* ignore (MS-DOS CR LF line endings) */
                else if (c == '\n') {
                    if (l != size_t(0) && line[l-1] == '\\') {
                        line[--l] = ' '; /* erase \ char */
                        col = 0;
                        continue;
                    }
                    else {
                        break; /* stop, do not store */
                    }
                }
                else if (c == '\t' || c == ' ') {
                    if (col == 0) continue;
                }
                else if (c == '*' && l != size_t(0) && line[l-1] == '/') { /* open C comment */
                    int pc = c;

                    line[--l] = ' '; /* erase \ char */
                    c_comment = 1;
                    do {
                        c = fgetc(fp);
                        if (c == EOF)
                            throw runtime_error("C comment did not close at EOF");

                        if (pc == '/' && c == '*') /* open */
                            c_comment++;
                        else if (pc == '*' && c == '/') /* close */
                            c_comment--;

                        pc = c;
                    } while (c_comment > 0);
                    continue;
                }

                line[l++] = (char)c;
                col++;
            } while (1);
            line[l] = 0;

            lineno++;
            return line;
        }
    }

    return nullptr;
}

