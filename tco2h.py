#!/usr/bin/env python3

# Create a C header from an occam TCO listing
import sys
import string
import re

si =  sys.stdin.readlines()

for line in si:
    line = line.strip()
    res = re.findall('origin: (.+)\.occ', line)
    if res:
        name = res[0]
    res = re.findall('ws: (\d+)', line)
    if res:
        workspace = res[0]
    res = re.findall('LOAD_TEXT bytes: (\d+)', line)
    if res:
        code_size = int(res[0]) + 4

offset = 0
vectorspace = 0
bpw = 4

print (f"struct {name}_struct {{")
print (f"\tunsigned char Code[{code_size}];")
print ("\tlong CodeSize;")
print ("\tlong Offset;")
print ("\tlong WorkSpace;")
print ("\tlong VectorSpace;")
print ("\tint  BytesPerWord;")
print ("\t};")

print (f"struct {name}_struct {name}_code = {{")
print ("\t{ /* code */\n")
# HEX
print ("\t},")
print (f"\t{code_size},      /* code size */")
print (f"\t{offset},      /* offset */")
print (f"\t{workspace},      /* workspace */")
print (f"\t{vectorspace},      /* vectorspace */")
print (f"\t{bpw}      /* bytes per word */")
print ("};")
