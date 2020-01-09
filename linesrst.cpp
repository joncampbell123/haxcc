
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>

#include "util.h"
#include "linesink.h"
#include "linesrst.h"

#include <stdexcept>
#include <vector>

using namespace std;

haxpp_linesourcestack::haxpp_linesourcestack() {
}

haxpp_linesourcestack::~haxpp_linesourcestack() {
    freestack();
}

void haxpp_linesourcestack::allocstack() {
    if (in_ls == NULL) {
        in_ls = new haxpp_linesource[max_source_stack];
        in_ls_sp = -1;
    }
}

void haxpp_linesourcestack::freestack() {
    if (in_ls != NULL) {
        clear();
        delete[] in_ls;
        in_ls = NULL;
    }
    in_ls_sp = -1;
}

void haxpp_linesourcestack::push() {
    allocstack();

    /* NTS: when in_ls_sp == -1, in_ls_sp+1 == 0 */
    if (size_t(in_ls_sp+1) < max_source_stack)
        in_ls_sp++;
    else
        throw overflow_error("linesourcestack overflow");
}

void haxpp_linesourcestack::pop() {
    /* in_ls_sp >= 0 should mean in_ls != NULL or else this code would not permit in_ls_sp >= 0 */
    if (in_ls_sp >= ssize_t(0))
        in_ls[in_ls_sp--].close();
    else
        throw underflow_error("linesourcestack underflow");
}

haxpp_linesource& haxpp_linesourcestack::top() {
    /* in_ls_sp >= 0 should mean in_ls != NULL or else this code would not permit in_ls_sp >= 0 */
    if (in_ls_sp >= ssize_t(0))
        return in_ls[in_ls_sp];

    throw underflow_error("linesourcestack attempt to read top() when empty");
}

const haxpp_linesource& haxpp_linesourcestack::top() const {
    /* in_ls_sp >= 0 should mean in_ls != NULL or else this code would not permit in_ls_sp >= 0 */
    if (in_ls_sp >= ssize_t(0))
        return in_ls[in_ls_sp];

    throw underflow_error("linesourcestack attempt to read top() when empty");
}

void haxpp_linesourcestack::clear() {
    while (!empty()) pop();
}

bool haxpp_linesourcestack::empty() const {
    /* in_ls_sp >= 0 should mean in_ls != NULL or else this code would not permit in_ls_sp >= 0 */
    return (in_ls_sp == ssize_t(-1));
}

