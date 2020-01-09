
class haxpp_linesink {
private:
    linecount_t         lineno = 0;
    std::string         sinkpath;
    FILE*               fp = nullptr;
    bool                fp_owner = false;
public:
    inline const std::string& getsinkname() const {
        return sinkpath;
    }
public:
    haxpp_linesink();
    ~haxpp_linesink();
public:
    void setsink();
    void setsink(FILE *_fp);
    void setsink(const std::string &path);
    void close();
    bool is_open() const;
    bool eof() const;
    bool error() const;
    bool open();
    bool write(const char * const s);
};

