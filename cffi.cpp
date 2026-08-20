#include <covscript/impl/impl.hpp>
#include <covscript/cni.hpp>
#include <covscript/dll.hpp>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cerrno>
#include <ffi.h>

#if COVSCRIPT_API_VERSION >= 260900
#define CS_DLL_OPEN(path) cs::dll::open(path)
#define CS_DLL_CLOSE(handle) cs::dll::close(handle)
#define CS_DLL_FIND_SYMBOL(handle, sym) cs::dll::find_symbol(handle, sym)
#else
#define CS_DLL_OPEN(path) (new cov::dll(path))
#define CS_DLL_CLOSE(handle) (delete static_cast<cov::dll *>(handle))
#define CS_DLL_FIND_SYMBOL(handle, sym) (static_cast<cov::dll *>(handle)->get_address(sym))
#endif

enum class cffi_type
{
	ffi_void,
	ffi_pointer,
	ffi_double,
	ffi_float,
	ffi_schar,
	ffi_sshort,
	ffi_sint,
	ffi_slong,
	ffi_uchar,
	ffi_ushort,
	ffi_uint,
	ffi_ulong,
	ffi_sint8,
	ffi_sint16,
	ffi_sint32,
	ffi_sint64,
	ffi_uint8,
	ffi_uint16,
	ffi_uint32,
	ffi_uint64,
	ffi_string
};

bool is_integer(cffi_type t) noexcept
{
	switch (t) {
	case cffi_type::ffi_schar:
	case cffi_type::ffi_sshort:
	case cffi_type::ffi_sint:
	case cffi_type::ffi_slong:
	case cffi_type::ffi_uchar:
	case cffi_type::ffi_ushort:
	case cffi_type::ffi_uint:
	case cffi_type::ffi_ulong:
	case cffi_type::ffi_sint8:
	case cffi_type::ffi_sint16:
	case cffi_type::ffi_sint32:
	case cffi_type::ffi_sint64:
	case cffi_type::ffi_uint8:
	case cffi_type::ffi_uint16:
	case cffi_type::ffi_uint32:
	case cffi_type::ffi_uint64:
		return true;
	default:
		return false;
	}
}

bool is_float(cffi_type t) noexcept
{
	switch (t) {
	case cffi_type::ffi_double:
	case cffi_type::ffi_float:
		return true;
	default:
		return false;
	}
}

ffi_type *get_actual_type(cffi_type t) noexcept
{
	switch (t) {
	default:
	case cffi_type::ffi_void:
		return &ffi_type_void;
	case cffi_type::ffi_pointer:
	case cffi_type::ffi_string:
		return &ffi_type_pointer;
	case cffi_type::ffi_double:
		return &ffi_type_double;
	case cffi_type::ffi_float:
		return &ffi_type_float;
	case cffi_type::ffi_schar:
		return &ffi_type_schar;
	case cffi_type::ffi_sshort:
		return &ffi_type_sshort;
	case cffi_type::ffi_sint:
		return &ffi_type_sint;
	case cffi_type::ffi_slong:
		return &ffi_type_slong;
	case cffi_type::ffi_uchar:
		return &ffi_type_uchar;
	case cffi_type::ffi_ushort:
		return &ffi_type_ushort;
	case cffi_type::ffi_uint:
		return &ffi_type_uint;
	case cffi_type::ffi_ulong:
		return &ffi_type_ulong;
	case cffi_type::ffi_sint8:
		return &ffi_type_sint8;
	case cffi_type::ffi_sint16:
		return &ffi_type_sint16;
	case cffi_type::ffi_sint32:
		return &ffi_type_sint32;
	case cffi_type::ffi_sint64:
		return &ffi_type_sint64;
	case cffi_type::ffi_uint8:
		return &ffi_type_uint8;
	case cffi_type::ffi_uint16:
		return &ffi_type_uint16;
	case cffi_type::ffi_uint32:
		return &ffi_type_uint32;
	case cffi_type::ffi_uint64:
		return &ffi_type_uint64;
	}
}

struct resource_holder
{
	resource_holder() = default;
	resource_holder(const resource_holder &) = delete;
	resource_holder(resource_holder &&) noexcept = delete;
	virtual ~resource_holder() = default;
	virtual cs::var get_var() const = 0;
	virtual void *get_ptr() const = 0;
};

