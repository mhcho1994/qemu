/*
 * Copyright (C) 2018, Emilio G. Cota <cota@braap.org>
 *
 * License: GNU GPL, version 2 or later.
 *   See the COPYING file in the top-level directory.
 */
int isdigit(int c);
#include <ctype.h>
#include <inttypes.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <glib.h>

#include <qemu-plugin.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "virtual.h"

#define MAX_ENTRIES 1024
#define MAX_LINE_LEN 128
#define MAX_RULES 256

typedef enum {
    TARGET_REGISTER,
    TARGET_MEMORY
} TargetType;

typedef struct {
    unsigned long update_point;
    TargetType type;
    union {
        int reg_num;
        unsigned long addr;
    } target;
    unsigned long value;
} UpdateEntry;

rule_t rules[MAX_RULES];
size_t rules_count = 0;

// Global storage
UpdateEntry update_entries[MAX_ENTRIES];
size_t update_entry_count = 0;



// Helper to parse a single line
static int parse_update_line(const char *line, UpdateEntry *entry);
static int parse_update_line(const char *line, UpdateEntry *entry) {
    char target_str[32];

    if (sscanf(line, "%lx %31s %lx",
               &entry->update_point,
               target_str,
               &entry->value) != 3) {
        return 0;
    }

    if (target_str[0] == 'r' && isdigit(target_str[1])) {
        entry->type = TARGET_REGISTER;
        entry->target.reg_num = atoi(target_str + 1);
    } else if (strncmp(target_str, "0x", 2) == 0) {
        entry->type = TARGET_MEMORY;
        entry->target.addr = strtoul(target_str, NULL, 16);
    } else {
        return 0;
    }

    return 1;
}

// Function to load all updates from a file into the global array
int load_update_entries(const char *filename);
int load_update_entries(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        return 0;
    }

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), f)) {
        // Strip newline
        line[strcspn(line, "\r\n")] = 0;
        if (line[0] == '\0' || line[0] == '#') continue;

        if (update_entry_count >= MAX_ENTRIES) {
            fprintf(stderr, "Too many entries (limit: %d)\n", MAX_ENTRIES);
            fclose(f);
            return 0;
        }

        if (parse_update_line(line, &update_entries[update_entry_count])) {
            update_entry_count++;
        } else {
            fprintf(stderr, "Error parsing line: %s\n", line);
        }
    }

    fclose(f);
    return 1;
}

// Optional debug printer
void print_all_updates(void);
void print_all_updates(void) {
    for (size_t i = 0; i < update_entry_count; ++i) {
        UpdateEntry *e = &update_entries[i];
        printf("Update Point: 0x%lx, ", e->update_point);
        if (e->type == TARGET_REGISTER) {
            printf("Register: r%d, ", e->target.reg_num);
        } else {
            printf("Memory Addr: 0x%lx, ", e->target.addr);
        }
        printf("Value: 0x%lx\n", e->value);
    }
}

#define MAX_TUPLES 1000

typedef struct {
    uintptr_t anchor;
    uintptr_t target;
} AddressTuple;

static AddressTuple address_tuples[MAX_TUPLES];
static size_t num_tuples = 0;

/* Prototypes */
AddressTuple * is_target_address(uintptr_t addr);
void print_tuples(AddressTuple *tuples, size_t count);
size_t read_tuples_from_file(const char *filename, AddressTuple *tuples, size_t max_tuples);

AddressTuple * is_target_address(uintptr_t addr) {
    for (size_t i = 0; i < num_tuples; ++i) {
        if (address_tuples[i].target == addr) {
            return &address_tuples[i];
        }
    }
    return NULL;
}

size_t read_tuples_from_file(const char *filename, AddressTuple *tuples, size_t max_tuples) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        exit(1);
    }

    size_t count = 0;
    while (count < max_tuples &&
           fscanf(f, "%lx %lx", &tuples[count].anchor, &tuples[count].target) == 2) {
        count++;
    }

    fclose(f);
    return count;
}

void print_tuples(AddressTuple *tuples, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        printf("Tuple %zu: %p -> %p\n", i,
               (void *)tuples[i].anchor,
               (void *)tuples[i].target);
    }
}
/* You can use these functions like this 

int main() {
    AddressTuple tuples[MAX_TUPLES];

    size_t count = read_tuples_from_file("addrs.txt", tuples, MAX_TUPLES);
    print_tuples(tuples, count);

    return 0;
}

*/


