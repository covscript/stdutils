#include <covscript/impl/impl.hpp>
#include <covscript/cni.hpp>
#include <covscript/dll.hpp>
#include <cstring>
#include <ffi.h>

struct resource_holder
{
    resource_holder() = default;
    resource_holder(const resource_holder&) = delete;
    resource_holder(resource_holder &&) noexcept = delete;
    virtual ~resource_holder() = default;
    virtual void *get_ptr() const = 0;
};

template<typename T>
struct resource_holder_impl final : public resource_holder
{
    T data;
    template<typename ...ArgsT>
    resource_holder_impl(ArgsT &&...args) : data(std::forward<ArgsT>(args)...) {}
    virtual ~resource_holder_impl() = default;
    virtual void *get_ptr() const
    {
        return (void*)&data;
    }
};

template<>
struct resource_holder_impl<cs::string> final : public resource_holder
{
    char *data;
    resource_holder_impl(const cs::string &str)
    {
        data = new char[str.size()];
        std::strcpy(data, str.c_str());
    }
    virtual ~resource_holder_impl()
    {
        delete[] data;
    }
    virtual void *get_ptr() const
    {
        return (void*)&data;
    }
};

struct ffi_callable final
{
    void (*target_func)() = nullptr;
    cs::var operator()(cs::vector &args) const
    {
        std::unique_ptr<ffi_type *[]> arg_types(new ffi_type *[args.size()]);
        std::unique_ptr<void *[]> arg_data(new void *[args.size()]);
        std::vector<std::unique_ptr<resource_holder>> hosted_res;
        for (std::size_t i = 0; i < args.size(); ++i)
        {
            const cs::var &it = args[i];
            if (it.type() == typeid(cs::numeric))
            {
                const cs::numeric &num = it.const_val<cs::numeric>();
                if (num.is_integer())
                {
                    hosted_res.emplace_back(new resource_holder_impl<cs::numeric_integer>(num.as_integer()));
                    arg_types[i] = &ffi_type_sint64;
                } else {
                    hosted_res.emplace_back(new resource_holder_impl<cs::numeric_float>(num.as_float()));
                    arg_types[i] = &ffi_type_longdouble;
                }
                
            } else if (it.type() == typeid(cs::string))
            {
                hosted_res.emplace_back(new resource_holder_impl<cs::string>(it.const_val<cs::string>()));
                arg_types[i] = &ffi_type_pointer;
            } else
                throw cs::runtime_error("Unsupported type in cffi.");
            arg_data[i] = hosted_res.back()->get_ptr();
        }
        ffi_cif cif;
        ffi_prep_cif(&cif, FFI_DEFAULT_ABI, args.size(), &ffi_type_void, arg_types.get());
        ffi_call(&cif, target_func, nullptr, arg_data.get());
        return cs::null_pointer;
    }
};

using dll_type = std::shared_ptr<cov::dll>;

CNI_ROOT_NAMESPACE {
	using namespace cs;   

    dll_type open_shared_lib(const std::string& path)
    {
        return std::make_shared<cov::dll>(path);
    }

    CNI(open_shared_lib)

    CNI_NAMESPACE(dll)
    {
        cs::callable get_function(const dll_type &dll, const std::string &name)
        {
            return cs::callable(ffi_callable{(void(*)())dll->get_address(name)});
        }

        CNI(get_function)
    }
}

CNI_ENABLE_TYPE_EXT(dll, dll_type)