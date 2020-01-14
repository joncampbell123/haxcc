
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdexcept>
#include <string>
#include <vector>
#include <stack>

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

class FileSourceStack {
public:
    static constexpr size_t     default_size = 64;
public:
    vector<FileSource>          src;
    ssize_t                     stkpos = -1;
public:
    void alloc() {
        alloc(default_size);
    }
    void alloc(const size_t ns) {
        if (src.empty())
            src.resize(ns);
        else if (src.size() != ns)
            throw runtime_error("FileSourceStack attempt to alloc when already allocated");
    }
    void free() {
        src.clear();
    }
    bool empty() const {
        return stkpos == -1;
    }
    FileSource& top() {
        if (stkpos >= 0 && size_t(stkpos) < src.size())
            return src[stkpos];

        throw overflow_error("FileSourceStack top() called when empty");
    }
    void push() {
        const size_t np = size_t(stkpos+1);

        if (np < src.size())
            stkpos = np;
        else
            throw overflow_error("FileSourceStack stack overflow");
    }
    void pop() {
        if (stkpos >= 0) {
            src[stkpos].close();
            stkpos--;
        }
        else {
            throw underflow_error("FileSourceStack stack underflow");
        }
    }
};

static FileSourceStack          in_src_stk;
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

    in_src_stk.alloc();
    in_src_stk.push();
    if (in_file == "-")
        in_src_stk.top().set(stdin);
    else
        in_src_stk.top().set(in_file);

    in_src_stk.top().open();
    if (!in_src_stk.top().is_open()) {
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

    while (!in_src_stk.empty()) {
        c = in_src_stk.top().getc();
        if (c == EOF) {
            in_src_stk.pop();
            continue;
        }

        out_dst.putc(c);
    }

    return 0;
}

