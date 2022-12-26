#!/usr/bin/env python3

# Create a C header from an occam TCO listing
import sys
import string
import re

si = sys.stdin.read()

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
res = re.findall('LOAD_TEXT bytes: (\d+)', si)
if res:
    code_size = int(res[0])
    res2 = re.match('LOAD_TEXT bytes: \d+', si, re.MULTILINE | re.DEBUG)
    print (res2)
#000000BB 2060B221 204421FB D424F2DA 24F251DB       `.! D!..$..$.Q.
    

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
print ("\t{ /* code */\n")
# HEX
print ("\t},")
print (f"\t{code_size},      /* code size */")
print (f"\t{offset},      /* offset */")
print (f"\t{workspace},      /* workspace */")
print (f"\t{vectorspace},      /* vectorspace */")
print (f"\t{bpw}      /* bytes per word */")
print ("};")
