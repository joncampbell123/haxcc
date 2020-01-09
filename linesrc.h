
#include <stdint.h>

#include <string>

typedef uint32_t linecount_t;

class haxpp_linesource {
private:
    linecount_t         lineno = 0;
    std::string         sourcepath;
    FILE*               fp = NULL;
    char*               line = NULL;
    size_t              line_alloc = 0;
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
public:
                        haxpp_linesource();
                        haxpp_linesource(const std::string &path);
                        ~haxpp_linesource();
                        bool lineresize(const size_t newsz);
                        void close();
                        bool is_open() const;
                        bool eof() const;
                        bool error() const;
                        bool open();
                        char *readline();
};

