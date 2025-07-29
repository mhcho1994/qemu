# GDB may have ./.gdbinit loading disabled by default.  In that case you can
# follow the instructions it prints.  They boil down to adding the following to
# your home directory's ~/.gdbinit file:
#
#   add-auto-load-safe-path /path/to/qemu/.gdbinit

# Load QEMU-specific sub-commands and settings
source scripts/qemu-gdb.py
b vcpu_insn_exec_before
#run --plugin ./tests/tcg/plugins/libvirtual.so,monitor=../ws/monitor.elf,logger=../ws/log_config.txt,virtual=../ws/virtuals.txt,detour=../ws/detours.txt,modifier=../ws/modifiers.txt -d in_asm,op -D qemu.log -machine mps2-an385 -monitor telnet:127.0.0.1:5555,server,nowait -semihosting --semihosting-config enable=on,target=native -qmp unix:/tmp/qmp-sock,server,nowait -kernel ../ws/RTOSDemo.axf -serial stdio -nographic -gdb tcp::1235 -S

#run --plugin ./tests/tcg/plugins/libvirtual.so,monitor=../ws/monitor.elf,logger=../ws/log_config.txt,virtual=../ws/virtuals.txt,detour=../ws/detours.txt,modifier=../ws/modifiers.txt -d in_asm,op -D qemu.log -machine pixhawk1 -monitor telnet:127.0.0.1:5555,server,nowait -semihosting --semihosting-config enable=on,target=native -qmp unix:/tmp/qmp-sock,server,nowait -kernel ../ws/RTOSDemo.axf -serial stdio -nographic -gdb tcp::1235 -S

#This was the last one
#run --plugin ./tests/tcg/plugins/libvirtual.so,monitor=../ws/monitor.elf,logger=../ws/log_config.txt,virtual=../ws/virtuals.txt,detour=../ws/detours.txt,modifier=../ws/modifiers.txt -d in_asm,op -D qemu.log -machine cortexm4,memory-backend=ram0 -monitor telnet:127.0.0.1:5555,server,nowait -semihosting --semihosting-config enable=on,target=native -qmp unix:/tmp/qmp-sock,server,nowait -kernel ../ws/RTOSDemo.axf -serial stdio -nographic -gdb tcp::1235 -S -object memory-backend-file,id=ram0,mem-path=/dev/shm/my_m4_ram3,size=512M,share=on -object memory-backend-file,id=ram1,mem-path=/dev/shm/my_m4_ram,size=512K,share=on -global cortexm4-soc.shram_backend=ram1  -global cortexm4-soc.ram_baseaddr=0x20000000  -global cortexm4-soc.shram_baseaddr=0x30000000 -qmp unix:/tmp/qmp.sock,server=on,wait=off


run --plugin ./tests/tcg/plugins/libvirtual.so,monitor=../ws/monitor.elf,logger=../ws/log_config.txt,virtual=../ws/virtuals.txt,detour=../ws/detours.txt,modifier=../ws/modifiers.txt -d in_asm,op -D qemu.log -machine cortexm4,memory-backend=ram0 -monitor telnet:127.0.0.1:5555,server,nowait -semihosting --semihosting-config enable=on,target=native -qmp unix:/tmp/qmp-sock,server,nowait -kernel ../ws/RTOSDemo.axf -serial stdio -nographic -object memory-backend-file,id=ram0,mem-path=/dev/shm/my_m4_ram3,size=512M,share=on -object memory-backend-file,id=ram1,mem-path=/dev/shm/my_m4_ram,size=512K,share=on -global cortexm4-soc.shram_backend=ram1  -global cortexm4-soc.ram_baseaddr=0x20000000  -global cortexm4-soc.shram_baseaddr=0x30000000 -qmp unix:/tmp/qmp.sock,server=on,wait=off

#run --plugin ./tests/tcg/plugins/libvirtual.so,logger=../ws/log_config.txt,virtual=../ws/virtuals.txt,detour=../ws/detours.txt,modifier=../ws/modifiers.txt -d in_asm,op -D qemu.log -machine mps2-an385 -monitor telnet:127.0.0.1:5555,server,nowait -semihosting --semihosting-config enable=on,target=native -qmp unix:/tmp/qmp-sock,server,nowait -kernel ../ws/RTOSDemo.axf -serial stdio -nographic -gdb tcp::1235 -S

#run --plugin ./tests/tcg/plugins/libvirtual.so,monitor=../ws/monitor.elf,logger=../ws/log_config.txt,virtual=../ws/virtuals.txt,detour=../ws/detours.txt,modifier=../ws/modifiers.txt -d in_asm,op -D qemu.log -machine cortexm7 -monitor telnet:127.0.0.1:5555,server,nowait -semihosting --semihosting-config enable=on,target=native -qmp unix:/tmp/qmp-sock,server,nowait -kernel ../ws/renesas_M4_onchipflash_programflash_00000000h.elf -serial stdio -nographic -gdb tcp::1235 -S

#run --plugin ./tests/tcg/plugins/libvirtual.so -d in_asm,op -D qemu.log -machine cortexm4 -monitor telnet:127.0.0.1:5555,server,nowait -semihosting --semihosting-config enable=on,target=native -qmp unix:/tmp/qmp-sock,server,nowait -serial stdio -nographic -gdb tcp::1235 -S


#b fastdyn_callback
#run --plugin ./tests/tcg/plugins/libvirtual.so -machine mps2-an385 -monitor null -kernel ../ws/RTOSDemo.axf -serial stdio -nographic


#run  -machine olimex-stm32-h405 -monitor telnet:127.0.0.1:5555,server,nowait -serial stdio -nographic -nographic -gdb tcp::1235 -S

#run -machine cortexm4,memory-backend=ram0 -object memory-backend-file,id=ram0,mem-path=/dev/shm/my_m4_ram2,size=1G,share=on -nographic -S -monitor telnet:127.0.0.1:4444,server,nowait -gdb tcp::1235 -kernel ../ws/RTOSDemo.axf


#run -machine cortexm4,memory-backend=ram0 -monitor telnet:127.0.0.1:4444,server,nowait -object memory-backend-file,id=ram0,mem-path=/dev/shm/my_m4_ram3,size=512M,share=on -object memory-backend-file,id=ram1,mem-path=/dev/shm/my_m4_ram,size=512K,share=on -global cortexm4-soc.shram_backend=ram1  -global cortexm4-soc.ram_baseaddr=0x20000000  -global cortexm4-soc.shram_baseaddr=0x30000000 -S -nographic
