#include <covscript/cni.hpp>
#include <covscript/covscript.hpp>
#include <covscript/dll.hpp>
#include <iostream>

#ifdef COVSCRIPT_PLATFORM_WIN32

#include <windows.h>

bool ctrlhandler(DWORD fdwctrltype)
{
	switch (fdwctrltype) {
	case CTRL_C_EVENT:
		std::cout << "Keyboard Interrupt (Ctrl+C Received)" << std::endl;
		cs::current_process->raise_sigint();
		return true;
	case CTRL_BREAK_EVENT: {
		int code = 0;
		cs::process_context::on_process_exit_default_handler(&code);
		return true;
	}
	default:
		return false;
	}
}

void activate_sigint_handler()
{
	::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrlhandler, true);
}

#else

#include <signal.h>
#include <unistd.h>

void signal_handler(int sig)
{
	std::cout << "Keyboard Interrupt (Ctrl+C Received)" << std::endl;
	cs::current_process->raise_sigint();
}

void activate_sigint_handler()
{
	struct sigaction sa_usr {};
	sa_usr.sa_handler = &signal_handler;
	sigemptyset(&sa_usr.sa_mask);
	// sa_usr.sa_flags = SA_RESTART | SA_NODEFER;
	sigaction(SIGINT, &sa_usr, NULL);
}

#endif

class repl_instance final {
	cs::context_t context;
	cs::raii_collector context_gc;

public:
	cs::repl repl_impl;

	repl_instance(const cs::array &args)
		: context(cs::create_context(args)),
		  context_gc(context),
		  repl_impl(context)
	{
		activate_sigint_handler();
		cs::current_process->on_process_exit.add_listener([](void *code) -> bool {
			cs::current_process->exit_code = *static_cast<int *>(code);
			throw cs::fatal_error("CS_EXIT");
		});
		cs::current_process->on_process_sigint.add_listener(
		    [](void *) -> bool { throw cs::fatal_error("CS_SIGINT"); });
		cs::current_process->on_process_sigint.add_listener([](void *) -> bool {
			std::cin.clear();
			return false;
		});
	}
	std::string readline()
	{
		std::string line;
#ifdef COVSCRIPT_PLATFORM_WIN32
		// Workaround: https://stackoverflow.com/a/26763490
		while (true) {
			cs::current_process->poll_event();
			std::getline(std::cin, line);
			if (std::cin) break;
		}
#else
		if (!std::cin) {
			int code = 0;
			cs::process_context::on_process_exit_default_handler(&code);
		}
		std::getline(std::cin, line);
		cs::current_process->poll_event();
#endif
		return std::move(line);
	}
	bool exec(const std::string &code)
	{
		try {
			repl_impl.exec(code);
		}
		catch (const std::exception &e) {
			if (std::strstr(e.what(), "CS_SIGINT") != nullptr) {
				cs::process_context::cleanup_context();
				repl_impl.reset_status();
				activate_sigint_handler();
			}
			else if (std::strstr(e.what(), "CS_EXIT") == nullptr)
				throw cs::lang_error(e.what());
			else
				return false;
		}
		catch (...) {
			throw cs::lang_error("Uncaught exception: Unknown exception");
		}
		return true;
	}
};

using repl_instance_t = std::shared_ptr<repl_instance>;

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
		CNI_VALUE_CONST(structure, type_id(typeid(cs::structure)))

		type_id get_real(const var &val) {
			return type_id(val.type());
		}

		CNI(get_real)
	}

	CNI_NAMESPACE(variable)
	{
		bool is_rvalue(const var &val) {
			return val.is_rvalue();
		}

		CNI(is_rvalue)

		bool is_protect(const var &val) {
			return val.is_protect();
		}

		CNI(is_protect)

		bool is_constant(const var &val) {
			return val.is_constant();
		}

		CNI(is_constant)

		bool is_single(const var &val) {
			return val.is_single();
		}

		CNI(is_single)
	}

	CNI_NAMESPACE(function)
	{
		const var & get_callable(const cs::object_method &om) {
			return om.callable;
		}

		CNI(get_callable)

		string get_type(const cs::callable &func) {
			switch (func.type()) {
			/**
			 * A Regular function
			 */
			case callable::types::normal:
				return "regular";
			/**
			 * A Foldable(marker for optimizer) regular function
			 */
			case callable::types::request_fold:
				return "constant";
			/**
			 * A Member function
			 */
			case callable::types::member_fn:
				return "member_function";
			/**
			 * Automatically called member visitor
			 */
			case callable::types::member_visitor:
				return "member_visitor";
			/**
			 * A regular function cannot convert into object method
			 */
			case callable::types::force_regular:
				return "force_regular";
			}
			return "null";
		}

		CNI(get_type)
	}

	var catch_stdexcept(const var &func, const cs::array &argument)
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

	template <typename T>
	var predict_domain(T &&domain)
	{
		cs::var ret = cs::var::make<cs::hash_set>();
		cs::hash_set &set = ret.val<cs::hash_set>();
		for (auto &it : domain) set.insert(cs::var::make<cs::string>(it.first));
		return ret;
	}

	var predict_symbols(const var &a)
	{
		if (a.type() == typeid(namespace_t))
			return predict_domain(a.val<namespace_t>()->get_domain());
		else if (a.type() == typeid(type_t))
			return predict_domain(a.const_val<type_t>().extensions->get_domain());
		else if (a.type() == typeid(structure))
			return predict_domain(a.val<structure>().get_domain());
		else
			return predict_domain(a.get_ext()->get_domain());
	}

	CNI(predict_symbols)

	var predict_current_symbols(const cs::context_t &cxt)
	{
		return predict_domain(cxt->instance->storage.get_domain());
	}

	CNI(predict_current_symbols)

	var predict_global_symbols(const cs::context_t &cxt)
	{
		return predict_domain(cxt->instance->storage.get_global());
	}

	CNI(predict_global_symbols)

	void extend_type(cs::type_t &type, const std::string &name, const var &obj)
	{
		type.extensions->add_var(name, obj);
	}

	CNI(extend_type)

	CNI_NAMESPACE(repl)
	{
		repl_instance_t create(const cs::array &args) {
			return std::make_shared<repl_instance>(args);
		}
		CNI(create)
		std::string readline(repl_instance_t & repl) {
			return repl->readline();
		}
		CNI(readline)
		bool exec(repl_instance_t & repl, const std::string &code) {
			return repl->exec(code);
		}
		CNI(exec)
		void reset(repl_instance_t & repl) {
			repl->repl_impl.reset_status();
		}
		CNI(reset)
		void echo(repl_instance_t & repl, bool value) {
			repl->repl_impl.echo = value;
		}
		CNI(echo)
	}
}

CNI_ENABLE_TYPE_EXT(repl, repl_instance_t)
