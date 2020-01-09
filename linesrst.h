
#include <stdint.h>

#include "linesrc.h"

class haxpp_linesourcestack {
    public:
        haxpp_linesourcestack();
        ~haxpp_linesourcestack();
    public:
        void                        allocstack();
        void                        freestack();
        void                        push();
        void                        pop();
        haxpp_linesource&           top();
        const haxpp_linesource&     top() const;
        void                        clear();
        bool                        empty() const;
    private:
        static constexpr size_t     max_source_stack_default = 64;
        size_t                      max_source_stack = max_source_stack_default;
    private:
        haxpp_linesource*           in_ls = NULL;
        ssize_t                     in_ls_sp = -1;
};

