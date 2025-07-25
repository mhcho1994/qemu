import gdb
import os

#Carefully use last 256 bytes
CONTEXT_ADDR = 0x3FFFFF00
REG_NAMES = [
    'r0', 'r1', 'r2', 'r3', 'r4', 'r5', 'r6', 'r7',
    'r8', 'r9', 'r10', 'r11', 'r12', 'sp', 'lr', 'pc'
]

script_dir = os.path.dirname(os.path.abspath(__file__))
MCALLS_FILE = os.path.join(script_dir, "mcalls")


def load_mcall_names():
    if not os.path.isfile(MCALLS_FILE):
        return []
    with open(MCALLS_FILE, 'r') as f:
        return [line.strip() for line in f if line.strip()]

def save_context():
    for i, reg in enumerate(REG_NAMES):
        val = int(gdb.parse_and_eval(f"${reg}"))
        gdb.execute(f"set *(unsigned long *)({CONTEXT_ADDR + i*4}) = {val}")

def restore_context():
    for i, reg in enumerate(REG_NAMES):
        val = gdb.parse_and_eval(f"*(unsigned long *)({CONTEXT_ADDR + i*4})")
        if reg == "pc":
            continue
        gdb.execute(f"set ${reg} = {val}")

class MCall(gdb.Command):
    """Call a function with context save/restore: mcall <function>"""

    def __init__(self):
        super(MCall, self).__init__("mcall", gdb.COMMAND_USER)
        self.candidates = self.load_mcall_names()

    def load_mcall_names(self):
        if not os.path.isfile(MCALLS_FILE):
            return []
        with open(MCALLS_FILE, 'r') as f:
            return [line.strip() for line in f if line.strip()]

    def invoke(self, arg, from_tty):
        if not arg:
            print("Usage: mcall <function_name>")
            return
        save_context()
        gdb.execute(f"call {arg}")
        restore_context()

    def complete(self, text, word):
        return [c for c in self.candidates if c.startswith(text)]

MCall()

# Dynamically create commands from mcalls file
def make_mcall_command(name):
    class DynamicMCall(gdb.Command):
        def __init__(self):
            super(DynamicMCall, self).__init__(name, gdb.COMMAND_USER)

        def invoke(self, arg, from_tty):
            # Call mcall <name> command, forwarding any arguments
            call_str = f"mcall {name}()"
            if arg:
                call_str += f" {arg}"
            gdb.execute(call_str)

    return DynamicMCall()

for keyword in load_mcall_names():
    make_mcall_command(keyword)
