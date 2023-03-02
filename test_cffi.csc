import cffi
var dll = cffi.open_shared_lib("./build/tests/test_cffi.csx")
var print = dll.get_function("print"), add = dll.get_function("add")
foreach i in range(10)
    add(i, i*2)
end
loop
    system.out.print("input from host: ")
    print(system.in.getline())
end
