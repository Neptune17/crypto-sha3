import argparse
import hashlib
import os

parser = argparse.ArgumentParser()
parser.add_argument('-x', dest='method', type=str, help='SHA3-224||SHA3-256||SHA3-384||SHA3-512')
parser.add_argument('-s', dest='strmessage', type=str, help='str Message To Hash')
parser.add_argument('-f', dest='filename', type=str, help='File To Hash')
args = parser.parse_args()

sh = None
type_id = -1
if args.method == "SHA3-224":
    sh = hashlib.sha3_224()
    type_id = 0
if args.method == "SHA3-256":
    sh = hashlib.sha3_256()
    type_id = 1
if args.method == "SHA3-384":
    sh = hashlib.sha3_384()
    type_id = 2
if args.method == "SHA3-512":
    sh = hashlib.sha3_512()
    type_id = 3

def encode(s):
    temp_list = []
    for c in s:
        tempstr = bin(ord(c)).replace('0b', '')
        if(len(tempstr) != 8):
            tempstr = '0'*(8 - len(tempstr)) + tempstr
        temp_list.append(tempstr[::-1])
    x = ''.join(temp_list)
    return x

if args.strmessage is not None:
    sh.update(args.strmessage.encode('utf-8'))
    print("expect:", sh.hexdigest())
    pipe = os.popen("./mysha3_old -x " + str(type_id) + " -m " + encode(args.strmessage))
    print("oldver:", pipe.read(), end='')
    pipe = os.popen("./mysha3 -x " + str(type_id) + " -m " + encode(args.strmessage))
    print("result:", pipe.read(), end='')
elif args.filename is not None:
    message = open(args.filename).read()
    sh.update(message.encode('utf-8'))
    print("expect:", sh.hexdigest())
    pipe = os.popen("./mysha3_old -x " + str(type_id) + " -f " + args.filename)
    print("oldver:", pipe.read(), end='')
    pipe = os.popen("./mysha3 -x " + str(type_id) + " -f " + args.filename)
    print("result:", pipe.read(), end='')
else:
    sh.update(''.encode('utf-8'))
    print("expect:", sh.hexdigest())
    pipe = os.popen("./mysha3_old -x " + str(type_id))
    print("oldver:", pipe.read(), end='')
    pipe = os.popen("./mysha3 -x " + str(type_id))
    print("result:", pipe.read(), end='')