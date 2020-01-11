
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
#include "linesink.h"

haxpp_linesink::haxpp_linesink() {
}

void haxpp_linesink::setsink() {
    close();
}

void haxpp_linesink::setsink(FILE *_fp) {
    close();
    fp = _fp;
    fp_owner = false;
}

void haxpp_linesink::setsink(const std::string &path) {
    close();
    sinkpath = path;
}

haxpp_linesink::~haxpp_linesink() {
    close();
}

void haxpp_linesink::close() {
    if (fp != nullptr) {
        if (fp_owner) fclose(fp);
        fp = nullptr;
    }
    fp_owner = false;
}

bool haxpp_linesink::is_open() const {
    return (fp != nullptr);
}

bool haxpp_linesink::eof() const {
    if (is_open())
        return (feof(fp) != 0);

    return true;
}

bool haxpp_linesink::error() const {
    if (is_open())
        return (ferror(fp) != 0);

    return true;
}

bool haxpp_linesink::open() {
    if (!is_open()) {
        if (sinkpath.empty()) {
            errno = EINVAL;
            return false;
        }

        if (!is_out_file(sinkpath)) {
            errno = EINVAL;
            return false;
        }

        fp = fopen(sinkpath.c_str(),"w");
        if (fp == nullptr) {
            /* fopen set errno */
            return false;
        }

        fp_owner = true;
        lineno = 1;
    }

    return true;
}

bool haxpp_linesink::write(const char * const s) {
    if (is_open()) {
        if (fputs(s,fp) > 0)
            return true;
    }

    return false;
}

bool haxpp_linesink::writeline(const char * const s) {
    if (is_open()) {
        if (fputs(s,fp) <= 0)
            return false;
        if (fputs("\n",fp) <= 0)
            return false;

        return true;
    }

    return false;
}

