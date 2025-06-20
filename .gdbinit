# GDB may have ./.gdbinit loading disabled by default.  In that case you can
# follow the instructions it prints.  They boil down to adding the following to
# your home directory's ~/.gdbinit file:
#
#   add-auto-load-safe-path /path/to/qemu/.gdbinit

# Load QEMU-specific sub-commands and settings
source scripts/qemu-gdb.py
b vcpu_insn_exec_before
#run --plugin ./tests/tcg/plugins/libvirtual.so,monitor=../ws/monitor.elf,logger=ws/log_config.txt,virtual=ws/virtuals.txt,detour=ws/detours.txt,modifier=ws/modifiers.txt -d in_asm,op -D qemu.log -machine mps2-an385 -monitor telnet:127.0.0.1:5555,server,nowait -semihosting --semihosting-config enable=on,target=native -qmp unix:/tmp/qmp-sock,server,nowait -kernel ../ws/RTOSDemo.axf -serial stdio -nographic -gdb tcp::1235 -S

#run --plugin ./tests/tcg/plugins/libvirtual.so,monitor=../ws/monitor.elf,logger=ws/log_config.txt,virtual=ws/virtuals.txt,detour=ws/detours.txt,modifier=ws/modifiers.txt -d in_asm,op -D qemu.log -machine pixhawk1 -monitor telnet:127.0.0.1:5555,server,nowait -semihosting --semihosting-config enable=on,target=native -qmp unix:/tmp/qmp-sock,server,nowait -kernel ../ws/RTOSDemo.axf -serial stdio -nographic -gdb tcp::1235 -S

run --plugin ./tests/tcg/plugins/libvirtual.so,monitor=../ws/monitor.elf,logger=ws/log_config.txt,virtual=ws/virtuals.txt,detour=ws/detours.txt,modifier=ws/modifiers.txt -d in_asm,op -D qemu.log -machine cortexm4 -monitor telnet:127.0.0.1:5555,server,nowait -semihosting --semihosting-config enable=on,target=native -qmp unix:/tmp/qmp-sock,server,nowait -kernel ../ws/RTOSDemo.axf -serial stdio -nographic -gdb tcp::1235 -S


b my_unimp_read
b my_unimp_write

#run --plugin ./tests/tcg/plugins/libvirtual.so -machine mps2-an385 -monitor null -kernel ws/RTOSDemo.axf -serial stdio -nographic
