
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

char                dumppre = 0;

char*               source_file = NULL;
char*               output_file = NULL;

FILE*               source_fp = NULL;
FILE*               output_fp = NULL;

#define MAX_DEFINES         4096
#define MAX_SOURCE_FILES    4096
#define MAX_INCLUDE_PATHS   256

struct source_path {
    char*           path;
};

struct source_path      source_path[MAX_SOURCE_FILES];
int                     source_path_count = 0;

static int source_path_ptr2index(struct source_path *rec) {
    return (int)(rec - &source_path[0]); /* C pointer math says result is byte count difference divided by sizeof struct */
}

static struct source_path *source_path_add(const char *path) {
    struct source_path *ret;
    int i=0;

    /* match an existing one */
    while (i < source_path_count) {
        ret = &source_path[i++];
        if (!strcmp(path,ret->path))
            return ret;
    }

    /* then we have to allocate a new one */
    ret = &source_path[source_path_count];
    ret->path = strdup(path);
    if (ret->path == NULL)
        return NULL;

    source_path_count++;
    return ret;
}

struct macro {
    int             source_path_index;      /* where it came from (source_path index), or -1 for command line */
    long            source_line;            /* what line */
    char*           name;                   /* macro name */
    char*           value;                  /* macro value */
};

struct macro            macro[MAX_DEFINES];
int                     macro_count=0;

static int macro_delete(struct macro *mac) {
    if (mac->name) {
        free(mac->name);
        mac->name = NULL;
    }
    if (mac->value != NULL) {
        free(mac->value);
        mac->value = NULL;
    }
    mac->source_line = 0;
    mac->source_path_index = -1;
    return 0;
}

static struct macro *macro_find(const char *name) {
    struct macro *ret;
    int i=0;

    while (i < macro_count) {
        ret = &macro[i++];
        if (ret->name && !strcmp(name,ret->name))
            return ret;
    }

    return NULL;
}

static struct macro *macro_add(const char *name,const char *value,int source_path_index,long line) {
    struct macro *mac;

    /* NTS: for perf reasons, this does not look for existing macros of the same name. */
    /*      when adding a macro you're supposed to call macro_find() first to determine */
    /*      if it already exists */
    if (*name == 0) /* "" is not a valid name */
        return NULL;

    if (macro_count >= MAX_DEFINES) {
        fprintf(stderr,"Too many defines\n");
        return NULL;
    }

    mac = &macro[macro_count];
    mac->source_path_index = source_path_index;
    mac->source_line = line;
    mac->name = strdup(name);
    mac->value = strdup(value);
    if (mac->name == NULL || mac->value == NULL)
        return NULL;

    macro_count++;
    return mac;
}

int macro_value_replace(struct macro *mac,const char *value) {
    if (mac->value != NULL)
        free(mac->value);

    mac->value = strdup(value);
    if (mac->value == NULL)
        return -1;

    return 0;
}

struct include_path {
    char*           path;
};

struct include_path     include_path[MAX_INCLUDE_PATHS];
int                     include_path_count=0;

static int include_path_add(const char *path) {
    struct include_path *inc;
    int i=0;

    if (*path == 0) /* "" is not a valid path */
        return -1;

    if (include_path_count >= MAX_INCLUDE_PATHS) {
        fprintf(stderr,"Too many include paths\n");
        return -1;
    }

    /* match an existing one */
    while (i < include_path_count) {
        inc = &include_path[i++];
        if (!strcmp(inc->path,path))
            return 0;
    }

    /* alloc a new one */
    inc = &include_path[include_path_count];
    inc->path = strdup(path);
    if (inc->path == NULL)
        return -1;

    include_path_count++;
    return 0;
}

static const char *source_path_name(int idx) {
    if (idx == -1)
        return "<command line>";
    else if (idx == -2)
        return "<stdin>";
    else if (idx < -2 || idx >= source_path_count)
        return "<ERANGE>";

    return source_path[idx].path;
}

static void dump_macros(void) {
    struct macro *mac;
    int i;

    fprintf(stderr,"Macros:\n");
    for (i=0;i < macro_count;i++) {
        mac = &macro[i];
        if (mac->name == NULL) continue;
        fprintf(stderr,"  From: %s\n",source_path_name(mac->source_path_index));
        fprintf(stderr,"  Line: %ld\n",mac->source_line);
        fprintf(stderr,"  Name: %s\n",mac->name);
        fprintf(stderr,"  Valu: %s\n",mac->value);
        fprintf(stderr,"\n");
    }
}

static void dump_includes(void) {
    struct include_path *path;
    int i;

    fprintf(stderr,"Include paths:\n");
    for (i=0;i < include_path_count;i++) {
        path = &include_path[i];
        fprintf(stderr,"    Path: %s\n",path->path);
    }
}

static void help(void) {
    fprintf(stderr,"haxcpp [options] <source file>\n");
    fprintf(stderr,"HaxCC preprocessor.\n");
    fprintf(stderr,"Processes source file, and emits to STDOUT by default.\n");
    fprintf(stderr,"If the source file is not specified, STDIN is processed instead.\n");
    fprintf(stderr,"\n");
    fprintf(stderr," -o <file>          Emit to this file instead\n");
    fprintf(stderr," -Dmacro[=val]      Define a macro, optionally with a value\n");
    fprintf(stderr," -I<path>           Add <path> to the search path\n");
    fprintf(stderr," --dumppre          Dump state prior to processing.\n");
}

