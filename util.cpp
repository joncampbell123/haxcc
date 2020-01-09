
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

