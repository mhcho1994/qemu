#Run the QEMU commmand in the same directory as of this file...
#Make sure you have installed the halucinator environment.
#If not, please run ./setup.py ->> comment the building of qemu, avatar2 to save your time.

import halucinator.bp_handlers.intercepts

def fastdyn_callback(pc):
    '''
    This function takes the pc and using the global hashmap for the breakpoint_num -> pc
    converts this to the breakpoint and then calls the interceptor function responsible
    for dispatching the respective intercept handler.
    '''

    intercept = True
    ret_val = 0
    return intercept, ret_val