#define MAX_BUFFER_SIZE 256

typedef struct {
    uint32_t address;
    char mode; // 'r' or 'w'
    uint32_t length;
    uint8_t buffer[MAX_BUFFER_SIZE];
} MemAccess;


int parse_update_mem_arg(const char *input, MemAccess *out);
int parse_update_mem_arg(const char *input, MemAccess *out) {
    if (!input || !out) return -1;

    // Temporary copy of input string for tokenizing
	char *temp = malloc(strlen(input) + 1);
	if (!temp) return -1;
	strcpy(temp, input);


    char *token = strtok(temp, ":");
    if (!token) { free(temp); return -1;}
    out->address = strtoul(token, NULL, 0); // parse address

    token = strtok(NULL, ":");
    if (!token || (token[0] != 'r' && token[0] != 'w')) return -1;
    out->mode = token[0]; // parse mode

    token = strtok(NULL, ":");
    if (!token) { free(temp); return -1;}
    out->length = strtoul(token, NULL, 0); // parse length
    if (out->length > MAX_BUFFER_SIZE) return -1;

    token = strtok(NULL, ":");
    if (!token) { free(temp); return -1;}

    // Now parse comma-separated bytes
    uint32_t i = 0;
    char *byte_str = strtok(token, ",");
    while (byte_str && i < out->length) {
        out->buffer[i++] = (uint8_t)strtoul(byte_str, NULL, 0);
        byte_str = strtok(NULL, ",");
    }

    if (i != out->length) { printf("Invalid Argument \n"); free(temp); return -1;}

	free(temp);
    return 0; // success
}

unsigned long long* parse_addresses(const char *input, size_t *count);
unsigned long long* parse_addresses(const char *input, size_t *count) {
    // Make a copy of input so we don't modify the original
    char *input_copy = strdup(input);
    if (!input_copy) return NULL;

    size_t capacity = 8;
    *count = 0;
    unsigned long long *addresses = malloc(capacity * sizeof(unsigned long long));
    if (!addresses) {
        free(input_copy);
        return NULL;
    }

    char *token = strtok(input_copy, ",");
    while (token) {
        // Remove leading/trailing whitespace
        while (*token == ' ' || *token == '\t') token++;
        char *endptr;
        unsigned long long addr = strtoull(token, &endptr, 0);
        if (token == endptr) {
            // Invalid conversion
            free(addresses);
            free(input_copy);
            return NULL;
        }

        if (*count >= capacity) {
            capacity *= 2;
            addresses = realloc(addresses, capacity * sizeof(unsigned long long));
            if (!addresses) {
                free(input_copy);
                return NULL;
            }
        }

        addresses[(*count)++] = addr;
        token = strtok(NULL, ",");
    }

    free(input_copy);
    return addresses;
}



QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;
int counter;

#define HOOK_POINT	(0x106d6)
#define ANCHOR		(0x106cc)


static void raiseirq(unsigned int cpu_index, void *udata);
static void updatepc(unsigned int cpu_index, void *udata);
static void updatereg(unsigned int cpu_index, void *udata);
static void updatemem(unsigned int cpu_index, void *udata);
static void randstate(unsigned int cpu_index, void *udata);

cb_entry_t cb_registry[] = {
    { "updatepc", updatepc },
	{ "updatereg", updatereg},
	{ "updatemem", updatemem},
	{ "randstate", randstate},
    { "raiseirq", raiseirq },
};

const size_t cb_registry_len = sizeof(cb_registry) / sizeof(cb_registry[0]);

static void raiseirq(unsigned int cpu_index, void *udata){
	qemu_plugin_raise_irq(15);
}

static void updatemem(unsigned int cpu_index, void *udata) {
	const char *input = (const char *) udata;
	MemAccess mem;

    if (parse_update_mem_arg(input, &mem) == 0) {
		if (mem.mode == 'r') {
			qemu_plugin_read_memory(mem.address, mem.buffer, mem.length);
		} else {
			qemu_plugin_write_memory(mem.address, mem.buffer, mem.length);
		}
	}
}

#include <stdio.h>
#include <stdlib.h>

