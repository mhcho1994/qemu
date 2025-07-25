target remote :1235
source ./scripts/mcall.py
source ./scripts/detect_rtos.py
source ./scripts/freertos_threads.py
source ./scripts/reginfo.py
add-symbol-file ./monitor.elf
b *0x00025c80
#b *0x106ca
#set debug remote on