/* this enables "-Dmacro" or "-D" "macro" */
static char *parse_short_switch_strval(int *argc,char **argv,char *a) {
    if (*a == 0) { /* if we immediately hit the end of the argv, go to next one */
        a = argv[(*argc)++];
        if (a == NULL)
            return NULL;
    }

    return a;
}

static int parse_argv(int argc,char **argv) {
    int noswp=0;
    char *a,sw;
    int i;

    for (i=1;i < argc;) {
        a = argv[i++];

        if (*a == '-') {
            a++;
            if (*a == '-') {
                /* long form */
                a++;
                if (!strcmp(a,"help")) {
                    help();
                    return 1;
                }
                else if (!strcmp(a,"dumppre")) {
                    dumppre = 1;
                }
                else {
                    fprintf(stderr,"Unknown switch '%s'\n",a);
                    return 1;
                }
            }
            else {
                /* one char short form */
                switch (sw=*a++) {
                    case 'o':
                        if ((a=parse_short_switch_strval(&i,argv,a)) == NULL)
                            return 1;
                        if ((output_file=strdup(a)) == NULL)
                            return -1;
                        break;
                    case 'D':
                        if ((a=parse_short_switch_strval(&i,argv,a)) == NULL)
                            return 1;
                        {
                            /* we need to cut the string at the first equals sign, divide name=value.
                             * if there is no equals sign, then the name is the full string and value is "" */
                            char *name = a;
                            char *value = strchr(name,'=');
                            struct macro *mac;

                            if (value != NULL) 
                                *value++ = 0; /* ASCIIz SNIP */
                            else
                                value = "";

                            mac = macro_find(name);
                            if (mac != NULL) {
                                if (strcmp(mac->value,value) != 0) {
                                    /* only warn if the name is the same, and the value is not */
                                    fprintf(stderr,"WARNING: Macro '%s' multiply defined, with different values.\n",name);
                                    /* and then let the latter one replace the former */
                                    if (macro_value_replace(mac,value) < 0)
                                        return -1;
                                }
                            }
                            else {
                                if ((mac=macro_add(name,value,-1/*command line*/,-1L/*line*/)) == NULL)
                                    return -1;
                            }
                        }
                        break;
                    case 'I':
                        if ((a=parse_short_switch_strval(&i,argv,a)) == NULL)
                            return 1;
                        if (include_path_add(a) < 0)
                            return 1;
                        break;
                    case 'h':
                        help();
                        return 1;
                    default:
                        fprintf(stderr,"Unknown switch '%s'\n",a);
                        return 1;
                }
            }
        }
        else {
            switch (noswp++) {
                case 0:
                    if ((source_file=strdup(a)) == NULL)
                        return -1;
                    break;
                default:
                    fprintf(stderr,"Unhandled arg '%s'\n",a);
                    return 1;
            }
        }
    }

    return 0;
}

static void strp_eatwhitespace(char **s) {
    char c;

    while ((c=*(*s)) && isspace(c))
        (*s)++;
}

static int strp_parsedirective(char *dst,size_t dstmax,char **s) {
    size_t o=0;
    char c;

    strp_eatwhitespace(s);

    while ((c=*(*s)) && !(isspace(c) || c == '<')) {
        if ((o+1) >= dstmax)
            return -1;

        if (!isalpha(c))
            return -1;

        dst[o++] = c;
        (*s)++;
    }
    dst[o] = 0;
    return 0;
}

static int ismacronamechar(const char c) {
    return isalnum(c) || c == '_';
}

static int strp_macroname(char *dst,size_t dstmax,char **s) {
    size_t o=0;
    char c;

    strp_eatwhitespace(s);

    while ((c=*(*s)) && !(isspace(c) || c == '(')) {
        if ((o+1) >= dstmax)
            return -1;

        if (!ismacronamechar(c))
            return -1;

        dst[o++] = c;
        (*s)++;
    }
    dst[o] = 0;
    return 0;
}

/* line (and where string expansion occurs) */
static char token[1024];
static char line[32768];
static long current_source_path_line;
static int current_source_path_index;

enum {
    INFO=0,
    WARNING,
    ERROR
};

const char *stderr_msg[] = {
    "Info",
    "Warning",
    "Error"
};

void fprintf_stderr_current_source(const int wtype) {
    fprintf(stderr,"%s: %s:%ld ",stderr_msg[wtype],source_path_name(current_source_path_index),current_source_path_line);
}

