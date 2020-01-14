
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdexcept>
#include <string>

using namespace std;

class FileSource {
public:
                                FileSource() : fp(NULL), ownership(false) { }
                                ~FileSource() { close(); }
public:
    void                        set(FILE *_fp);
    void                        set(const string &_path);
    void                        close();
    void                        open();
    bool                        is_open() const;
    bool                        eof() const;
    const string&               get_path() const;
    int                         getc();
private:
    FILE*                       fp;
    bool                        ownership;
    string                      path;
};

class FileDest {
public:
                                FileDest() : fp(NULL), ownership(false) { }
                                ~FileDest() { close(); }
public:
    void                        set(FILE *_fp);
    void                        set(const string &_path);
    void                        close();
    void                        open();
    bool                        is_open() const;
    const string&               get_path() const;
    void                        putc(char c);
private:
    FILE*                       fp;
    bool                        ownership;
    string                      path;
};

void FileSource::set(FILE *_fp) {
    close();
    fp = _fp;
    path.clear();
}

void FileSource::set(const string &_path) {
    close();
    path = _path;
}

void FileSource::close() {
    if (fp != NULL) {
        if (ownership) fclose(fp);
        fp = NULL;
    }
    ownership = false;
}

void FileSource::open() {
    if (fp == NULL) {
        fp = fopen(path.c_str(),"rb");
        if (fp != NULL)
            ownership = true;
    }
}

bool FileSource::is_open() const {
    return (fp != NULL);
}

bool FileSource::eof() const {
    if (fp != NULL)
        return feof(fp);
    return true;
}

const string& FileSource::get_path() const {
    return path;
}

int FileSource::getc() {
    int c = EOF;

    if (fp != NULL) {
        c = fgetc(fp);
        if (ferror(fp))
            throw runtime_error("File I/O error, reading");
    }

    return c;
}

void FileDest::set(FILE *_fp) {
    close();
    fp = _fp;
    path.clear();
}

void FileDest::set(const string &_path) {
    close();
    path = _path;
}

void FileDest::close() {
    if (fp != NULL) {
        if (ownership) fclose(fp);
        fp = NULL;
    }
    ownership = false;
}

void FileDest::open() {
    if (fp == NULL) {
        fp = fopen(path.c_str(),"wb");
        if (fp != NULL)
            ownership = true;
    }
}

bool FileDest::is_open() const {
    return (fp != NULL);
}

const string& FileDest::get_path() const {
    return path;
}

void FileDest::putc(char c) {
    if (fp != NULL) {
        if (fputc((int)c,fp) == EOF) {
            if (ferror(fp))
                throw runtime_error("File I/O error, writing");
        }
    }
}

static FileSource               in_src;
static FileDest                 out_dst;

static string                   in_file = "-";
static string                   out_file = "-";

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
    int c;

    if (parse_argv(argc,argv))
        return 1;

    if (in_file == "-")
        in_src.set(stdin);
    else
        in_src.set(in_file);

    in_src.open();
    if (!in_src.is_open()) {
        fprintf(stderr,"Unable to open source\n");
        return 1;
    }

    if (out_file == "-")
        out_dst.set(stdout);
    else
        out_dst.set(out_file);

    out_dst.open();
    if (!out_dst.is_open()) {
        fprintf(stderr,"Unable to open dest\n");
        return 1;
    }

    while (!in_src.eof()) {
        c = in_src.getc();
        if (c == EOF) break;
        out_dst.putc(c);
    }

    return 0;
}

