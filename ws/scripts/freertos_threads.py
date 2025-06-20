import gdb

class FreeRTOSThreadList(gdb.Command):
    """List FreeRTOS tasks from target memory using ELF symbols."""

    def __init__(self):
        super(FreeRTOSThreadList, self).__init__("freertos-threads", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        try:
            # Adjust these as per your FreeRTOS config (or detect dynamically)
#max_priorities = int(gdb.parse_and_eval("configMAX_PRIORITIES"))
            max_priorities = 10
            ready_lists = gdb.parse_and_eval("pxReadyTasksLists")

            print(f"FreeRTOS Tasks (up to {max_priorities} priorities):")
            print("-" * 60)

            for i in range(max_priorities):
                list_head = ready_lists[i]
                list_item = list_head['xListEnd']['pxNext']

                # Walk the linked list
                while list_item != list_head.address:
                    tcb_ptr = list_item['pvOwner']
                    tcb = tcb_ptr.dereference()

                    task_name = tcb['pcTaskName'].string()
                    top_of_stack = tcb['pxTopOfStack']
                    priority = int(tcb['uxPriority'])

                    print(f"Task: {task_name:<16} Priority: {priority:<3} TopOfStack: {top_of_stack}")

                    list_item = list_item['pxNext']

        except gdb.error as e:
            print(f"GDB Error: {e}")
        except Exception as e:
            print(f"Error: {e}")

FreeRTOSThreadList()
