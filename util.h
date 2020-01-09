
#include <string>

typedef uint32_t linecount_t;

bool is_file(const char * const path);
bool is_file(const std::string &path);

bool is_out_file(const char * const path);
bool is_out_file(const std::string &path);

bool iswordchar(char c);
std::string cstrgetword(char* &s);
void cstrskipwhitespace(char* &s);
std::string cstrgetstringenclosed(char* &s,char delim,char delimend);

