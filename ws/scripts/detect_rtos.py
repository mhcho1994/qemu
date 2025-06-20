import gdb

def detect_rtos():
    # List of known RTOS symbols for detection
    rtos_signatures = {
        'FreeRTOS': ['pxReadyTasksLists', 'uxCurrentNumberOfTasks'],
        'RTX': ['osRtxInfo'],
        'Zephyr': ['z_ready_q', 'z_thread_runtime_stats'],
        'ThreadX': ['_tx_thread_created_ptr'],
    }

    found_rtos = []

    # Iterate over each RTOS and its known symbols
    for rtos, symbols in rtos_signatures.items():
        detected = True
        for symbol in symbols:
            try:
                gdb.lookup_symbol(symbol)
            except gdb.error:
                detected = False
                break
            # If symbol is not found
            if gdb.lookup_symbol(symbol)[0] is None:
                detected = False
                break
        if detected:
            found_rtos.append(rtos)

    if found_rtos:
        print("Detected RTOS: " + ", ".join(found_rtos))
    else:
        print("No known RTOS detected.")

# Register as GDB command
class DetectRTOSCommand(gdb.Command):
    """Detect RTOS in target binary"""

    def __init__(self):
        super(DetectRTOSCommand, self).__init__("detect_rtos", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        detect_rtos()

DetectRTOSCommand()

