
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>
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
        PREPROCDIR,
        IDENTIFIER,
        KEYWORD,
        INTEGER,
        FLOAT,
        STRING,
        MINUS,
        PLUS,
        DECREMENT,
        INCREMENT,
        COMMA,
        PERIOD,
        DOTDOTDOT,
        PTRARROW,
        COMPLEMENT,
        NOT,
        AMPERSAND,
        STAR,
        OPEN_PARENS,
        CLOSE_PARENS,
        OPEN_SBRACKET,
        CLOSE_SBRACKET,
        OPEN_CBRACKET,
        CLOSE_CBRACKET,
        SIZEOF,

        MAX_TOKEN
    };
public:
    token_t                     tval = NONE;
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

        inline long double get_double() const {
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
    string                      sval;

    token();
    token(const long long v);
    token(const long double v);
    token(const stringref_t sr);
    token(const unsigned long long v);
    token(const token_t t);
    explicit token(const token_t t,const string _sval);
};

token::token() {
}

token::token(const stringref_t sr) : tval(STRING) {
    s.strref = sr;
}

token::token(const long long v) : tval(INTEGER) {
    i.s = v;
    i.sign = true;
}

token::token(const long double v) : tval(FLOAT) {
    f = v;
}

token::token(const unsigned long long v) : tval(INTEGER) {
    i.u = v;
    i.sign = false;
}

token::token(const token_t t,const string _sval) : tval(t), sval(_sval) {
}

token::token(const token_t t) : tval(t) {
}

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

void parse_skip_whitespace(string::iterator &li,const string::iterator lie) {
    while (li != lie && (*li == ' ' || *li == '\t')) li++;
}

enum token::token_t is_keyword(const string &s) {
    if (s == "sizeof")
        return token::SIZEOF;

    return token::NONE;
}

bool is_macro(const string &s) {
    (void)s;
    return false;
}

/* ANSI C89 Sec 3.1.2 "Identifiers" */
bool isidentifier_fc(const char c) { /* first char */
    return (isalpha(c) || c == '_');
}

bool isidentifier_mc(const char c) { /* non-first char */
    return isdigit(c) || isidentifier_fc(c);
}

bool isoctdigit(const char c) {
    return (c >= '0' && c <= '7');
}

bool isoctdigit_throw(const char c) {
    if (c == '8' || c == '9')
        throw invalid_argument("invalid octal digit");

    return (c >= '0' && c <= '7');
}

unsigned char char2dec_assume(const char c) { /* assumes you checked first! */
    return (c - '0');
}

unsigned char char2oct_assume(const char c) { /* assumes you checked first! */
    return (c - '0');
}

