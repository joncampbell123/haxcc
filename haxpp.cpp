
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
#include "linesrst.h"

#include <algorithm>
#include <stdexcept>
#include <vector>
#include <map>

using namespace std;

static string                   in_file = "-";
static string                   out_file = "-";

static haxpp_linesink           out_ls;

static vector<string>           include_search;

static haxpp_linesourcestack    in_lstk;

class haxpp_macro {
public:
    struct macro_subst {
        enum class type_t {
            STRING,
            PARAMETER
        };
        type_t                  type = type_t::STRING;
        string                  stringval;
        size_t                  parameter = 0;
    };
public:
    vector<string>              parameters;
    vector<macro_subst>         substitution;
};

map<string,haxpp_macro>         haxpp_macros;

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
            else if (!strcmp(a,"I")) { /* GCC style -I <path> */
                a = argv[i++];
                if (a == NULL) return 1;
                if (*a == 0) return 1;
                include_search.push_back(a);
            }
            else if (*a == 'I') { /* GCC style -I<path> */
                a++;
                if (*a == 0) return 1;
                include_search.push_back(a);
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

string lookup_header(const string &rpath) {
    if (is_file(rpath))
        return rpath;

    for (const auto &spath : include_search) {
        string fpath = spath + "/" + rpath;
        if (is_file(fpath))
            return fpath;
    }

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

                if (what == "define") {
                    string macroname;

                    macroname = cstrgetword(s);
                    if (macroname.empty()) {
                        fprintf(stderr,"#define without macro name\n");
                        return 1;
                    }

                    /* NTS: The scheme, at least as GCC behaves, is
                     *      that a macro can have parameters IF the
                     *      parenthesis are right up against the
                     *      macro name. If they're spaced apart,
                     *      then the macro has no parameters and the
                     *      parenthesis become part of the string
                     *      the macro expands to.
                     *
                     *      #define MACRO (x)              ->       MACRO       (x)
                     *      #define MACRO(x)               ->       MACRO(y)    (y) */
                    haxpp_macro macro;

                    if (*s == '(') {
                        s++;
                        cstrskipwhitespace(s);
                        if (*s != ')') { /* make sure it's not just () */
                            /* yes, it has parameters */
                            while (*s != 0) {
                                string param;

                                cstrskipwhitespace(s);
                                if (cstrparsedotdotdot(s))
                                    param = "...";
                                else
                                    param = cstrgetword(s);

                                if (param.empty()) {
                                    fprintf(stderr,"macro param parsing error at '%s'\n",s);
                                    return 1;
                                }

                                {
                                    auto it = find(macro.parameters.begin(),macro.parameters.end(),param);
                                    if (it != macro.parameters.end()) {
                                        fprintf(stderr,"Macro param '%s' defined twice\n",param.c_str());
                                        return 1;
                                    }
                                }

                                macro.parameters.push_back(param);
                                cstrskipwhitespace(s);

                                if (*s == ')') {
                                    s++;
                                    break;
                                }
                                else if (*s == ',') {
                                    if (param == "...") {
                                        /* must be last param! */
                                        fprintf(stderr,"Variadic macro param ... must come last\n");
                                        return 1;
                                    }

                                    s++; /* more to do */
                                }
                                else {
                                    fprintf(stderr,"Junk while parsing macro params\n");
                                    return 1;
                                }
                            }
                        }
                        if (*s == ')')
                            s++;
                    }
                    if (*s != 0) {
                        if (!isspace(*s)) {
                            fprintf(stderr,"Macro must have a space between name and string. '%s'\n",s);
                            return 1;
                        }
                        cstrskipwhitespace(s);
                    }

                    continue; /* do not send to output */
                }
                else if (what == "include") {
                    string path;

                    /* TODO: Apparently it's legal to write #include SOME_MACRO where SOME_MACRO has the file to include.
                     *       If it were not, FreeType would not compile. */

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