unsigned char get_random_byte(void);
unsigned char get_random_byte(void) {
	//This will make things linux specific, but lot of hardcoded things.
    FILE *fp = fopen("/dev/urandom", "rb");
    if (!fp) {
        perror("fopen /dev/urandom");
        exit(EXIT_FAILURE);
    }

    unsigned char byte;
    size_t result = fread(&byte, 1, 1, fp);
    fclose(fp);

    if (result != 1) {
        fprintf(stderr, "Failed to read from /dev/urandom\n");
        exit(EXIT_FAILURE);
    }

    return byte;
}
uint32_t get_random_word(void);
uint32_t get_random_word(void) {
    //This will make things linux specific, but lot of hardcoded things.
    FILE *fp = fopen("/dev/urandom", "rb");
    if (!fp) {
        perror("fopen /dev/urandom");
        exit(EXIT_FAILURE);
    }

    uint32_t word;
    size_t result = fread(&word, sizeof(uint32_t), 1, fp);
    fclose(fp);

    if (result != 1) {
        fprintf(stderr, "Failed to read from /dev/urandom\n");
        exit(EXIT_FAILURE);
    }

    return word;
}


static void randstate(unsigned int cpu_index, void *udata) {
	const char *input = (const char *) udata;

	size_t count = 0;
	unsigned long long *addrs = parse_addresses(input, &count);
	if (addrs) {
        for (size_t i = 0; i < count; i++) {
			if (addrs[i] < 100) {
				uint32_t val = get_random_word();
				//PC not supported 
				if (addrs[i] != 15) {
					qemu_plugin_set_register((uint8_t *)&val,addrs[i] );
				}
			} else {
				uint8_t val = get_random_byte();
				qemu_plugin_write_memory(addrs[i], &val, 1);
			}
        }
        free(addrs);
    } else {
        printf("Failed to parse addresses.\n");
    }

}
static void updatepc(unsigned int cpu_index, void *udata)
{
	// BUGON: This wont' work anymore
	uint32_t val = 0xdeadbeef;
	val = (0x106cc | 1);
	qemu_plugin_set_register((uint8_t *)&val, 15);
}

static void updatereg(unsigned int cpu_index, void *udata)
{
    uint32_t val = 0xdeadbeef;
    qemu_plugin_set_register((uint8_t *)&val, 1);
}



// Finds all update entries targeting the given memory address.
// `matches` is an output array to be filled with pointers to matching entries.
// `max_matches` limits how many results to return.
// Returns the number of matches found.
size_t find_updates_for_address(unsigned long addr,
                                UpdateEntry **matches,
                                size_t max_matches);
size_t find_updates_for_address(unsigned long addr,
                                UpdateEntry **matches,
                                size_t max_matches)
{
    size_t count = 0;
    for (size_t i = 0; i < update_entry_count && count < max_matches; ++i) {
        if (update_entries[i].update_point == addr)
        {
            matches[count++] = &update_entries[i];
        }
    }
    return count;
}

int inline_ins = 0;
#define MAX_MATCHES 10
static void vcpu_tb_trans(qemu_plugin_id_t id, struct qemu_plugin_tb *tb)
{
    size_t n = qemu_plugin_tb_n_insns(tb);
    size_t i;
	UpdateEntry *matches[MAX_MATCHES];

    for (i = 0; i < n; i++) {
        struct qemu_plugin_insn *insn = qemu_plugin_tb_get_insn(tb, i);


		rule_t  *rule; 
		if (find_rule_by_address(qemu_plugin_insn_vaddr(insn), &rule)) {
				qemu_plugin_register_vcpu_insn_exec_cb(
                    insn, rule->func, QEMU_PLUGIN_CB_RW_REGS, rule->args);
		}


		AddressTuple * tuple = is_target_address(qemu_plugin_insn_vaddr(insn)); 
		if (tuple) {
				qemu_plugin_u64 entry;
				// In TCG frontend it is already set, if you want to modify it you will have to 
				// change CPSR. 
				entry.offset = (tuple->anchor & ~(0x1));
				qemu_plugin_register_vcpu_insn_exec_inline_per_vcpu(insn, QEMU_PLUGIN_INLINE_UPDATE_REG, entry, 15);
		}

		//void * handle= qemu_plugin_register_vcpu_insn_exec_inline_per_vcpu(insn,  QEMU_PLUGIN_CB_GEN_LABEL, NULL, 0);
		size_t count = find_updates_for_address(qemu_plugin_insn_vaddr(insn), matches, MAX_MATCHES);
		if (count > 0) {
		for (size_t match_idx = 0; match_idx < count; ++match_idx) {
			UpdateEntry *e = matches[match_idx];

			printf("  Update Point: 0x%lx, ", e->update_point);
	        if (e->type == TARGET_REGISTER) {
    	        printf("Target: r%d, ", e->target.reg_num);
				qemu_plugin_u64 entry;
                // In TCG frontend it is already set, if you want to modify it you will have to
                // change CPSR.
                entry.offset = (e->value);
                qemu_plugin_register_vcpu_insn_exec_inline_per_vcpu(insn, QEMU_PLUGIN_INLINE_UPDATE_REG, entry, e->target.reg_num);
	        } else if (e->type == TARGET_MEMORY) {
				printf("Target: r%d, ", e->target.reg_num);
                qemu_plugin_u64 entry;
                // In TCG frontend it is already set, if you want to modify it you will have to
                // change CPSR.
                entry.offset = (e->value);
                qemu_plugin_register_vcpu_insn_exec_inline_per_vcpu(insn, QEMU_PLUGIN_INLINE_UPDATE_MEM, entry, e->target.addr);
    	        printf("Target: 0x%lx, ", e->target.addr);
       		}

        printf("Value: 0x%lx\n", e->value);

    	}
		}
    }
}

