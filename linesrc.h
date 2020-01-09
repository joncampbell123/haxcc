
#ifndef LINESRC_HPP
#define LINESRC_HPP

#include <stdint.h>

#include <string>

class haxpp_linesource {
private:
    linecount_t         lineno = 0;
    std::string         sourcepath;
    FILE*               fp = nullptr;
    char*               line = nullptr;
    size_t              line_alloc = 0;
    bool                fp_owner = false;
public:
    static constexpr size_t    line_alloc_minimum = 32u;
    static constexpr size_t    line_alloc_default = 1200u;
    static constexpr size_t    line_alloc_maximum = 65536u;
public:
    inline const std::string& getsourcename() const {
        return sourcepath;
    }
    inline size_t linesize() const { /* total buffer size including room for NUL terminator */
        return line_alloc;
    }
    inline linecount_t currentline() const {
        return lineno;
    }
public:
    haxpp_linesource();
    ~haxpp_linesource();
public:
    void setsource();
    void setsource(FILE *_fp);
    void setsource(const std::string &path);
    bool lineresize(const size_t newsz);
    void close();
    bool is_open() const;
    bool eof() const;
    bool error() const;
    bool open();
    char *readline();
};

#endif /*LINESRC_HPP*/

