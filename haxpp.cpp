
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
    void                        reset_counters();
public:
    inline int32_t current_line() const {
        return line;
    }
    inline int current_column() const {
        return column;
    }
private:
    FILE*                       fp;
    bool                        ownership;
    string                      path;
    int32_t                     line;
    int                         column;
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
    void                        puts(const char *s);
    void                        puts(const string &s);
private:
    FILE*                       fp;
    bool                        ownership;
    string                      path;
};

void FileSource::reset_counters() {
    line = 1;
    column = 0;
}

void FileSource::set(FILE *_fp) {
    close();
    fp = _fp;
    path.clear();
    reset_counters();
}

void FileSource::set(const string &_path) {
    close();
    path = _path;
    reset_counters();
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
        do {
            c = fgetc(fp);
        } while (c == '\r'/*chars to ignore*/);

        if (c == '\n') {
            line++;
            column = 1;
        }
        else {
            column++;
        }

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

void FileDest::puts(const char *s) {
    if (fp != NULL) {
        if (fputs(s,fp) == EOF) {
            if (ferror(fp))
                throw runtime_error("File I/O error, writing");
        }
    }
}

void FileDest::puts(const string &s) {
    puts(s.c_str());
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

static bool                     pp_only = false;

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
            else if (!strcmp(a,"E")) {
                pp_only = true;
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

/* caller just read a backslash '\\' */
static void read_line_esc(string &line,FileSource &src) {
    int c2 = src.getc();
    if (c2 != EOF) { /* trailing \<EOF> should just do nothing */
        if (c2 == '\n') { /* \<newline>, so it continues on the next line */
            line += ' ';
        }
        else {
            line += '\\';
            line += c2;
        }
    }
}

/* caller just read a quote '\'' or '\"' in (c) */
static void read_line_quote(string &line,FileSource &src,const char c) {
    int c2;

    line += c;
    do {
        c2 = src.getc();
        if (c == EOF)
            break;
        else if (c == '\n')
            break;
        else if (c == '\\')
            read_line_esc(line,src);
        else {
            line += c2;
            if (c2 == c)
                break;
        }
    } while(1);
}

/* caller just read // */
static bool read_line_skip_cpp_comment(FileSource &src) {
    string tmp;
    int c;

    do {
        c = src.getc();
        if (c == EOF)
            break;
        else if (c == '\n')
            return false;
        else if (c == '\\') {
            c = src.getc();
            if (c == '\n') break;
        }
    } while(1);

    return true;
}

/* caller just read / * * / */
static void read_line_skip_c_comment(FileSource &src) {
    string tmp;
    int c;

    do {
        c = src.getc();
        if (c == EOF)
            break;
        else if (c == '*') { /* C comment closing */
            c = src.getc();
            if (c == '/') break;
        }
        else if (c == '/') {
            c = src.getc();
            if (c == '*') /* another C comment opening. we allow nesting */
                read_line_skip_c_comment(src);
        }
    } while(1);
}

bool read_line(string &line,FileSource &src) {
    int c;

    line.clear();
    while (!src.eof()) {
        c = src.getc();

        if (c == EOF)
            break;
        else if (c == '\n')
            break;
        else if (c == '\\')
            read_line_esc(line,src);
        else if (c == '\'')
            read_line_quote(line,src,c);
        else if (c == '/') {
            int c2 = src.getc();
            if (c2 == EOF) {
                line += c;
            }
            else if (c2 == '/') { /* C++ // comment */
                if (!read_line_skip_cpp_comment(src))
                    break;
            }
            else if (c2 == '*') { /* C comment like this */
                read_line_skip_c_comment(src);
            }
            else {
                line += c;
                line += c2;
            }
        }
        else
            line += c;
    }

    if (line.empty() && src.eof())
        return false;

    return true;
}

int main(int argc,char **argv) {
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

    string line;
    bool emit_line = false;
    int32_t lineno_expect = -1;
    while (!in_src_stk.empty()) {
        const int32_t lineno = in_src_stk.top().current_line();
        const string &source = in_src_stk.top().get_path();

        if (read_line(/*&*/line,in_src_stk.top())) {
            if (lineno_expect != lineno)
                emit_line = true;

            if (pp_only) {
                if (emit_line) {
                    out_dst.puts(string("#line ") + to_string(lineno) + " " + source + "\n");
                    emit_line = false;
                }

                out_dst.puts(line);
                out_dst.putc('\n');
            }

            lineno_expect = lineno + int32_t(1);
        }
        else if (in_src_stk.top().eof()) {
            emit_line = true;
            in_src_stk.pop();
        }
    }

    return 0;
}

