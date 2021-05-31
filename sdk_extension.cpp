#include <covscript/cni.hpp>
#include <covscript/dll.hpp>

CNI_ROOT_NAMESPACE {
	using namespace cs;

	CNI_NAMESPACE(typeids)
	{
        CNI_VALUE_CONST(type, type_id(typeid(cs::type_t)))
        CNI_VALUE_CONST(range, type_id(typeid(cs::range_type)))
        CNI_VALUE_CONST(istream, type_id(typeid(cs::istream)))
        CNI_VALUE_CONST(ostream, type_id(typeid(cs::ostream)))
        CNI_VALUE_CONST(context, type_id(typeid(cs::context_t)))
        CNI_VALUE_CONST(callable, type_id(typeid(cs::callable)))
        CNI_VALUE_CONST(memberfn, type_id(typeid(cs::object_method)))
        CNI_VALUE_CONST(exception, type_id(typeid(cs::lang_error)))
        CNI_VALUE_CONST(namespace, type_id(typeid(cs::namespace_t)))
	}

	var catch_stdexcept(const var& func, const cs::array &argument)
	{
		try {
			if (func.type() == typeid(cs::callable)) {
				cs::vector args(argument.begin(), argument.end());
				return func.const_val<callable>().call(args);
			}
			else if (func.type() == typeid(cs::object_method)) {
				const auto &om = func.const_val<cs::object_method>();
				cs::vector args{om.object};
				args.insert(args.end(), argument.begin(), argument.end());
				return om.callable.const_val<callable>().call(args);
			}
			else
				throw cs::lang_error("Invoke non-callable object.");
		}
		catch (const std::exception &e) {
			throw lang_error(e.what());
		}
	}

	CNI(catch_stdexcept)
}