template <typename T> struct resource_holder_impl final : public resource_holder
{
	T data;
	template <typename... ArgsT> resource_holder_impl(ArgsT &&...args) : data(std::forward<ArgsT>(args)...) {}
	~resource_holder_impl() override = default;
	cs::var get_var() const override { return data; }
	void *get_ptr() const override { return const_cast<void *>(static_cast<const void *>(&data)); }
};

// Optimize for numeric

template <typename T> struct numeric_holder
{
};

template <typename T> struct resource_holder_impl<numeric_holder<T>> final : public resource_holder
{
	T data;
	resource_holder_impl() = default;
	resource_holder_impl(T val) : data(val) {}
	cs::var get_var() const override { return cs::var::make<cs::numeric>(data); }
	void *get_ptr() const override { return const_cast<void *>(static_cast<const void *>(&data)); }
};

// Optimize for strings

template <> struct resource_holder_impl<const char *> final : public resource_holder
{
	const char *data;
	resource_holder_impl() = default;
	resource_holder_impl(const char *str) : data(str) {}
	~resource_holder_impl() override = default;
	cs::var get_var() const override { return cs::var::make<cs::string>(data); }
	void *get_ptr() const override { return const_cast<const char **>(&data); }
};

template <> struct resource_holder_impl<cs::string> final : public resource_holder
{
	char *data;
	resource_holder_impl(const cs::string &str)
	{
		data = new char[str.size() + 1];
		std::strcpy(data, str.c_str());
	}
	~resource_holder_impl() override { delete[] data; }
	cs::var get_var() const override { return cs::var::make<cs::string>(data); }
	void *get_ptr() const override { return const_cast<void *>(static_cast<const void *>(&data)); }
};

resource_holder *make_resource_holder(cffi_type t)
{
	switch (t) {
	default:
	case cffi_type::ffi_pointer:
		return new resource_holder_impl<void *>();
	case cffi_type::ffi_string:
		return new resource_holder_impl<const char *>();
	case cffi_type::ffi_double:
		return new resource_holder_impl<numeric_holder<double>>();
	case cffi_type::ffi_float:
		return new resource_holder_impl<numeric_holder<float>>();
	case cffi_type::ffi_schar:
		return new resource_holder_impl<char>();
	case cffi_type::ffi_sshort:
		return new resource_holder_impl<numeric_holder<short>>();
	case cffi_type::ffi_sint:
		return new resource_holder_impl<numeric_holder<int>>();
	case cffi_type::ffi_slong:
		return new resource_holder_impl<numeric_holder<long>>();
	case cffi_type::ffi_uchar:
		return new resource_holder_impl<numeric_holder<unsigned char>>();
	case cffi_type::ffi_ushort:
		return new resource_holder_impl<numeric_holder<unsigned short>>();
	case cffi_type::ffi_uint:
		return new resource_holder_impl<numeric_holder<unsigned int>>();
	case cffi_type::ffi_ulong:
		return new resource_holder_impl<numeric_holder<unsigned long>>();
	case cffi_type::ffi_sint8:
		return new resource_holder_impl<numeric_holder<int8_t>>();
	case cffi_type::ffi_sint16:
		return new resource_holder_impl<numeric_holder<int16_t>>();
	case cffi_type::ffi_sint32:
		return new resource_holder_impl<numeric_holder<int32_t>>();
	case cffi_type::ffi_sint64:
		return new resource_holder_impl<numeric_holder<int64_t>>();
	case cffi_type::ffi_uint8:
		return new resource_holder_impl<numeric_holder<uint8_t>>();
	case cffi_type::ffi_uint16:
		return new resource_holder_impl<numeric_holder<uint16_t>>();
	case cffi_type::ffi_uint32:
		return new resource_holder_impl<numeric_holder<uint32_t>>();
	case cffi_type::ffi_uint64:
		return new resource_holder_impl<numeric_holder<uint64_t>>();
	}
}

class cffi_simple_callable final
{
	void (*target_func)() = nullptr;

