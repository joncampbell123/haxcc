
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
#include <stack>
#include <map>

using namespace std;

static bool                     dump_macros = false;
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
            NEWLINE,
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
    bool                        needs_parens = false;
    bool                        last_param_variadic = false;
public:
    void                        dump(FILE *fp=NULL) const;
    void                        add_string_subst(char* &base,char* const s);
    void                        add_parameter_subst(size_t pidx);
    void                        add_parameter_subst_va_opt(size_t pidx,const string &va_opt_sub);
    void                        add_parameter_subst_stringify(size_t pidx);
    void                        add_newline_subst();
    bool                        parse_identifier(string &macroname,char* &s);
    bool                        parse_token_string(bool &to_be_continued,char* &s);
};

bool haxpp_macro::parse_token_string(bool &to_be_continued,char* &s) {
    char *base = s;
    while (*s != 0) {
        bool stringify = false;

        if (s[0] == '\\' && (s[1] == '\n' || s[1] == '\r' || s[1] == 0)) {
            to_be_continued = true;
            break;
        }
        if (*s == '\n' || *s == '\r')
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
                lookup = "__VA_ARGS__";
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
            if (word == "__VA_OPT__") {
                string va_opt_sub;

                if (stringify) fprintf(stderr,"WARNING: Attempting to stringify __VA_OPT__?? Ignored.\n");

                if (*s == '(') {
                    int paren = 1;
                    s++;

                    char *vbase = s;
                    while (*s && paren > 0) {
                        if (*s == '(') {
                            paren++;
                            s++;
                        }
                        else if (*s == ')') {
                            paren--;
                            if (paren == 0)
                                va_opt_sub = string(vbase,(size_t)(s-vbase));

                            s++;
                        }
                        else if (*s == '\'') {
                            cstrskipsquote(s);
                        }
                        else if (*s == '\"') {
                            cstrskipstring(s);
                        }
                        else {
                            s++;
                        }
                    }

                    if (paren != 0) {
                        fprintf(stderr,"__VA_OPT__(param) paren mismatch\n");
                        return false;
                    }
                }

                auto pi = find(parameters.begin(),parameters.end(),lookup);
                if (pi != parameters.end()) {
                    add_string_subst(base,wordbase);

                    /* add a reference to this parameter */
                    add_parameter_subst_va_opt(size_t(pi-parameters.begin()),va_opt_sub);

                    /* then resume string scanning after the word */
                    base = s;
                }
            }
            else {
                auto pi = find(parameters.begin(),parameters.end(),lookup);
                if (pi != parameters.end()) {
                    /* cut the string up to the first char of the word. */
                    add_string_subst(base,cutbase);

                    /* add a reference to this parameter */
                    if (stringify)
                        add_parameter_subst_stringify(size_t(pi-parameters.begin()));
                    else
                        add_parameter_subst(size_t(pi-parameters.begin()));

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

    add_string_subst(base,s);
    return true;
}

bool haxpp_macro::parse_identifier(string &macroname,char* &s) {
    macroname = cstrgetword(s);
    if (macroname.empty()) {
        fprintf(stderr,"#define without macro name\n");
        return false;
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

    if (*s == '(') {
        s++;
        needs_parens = true;
        cstrskipwhitespace(s);
        if (*s != 0 && *s != ')') { /* make sure it's not just () */
            /* yes, it has parameters */
            while (*s != 0) {
                string param;

                cstrskipwhitespace(s);
                if (cstrparsedotdotdot(s)) {
                    last_param_variadic = true;
                    param = "__VA_ARGS__";
                }
                else {
                    param = cstrgetword(s);
                }

                if (param.empty()) {
                    fprintf(stderr,"macro param parsing error at '%s'\n",s);
                    return false;
                }

                {
                    auto it = find(parameters.begin(),parameters.end(),param);
                    if (it != parameters.end()) {
                        fprintf(stderr,"Macro param '%s' defined twice\n",param.c_str());
                        return false;
                    }
                }

                parameters.push_back(param);
                cstrskipwhitespace(s);

                if (*s == ')') {
                    break;
                }
                else if (*s == ',') {
                    if (param == "__VA_ARGS__") {
                        /* must be last param! */
                        fprintf(stderr,"Variadic macro param ... must come last\n");
                        return false;
                    }

                    s++; /* more to do */
                }
                else if (cstrparsedotdotdot(s)) {
                    /* According to GCC you're allowed to say #define macro(b,a...) b a which is the same as #define macro(b,...) b __VA_ARGS__ */
                    last_param_variadic = true;
                    cstrskipwhitespace(s);
                    if (*s == ')') {
                        break;
                    }
                    else {
                        fprintf(stderr,"Junk while parsing macro params. Expected close parens after \"...\"\n");
                        return false;
                    }
                }
                else {
                    fprintf(stderr,"Junk while parsing macro params\n");
                    return false;
                }
            }
        }

        if (*s == ')')
            s++;
    }

    return true;
}

void haxpp_macro::add_newline_subst() {
        macro_subst ms;

        ms.type = macro_subst::type_t::NEWLINE;
        substitution.push_back(ms);
}

void haxpp_macro::add_parameter_subst_stringify(size_t pidx) {
    if (pidx < parameters.size()) {
        macro_subst ms;

        ms.type = macro_subst::type_t::STRINGIFY_PARAMETER;
        ms.parameter = pidx;
        substitution.push_back(ms);
    }
}

void haxpp_macro::add_parameter_subst_va_opt(size_t pidx,const string &va_opt_sub) {
    if (pidx < parameters.size()) {
        macro_subst ms;

        ms.type = macro_subst::type_t::VA_OPT;
        ms.stringval = va_opt_sub;
        substitution.push_back(ms);
    }
}

void haxpp_macro::add_parameter_subst(size_t pidx) {
    if (pidx < parameters.size()) {
        macro_subst ms;

        if (parameters[pidx] == "__VA_ARGS__")
            ms.type = macro_subst::type_t::VA_ARGS;
        else
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
        case type_t::NEWLINE:               return "NEWLINE";
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

void dump_all_macros() {
    for (auto mi=haxpp_macros.begin();mi!=haxpp_macros.end();mi++) {
        fprintf(stderr,"Macro dump for '%s':\n",mi->first.c_str());
        mi->second.dump();
    }
}

bool add_macro(const string &macroname,const haxpp_macro &macro) {
    /* TODO: Compare if different and then emit warning?
     *       Should this be an error? */
    {
        auto mi = haxpp_macros.find(macroname);
        if (mi != haxpp_macros.end())
            fprintf(stderr,"WARNING: Macro %s already exists\n",macroname.c_str());
    }

#if 0
    fprintf(stderr,"Macro: '%s'\n",macroname.c_str());
    macro.dump();
#endif

    haxpp_macros[macroname] = macro;
    return true;
}

bool parse_cmdline_macrodef(char* &a) {
    bool to_be_continued = false;
    haxpp_macro macro;
    string macroname;

    if (!macro.parse_identifier(macroname,a))
        return false;

    if (*a != 0) {
        if (*a != '=') {
            fprintf(stderr,"Macro must have an equals sign between name and string. '%s'\n",a);
            return false;
        }
        a++;
    }

    to_be_continued = false;
    if (!macro.parse_token_string(to_be_continued,a))
        return false;

    if (to_be_continued) {
        fprintf(stderr,"-D cannot define multi-line macros\n");
        return false;
    }

    if (!add_macro(macroname,macro))
        return false;

    return true;
}

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
            else if (!strcmp(a,"dumpmacros")) {
                dump_macros = true;
            }
            else if (*a == 'I') { /* GCC style -I<path> */
                a++;
                if (*a == 0) return 1;
                include_search.push_back(a);
            }
            else if (*a == 'D') { /* GCC style -DNAME=value */
                a++;
                if (*a == 0) return 1;

                if (!parse_cmdline_macrodef(a))
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

bool eval_ifdef(const string &name) {
    auto i = haxpp_macros.find(name);
    return (i != haxpp_macros.end());
}

string expand_macro_string(haxpp_macro &macro,bool &multiline,const vector<string> &ivparam) {
    bool va_opt = false;
    string r;

    if (macro.parameters.size() != 0) {
        size_t idx = macro.parameters.size() - size_t(1);
        if (macro.parameters[idx] == "__VA_ARGS__") {
            if (ivparam.size() > idx) {
                if (!ivparam[idx].empty())
                    va_opt = true;
            }
        }
    }

    for (auto i=macro.substitution.begin();i!=macro.substitution.end();i++) {
        const auto &sub = *i;
        size_t pi;

        switch (sub.type) {
            case haxpp_macro::macro_subst::type_t::STRING:
                r += sub.stringval;
                break;
            case haxpp_macro::macro_subst::type_t::PARAMETER:
            case haxpp_macro::macro_subst::type_t::VA_ARGS:
                pi = sub.parameter;
                if (pi >= ivparam.size())
                    throw runtime_error("param out of range");
                r += ivparam[pi];
                break;
            case haxpp_macro::macro_subst::type_t::STRINGIFY_PARAMETER:
                pi = sub.parameter;
                if (pi >= ivparam.size())
                    throw runtime_error("param out of range");
                r += "\"";
                r += ivparam[pi];
                r += "\"";
                break;
            case haxpp_macro::macro_subst::type_t::NEWLINE:
                multiline = true;
                r += "\n";
                break;
            case haxpp_macro::macro_subst::type_t::VA_OPT:
                if (va_opt) r += sub.stringval;
                break;
            default:
                break;
        }
    }

    return r;
}

void macro_replace(char *fence,char *base,char *to,const string &after,const string &with) {
    if (to >= fence || to < base || fence <= base)
        throw invalid_argument("macro_replace() to pointer out of bounds");

    if ((to+after.length()+with.length()) >= (fence - 1))
        throw overflow_error("macro_replace() not enough buffer space");

    const char *s;

    s = with.c_str();
    while (*s != 0) *to++ = *s++;

    s = after.c_str();
    while (*s != 0) *to++ = *s++;

    *to = 0;
    if (to >= fence)
        throw runtime_error("macro_replace() buffer overrun detected");
}

/* *s == '(' scan until ')' and return with s just past it */
void macro_param_scan_parenpair(char* &s) {
    int paren = 0;

    while (*s != 0) {
        if (*s == '(')
            paren++;
        else if (*s == ')') {
            if (--paren <= 0) {
                s++;
                break;
            }
        }

        s++;
    }

    if (paren != 0)
        throw runtime_error("mismatched parens in macro expansion");
}

void macro_param_scan_va_args(string &dst,char* &s) {
    char *base = s;

    while (*s != 0) {
        if (*s == ')')
            break;
        else if (*s == '(')
            macro_param_scan_parenpair(s);
        else
            s++;
    }

    dst = string(base,(size_t)(s-base));
}

void macro_param_scan(string &dst,char* &s) {
     char *base = s;

     while (*s != 0) {
         if (*s == ')' || *s == ',')
             break;
         else if (*s == '(')
             macro_param_scan_parenpair(s);
         else
             s++;
    }

    dst = string(base,(size_t)(s-base));
}

/* caller has already handled '(' */
void parse_macro_invoke_params(vector<string> &ivparam,char* &s,haxpp_macro &macro) {
    size_t param = 0;

    ivparam.resize(macro.parameters.size());

    cstrskipwhitespace(s);
    while (*s != 0) {
        if (*s == ')') break;

        if (param >= macro.parameters.size())
            throw overflow_error("Too many parameters in macro invocation "+to_string(param)+"/"+to_string(macro.parameters.size())+" at "+s);

        if ((param+size_t(1)) == macro.parameters.size()) {
            if (macro.last_param_variadic) {
                /* ... param, the string parameter will be from here until closing parens
                 * whether or not any commas are in the way, __VA_ARGS__ */
                macro_param_scan_va_args(ivparam[param],s);
                param++;
            }
            else {
                /* last param, expected to end with closing parens */
                macro_param_scan(ivparam[param],s);
                param++;
            }
        }
        else {
            /* mid param, expected to end with comma */
            macro_param_scan(ivparam[param],s);
            param++;
        }

        if (*s == ',') {
            if (param >= macro.parameters.size())
                throw overflow_error("Too many parameters in macro invocation "+to_string(param)+"/"+to_string(macro.parameters.size())+" at "+s);

            s++;
        }

        cstrskipwhitespace(s);
    }

    if (*s == ')')
        s++;
    else
        throw runtime_error("macro() invocation end did not close in closing parens");

    /* if one parameter short and last param is __VA_ARGS__ aka ..., then treat it as
     * if "" had been given and accept it */
    if ((param+size_t(1)) == macro.parameters.size()) {
        if (macro.last_param_variadic) {
            ivparam[param++] = "";
        }
    }

    if (param != macro.parameters.size())
        throw runtime_error("macro() invocation wrong number of parameters");
}

bool macro_expand(char *line,char *linefence,bool &multiline,bool nullnonexist) {
    bool changed = false;
    char *scan = line;

    multiline = false;
    while (*scan != 0) {
        if (iswordcharfirst(*scan)) {
            char *wordbase = scan;
            string word = cstrgetword(scan);

            vector<string> ivparam;

            auto i = haxpp_macros.find(word);
            if (i != haxpp_macros.end()) { /* macro() must be used as macro(), treat macro without parens as not a match */
                if (i->second.needs_parens) {
                    cstrskipwhitespace(scan); /* GCC behavior suggests that you can invoke it as macro() or macro () */
                    if (*scan == '(') {
                        scan++;
                        parse_macro_invoke_params(ivparam,scan,i->second);
                    }
                    else {
                        /* NOPE: Macro can only be invoked with () */
                        i = haxpp_macros.end();
                    }
                }
            }

            if (i != haxpp_macros.end()) {
                string newstring = expand_macro_string(i->second,multiline,ivparam);
                string remstr = scan;

                macro_replace(linefence,line,wordbase,remstr,newstring);
                scan = wordbase;
                changed = true;
            }
            else if (nullnonexist) {
                string remstr = scan;

                macro_replace(linefence,line,wordbase,remstr,"");
                scan = wordbase;
                changed = true;
            }
        }
        else if (*scan == '\'') {
            cstrskipsquote(scan);
        }
        else if (*scan == '\"') {
            cstrskipstring(scan);
        }
        else {
            scan++;
        }
    }

    return changed;
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

    if (out_file == "-")
        out_ls.setsink(stdout);
    else
        out_ls.setsink(out_file);

    if (!out_ls.open()) {
        fprintf(stderr,"Unable to open outfile, %s\n",strerror(errno));
        return 1;
    }

    struct cond_tracking_t {
        bool        cond = true;
        bool        allow_else = false;

        cond_tracking_t() { }
        cond_tracking_t(const bool v) : cond(v) { }

        inline bool eval() const {
            return cond;
        }
    };

    bool emit_line = true;
    cond_tracking_t if_cond;
    stack<cond_tracking_t> if_cond_stack;

    while (!in_lstk.top().eof()) {
        char *line = in_lstk.top().readline();
        if (line == nullptr) {
            if (!in_lstk.top().error() && in_lstk.top().eof()) {
                in_lstk.pop();
                if (in_lstk.empty())
                    break;

                emit_line = true;
                continue;
            }
            else {
                fprintf(stderr,"Problem reading. error=%u eof=%u errno=%s\n",in_lstk.top().error(),in_lstk.top().eof(),strerror(errno));
                return 1;
            }
        }

        string linesource = in_lstk.top().getsourcename();
        linecount_t lineno = in_lstk.top().currentline();
        size_t linebufsize = in_lstk.top().linesize();

        /* look for preprocessor directives this code handles by itself */
        {
            char *s = line; cstrskipwhitespace(s);

            if (*s == '#') {
                s++; cstrskipwhitespace(s);

                const string what = cstrgetword(s); cstrskipwhitespace(s);

                if (what == "endif") {
                    if (if_cond_stack.empty()) {
                        fprintf(stderr,"#endif with no #if... condition\n");
                        return 1;
                    }

                    if_cond = if_cond_stack.top();
                    if_cond_stack.pop();

                    emit_line = true;
                    continue; /* do not send to output */
                }
                else if (what == "ifdef") {
                    string macroname;

                    macroname = cstrgetword(s);
                    if (macroname.empty()) {
                        fprintf(stderr,"#ifdef without macro name\n");
                        return 1;
                    }

                    if_cond_stack.push(if_cond);
                    if_cond = if_cond.eval() && eval_ifdef(macroname);
                    if_cond.allow_else = true; /* #if enables #else */

                    emit_line = true;
                    continue; /* do not send to output */
                }
                else if (what == "ifndef") {
                    string macroname;

                    macroname = cstrgetword(s);
                    if (macroname.empty()) {
                        fprintf(stderr,"#ifdef without macro name\n");
                        return 1;
                    }

                    if_cond_stack.push(if_cond);
                    if_cond = if_cond.eval() && !eval_ifdef(macroname);
                    if_cond.allow_else = true; /* #if enables #else */

                    emit_line = true;
                    continue; /* do not send to output */
                }
                else if (what == "else") {
                    if (!if_cond.allow_else) {
                        fprintf(stderr,"#else is invalid here\n");
                        return 1;
                    }

                    /* #else was used, no longer allowed at this level.
                     * invert conditional and continue. */
                    bool parent_cond = true;
                    if (!if_cond_stack.empty())
                        parent_cond = if_cond_stack.top().eval();

                    if_cond.cond = parent_cond && !if_cond.cond;
                    if_cond.allow_else = false;
                    continue; /* do not send to output */
                }

                if (!if_cond.eval())
                    continue;

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

                    emit_line = true;
                    continue; /* do not send to output */
                }
                else if (what == "define") {
                    bool to_be_continued = false;
                    haxpp_macro macro;
                    string macroname;

                    if (!macro.parse_identifier(macroname,s))
                        return 1;

                    if (*s != 0) {
                        if (!isspace(*s)) {
                            fprintf(stderr,"Macro must have a space between name and string. '%s'\n",s);
                            return 1;
                        }
                        cstrskipwhitespace(s);
                    }

                    do {
                        to_be_continued = false;
                        if (!macro.parse_token_string(to_be_continued,s))
                            return 1;

                        if (to_be_continued) {
                            line = in_lstk.top().readline();
                            if (line == nullptr) {
                                fprintf(stderr,"ERROR: Multiline macro cut off at EOF\n");
                                return 1;
                            }
                            s = line;
                            macro.add_newline_subst();
                        }
                    } while (to_be_continued);

                    if (!add_macro(macroname,macro))
                        return 1;

                    emit_line = true;
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

                    emit_line = true;
                    continue; /* do not send to output */
                }
            }
            else {
                if (!if_cond.eval())
                    continue;
            }
        }

        bool expand_multiline = false;

        macro_expand(line,line+linebufsize,expand_multiline,false/*whether to replace non-existing macros with nothing*/);

        if (emit_line) {
            send_line(out_ls,linesource,lineno);
            emit_line = false;
        }

        if (!out_ls.write(line)) {
            fprintf(stderr,"Error writing output\n");
            return 1;
        }

        if (expand_multiline)
            emit_line = true;
    }

    if (!if_cond_stack.empty()) {
        fprintf(stderr,"#if...#endif block mismatch after parsing\n");
        return 1;
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

    if (dump_macros)
        dump_all_macros();

    out_ls.close();
    in_lstk.clear();
    return 0;
}

