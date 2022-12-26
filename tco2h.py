#!/usr/bin/env python3

# Create a C header from an occam TCO listing
import sys
import string
import re

#si = sys.stdin.read()
with open('TYPE32.TCL') as f:
    si = f.read()

res = re.findall('origin: (.+)\.occ', si)
if res:
    name = res[0]
res = re.findall('ws: (\d+)', si)
if res:
    workspace = res[0]
res = re.findall('vs: (\d+)', si)
if res:
    vectorspace = res[0]
res = re.findall('BIT32', si)
if res:
    bpw = 4
res = re.findall('BIT16', si)
if res:
    bpw = 2

code = ''
# lines after LOAD_TEXT are the hex to be extracted
res = re.search('LOAD_TEXT bytes: (\d+)', si, re.MULTILINE)
if res:
    code_size = int(res.groups()[0])
    todo = code_size
    remaining = si[res.span()[1]:]
    lines = remaining.splitlines()
    for line in lines:
        line = line.strip()
        m = re.match('([a-fA-F0-9]{8})([\sa-fA-F0-9]+)\s{6}', line)
        if m:
            addr = m.groups()[0]
            val = m.groups()[1]
            val = ''.join(val.split())
            code += val
            todo -= len(val)/2
            if todo == 0:
                break
    
code = bytes.fromhex(code)

offset = 0

print (f"struct {name}_struct {{")
print (f"\tunsigned char Code[{code_size}];")
print ("\tlong CodeSize;")
print ("\tlong Offset;")
print ("\tlong WorkSpace;")
print ("\tlong VectorSpace;")
print ("\tint  BytesPerWord;")
print ("\t};")

print (f"struct {name}_struct {name}_code = {{")
print ("\t{ /* code */")
i=1
for byte in code:
    if i < code_size:
        print("0x%02X" % byte, end=',')
        if i>0 and i%16==0:
            print ("")
        elif i>0 and i%4==0:
            print ("", end=' ')
    else:
        print("0x%02X" % byte, end='')
    i+=1
# HEX
print ("\t},")
print (f"\t{code_size},      /* code size */")
print (f"\t{offset},      /* offset */")
print (f"\t{workspace},      /* workspace */")
print (f"\t{vectorspace},      /* vectorspace */")
print (f"\t{bpw}      /* bytes per word */")
print ("};")
