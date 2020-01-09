
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

static string           in_file = "-";
static string           out_file = "-";

class haxpp_linesink {
private:
    linecount_t         lineno = 0;
    std::string         sinkpath;
    FILE*               fp = nullptr;
    bool                fp_owner = false;
public:
    inline const std::string& getsinkname() const {
        return sinkpath;
    }
public:
    haxpp_linesink();
    ~haxpp_linesink();
public:
    void setsink();
    void setsink(FILE *_fp);
    void setsink(const std::string &path);
    void close();
    bool is_open() const;
    bool eof() const;
    bool error() const;
    bool open();
    bool write(const char * const s);
};

static haxpp_linesink   out_ls;
static haxpp_linesource in_ls;

static void help() {
    fprintf(stderr,"haxpp infile outfile\n");
}

static int parse_argv(int argc,char **argv) {
    int nwac=0;
    char *a;
    int i=1;

    while (i < argc) {
        a = argv[i++];

        if (*a == '-' && strcmp(a,"-") != 0/*not "-"*/) {
            do { a++; } while (*a == '-');

            if (!strcmp(a,"h") || !strcmp(a,"help")) {
                help();
                return 1;
            }
            else {
                fprintf(stderr,"Unknown switch %s\n",a);
                return 1;
            }
        }
        else {
            switch (nwac++) {
                case 0:
                    in_file = a;
                    break;
                case 1:
                    out_file = a;
                    break;
                default:
                    fprintf(stderr,"Unexpected arg %s\n",a);
                    return 1;
            }
        }
    }

    return 0;
}

int main(int argc,char **argv) {
    if (parse_argv(argc,argv))
        return 1;

    if (in_file == "-")
        in_ls.setsource(stdin);
    else
        in_ls.setsource(in_file);

    if (!in_ls.open()) {
        fprintf(stderr,"Unable to open infile, %s\n",strerror(errno));
        return 1;
    }

    if (out_file == "-")
        out_ls.setsink(stdout);
    else
        out_ls.setsink(out_file);

    if (!out_ls.open()) {
        fprintf(stderr,"Unable to open outfile, %s\n",strerror(errno));
        return 1;
    }

    while (!in_ls.eof()) {
        char *line = in_ls.readline();
        if (line == nullptr) {
            if (!in_ls.error() && in_ls.eof()) {
                break;
            }
            else {
                fprintf(stderr,"Problem reading. error=%u eof=%u errno=%s\n",in_ls.error(),in_ls.eof(),strerror(errno));
                return 1;
            }
        }

        if (!out_ls.write(line)) {
            fprintf(stderr,"Error writing output\n");
            return 1;
        }
    }

    if (in_ls.error()) {
        fprintf(stderr,"An error occurred while parsing %s\n",in_ls.getsourcename().c_str());
        return 1;
    }

    if (out_ls.error()) {
        fprintf(stderr,"An error occurred while parsing %s\n",out_ls.getsinkname().c_str());
        return 1;
    }

    out_ls.close();
    in_ls.close();
    return 0;
}

/******************************/

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

