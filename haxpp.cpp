
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

#include <stdexcept>

using namespace std;

static string                   in_file = "-";
static string                   out_file = "-";

static haxpp_linesink           out_ls;

class haxpp_linesourcestack {
    public:
        haxpp_linesourcestack();
        ~haxpp_linesourcestack();
    public:
        void                        allocstack();
        void                        freestack();
        void                        push();
        void                        pop();
        haxpp_linesource&           top();
        const haxpp_linesource&     top() const;
        void                        clear();
        bool                        empty() const;
        bool                        error() const;
        bool                        eof() const;
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

void haxpp_linesourcestack::allocstack() {
    if (in_ls == NULL) {
        in_ls = new haxpp_linesource[max_source_stack];
        in_ls_sp = -1;
    }
}

void haxpp_linesourcestack::freestack() {
    if (in_ls != NULL) {
        clear();
        delete[] in_ls;
        in_ls = NULL;
    }
    in_ls_sp = -1;
}

void haxpp_linesourcestack::push() {
    allocstack();

    /* NTS: when in_ls_sp == -1, in_ls_sp+1 == 0 */
    if (size_t(in_ls_sp+1) < max_source_stack)
        in_ls_sp++;
    else
        throw overflow_error("linesourcestack overflow");
}

void haxpp_linesourcestack::pop() {
    /* in_ls_sp >= 0 should mean in_ls != NULL or else this code would not permit in_ls_sp >= 0 */
    if (in_ls_sp >= ssize_t(0))
        in_ls[in_ls_sp--].close();
    else
        throw underflow_error("linesourcestack underflow");
}

haxpp_linesource& haxpp_linesourcestack::top() {
    /* in_ls_sp >= 0 should mean in_ls != NULL or else this code would not permit in_ls_sp >= 0 */
    if (in_ls_sp >= ssize_t(0))
        return in_ls[in_ls_sp];

    throw underflow_error("linesourcestack attempt to read top() when empty");
}

const haxpp_linesource& haxpp_linesourcestack::top() const {
    /* in_ls_sp >= 0 should mean in_ls != NULL or else this code would not permit in_ls_sp >= 0 */
    if (in_ls_sp >= ssize_t(0))
        return in_ls[in_ls_sp];

    throw underflow_error("linesourcestack attempt to read top() when empty");
}

void haxpp_linesourcestack::clear() {
    while (!empty()) pop();
}

bool haxpp_linesourcestack::empty() const {
    /* in_ls_sp >= 0 should mean in_ls != NULL or else this code would not permit in_ls_sp >= 0 */
    return (in_ls_sp == ssize_t(-1));
}

bool haxpp_linesourcestack::error() const {
    if (!empty())
        return top().error();

    return false;
}

bool haxpp_linesourcestack::eof() const {
    return empty();
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

    in_lstk.push();
    if (in_file == "-")
        in_lstk.top().setsource(stdin);
    else
        in_lstk.top().setsource(in_file);

    if (!in_lstk.top().open()) {
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

    while (!in_lstk.eof()) {
        char *line = in_lstk.top().readline();
        if (line == nullptr) {
            if (!in_lstk.error() && in_lstk.eof()) {
                break;
            }
            else {
                fprintf(stderr,"Problem reading. error=%u eof=%u errno=%s\n",in_lstk.error(),in_lstk.eof(),strerror(errno));
                return 1;
            }
        }

        if (!out_ls.write(line)) {
            fprintf(stderr,"Error writing output\n");
            return 1;
        }
    }

    if (in_lstk.error()) {
        fprintf(stderr,"An error occurred while parsing %s\n",in_lstk.top().getsourcename().c_str());
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

