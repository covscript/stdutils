import stdutils.arr as arr
import stdutils as utils

var v = {1, 3, 6, 4}
arr.print(v)
arr.insert(v, 1, 2)
arr.print(v)
arr.erase(v, 3)
arr.print(v)

iostream.setprecision(32)
system.out.println(utils.crc32("Hello, world!"))
system.out.println(utils.crc32_file("./build/imports/bitwise.cse"))