regs = 64

for i in range(0,65):
    if i % 8 == 0:
        print("\"gpr r{} .32    {} 0\\n\n".format(str(i).rjust(2, '0'), str(i*4)), end="")
    else:
        print("gpr r{} .32    {} 0\\n ".format(str(i).rjust(2, '0'), str(i*4)), end="")
