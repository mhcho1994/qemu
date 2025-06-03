/*
 * Copyright (C) 2018, Emilio G. Cota <cota@braap.org>
 *
 * License: GNU GPL, version 2 or later.
 *   See the COPYING file in the top-level directory.
 */
#include <inttypes.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <glib.h>

#include <qemu-plugin.h>

QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;
int counter;
static void vcpu_insn_exec_before(unsigned int cpu_index, void *udata)
{
	uint32_t val = 0xdeadbeef;
#if 0 
//	qemu_plugin_set_register((uint8_t *)&val, 0);
	g_autoptr(GArray) reg_list = qemu_plugin_get_registers();
    g_autoptr(GByteArray) reg_value = g_byte_array_new();
	uint64_t pc_val = 0;
    uint64_t lr_val = 0;

	if (reg_list) {
        for (int i = 0; i < reg_list->len; i++) {
            qemu_plugin_reg_descriptor *rd = &g_array_index(
                reg_list, qemu_plugin_reg_descriptor, i);

			qemu_plugin_read_register(rd->handle, reg_value);
			memcpy(&val, reg_value->data, 4);
			// Check if the current register is PC or LR by its name
        if (strcmp(rd->name, "pc") == 0 || strcmp(rd->name, "lr") == 0) {
            int count = qemu_plugin_read_register(rd->handle, reg_value);
            g_assert(count > 0 && "Failed to read register");

            uint64_t value = 0;
            // Safely copy the bytes to a uint64_t variable.
            // This handles different register sizes (e.g., 32-bit or 64-bit ARM)
            // and guest endianness correctly.
            memcpy(&value, reg_value->data, count);

            if (strcmp(rd->name, "pc") == 0) {
                pc_val = value;
            } else { // It must be LR
                lr_val = value;
            }
       
            g_assert(count > 0);
		}
        }
    }
#endif 
	val = (0x106cc | 1);
	qemu_plugin_set_register((uint8_t *)&val, 15);

	if (++counter == 10000) {
			fprintf(stderr, "%lld\n", (long long)qemu_plugin_host_start_ns());
	}
#if 0
	lr_val = lr_val;
	    if (reg_list) {
        for (int i = 0; i < reg_list->len; i++) {
            qemu_plugin_reg_descriptor *rd = &g_array_index(
                reg_list, qemu_plugin_reg_descriptor, i);
            // Check if the current register is PC or LR by its name
        if (strcmp(rd->name, "pc") == 0 || strcmp(rd->name, "lr") == 0) {
            int count = qemu_plugin_read_register(rd->handle, reg_value);
            g_assert(count > 0 && "Failed to read register");

            uint64_t value = 0;
            // Safely copy the bytes to a uint64_t variable.
            // This handles different register sizes (e.g., 32-bit or 64-bit ARM)
            // and guest endianness correctly.
            memcpy(&value, reg_value->data, count);

            if (strcmp(rd->name, "pc") == 0) {
                pc_val = value;
            } else { // It must be LR
                lr_val = value;
            }

            g_assert(count > 0);
        }
        }
    }
#endif 


}

static void vcpu_tb_trans(qemu_plugin_id_t id, struct qemu_plugin_tb *tb)
{
    size_t n = qemu_plugin_tb_n_insns(tb);
    size_t i;

    for (i = 0; i < n; i++) {
        struct qemu_plugin_insn *insn = qemu_plugin_tb_get_insn(tb, i);
		if (qemu_plugin_insn_vaddr(insn) == 0x106d6) {
	        qemu_plugin_register_vcpu_insn_exec_cb(
    	            insn, vcpu_insn_exec_before, QEMU_PLUGIN_CB_RW_REGS | QEMU_PLUGIN_CB_RW_CFI, NULL);
		}
    }
}

static void plugin_exit(qemu_plugin_id_t id, void *p)
{
}


QEMU_PLUGIN_EXPORT int qemu_plugin_install(qemu_plugin_id_t id,
                                           const qemu_info_t *info,
                                           int argc, char **argv)
{
    qemu_plugin_register_vcpu_tb_trans_cb(id, vcpu_tb_trans);
    qemu_plugin_register_atexit_cb(id, plugin_exit, NULL);
    return 0;
}
