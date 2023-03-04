import cffi
var lib = cffi.import_lib("./build/tests/test_cffi.csx")
var print = lib.import_func("print"), add = lib.import_func_s("add", cffi.types.sint, {cffi.types.sint, cffi.types.sint})
foreach i in range(10)
    system.out.println("value from host: " + add(i, i*2))
end
loop
    system.out.print("input from host: ")
    print(system.in.getline())
end
