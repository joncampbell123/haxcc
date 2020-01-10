
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
            PARAMETER,
            STRINGIFY_PARAMETER,
            VA_ARGS,            /* __VA_ARGS__ */
            VA_OPT              /* __VA_OPT(x)__ */
        };
        type_t                  type = type_t::STRING;
        string                  stringval;
        size_t                  parameter = 0;

        void                    dump(FILE *fp=NULL) const;
        const char*             type_str() const;
    };
public:
    vector<string>              parameters;
    vector<macro_subst>         substitution;
public:
    void                        dump(FILE *fp=NULL) const;
    void                        add_string_subst(char* &base,char* const s);
    void                        add_parameter_subst(size_t pidx);
    void                        add_parameter_subst_stringify(size_t pidx);
};

void haxpp_macro::add_parameter_subst_stringify(size_t pidx) {
    if (pidx < parameters.size()) {
        macro_subst ms;

        ms.type = macro_subst::type_t::STRINGIFY_PARAMETER;
        ms.parameter = pidx;
        substitution.push_back(ms);
    }
}

void haxpp_macro::add_parameter_subst(size_t pidx) {
    if (pidx < parameters.size()) {
        macro_subst ms;

        ms.type = macro_subst::type_t::PARAMETER;
        ms.parameter = pidx;
        substitution.push_back(ms);
    }
}

void haxpp_macro::add_string_subst(char* &base,char* const s) {
    if (base < s) {
        macro_subst ms;

        ms.type = macro_subst::type_t::STRING;
        ms.stringval = string(base,(size_t)(s-base));
        substitution.push_back(ms);

        base = s;
    }
}

const char* haxpp_macro::macro_subst::type_str() const {
    switch (type) {
        case type_t::STRING:                return "STRING";
        case type_t::PARAMETER:             return "PARAMETER";
        case type_t::STRINGIFY_PARAMETER:   return "STRINGIFY_PARAMETER";
        case type_t::VA_ARGS:               return "VA_ARGS";
        case type_t::VA_OPT:                return "VA_OPT";
        default:                            break;
    }

    return "?";
}

void haxpp_macro::macro_subst::dump(FILE *fp) const {
    if (fp == NULL)
        fp = stderr;

    fprintf(stderr,"{ type:%s ",type_str());
    switch (type) {
        case type_t::STRING:
        case type_t::VA_OPT:
            fprintf(stderr,"str:'%s' ",stringval.c_str());
            break;
        case type_t::PARAMETER:
        case type_t::STRINGIFY_PARAMETER:
        case type_t::VA_ARGS:
            fprintf(stderr,"param:%zu ",parameter);
            break;
        default:
            break;
    };
    fprintf(stderr,"} ");
}

void haxpp_macro::dump(FILE *fp) const {
    if (fp == NULL)
        fp = stderr;

    fprintf(stderr,"  parameters:");
    for (const auto &x : parameters) fprintf(stderr," '%s'",x.c_str());
    fprintf(stderr,"\n");
    fprintf(stderr,"  subst:");
    for (const auto &x : substitution) x.dump(fp);
    fprintf(stderr,"\n");
}

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

                if (what == "undef") {
                    string macroname;

                    macroname = cstrgetword(s);
                    if (macroname.empty()) {
                        fprintf(stderr,"#undef without macro name\n");
                        return 1;
                    }

                    {
                        auto mi = haxpp_macros.find(macroname);
                        if (mi != haxpp_macros.end())
                            haxpp_macros.erase(mi);
                        else
                            fprintf(stderr,"WARNING: Macro %s not defined at #undef\n",macroname.c_str());
                    }

                    continue; /* do not send to output */
                }
                else if (what == "define") {
                    bool to_be_continued = false;
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

                    do {
                        to_be_continued = false;

                        char *base = s;
                        while (*s != 0) {
                            bool stringify = false;

                            if (s[0] == '\\' && (s[1] == '\n' || s[1] == 0)) {
                                to_be_continued = true;
                                break;
                            }
                            if (*s == '\n')
                                break;

                            /* # followed by identifier means to stringify it */
                            if (s[0] == '#' && iswordcharfirst(s[1])) {
                                stringify = true;
                                s++;
                            }

                            if (iswordcharfirst(*s)) {
                                char *wordbase = s;
                                char *cutbase = s;

                                /* unused? */
                                (void)wordbase;

                                /* if #ident then adjust pointer to cut off '#' */
                                if (stringify) cutbase--;

                                string word = cstrgetword(s);
                                string lookup;

                                if (word == "__VA_ARGS__" || word == "__VA_OPT__")
                                    lookup = "...";
                                else
                                    lookup = word;

                                /* TODO: __VA_OPT__(x)
                                 *
                                 *       resolves to (x) if variadic parameter has something,
                                 *       resolves to nothing otherwise.
                                 *
                                 *       That means this code needs to copy the contents within the (..) following __VA_OPT__
                                 *
                                 *       As far as GCC documentation suggests this is a way to do macro wrappers around sprintf */

                                {
                                    auto pi = find(macro.parameters.begin(),macro.parameters.end(),lookup);
                                    if (pi != macro.parameters.end()) {
                                        /* cut the string up to the first char of the word. */
                                        macro.add_string_subst(base,cutbase);

                                        /* add a reference to this parameter */
                                        if (stringify)
                                            macro.add_parameter_subst_stringify(size_t(pi-macro.parameters.begin()));
                                        else
                                            macro.add_parameter_subst(size_t(pi-macro.parameters.begin()));

                                        /* then resume string scanning after the word */
                                        base = s;

                                        /* OK, but does '##' follow? If so, we're being asked to paste tokens together.
                                         * we can accomodate that by eating the "##" and then looping back around to process again. */
                                        /* NTS: It is expected that if we're at the end of the string for some reason, s[0] == 0 and
                                         *      execution will skip s[1] == '#', etc. and avoid buffer overrun. */
                                        if (s[0] == '#' && s[1] == '#' && iswordcharfirst(s[2])) {
                                            s += 2;
                                            base = s; /* do not include '##' in the string */
                                            continue;
                                        }
                                    }
                                }
                            }
                            else {
                                s++;
                            }
                        }
                        macro.add_string_subst(base,s);

                        if (to_be_continued) {
                            line = in_lstk.top().readline();
                            if (line == nullptr) {
                                fprintf(stderr,"ERROR: Multiline macro cut off at EOF\n");
                                return 1;
                            }
                            s = line;
                        }
                    } while (to_be_continued);

                    /* TODO: Compare if different and then emit warning?
                     *       Should this be an error? */
                    {
                        auto mi = haxpp_macros.find(macroname);
                        if (mi != haxpp_macros.end())
                            fprintf(stderr,"WARNING: Macro %s already exists\n",macroname.c_str());
                    }

                    haxpp_macros[macroname] = macro;
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