static void plugin_exit(qemu_plugin_id_t id, void *p)
{
}

char * get_arg(const char * key, int argc, char **argv);
char * get_arg(const char * key, int argc, char **argv) {
	int len = strlen(key);
	for (int i =0; i < argc; i ++) {
        if (strncmp(argv[i], key, len) == 0) {
            return (argv[i] + len + 1);
        }
    }

	return NULL;
}

static cb_func_t lookup_callback(const char *name) {
    for (size_t i = 0; i < cb_registry_len; i++) {
        if (strcmp(cb_registry[i].name, name) == 0)
            return cb_registry[i].func;
    }
    return NULL;
}


bool find_rule_by_address(unsigned long long addr, rule_t **out_rule) {
    for (size_t i = 0; i < rules_count; i++) {
        if (rules[i].address == addr) {
            if (out_rule) {
                *out_rule = &rules[i];
            }
            return true;
        }
    }
    return false;
}

void parse_rules_file(const char *filename);
void parse_rules_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Failed to open rules file");
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '\n' || line[0] == '#') continue;
        line[strcspn(line, "\r\n")] = 0;

        char addr_str[32];
        char cb_name[64];
        char args[384] = {0};

        int n = sscanf(line, "%31s %63s %383[^\n]", addr_str, cb_name, args);
        if (n < 2) {
            fprintf(stderr, "Invalid line in rules file: '%s'\n", line);
            continue;
        }

        cb_func_t cb = lookup_callback(cb_name);
        if (!cb) {
            fprintf(stderr, "Error: Callback '%s' not found in registry (line: '%s')\n", cb_name, line);
            continue;
        }

        if (rules_count >= MAX_RULES) {
            fprintf(stderr, "Max rules limit reached (%d), skipping rest\n", MAX_RULES);
            break;
        }

        rules[rules_count].address = strtoull(addr_str, NULL, 0);
        rules[rules_count].func = cb;

        if (n == 3) {
            strncpy(rules[rules_count].args, args, sizeof(rules[rules_count].args) - 1);
            rules[rules_count].args[sizeof(rules[rules_count].args) - 1] = '\0';
        } else {
            rules[rules_count].args[0] = '\0';
        }

        rules_count++;
    }

    fclose(f);
}
#if 0
static void print_rules(void) {
    printf("Parsed %zu rules:\n", rules_count);
    for (size_t i = 0; i < rules_count; i++) {
        printf("Rule %zu: Addr=0x%llx, Func=%p, Args='%s'\n",
               i, rules[i].address, (void *)rules[i].func, rules[i].args);
    }
}
#endif 
QEMU_PLUGIN_EXPORT int qemu_plugin_install(qemu_plugin_id_t id,
                                           const qemu_info_t *info,
                                           int argc, char **argv)
{
	if (argc < 1) {
        fprintf(stderr, "Usage: plugin.so <address_file.txt>\n");
        return -1;
    }

	const char *filename= get_arg("detour", argc, argv);
    num_tuples = read_tuples_from_file(filename, address_tuples, MAX_TUPLES);

	filename= get_arg("modifier", argc, argv);
	load_update_entries(filename);

	filename = get_arg("virtual", argc, argv);
	parse_rules_file(filename);



    qemu_plugin_register_vcpu_tb_trans_cb(id, vcpu_tb_trans);
    qemu_plugin_register_atexit_cb(id, plugin_exit, NULL);
    return 0;
}
