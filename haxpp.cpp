
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>

using namespace std;

#include <string>

typedef uint32_t linecount_t;

bool is_file(const char * const path) {
    struct stat st;

    if (stat(path,&st))
        return false;

    return S_ISREG(st.st_mode);
}

bool is_file(const string &path) {
    return is_file(path.c_str());
}

class haxpp_linesource {
private:
    linecount_t         lineno = 0;
    string              sourcepath;
    FILE*               fp = NULL;
    char                line[2048]; /* TODO: Perhaps in the future this could be dynamically chosen */
public:
    inline size_t       linesize() const; /* total buffer size including room for NUL terminator */
    inline const string&getsourcename() const;

                        haxpp_linesource();
                        haxpp_linesource(const string &path);
                        ~haxpp_linesource();
                        void close();
                        bool is_open() const;
                        bool eof() const;
                        bool error() const;
                        bool open();
                        char *readline();
};

inline const string& haxpp_linesource::getsourcename() const {
    return sourcepath;
}

inline size_t haxpp_linesource::linesize() const {
    return sizeof(line);
}

haxpp_linesource::haxpp_linesource() {
}

haxpp_linesource::haxpp_linesource(const string &path) {
    sourcepath = path;
}

haxpp_linesource::~haxpp_linesource() {
    close();
}

void haxpp_linesource::close() {
    if (fp != nullptr) {
        fclose(fp);
        fp = nullptr;
    }
}

bool haxpp_linesource::is_open() const {
    return (fp != NULL);
}

bool haxpp_linesource::eof() const {
    if (is_open())
        return (feof(fp) != 0);

    return true;
}

bool haxpp_linesource::error() const {
    if (is_open())
        return (ferror(fp) != 0);

    return true;
}

bool haxpp_linesource::open() {
    if (!is_open()) {
        if (sourcepath.empty()) {
            errno = EINVAL;
            return false;
        }

        if (!is_file(sourcepath)) {
            errno = EINVAL;
            return false;
        }

        fp = fopen(sourcepath.c_str(),"r");
        if (fp == NULL) {
            /* fopen set errno */
            return false;
        }

        lineno = 1;
    }

    return true;
}

char *haxpp_linesource::readline() {
    errno = 0;
    if (is_open()) {
        if (!ferror(fp) && !feof(fp)) {
            /* write a NUL to the last two bytes of the buffer.
             * if those bytes are no longer NUL it means fgets() read a line that's too long.
             * This is faster than using strlen() on the returned string. */
            const size_t s = linesize(); /* must be >= 2 */
            line[s-2] = line[s-1] = 0;

            /* fgets() will read up to s-1 bytes, and store a NUL where it stops (which is why we test line[s-2] instead).
             * the returned string will contain the entire line including the '\n' newline. */
            if (fgets(line,s,fp) == NULL)
                return NULL;

            /* check for line too long */
            if (line[s-2] != 0) {
                errno = E2BIG;
                return NULL;
            }

            return line;
        }
    }

    return NULL;
}

int main(int argc,char **argv) {
    for (int i=1;i < argc;i++) {
        haxpp_linesource ls(argv[i]);

        if (!ls.open()) {
            fprintf(stderr,"Unable to open %s, error %s\n",ls.getsourcename().c_str(),strerror(errno));
            return 1;
        }

        while (!ls.eof()) {
            char *line = ls.readline();
            if (line == NULL) {
                if (!ls.error() && ls.eof()) {
                    break;
                }
                else {
                    fprintf(stderr,"Problem reading. error=%u eof=%u errno=%s\n",ls.error(),ls.eof(),strerror(errno));
                    return 1;
                }
            }

            printf("%s",line); /* often provides it's own newline */
        }

        if (ls.error()) {
            fprintf(stderr,"An error occurred while parsing %s\n",ls.getsourcename().c_str());
            return 1;
        }
    }

    return 0;
}

