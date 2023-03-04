import cffi
var dll = cffi.import_lib("C:\\WINDOWS\\system32\\user32.dll")
var msgBox = dll.import_func("MessageBoxA"), MB_OK = 3
msgBox(null, "Hello world", "CovScript", MB_OK)
