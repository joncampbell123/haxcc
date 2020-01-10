
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

using namespace std;

bool is_file(const char * const path) {
    struct stat st;

    if (stat(path,&st))
        return false;

    return S_ISREG(st.st_mode);
}

bool is_file(const string &path) {
    return is_file(path.c_str());
}

bool is_out_file(const char * const path) {
    struct stat st;

    if (stat(path,&st)) {
        if (errno == ENOENT) /* doesn't exist? fine */
            return true;

        return false;
    }

    return S_ISREG(st.st_mode);
}

bool is_out_file(const string &path) {
    return is_out_file(path.c_str());
}

/* 's' should point to the first char of a word.
 * A word is a sequence of chars [a-zA-Z0-9_].
 * assumes s != NULL */
bool iswordchar(char c) {
    return isalnum(c) || c == '_';
}

bool iswordcharfirst(char c) {
    return isalpha(c) || c == '_';
}

void cstrskipwhitespace(char* &s) {
    while (*s == ' ' || *s == '\t') s++;
}

bool cstrparsedotdotdot(char* &s) {
    char *scan = s;
    int dots = 0;

    while (*scan == '.' && dots < 3) {
        scan++;
        dots++;
    }

    if (dots == 3 && *scan != '.') {
        s = scan;
        return true;
    }

    return false;
}

string cstrgetword(char* &s) {
    char *base = s;

    if (iswordcharfirst(*s)) {
        while (*s != 0) {
            if (iswordchar(*s)) {
                s++;
            }
            else {
                break;
            }
        }
    }

    return string(base,(size_t)(s-base));
}

string cstrgetstringenclosed(char* &s,char delim,char delimend) {
    while (*s && *s != delim) s++;

    if (*s == delim) {
        char *base = ++s; /* skip delim and point to char after */
        while (*s && *s != delimend) s++;
        return string(base,(size_t)(s-base));
    }

    return string();
}

void cstrskipsquote(char* &s) {
    if (*s == '\'') {
        s++;
        while (*s && *s != '\'') {
            if (*s == '\\') s++;
            if (*s != 0) s++;
        }
        if (*s == '\'') s++;
    }
}

void cstrskipstring(char* &s) {
    if (*s == '\"') {
        s++;
        while (*s && *s != '\"') {
            if (*s == '\\') s++;
            if (*s != 0) s++;
        }
        if (*s == '\"') s++;
    }
}

