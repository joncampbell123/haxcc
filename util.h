
#include <string>

typedef uint32_t linecount_t;

bool is_file(const char * const path);
bool is_file(const std::string &path);

bool is_out_file(const char * const path);
bool is_out_file(const std::string &path);

bool iswordchar(char c);
bool iswordcharfirst(char c);
void cstrskipsquote(char* &s);
void cstrskipstring(char* &s);
std::string cstrgetword(char* &s);
void cstrskipwhitespace(char* &s);
bool cstrparsedotdotdot(char* &s);
std::string cstrgetstringenclosed(char* &s,char delim,char delimend);

