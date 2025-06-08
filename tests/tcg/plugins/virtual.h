#ifndef VIRTUAL_H
#define VIRTUAL_H

#include <qemu-plugin.h>


typedef void (*cb_func_t)(unsigned int cpu_index, void *userdata);

typedef struct {
    const char *name;
    cb_func_t func;
} cb_entry_t;

typedef struct {
    unsigned long long address;
    cb_func_t func;        // function pointer, NOT the name
    char args[384];
} rule_t;

bool find_rule_by_address(unsigned long long addr, rule_t **out_rule);

#endif // VIRTUAL_H