      public:
	cffi_simple_callable(void (*ptr)()) : target_func(ptr) {}
	cs::var operator()(cs::vector &args) const
	{
		std::unique_ptr<ffi_type *[]> arg_types(new ffi_type *[args.size()]);
		std::unique_ptr<void *[]> arg_data(new void *[args.size()]);
		std::vector<std::unique_ptr<resource_holder>> hosted_res;
		for (std::size_t i = 0; i < args.size(); ++i) {
			const cs::var &it = args[i];
			if (it.type() == typeid(cs::numeric)) {
				const cs::numeric &num = it.const_val<cs::numeric>();
				if (num.is_integer()) {
					hosted_res.emplace_back(
					    new resource_holder_impl<cs::numeric_integer>(num.as_integer()));
					arg_types[i] = &ffi_type_sint64;
				} else {
					hosted_res.emplace_back(
					    new resource_holder_impl<cs::numeric_float>(num.as_float()));
					arg_types[i] = &ffi_type_double;
				}
			} else if (it.type() == typeid(cs::string)) {
				hosted_res.emplace_back(
				    new resource_holder_impl<cs::string>(it.const_val<cs::string>()));
				arg_types[i] = &ffi_type_pointer;
			} else if (it.type() == typeid(void *)) {
				hosted_res.emplace_back(new resource_holder_impl<void *>(it.const_val<void *>()));
				arg_types[i] = &ffi_type_pointer;
			} else if (it == cs::null_pointer) {
				hosted_res.emplace_back(new resource_holder_impl<void *>(nullptr));
				arg_types[i] = &ffi_type_pointer;
			} else
				throw cs::runtime_error("Unsupported type in cffi.");
			arg_data[i] = hosted_res.back()->get_ptr();
		}
		ffi_cif cif;
		if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, args.size(), &ffi_type_void, arg_types.get()) == FFI_OK)
			ffi_call(&cif, target_func, nullptr, arg_data.get());
		return cs::null_pointer;
	}
};

class cffi_callable final
{
	// Properties
	cffi_type restype = cffi_type::ffi_void;
	std::vector<cffi_type> argtypes;
	void (*target_func)() = nullptr;
	// Internal Data
	mutable ffi_cif _ffi_cif;
	std::vector<ffi_type *> _ffi_types;

      public:
	cffi_callable(void (*ptr)(), cffi_type rt, std::vector<cffi_type> ats)
	    : target_func(ptr), restype(rt), argtypes(std::move(ats))
	{
		_ffi_types.resize(argtypes.size());
		for (std::size_t i = 0; i < argtypes.size(); ++i)
			_ffi_types[i] = get_actual_type(argtypes[i]);
		if (ffi_prep_cif(&_ffi_cif, FFI_DEFAULT_ABI, argtypes.size(), get_actual_type(restype),
				 _ffi_types.data()) != FFI_OK)
			throw cs::runtime_error("Init libffi CIF failed!");
	}
	cs::var operator()(cs::vector &args) const
	{
		if (args.size() != argtypes.size())
			throw cs::runtime_error("Unmatched argument size.");
		std::unique_ptr<void *[]> arg_data(new void *[args.size()]);
		std::vector<std::unique_ptr<resource_holder>> hosted_res;
		for (std::size_t i = 0; i < args.size(); ++i) {
			const cs::var &it = args[i];
			if (it.type() == typeid(cs::numeric)) {
				const cs::numeric &num = it.const_val<cs::numeric>();
				if (num.is_integer()) {
					if (!is_integer(argtypes[i]))
						throw cs::lang_error("Unmatched type in arguments.");
					hosted_res.emplace_back(
					    new resource_holder_impl<cs::numeric_integer>(num.as_integer()));
				} else {
					if (!is_float(argtypes[i]))
						throw cs::lang_error("Unmatched type in arguments.");
					hosted_res.emplace_back(
					    new resource_holder_impl<cs::numeric_float>(num.as_float()));
				}
			} else if (it.type() == typeid(cs::string)) {
				if (argtypes[i] != cffi_type::ffi_string)
					throw cs::lang_error("Unmatched type in arguments.");
				hosted_res.emplace_back(
				    new resource_holder_impl<cs::string>(it.const_val<cs::string>()));
			} else if (it.type() == typeid(void *)) {
				if (argtypes[i] != cffi_type::ffi_pointer)
					throw cs::lang_error("Unmatched type in arguments.");
				hosted_res.emplace_back(new resource_holder_impl<void *>(it.const_val<void *>()));
			} else if (it == cs::null_pointer) {
				if (argtypes[i] != cffi_type::ffi_pointer && !is_integer(argtypes[i]))
					throw cs::lang_error("Unmatched type in arguments.");
				hosted_res.emplace_back(new resource_holder_impl<void *>(nullptr));
			} else
				throw cs::runtime_error("Unsupported type in cffi.");
			arg_data[i] = hosted_res.back()->get_ptr();
		}
		if (restype != cffi_type::ffi_void) {
			std::unique_ptr<resource_holder> return_data(make_resource_holder(restype));
			ffi_call(&_ffi_cif, target_func, return_data->get_ptr(), arg_data.get());
			return return_data->get_var();
		} else {
			ffi_call(&_ffi_cif, target_func, nullptr, arg_data.get());
			return cs::null_pointer;
		}
	}
};

