import cffi
var lib = cffi.import_lib("./build/tests/test_cffi.csx")
# Test add
var add = lib.import_func_s("add", cffi.types.sint, {cffi.types.sint, cffi.types.sint})
foreach i in range(10)
    system.out.println("value from host: " + add(i, i*2))
end
# Test strings
var connect_str = lib.import_func_s("connect_str", cffi.types.pointer, {cffi.types.string, cffi.types.string})
var free_str = lib.import_func("free_str")
var ptr = connect_str("Hello, ", "world!")
system.out.println(cffi.utils.make_integer(ptr))
system.out.println(cffi.utils.make_string(ptr))
free_str(ptr)
# Test print
var print = lib.import_func("print")
loop
    system.out.print("input from host: ")
    print(system.in.getline())
end
