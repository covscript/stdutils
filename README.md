# stdutils

Standard utilities library for Covariant Script — C FFI, bitwise operations, and SDK extensions.

## Modules

### cffi — C Foreign Function Interface (`cffi.cse`)

Call C functions in dynamic libraries directly from CovScript.

| Function | Description |
|----------|-------------|
| `load_library(path)` | Load a shared library |
| `get_function(lib, name)` | Get a function pointer by name |
| `call(func, args...)` | Call a C function with type conversion |

Supports types: void, pointer, int8/16/32/64, uint8/16/32/64, float, double, string.

### bitwise — Bitwise operations (`bitwise.cse`)

| Method | Description |
|--------|-------------|
| `bitset(value)` | Create a 64-bit bitset from integer or hex string |
| `set(pos, val)` | Set a bit at position |
| `get(pos)` | Get a bit at position |
| `flip(pos)` | Flip a bit at position |
| `count()` | Count set bits |
| `to_int()` | Convert to integer |

### sdk_extension — SDK utilities (`sdk_extension.cse`)

Internal SDK functions including signal handling (SIGINT on Unix, Ctrl+C on Windows).

## Build

```bash
# macOS: install libffi first
brew install libffi

mkdir build && cd build
cmake ..
cmake --build .
```

## License

MIT — see [LICENSE](./LICENSE)