int do_undef(char *s) {
    struct macro *mac;

/* #undef macro */
/* s = points at first char of macro name */
    if (*s == 0) {
        fprintf_stderr_current_source(ERROR);
        fprintf(stderr,"#define requires macro name\n");
        return -1;
    }
    if (strp_macroname(token,sizeof(token),&s) < 0) {
        fprintf_stderr_current_source(ERROR);
        fprintf(stderr,"Failure to parse macro name\n");
        return -1;
    }
    if (*s == 0) {
        /* #define macro with no value */
    }
    else if (!isspace(*s)) {
        fprintf_stderr_current_source(ERROR);
        fprintf(stderr,"Junk after macro name '%s'\n",s);
        return -1;
    }
    strp_eatwhitespace(&s);

    mac = macro_find(token);
    if (mac != NULL) {
        macro_delete(mac);
    }
    else {
        fprintf_stderr_current_source(WARNING);
        fprintf(stderr,"Macro '%s' not defined\n",token);
    }

    return 0;
}

int do_define(char *s) {
    struct macro *mac;

/* #define macro
 * #define macro value */
/* s = points at first char of macro name */
    if (*s == 0) {
        fprintf_stderr_current_source(ERROR);
        fprintf(stderr,"#define requires macro name\n");
        return -1;
    }
    if (strp_macroname(token,sizeof(token),&s) < 0) {
        fprintf_stderr_current_source(ERROR);
        fprintf(stderr,"Failure to parse macro name\n");
        return -1;
    }
    if (*s == '(') {
        fprintf_stderr_current_source(ERROR);
        fprintf(stderr,"Macros with parameters not yet supported\n");
        return -1;
    }
    if (*s == 0) {
        /* #define macro with no value */
    }
    else if (!isspace(*s)) {
        fprintf_stderr_current_source(ERROR);
        fprintf(stderr,"Junk after macro name '%s'\n",s);
        return -1;
    }
    strp_eatwhitespace(&s);

    mac = macro_find(token);
    if (mac != NULL) {
        if (strcmp(mac->value,s) != 0) {
            /* only warn if the name is the same, and the value is not */
            fprintf_stderr_current_source(WARNING);
            fprintf(stderr,"Macro '%s' multiply defined, with different values.\n",token);
            /* and then let the latter one replace the former */
            if (macro_value_replace(mac,s) < 0)
                return -1;
        }
    }
    else {
        if ((mac=macro_add(token,s,current_source_path_index,current_source_path_line)) == NULL)
            return -1;
    }

    return 0;
}

int main(int argc,char **argv) {
    struct source_path *srcpath;
    int err;

    if ((err=parse_argv(argc,argv)))
        return err;

    if (dumppre) {
        dump_macros();
        dump_includes();
    }

    if (source_file != NULL) {
        source_fp = fopen(source_file,"r");
        if (!source_fp) {
            fprintf(stderr,"Unable to open source file '%s', %s\n",source_file,strerror(errno));
            return 1;
        }

        srcpath = source_path_add(source_file);
        if (srcpath == NULL) {
            fprintf(stderr,"Unable to add source file path\n");
            return 1;
        }
        current_source_path_index = source_path_ptr2index(srcpath);
    }
    else {
        source_fp = stdin;
        current_source_path_index = -2; /* STDIN */
    }

    if (output_file != NULL) {
        output_fp = fopen(output_file,"w");
        if (!output_fp) {
            fprintf(stderr,"Unable to open output file '%s', %s\n",output_file,strerror(errno));
            return 1;
        }
    }
    else {
        output_fp = stdout;
    }

    current_source_path_line = 0;
    while (!feof(source_fp)) {
        if (ferror(source_fp)) {
            fprintf_stderr_current_source(ERROR);
            fprintf(stderr,"Source file error, %s\n",strerror(errno));
            goto fail;
        }
        current_source_path_line++;
        if (fgets(line,sizeof(line),source_fp) == NULL)
            break;

        /* eat newline */
        {
            char *s = line + strlen(line) - 1;
            while (s >= line && (*s == '\r' || *s == '\n')) *s-- = 0;
        }

        /* handle the preprocessor directives we understand, pass the ones we don't */
        {
            char *s = line;
            char *n;

            /* skip non-space */
            while (*s && isspace(*s)) s++;

            /* is this is a directive we know? */
            if (*s == '#') {
                s++;
                if (strp_parsedirective(token,sizeof(token),&s) == 0) {
                    if (!strcasecmp(token,"define")) {
                        /* do not carry through */
                        strp_eatwhitespace(&s);
                        if (do_define(s) < 0)
                            goto fail;

                        continue;
                    }
                    else if (!strcasecmp(token,"undef")) {
                        /* do not carry through */
                        strp_eatwhitespace(&s);
                        if (do_undef(s) < 0)
                            goto fail;

                        continue;
                    }
                    else if (!strcasecmp(token,"error")) {
                        /* error out, as instructed */
                        strp_eatwhitespace(&s);
                        fprintf_stderr_current_source(ERROR);
                        fprintf(stderr,"#error %s\n",s);
                        goto fail;
                    }
                }
            }
        }

        /* output */
        fprintf(output_fp,"%s\n",line);
    }

    if (source_fp != NULL) {
        fclose(source_fp);
        source_fp = NULL;
    }

    if (output_fp != NULL) {
        fclose(output_fp);
        output_fp = NULL;
    }

    if (dumppre) {
        dump_macros();
        dump_includes();
    }

    return 0;
fail:
    /* TODO: delete output file */
    return 1;
}

