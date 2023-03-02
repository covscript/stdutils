import cffi
var dll = cffi.open_shared_lib("./cmake-build/test_cffi.csx")
var fun = dll.get_function("print")
loop
    fun(system.in.getline()) 
end