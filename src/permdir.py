#!/usr/bin/python3

import os
import sys

d = sys.argv[1]

def transPerm(p):
  match p:
    case "0":
      return "---"
    case "1":
      return "--x"
    case "2":
      return "-w-"
    case "3":
      return "-wx"
    case "4":
      return "r--"
    case "5":
      return "r-x"
    case "6":
      return "rw-"
    case "7":
      return "rwx"

entries = os.listdir(d)
entries = [i for i in entries if os.path.isfile(os.path.join(d, i))]

permSet = set()

for f in entries:
  srcFilePath = os.path.join(d, f)
  status = oct(os.stat(srcFilePath).st_mode)[-3:]
  dirName = "-" + transPerm(status[0]) + transPerm(status[1]) + transPerm(status[2])
  if(status not in permSet):
    permSet.add(status)
    os.mkdir("./" + dirName)
  os.symlink(srcFilePath, os.path.join("./", dirName, f))