bool ishexdigit(const char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

unsigned char char2hex_assume(const char c) { /* assumes you checked first! */
    if (c >= 'A' && c <= 'F')
        return (c + 10 - 'A');
    if (c >= 'a' && c <= 'f')
        return (c + 10 - 'a');

    return (c - '0');
}

unsigned long long parse_escape_oct(string::iterator &li,const string::iterator lie,const unsigned int min_digits,const unsigned int max_digits) {
    unsigned long long r = 0;
    unsigned int digits = 0;

    while (li != lie && digits < max_digits) {
        if (isoctdigit(*li)) {
            r = (r << 3ull) + char2oct_assume(*(li++));
            digits++;
        }
        else {
            break;
        }
    }

    if (digits < min_digits)
        throw invalid_argument("octal parse: not enough digits");

    return r;
}

unsigned long long parse_escape_hex(string::iterator &li,const string::iterator lie,const unsigned int min_digits,const unsigned int max_digits) {
    unsigned long long r = 0;
    unsigned int digits = 0;

    while (li != lie && digits < max_digits) {
        if (ishexdigit(*li)) {
            r = (r << 4ull) + char2hex_assume(*(li++));
            digits++;
        }
        else {
            break;
        }
    }

    if (digits < min_digits)
        throw invalid_argument("hex parse: not enough digits");

    return r;
}

unsigned long long parse_string_char_escape(string::iterator &li,const string::iterator lie) {
    unsigned long long r = ' ';
    char c;

    if (li == lie) throw invalid_argument("expected string char escape");
    if (*li != '\\') throw invalid_argument("expected string char escape backslash");
    li++;
    if (li == lie) throw invalid_argument("expected string char escape and char");
    c = (*li++);

    switch (c) {
        case 'a':   return 0x07;
        case 'b':   return 0x08;
        case 'e':   return 0x1B;
        case 'f':   return 0x0C;
        case 'n':   return 0x0A;
        case 'r':   return 0x0D;
        case 't':   return 0x09;
        case 'v':   return 0x0B;
        case '\\':  return 0x5C;
        case '\'':  return 0x27;
        case '\"':  return 0x22;
        case '?':   return 0x3F;
        case 'x':   return parse_escape_hex(li,lie,2,2);
        case 'u':   return parse_escape_hex(li,lie,4,4);
        case 'U':   return parse_escape_hex(li,lie,8,8);
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            li--; /* step back so it can parse the first digit we just ate */
            return parse_escape_oct(li,lie,1,3);
        default:
            fprintf(stderr,"WARNING: Unknown char escape \\%c\n",c);
            break;
    };

    return r;
}

char parse_string_char(string::iterator &li,const string::iterator lie) {
    if (li == lie) throw invalid_argument("expected string char");

    if (*li == '\\') {
        unsigned long long c = parse_string_char_escape(li,lie);
        if (c > 0xFFu) fprintf(stderr,"WARNING: Char constant exceeds type range\n");

        return (char)(c & 0xFFul);
    }

    return *(li++);
}

unsigned long long parse_sq_char(string::iterator &li,const string::iterator lie) {
    /* at this point *li == '\'' */
    unsigned long long shf = 0;
    unsigned long long r = 0;
    char c;

    if (li == lie) throw invalid_argument("expected char, got end of line");
    if (*li != '\'') throw invalid_argument("expected char quote, got " + *li);
    li++;

    /* NTS: The reason we loop like this is to support things like 'abcd' to define a 32-bit value
     *      that, when written to memory, spells out 'abcd'. Used in i.e. Microsoft VFW libraries. */
    do {
        if (li == lie) throw invalid_argument("string cut short, expected close quote");
        if (*li == '\'') { li++; break; }
        c = parse_string_char(li,lie);
        r += ((unsigned long long)((unsigned char)c)) << shf;
        shf += 8ull;
    } while (1);

    if (shf > 64ull) fprintf(stderr,"WARNING: char constant too big for compiler storage, truncated");

    return r;
}

static inline char strit_next(string::iterator &ti,const string::iterator lie) {
    if (ti != lie)
        return *ti;

    return 0;
}

static inline bool strit_next_match_inc(string::iterator &ti,const string::iterator lie,const char c) {
    if (ti != lie && *ti == c) {
        ti++;
        return true;
    }

    return false;
}

static inline bool strit_next_match_inc(string::iterator &ti,const string::iterator lie,const char c,const char c2) {
    auto tmp = ti;

    if (tmp != lie && *tmp == c) {
        tmp++;
        if (tmp != lie && *tmp == c2) {
            tmp++;
            ti = tmp;
            return true;
        }
    }

    return false;
}

static inline bool strit_next_match_inc(string::iterator &ti,const string::iterator lie,const char c,const char c2,const char c3) {
    auto tmp = ti;

    if (tmp != lie && *tmp == c) {
        tmp++;
        if (tmp != lie && *tmp == c2) {
            tmp++;
            if (tmp != lie && *tmp == c3) {
                tmp++;
                ti = tmp;
                return true;
            }
        }
    }

    return false;
}

unsigned long long parse_hex_number(string::iterator &li,const string::iterator lie) {
    unsigned long long r = 0;

    while (li != lie && ishexdigit(*li))
        r = (r << 4ull) + char2hex_assume(*(li++));

    return r;
}

unsigned long long parse_oct_number(string::iterator &li,const string::iterator lie) {
    unsigned long long r = 0;

    while (li != lie && isoctdigit_throw(*li))
        r = (r << 3ull) + char2oct_assume(*(li++));

    return r;
}

unsigned long long parse_dec_number(string::iterator &li,const string::iterator lie) {
    unsigned long long r = 0;

    while (li != lie && isdigit(*li))
        r = (r * 10ull) + char2dec_assume(*(li++));

    return r;
}

void parse_float_hexponent(signed long long &exponent,string::iterator &li,const string::iterator lie) {
    /* e.g. "p-3". caller has already eaten the 'p' */
    signed long long exp_adjust;

    if (strit_next_match_inc(li,lie,'-'))
        exp_adjust = -((signed long long)parse_dec_number(li,lie));
    else if (strit_next_match_inc(li,lie,'+'))
        exp_adjust =   (signed long long)parse_dec_number(li,lie);
    else
        exp_adjust =   (signed long long)parse_dec_number(li,lie);

    exponent += exp_adjust;
}

void parse_float_exponent(long double &r,string::iterator &li,const string::iterator lie) {
    /* e.g. "e-3". caller has already eaten the 'e' */
    signed long long exp_adjust;

    if (strit_next_match_inc(li,lie,'-'))
        exp_adjust = -((signed long long)parse_dec_number(li,lie));
    else if (strit_next_match_inc(li,lie,'+'))
        exp_adjust =   (signed long long)parse_dec_number(li,lie);
    else
        exp_adjust =   (signed long long)parse_dec_number(li,lie);

    if (exp_adjust != 0) {
        long double m = 1;
        while (exp_adjust <= -9) {
            exp_adjust += 9;
            m /= 1000000000;
        }
        while (exp_adjust < 0) {
            exp_adjust++;
            m /= 10;
        }

        while (exp_adjust >= 9) {
            exp_adjust -= 9;
            m *= 1000000000;
        }
        while (exp_adjust > 0) {
            exp_adjust--;
            m *= 10;
        }

        r *= m;
    }
}

void parse_float_suffixes(token &r,string::iterator &li,const string::iterator lie) {
    if (strit_next_match_inc(li,lie,'f'))
        r.bsize = 32; /* float */
    else if (strit_next_match_inc(li,lie,'F'))
        r.bsize = 32; /* float */
    else if (strit_next_match_inc(li,lie,'d'))
        r.bsize = 64; /* double */
    else if (strit_next_match_inc(li,lie,'D'))
        r.bsize = 64; /* double */
    else if (strit_next_match_inc(li,lie,'l'))
        r.bsize = 80; /* long double */
    else if (strit_next_match_inc(li,lie,'L'))
        r.bsize = 80; /* long double */
}

void parse_int_suffixes(token &r,string::iterator &li,const string::iterator lie) {
    if (strit_next_match_inc(li,lie,'u'))
        r.i.sign = false;
    else if (strit_next_match_inc(li,lie,'U'))
        r.i.sign = false;

    if (strit_next_match_inc(li,lie,'i','6','4'))
        r.bsize = 64; /* long long */
    else if (strit_next_match_inc(li,lie,'I','6','4'))
        r.bsize = 64; /* long long */
    else if (strit_next_match_inc(li,lie,'l','l'))
        r.bsize = 64; /* long long */
    else if (strit_next_match_inc(li,lie,'L','L'))
        r.bsize = 64; /* long long */
    else if (strit_next_match_inc(li,lie,'l'))
        r.bsize = 32; /* long */
    else if (strit_next_match_inc(li,lie,'L'))
        r.bsize = 32; /* long */
}

token parse_hex_number_float_sub(string::iterator &li,const string::iterator lie) {
    const unsigned long long tmp_msd = 0xfull << (64ull - 4ull);
    unsigned long long tmp = 0;
    int tmpshf = 64;
    token r;

    if (li != lie) {
        while (li != lie && isxdigit(*li)) {
            if (tmpshf >= 4) {
                tmpshf -= 4;
                tmp += ((unsigned long long)char2hex_assume(*li)) << (unsigned long long)tmpshf;
            }

            li++;
        }
    }

    if (tmp != 0ull) { /* or else this will loop forever */
        while ((tmp & tmp_msd) == 0ull) {
            tmpshf += 4;
            tmp <<= 4ull;
        }

        assert(tmpshf <= 64);
    }

    const int dpshf = tmpshf;

    if (strit_next_match_inc(li,lie,'.')) {
        while (li != lie && isxdigit(*li)) {
            if (tmpshf >= 4) {
                tmpshf -= 4;
                tmp += ((unsigned long long)char2hex_assume(*li)) << (unsigned long long)tmpshf;
            }

            li++;
        }
    }

    signed long long exponent = -dpshf;

    /* FIXME: GCC behavior suggests the 'p' is mandatory. */
    if (strit_next_match_inc(li,lie,'p'))
        parse_float_hexponent(exponent,li,lie);
    else if (strit_next_match_inc(li,lie,'P'))
        parse_float_hexponent(exponent,li,lie);

    r.tval = token::FLOAT;
    r.f.fraction = tmp;
    r.f.exponent = exponent;
    return r;
}

long double parse_dec_number_float_sub(string::iterator &li,const string::iterator lie) {
    long double r = 0;

    if (li != lie) {
        unsigned long long tmp = 0,tmpdiv = 1;

        /* NTS: Process digits in batches to allow precision in integer math and reduce cumulative floating point error */
        while (li != lie && isdigit(*li)) {
            if (tmpdiv >= 100000000000ull) {
                r = (r * tmpdiv) + tmp;
                tmpdiv = 1;
                tmp = 0;
            }

            tmp = (tmp * 10ull) + ((unsigned long long)(*(li++) - '0'));
            tmpdiv *= 10ull;
        }

        if (tmpdiv > 1ull) {
            r = (r * tmpdiv) + tmp;
        }
    }

    if (li != lie && *li == '.') {
        unsigned long long tmp = 0,tmpdiv = 1;
        long double d = 1.0;

        /* skip '.' */
        li++;

        /* NTS: Process digits in batches to allow precision in integer math and reduce cumulative floating point error */
        while (li != lie && isdigit(*li)) {
            if (tmpdiv >= 100000000000ull) {
                d *= (signed long long)tmpdiv; /* only because on x86 there is only a signed divide by int */
                r += tmp / d;
                tmpdiv = 1;
                tmp = 0;
            }

            tmp = (tmp * 10ull) + ((unsigned long long)(*(li++) - '0'));
            tmpdiv *= 10ull;
        }

        if (tmpdiv > 1ull) {
            d *= (signed long long)tmpdiv; /* only because on x86 there is only a signed divide by int */
            r += tmp / d;
        }
    }

    if (strit_next_match_inc(li,lie,'e'))
        parse_float_exponent(r,li,lie);
    else if (strit_next_match_inc(li,lie,'E'))
        parse_float_exponent(r,li,lie);

    return r;
}

void print_token(FILE *fp,const token &t);

bool parse_number_looks_like_float(string::iterator /*does not modify caller copy*/li,const string::iterator lie) {
    if (strit_next_match_inc(li,lie,'0','x')) {
        while (li != lie && isxdigit(*li)) li++;
        if (li != lie) return (*li == '.' || *li == 'p' || *li == 'p');
    }
    else {
        while (li != lie && isdigit(*li)) li++;
        if (li != lie) return (*li == '.' || *li == 'e' || *li == 'E');
    }

    return false;
}

/* will return int or float */
token parse_number(string::iterator &li,const string::iterator lie) {
    token r;

    /* at this point isdigit(*li) */
    if (li == lie) throw invalid_argument("expected number, got end of line");
    if (!isdigit(*li)) throw invalid_argument(string("expected number, got ") + *li);

    if (parse_number_looks_like_float(li,lie)) {
        if (strit_next_match_inc(li,lie,'0','x'))
            r = parse_hex_number_float_sub(li,lie);
        else
            r = parse_dec_number_float_sub(li,lie);

        parse_float_suffixes(r,li,lie);
    }
    else {
        if (strit_next_match_inc(li,lie,'0')) {
            if (strit_next_match_inc(li,lie,'x')) /* hexadecimal integer */
                r = parse_hex_number(li,lie);
            else /* octal */
                r = parse_oct_number(li,lie);
        }
        else { /* integer */
            r = parse_dec_number(li,lie);
        }

        parse_int_suffixes(r,li,lie);
    }

    return r;
}

stringref_t parse_string(string::iterator &li,const string::iterator lie) {
    /* at this point *li == '\"' */
    string r;
    char c;

    if (li == lie) throw invalid_argument("expected string, got end of line");
    if (*li != '\"') throw invalid_argument("expected string, got " + *li);
    li++;

    do {
        if (li == lie) throw invalid_argument("string cut short, expected close quote");
        if (*li == '\"') { li++; break; }
        c = parse_string_char(li,lie);
        r += c;
    } while (1);

    return string_store.add(r);
}

string parse_identifier(string::iterator &li,const string::iterator lie) {
    string r;

    if (li == lie) throw invalid_argument("expected identifier, got end of line");
    if (!isidentifier_fc(*li)) throw invalid_argument("expected identifier");

    do {
        r += *(li++);
        if (li == lie) break;
        if (!isidentifier_mc(*li)) break;
    } while (1);

    return r;
}

/* standard C++ to_string() always prints 6 digits in GCC.
 * we want more digits! */
string a_better_float_to_string(const long double v) {
    char tmp[40];

    snprintf(tmp,sizeof(tmp),"%.40Lf",v); /* expect overprint, but snprintf to stop at 39 chars + NUL */
    return tmp;
}

string to_string(const token &t) {
    switch (t.tval) {
        case token::MACRO:
        case token::PREPROCDIR:
        case token::IDENTIFIER:
        case token::KEYWORD:
            return t.sval + " ";
        case token::INTEGER:
            return to_string(t.i.s) + " ";
        case token::FLOAT:
            return a_better_float_to_string(t.f.get_double()) + " ";
        case token::STRING:
            return string("\"") + string_store.get_char(t.s.strref) + string("\"") + " ";
        case token::MINUS:
            return "- ";
        case token::PLUS:
            return "+ ";
        case token::DECREMENT:
            return "-- ";
        case token::INCREMENT:
            return "++ ";
        case token::COMMA:
            return ",";
        case token::PERIOD:
            return ". ";
        case token::DOTDOTDOT:
            return "...";
        case token::PTRARROW:
            return "-> ";
        case token::COMPLEMENT:
            return "~";
        case token::NOT:
            return "!";
        case token::AMPERSAND:
            return "&";
        case token::STAR:
            return "*";
        case token::OPEN_PARENS:
            return "(";
        case token::CLOSE_PARENS:
            return ")";
        case token::OPEN_SBRACKET:
            return "[";
        case token::CLOSE_SBRACKET:
            return "]";
        case token::OPEN_CBRACKET:
            return "{";
        case token::CLOSE_CBRACKET:
            return "}";
        case token::SIZEOF:
            return "sizeof";
        default:
            break;
    };

    return "? ";
}

void print_token(FILE *fp,const token &t) {
    if (fp == NULL)
        fp = stderr;

    const string s = to_string(t);
    fputs(s.c_str(),fp);
}

void parse_tokens(token_string &tokens,const string::iterator lib,const string::iterator lie,const int32_t lineno,const string &source) {
    auto li = lib;

    (void)lineno;
    (void)source;

    /* initial whitespace skip */
    parse_skip_whitespace(li,lie);
    if (li == lie) return;

    /* li != lie */
    if (*li == '#') {
        /* preprocessor handling */
        li++;
        parse_skip_whitespace(li,lie);
        /* expect identifier, or else */
        string ident = parse_identifier(li,lie); /* will throw exception otherwise */
        tokens.push_back(move(token(token::PREPROCDIR,ident)));
        parse_skip_whitespace(li,lie);
    }

    /* general parsing. expects code to skip whitespace after doing it's part */
    while (li != lie) {
        if (*li == '\"')
            tokens.push_back(move(parse_string(li,lie)));
        else if (*li == '\'')
            tokens.push_back(move(parse_sq_char(li,lie)));
        else if (strit_next_match_inc(li,lie,'.','.','.'))
            tokens.push_back(token::DOTDOTDOT);
        else if (strit_next_match_inc(li,lie,'.'))
            tokens.push_back(token::PERIOD);
        else if (strit_next_match_inc(li,lie,','))
            tokens.push_back(token::COMMA);
        else if (strit_next_match_inc(li,lie,'-','>'))
            tokens.push_back(token::PTRARROW);
        else if (strit_next_match_inc(li,lie,'-','-'))
            tokens.push_back(token::DECREMENT);
        else if (strit_next_match_inc(li,lie,'-'))
            tokens.push_back(token::MINUS);
        else if (strit_next_match_inc(li,lie,'+','+'))
            tokens.push_back(token::INCREMENT);
        else if (strit_next_match_inc(li,lie,'+'))
            tokens.push_back(token::PLUS);
        else if (strit_next_match_inc(li,lie,'~'))
            tokens.push_back(token::COMPLEMENT);
        else if (strit_next_match_inc(li,lie,'!'))
            tokens.push_back(token::NOT);
        else if (strit_next_match_inc(li,lie,'&'))
            tokens.push_back(token::AMPERSAND);
        else if (strit_next_match_inc(li,lie,'*'))
            tokens.push_back(token::STAR);
        else if (strit_next_match_inc(li,lie,'('))
            tokens.push_back(token::OPEN_PARENS);
        else if (strit_next_match_inc(li,lie,')'))
            tokens.push_back(token::CLOSE_PARENS);
        else if (strit_next_match_inc(li,lie,'['))
            tokens.push_back(token::OPEN_SBRACKET);
        else if (strit_next_match_inc(li,lie,']'))
            tokens.push_back(token::CLOSE_SBRACKET);
        else if (strit_next_match_inc(li,lie,'{'))
            tokens.push_back(token::OPEN_CBRACKET);
        else if (strit_next_match_inc(li,lie,'}'))
            tokens.push_back(token::CLOSE_CBRACKET);
        else if (isdigit(*li))
            tokens.push_back(move(parse_number(li,lie)));
        else if (isidentifier_fc(*li)) {
            string ident = parse_identifier(li,lie); /* will throw exception otherwise */
            enum token::token_t tk;

            if ((tk=is_keyword(ident)) != token::NONE)
                tokens.push_back(tk);
            else if (is_macro(ident))
                tokens.push_back(move(token(token::MACRO,ident))); // TODO: In place macro expansion HERE, with recursion, and parameters
            else
                tokens.push_back(move(token(token::IDENTIFIER,ident)));
        }
        else {
            throw invalid_argument(string("token parser unexpected char ") + *li);
        }

        parse_skip_whitespace(li,lie);
    }
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
    token_string tokens;

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
            else {
                tokens.clear();
                parse_tokens(tokens,line.begin(),line.end(),lineno,source);

                if (pp_only) {
                    if (emit_line) {
                        out_dst.puts(string("#line ") + to_string(lineno) + " " + source + "\n");
                        emit_line = false;
                    }

                    for (const auto &t : tokens)
                        out_dst.puts(to_string(t));

                    out_dst.putc('\n');
                }
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

