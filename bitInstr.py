currentIndex = 0
shiftInstructions = [
    "rotateC(\'l\',&",
    "rotateC(\'r\',&",
    "rotate(\'l\',&",
    "rotate(\'r\',&",
    "shift(\'l\', &",
    "shift(\'r\', &",
    "logicalShift(\'l\', &",
    "logicalShift(\'r\', &",
    ]


bitInstructions = [    
    "bit(",
    "reset(",
    "set("]


registers = ["z80.B",
    "z80.C",
    "z80.D",
    "z80.E",
    "z80.H",
    "z80.L",
    "z80.HL",
    "z80.A"]

    
print("switch(opcode){")
for i in shiftInstructions:
    for j in registers:
        print("  ", end="")
        print(f"case {hex(currentIndex)}:")
        print("    ", end="")
        if(j == "z80.HL"):
            print("HLASINDEX(", end="")
        print (i + j + ")", end="")
        if(j == "z80.HL"):
            print(")", end="")
        print(";")
        print("    ", end="")
        print("break;")
        currentIndex += 1
for i in bitInstructions:
    for j in range(0,8):
        for k in registers:
                print("  ", end="")
                print(f"case {hex(currentIndex)}:")
                print("    ", end="")
                if(k == "z80.HL"):
                     print("HLASINDEX(", end="")
                print(i + str(j) + ", &" + k + ")", end="")
                if(k == "z80.HL"):
                    print(")", end="")
                print(";")
                print("    ", end="")
                print("break;")
                currentIndex += 1

print("}")
