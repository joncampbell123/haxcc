
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
#include "linesink.h"

using namespace std;

static string                   in_file = "-";
static string                   out_file = "-";

static haxpp_linesink           out_ls;

class haxpp_linesourcestack {
    public:
        haxpp_linesourcestack();
        ~haxpp_linesourcestack();
    public:
        bool                        allocstack();
        void                        freestack();
        bool                        push();
        bool                        pop();
        haxpp_linesource*           top();
        void                        clear();
        bool                        empty() const;
    private:
        static constexpr size_t     max_source_stack_default = 64;
        size_t                      max_source_stack = max_source_stack_default;
    private:
        haxpp_linesource*           in_ls = NULL;
        ssize_t                     in_ls_sp = -1;
};

haxpp_linesourcestack::haxpp_linesourcestack() {
}

haxpp_linesourcestack::~haxpp_linesourcestack() {
    freestack();
}

bool haxpp_linesourcestack::allocstack() {
    if (in_ls == NULL) {
        in_ls = new(nothrow) haxpp_linesource[max_source_stack];
        if (in_ls == NULL) return false;
        in_ls_sp = -1;
    }

    return true;
}

void haxpp_linesourcestack::freestack() {
    if (in_ls != NULL) {
        clear();
        delete[] in_ls;
    }
    in_ls = NULL;
    in_ls_sp = -1;
}

bool haxpp_linesourcestack::push() {
    if (in_ls == NULL) {
        if (!allocstack()) {
            return NULL;
        }
    }

    if (in_ls != NULL) {
        /* NTS: when in_ls_sp == -1, in_ls_sp+1 == 0 */
        if (size_t(in_ls_sp+1) < max_source_stack) {
            in_ls_sp++;
            return true;
        }
    }

    return false;
}

bool haxpp_linesourcestack::pop() {
    if (in_ls != NULL) {
        if (in_ls_sp >= 0) {
            in_ls[in_ls_sp--].close();
            return true;
        }
    }

    return false;
}

haxpp_linesource* haxpp_linesourcestack::top() {
    if (in_ls != NULL) {
        if (in_ls_sp >= ssize_t(0)) /* in_ls_sp == -1 stack is empty. normal state will not set sp >= max */
            return in_ls + in_ls_sp;
    }

    return NULL;
}

void haxpp_linesourcestack::clear() {
    while (!empty()) pop();
}

bool haxpp_linesourcestack::empty() const {
    return (in_ls == NULL) || (in_ls_sp == ssize_t(-1));
}

static haxpp_linesourcestack    in_lstk;

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

    if (!in_lstk.push()) {
        fprintf(stderr,"Failed to push\n");
        return 1;
    }

    if (in_file == "-")
        in_lstk.top()->setsource(stdin);
    else
        in_lstk.top()->setsource(in_file);

    if (!in_lstk.top()->open()) {
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

    while (!in_lstk.top()->eof()) {
        char *line = in_lstk.top()->readline();
        if (line == nullptr) {
            if (!in_lstk.top()->error() && in_lstk.top()->eof()) {
                break;
            }
            else {
                fprintf(stderr,"Problem reading. error=%u eof=%u errno=%s\n",in_lstk.top()->error(),in_lstk.top()->eof(),strerror(errno));
                return 1;
            }
        }

        if (!out_ls.write(line)) {
            fprintf(stderr,"Error writing output\n");
            return 1;
        }
    }

    if (in_lstk.top()->error()) {
        fprintf(stderr,"An error occurred while parsing %s\n",in_lstk.top()->getsourcename().c_str());
        return 1;
    }

    if (out_ls.error()) {
        fprintf(stderr,"An error occurred while parsing %s\n",out_ls.getsinkname().c_str());
        return 1;
    }

    out_ls.close();
    in_lstk.clear();
    return 0;
}

