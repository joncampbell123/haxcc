
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

void send_line(haxpp_linesink &ls,const string &name,const linecount_t line) {
    string msg = string("#line ") + to_string(line) + " \"" + name + "\"\n";
    ls.write(msg.c_str());
}

/* 's' should point to the first char of a word.
 * A word is a sequence of chars [a-zA-Z0-9_].
 * assumes s != NULL */
bool iswordchar(char c) {
    return isalnum(c) || c == '_';
}

void cstrskipwhitespace(char* &s) {
    while (*s == ' ' || *s == '\t') s++;
}

string cstrgetword(char* &s) {
    char *base = s;

    while (*s != 0) {
        if (iswordchar(*s)) {
            s++;
        }
        else {
            break;
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

string lookup_header(const string &rpath) {
    if (is_file(rpath))
        return rpath;

    return string();
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
    send_line(out_ls,in_lstk.top().getsourcename(),1);

    if (out_file == "-")
        out_ls.setsink(stdout);
    else
        out_ls.setsink(out_file);

    if (!out_ls.open()) {
        fprintf(stderr,"Unable to open outfile, %s\n",strerror(errno));
        return 1;
    }

    while (!in_lstk.top().eof()) {
        char *line = in_lstk.top().readline();
        if (line == nullptr) {
            if (!in_lstk.top().error() && in_lstk.top().eof()) {
                in_lstk.pop();
                if (in_lstk.empty())
                    break;

                send_line(out_ls,in_lstk.top().getsourcename(),in_lstk.top().currentline()+linecount_t(1));
                continue;
            }
            else {
                fprintf(stderr,"Problem reading. error=%u eof=%u errno=%s\n",in_lstk.top().error(),in_lstk.top().eof(),strerror(errno));
                return 1;
            }
        }

        /* look for preprocessor directives this code handles by itself */
        {
            char *s = line; cstrskipwhitespace(s);

            if (*s == '#') {
                s++; cstrskipwhitespace(s);

                const string what = cstrgetword(s); cstrskipwhitespace(s);

                if (what == "include") {
                    string path;

                    if (*s == '<')
                        path = cstrgetstringenclosed(s,'<','>');
                    else if (*s == '\"')
                        path = cstrgetstringenclosed(s,'\"','\"');
                    else {
                        fprintf(stderr,"#include statement with junk: %s\n",line);
                        return 1;
                    }

                    if (path.empty()) {
                        fprintf(stderr,"#include statement with no path\n");
                        return 1;
                    }

                    string respath = lookup_header(path);
                    if (respath.empty()) {
                        fprintf(stderr,"#include statement unable to find '%s'\n",path.c_str());
                        return 1;
                    }

                    /* NTS: Do not permit - to mean stdin */
                    in_lstk.push();
                    in_lstk.top().setsource(respath);
                    if (!in_lstk.top().open()) {
                        fprintf(stderr,"Unable to open infile '%s', %s\n",respath.c_str(),strerror(errno));
                        return 1;
                    }
                    send_line(out_ls,in_lstk.top().getsourcename(),1);

                    continue; /* do not send to output */
                }
            }
        }

        if (!out_ls.write(line)) {
            fprintf(stderr,"Error writing output\n");
            return 1;
        }
    }

    if (!in_lstk.empty()) {
        if (in_lstk.top().error()) {
            fprintf(stderr,"An error occurred while parsing %s\n",in_lstk.top().getsourcename().c_str());
            return 1;
        }
    }

    if (out_ls.error()) {
        fprintf(stderr,"An error occurred while parsing %s\n",out_ls.getsinkname().c_str());
        return 1;
    }

    out_ls.close();
    in_lstk.clear();
    return 0;
}

