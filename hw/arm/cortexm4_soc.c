/*
 * CORTEXM4 SoC
 *
 * Copyright (c) 2014 Alistair Francis <alistair@alistair23.me>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qom/object.h"
#include "system/address-spaces.h"
#include "system/system.h"
#include "system/hostmem.h"
#include "hw/arm/cortexm4_soc.h"
#include "hw/qdev-clock.h"
#include "hw/misc/unimp.h"


static void cortexm4_soc_initfn(Object *obj)
{
    CORTEXM4State *s = CORTEXM4_SOC(obj);

    object_initialize_child(obj, "armv7m", &s->armv7m, TYPE_ARMV7M);

    s->sysclk = qdev_init_clock_in(DEVICE(s), "sysclk", NULL, NULL, 0);
    s->refclk = qdev_init_clock_in(DEVICE(s), "refclk", NULL, NULL, 0);
}

static void cortexm4_soc_realize(DeviceState *dev_soc, Error **errp)
{
    CORTEXM4State *s = CORTEXM4_SOC(dev_soc);
    MemoryRegion *system_memory = get_system_memory();
    DeviceState *armv7m;
    Error *err = NULL;
	
	//MemoryRegion *ram;


    /*
     * We use s->refclk internally and only define it with qdev_init_clock_in()
     * so it is correctly parented and not leaked on an init/deinit; it is not
     * intended as an externally exposed clock.
     */
    if (clock_has_source(s->refclk)) {
        error_setg(errp, "refclk clock must not be wired up by the board code");
        return;
    }

    if (!clock_has_source(s->sysclk)) {
        error_setg(errp, "sysclk clock must be wired up by the board code");
        return;
    }

    /*
     * TODO: ideally we should model the SoC RCC and its ability to
     * change the sysclk frequency and define different sysclk sources.
     */

    /* The refclk always runs at frequency HCLK / 8 */
    clock_set_mul_div(s->refclk, 8, 1);
    clock_set_source(s->refclk, s->sysclk);

    memory_region_init_rom(&s->flash, OBJECT(dev_soc), "genric_flash",
                           FLASH_SIZE, &err);
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }
    memory_region_add_subregion(system_memory, FLASH_BASE_ADDRESS, &s->flash);
    memory_region_add_subregion(system_memory, 0, &s->flash_alias);
#if 0

    if (s->ram_backend) {
        ram = s->ram_backend;
    } else {
		ram = &s->sram;
		memory_region_init_ram(&s->sram, NULL, "CORTEXM4.sram", SRAM_SIZE,
                           &err);
		if (err != NULL) {
       		error_propagate(errp, err);
    	    return;
	    }
    }



	memory_region_add_subregion(system_memory, 0x20000000, ram);
#endif 

	if (s->shram_backend) {
			memory_region_add_subregion(system_memory, 0x3F800000, &s->shram_backend->mr);
	}



    armv7m = DEVICE(&s->armv7m);
	//M4 NVIC Supports upto 240
    qdev_prop_set_uint32(armv7m, "num-irq", 240);
    qdev_prop_set_uint8(armv7m, "num-prio-bits", 4);
    qdev_prop_set_string(armv7m, "cpu-type", ARM_CPU_TYPE_NAME("cortex-m4"));
    qdev_prop_set_bit(armv7m, "enable-bitband", true);
    qdev_connect_clock_in(armv7m, "cpuclk", s->sysclk);
    qdev_connect_clock_in(armv7m, "refclk", s->refclk);
    object_property_set_link(OBJECT(&s->armv7m), "memory",
                             OBJECT(system_memory), &error_abort);

    if (!sysbus_realize(SYS_BUS_DEVICE(&s->armv7m), errp)) {
		printf("Error in SOC Relaize \n");
        return;
    }

    create_unimplemented_device("generic_io",    0x40000000, 0x1FFFFFFF);
}

static void cortexm4_soc_class_init(ObjectClass *klass, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = cortexm4_soc_realize;

	object_class_property_add_link(klass, "shram_backend",
    TYPE_MEMORY_BACKEND,
    offsetof(CORTEXM4State, shram_backend),
    qdev_prop_allow_set_link_before_realize,
    0);

}

static const TypeInfo cortexm4_soc_info = {
    .name          = TYPE_CORTEXM4_SOC,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(CORTEXM4State),
    .instance_init = cortexm4_soc_initfn,
    .class_init    = cortexm4_soc_class_init,
};

static void cortexm4_soc_types(void)
{
    type_register_static(&cortexm4_soc_info);
}

type_init(cortexm4_soc_types)
