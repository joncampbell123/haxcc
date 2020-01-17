
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <math.h>

#include <stdexcept>
#include <string>
#include <vector>
#include <stack>

using namespace std;

template <typename T> struct bitmask_t {
    T                           start_bit;
    T                           num_bits;

    constexpr bitmask_t() : start_bit(0), num_bits(1) { }
    constexpr bitmask_t(const T start,const T num) : start_bit(start), num_bits(num) { }

    constexpr inline T get_field(const T t) const {
        return (t >> start_bit) & unshifted_bit_mask();
    }
    constexpr inline T unshifted_bit_mask() const {
        return (num_bits != T(0)) ? ((T(1) << num_bits) - T(1)) : T(0);
    }
    constexpr inline T bit_mask() const {
        return unshifted_bit_mask() << start_bit;
    }
    constexpr inline T make_field(const T t) const {
        return (t & unshifted_bit_mask()) << start_bit;
    }
};

typedef size_t                              stringref_t;

static constexpr stringref_t                stringref_t_invalid =       stringref_t( ~stringref_t(0) );
static constexpr stringref_t                stringref_t_bits =          CHAR_BIT * sizeof(stringref_t);
static constexpr stringref_t                stringref_t_class_bits =    stringref_t(2u);
static constexpr stringref_t                stringref_t_index_bits =    stringref_t_bits - stringref_t_class_bits;
static constexpr bitmask_t<stringref_t>     stringref_t_index =         { stringref_t(0), stringref_t(stringref_t_index_bits) };
static constexpr bitmask_t<stringref_t>     stringref_t_class =         { stringref_t(stringref_t_index_bits), stringref_t(stringref_t_class_bits) };

static constexpr inline stringref_t stringref_class(const stringref_t t) {
    return stringref_t_class.get_field(t);
}

static constexpr inline stringref_t stringref_index(const stringref_t t) {
    return stringref_t_index.get_field(t);
}

static constexpr inline stringref_t make_stringref(const stringref_t _class,const stringref_t _index) {
    return stringref_t_class.make_field(_class) + stringref_t_index.make_field(_index);
}

static_assert((stringref_t_class.bit_mask() | stringref_t_index.bit_mask()) == stringref_t_invalid, "index and class field added do not fill all bits");
static_assert((stringref_t_class.bit_mask() & stringref_t_index.bit_mask()) == stringref_t(0), "index and class overlap");
static_assert(stringref_t_class.get_field(make_stringref(2,55)) == 2, "class field corrupted round trip");
static_assert(stringref_t_index.get_field(make_stringref(2,55)) == 55, "index field corrupted round trip");

/* this one requires explanation: class is bits [n-1:n-2] and index is bits [n-3:0].
 * adding +1 to the index bitmask should overflow into the class bitmask */
static_assert(stringref_t_class.get_field(stringref_t_index.bit_mask() + stringref_t(1)) == stringref_t(1), "index and class not adjacent fields");

enum class strtype_t {
    CHAR,
    WIDE16,
    WIDE32
};

class string_storage {
public:
    const string&                           get_char(const stringref_t);
    const basic_string<uint16_t>&           get_wide16(const stringref_t);
    const basic_string<uint32_t>&           get_wide32(const stringref_t);
    stringref_t                             add(const string &x);
    stringref_t                             add(const basic_string<uint16_t> &x);
    stringref_t                             add(const basic_string<uint32_t> &x);
private:
    template <class T> const T&             get(const stringref_t s,const vector<T> &vec,const stringref_t type);
    template <class T> stringref_t          add(vector<T> &vec,const T &x);
private:
    vector<string>                          char_strings;
    vector<string>                          utf8_strings;
    vector< basic_string<uint16_t> >        wide16_strings;
    vector< basic_string<uint32_t> >        wide32_strings;
};

template <class T> const T& string_storage::get(const stringref_t s,const vector<T> &vec,const stringref_t type) {
    if (stringref_class(s) == type) {
        const stringref_t i = stringref_index(s);
        if (i < vec.size())
            return vec[i];

        throw range_error("get_char() out of range");
    }

    throw invalid_argument("get_char() wrong string type");
}

template <class T> stringref_t string_storage::add(vector<T> &vec,const T &x) {
    const stringref_t r = vec.size();
    vec.push_back(x);
    return r;
}

const string& string_storage::get_char(const stringref_t s) {
    return get(s,char_strings,stringref_t(strtype_t::CHAR));
}

const basic_string<uint16_t>& string_storage::get_wide16(const stringref_t s) {
    return get(s,wide16_strings,stringref_t(strtype_t::WIDE16));
}

const basic_string<uint32_t>& string_storage::get_wide32(const stringref_t s) {
    return get(s,wide32_strings,stringref_t(strtype_t::WIDE32));
}

stringref_t string_storage::add(const string &x) {
    return make_stringref(stringref_t(strtype_t::CHAR),add(char_strings,x));
}

stringref_t string_storage::add(const basic_string<uint16_t> &x) {
    return make_stringref(stringref_t(strtype_t::WIDE16),add(wide16_strings,x));
}

stringref_t string_storage::add(const basic_string<uint32_t> &x) {
    return make_stringref(stringref_t(strtype_t::WIDE32),add(wide32_strings,x));
}

class token {
public:
    enum token_t {
        NONE=0,
        MACRO,
        IDENTIFIER,
        INTEGER,
        FLOAT,
        STRING,
        COMMA
    };
public:
    token_t                     token = NONE;
    unsigned char               bsize = 0;
    struct int_t { /* also char constants like 'a' */
        union { /* bsize == 1, 2, 4, 8 */
            signed long long    s = 0;
            unsigned long long  u; /* this is a UNION, preinit only one */
        };
        bool                    sign = true;
    } i;
    struct float_t { /* bsize == 4, 8 */
        long double             fraction = 0;
        int                     exponent = 0;

        inline long double get_double() {
            return ldexpl(fraction,exponent);
        }
        inline float_t &operator=(const long double f) {
            exponent = 0;
            fraction = frexpl(f,&exponent);
            return *this;
        }
    } f;
    struct string_t {
        stringref_t             strref = stringref_t_invalid; /* NTS: stringref_t also encodes string type */
    } s;
};

typedef vector<token>           token_string;

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

static string_storage           string_store;

static FileSourceStack          in_src_stk;
static FileDest                 out_dst;

static bool                     ppp_only = false;
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
            else if (!strcmp(a,"EE")) {
                ppp_only = true;
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

            if (ppp_only) {
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