struct dll_holder
{
	void *handle = nullptr;

	dll_holder(std::string_view path) { handle = CS_DLL_OPEN(path); }

	dll_holder(const dll_holder &) = delete;

	~dll_holder()
	{
		if (handle != nullptr)
			CS_DLL_CLOSE(handle);
	}

	void *get_address(std::string_view sym) { return CS_DLL_FIND_SYMBOL(handle, sym); }
};

using dll_type = std::shared_ptr<dll_holder>;

CNI_ROOT_NAMESPACE
{
	using namespace cs;

	dll_type import_lib(const std::string &path)
	{
		return std::make_shared<dll_holder>(path);
	}

	CNI(import_lib)

	CNI_NAMESPACE(lib){
	    callable import_func(const dll_type &dll, const std::string &name){void *addr = dll->get_address(name);
	if (!addr)
		throw cs::lang_error("cffi: symbol not found: " + name);
	return callable(cffi_simple_callable(reinterpret_cast<void (*)()>(addr)));
}

CNI(import_func)

callable import_func_s(const dll_type &dll, const std::string &name, cffi_type restype, const array &ats)
{
	void *addr = dll->get_address(name);
	if (!addr)
		throw cs::lang_error("cffi: symbol not found: " + name);
	std::vector<cffi_type> argtypes;
	for (auto &it : ats)
		argtypes.emplace_back(it.const_val<cffi_type>());
	return callable(cffi_callable(reinterpret_cast<void (*)()>(addr), restype, std::move(argtypes)));
}

CNI(import_func_s)
}

CNI_NAMESPACE(utils){var make_integer(void *ptr){return var::make<numeric>(reinterpret_cast<std::size_t>(ptr));
}

CNI(make_integer)

var make_string(void *ptr)
{
	return var::make<string>(static_cast<const char *>(ptr));
}

CNI(make_string)

bool is_nullptr(void *ptr)
{
	return ptr == nullptr;
}

CNI(is_nullptr)

void *cs_malloc(cs::number size)
{
	return std::malloc(static_cast<std::size_t>(size));
}

CNI(cs_malloc)

void cs_free(void *ptr)
{
	std::free(ptr);
}

CNI(cs_free)

void *cs_memcpy(void *dst, const void *src, cs::number n)
{
	return std::memcpy(dst, src, static_cast<std::size_t>(n));
}

CNI(cs_memcpy)

void *ptr_add(void *ptr, cs::number offset)
{
	return static_cast<char *>(ptr) + static_cast<std::ptrdiff_t>(offset);
}

CNI(ptr_add)

cs::number get_errno()
{
	return static_cast<cs::number>(errno);
}

CNI(get_errno)
}

CNI_NAMESPACE(types)
{
	CNI_VALUE(void, cffi_type::ffi_void)
	CNI_VALUE(pointer, cffi_type::ffi_pointer)
	CNI_VALUE(double, cffi_type::ffi_double)
	CNI_VALUE(float, cffi_type::ffi_float)
	CNI_VALUE(schar, cffi_type::ffi_schar)
	CNI_VALUE(sshort, cffi_type::ffi_sshort)
	CNI_VALUE(sint, cffi_type::ffi_sint)
	CNI_VALUE(slong, cffi_type::ffi_slong)
	CNI_VALUE(uchar, cffi_type::ffi_uchar)
	CNI_VALUE(ushort, cffi_type::ffi_ushort)
	CNI_VALUE(uint, cffi_type::ffi_uint)
	CNI_VALUE(ulong, cffi_type::ffi_ulong)
	CNI_VALUE(string, cffi_type::ffi_string)
	CNI_VALUE(bool_, cffi_type::ffi_schar)
	CNI_VALUE(int8, cffi_type::ffi_sint8)
	CNI_VALUE(int16, cffi_type::ffi_sint16)
	CNI_VALUE(int32, cffi_type::ffi_sint32)
	CNI_VALUE(int64, cffi_type::ffi_sint64)
	CNI_VALUE(uint8, cffi_type::ffi_uint8)
	CNI_VALUE(uint16, cffi_type::ffi_uint16)
	CNI_VALUE(uint32, cffi_type::ffi_uint32)
	CNI_VALUE(uint64, cffi_type::ffi_uint64)
}
}

CNI_ENABLE_TYPE_EXT(lib, dll_type)