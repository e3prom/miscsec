# ida-poppopret.py
# Basic IDA Python script to search for POP/POP/RET gadgets in the .text segment
# of PE/COFF binaries. Returns the linear memory address of every found gadgets
# along with their instructions/operand pairs.
#
# This script has been sucessfully tested on IDA Pro 7.0 (x86).
print "-" * 63 + "\nRunning POP/POP/RET Gadget Finder\n" + "-" * 63

addr = SegByBase(SegByName(".text"))
end = SegEnd(addr)
nbr = 0

def display_operand(addr, instr, instr2, instr3, flag):
    global nbr
    nbr += 1
    op1 = GetOpnd(addr, 0)
    op2 = GetOpnd(int(addr+1), 0)
    oc1 = Byte(addr)
    oc2 = Byte(addr+1)
    oc3 = Byte(addr+2)
    if flag == 0:
        print "0x%08x: %s %s | %s %s | %s\t (%s %s %s)" % (addr,instr,op1,instr2,op2,instr3,hex(oc1),hex(oc2),hex(oc3))
    else:
        op3 = GetOpnd(int(addr+2), 0)
        print "0x%08x: %s %s | %s %s | %s %s\t (%s %s %s)" % (addr,instr,op1,instr2,op2,instr3,op3,hex(oc1),hex(oc2),hex(oc3))

while addr < end and addr != BADADDR:
    addr = NextAddr(addr)
    instr = GetMnem(addr)
    if str(instr) == "pop":
        instr2 = GetMnem(addr+1)
        if instr2 == "pop":
            instr3 = GetMnem(addr+2)
            if instr3 == "retn":
                if GetOperandValue(addr+2,0) == -1:
                    display_operand(addr, instr, instr2, instr3, 0)
                else:
                    display_operand(addr, instr, instr2, instr3, 1)

print "\n%i Gadgets found.\n" % nbr
print "Script Operation Completed.\n"
