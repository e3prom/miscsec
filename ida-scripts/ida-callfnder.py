# ida-callfnder.py
# This IDA script allow you to search for call instructions matching a specific
# operand types.
#
# Written by Nicolas Chabbey <e3prom>
# Tested on IDA Pro 7.0 (x86, x64)
banner = "\nRunning CALL Instructions Finder\n"
motd = """
Written by Nicolas Chabbey <e3prom>
Latest version available at:
 https://github.com/e3prom/miscsec/ida-callfnder
"""

bar = "-" * (len(banner)*2)
print bar + banner + bar + motd + bar + "\n"

optype = -1
nbr = 0

op = {
    1: ("General Register (e,g. eax, rbx)", "reg"),
    2: ("Direct Memory Reference", "addr"),
    3: ("Indirect Memory Reference", "phrase"),
    4: ("Register as Reference", "phrase+addr"),
    5: ("Immediate Value", "value"),
    6: ("Immediate Far Address", "addr"),
    7: ("Immediate Near Address", "addr"),
}

def display_found_call(addr, instr):
    global nbr
    nbr += 1
    op1 = GetOpnd(addr, 0)
    print "0x%08X: %s %s" % (addr, instr, op1)

def get_optype():
    global optype
    print "What kind of operand type are your looking for:"
    for key in op.items():
        print "%s = %s [%s]" % (key[0], key[1][0], key[1][1])
    optype = AskLong(1, "Register type:")

def start():
    get_optype()

    if optype in range(1,7):
        find_call()
    else:
        print "Please specify a supported operand type."
        start()

def find_call():
    addr = SegByBase(SegByName(".text"))
    end = SegEnd(addr)

    while addr < end and addr != BADADDR:
        try:
            addr = NextAddr(addr)
            instr = GetMnem(addr)
            if str(instr) == "call":
                if GetOpType(addr,0) == optype:
                    display_found_call(addr, instr)
        except KeyboardInterrupt:
            print "WARNING: The search operation has been interrupted."
            break

    print "\n%i CALL instruction(s) found.\n" % nbr

if __name__ == "__main__":
    start()
