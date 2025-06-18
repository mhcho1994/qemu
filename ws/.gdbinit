target remote :1235
b *0x23c
b *0x1dc
source ./scripts/mcall.py
add-symbol-file ./monitor.elf 
#b *0x106ca
#set debug remote on
