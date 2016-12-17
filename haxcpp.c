
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char                dumppre = 0;

char*               source_file = NULL;
char*               output_file = NULL;

#define MAX_DEFINES         4096
#define MAX_SOURCE_FILES    4096
#define MAX_INCLUDE_PATHS   256

struct source_path {
    char*           path;
};

struct source_path      source_path[MAX_SOURCE_FILES];
int                     source_path_count = 0;

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
    char*           name;                   /* macro name */
    char*           value;                  /* macro value */
};

struct macro            macro[MAX_DEFINES];
int                     macro_count=0;

static struct macro *macro_find(const char *name) {
    struct macro *ret;
    int i=0;

    while (i < macro_count) {
        ret = &macro[i++];
        if (!strcmp(name,ret->name))
            return ret;
    }

    return NULL;
}

static struct macro *macro_add(const char *name,const char *value,int source_path_index) {
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
    if (idx < 0)
        return "<command line>";
    if (idx >= source_path_count)
        return "<ERANGE>";

    return source_path[idx].path;
}

static void dump_macros(void) {
    struct macro *mac;
    int i;

    fprintf(stderr,"Macros:\n");
    for (i=0;i < macro_count;i++) {
        mac = &macro[i];
        fprintf(stderr,"  From: %s\n",source_path_name(mac->source_path_index));
        fprintf(stderr,"  Name: %s\n",mac->name);
        fprintf(stderr,"  Valu: %s\n",mac->value);
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
                                if ((mac=macro_add(name,value,-1/*command line*/)) == NULL)
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

int main(int argc,char **argv) {
    int err;

    if ((err=parse_argv(argc,argv)))
        return err;

    if (dumppre) {
        dump_macros();
        dump_includes();
    }

    return 0;
}

