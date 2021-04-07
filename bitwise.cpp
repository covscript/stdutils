#include <covscript/cni.hpp>
#include <covscript/dll.hpp>
#include <cstdint>
#include <bitset>

using bitset_t = std::bitset<std::numeric_limits<std::uint64_t>::digits>;

CNI_ROOT_NAMESPACE {
	bitset_t from_string(const std::string &data)
	{
		if (data.length() > 16)
			throw cs::lang_error("Wrong literal(length > 16).");
		std::uint64_t hex = 0;
		auto current = data.c_str() + 2;
		auto end = data.c_str() + data.length();

		while (current < end && ((*current >= '0' && *current <= '9')
		                         || (*current >= 'a' && *current <= 'f')
		                         || (*current >= 'A' && *current <= 'F'))) {
			hex = hex * 16
			+ (*current & 15U)
			+ (*current >= 'A' ? 9 : 0);
			++current;
		}

		if (current != end)
			throw cs::lang_error("Wrong literal.");
		return bitset_t(hex);
	}

	CNI_CONST_V(hex_literal, from_string)

	CNI_TYPE_EXT(bitset, bitset_t, bitset_t())
	{
		CNI_CONST_V(test,  &bitset_t::test)
		CNI_CONST_V(all,   &bitset_t::all)
		CNI_CONST_V(any,   &bitset_t::any)
		CNI_CONST_V(none,  &bitset_t::none)
		CNI_CONST_V(count, &bitset_t::count)
		CNI_CONST_V(set_all, [](bitset_t &val) {
			return val.set();
		})
		CNI_CONST_V(set, [](bitset_t &val, std::size_t pos) {
			return val.set(pos);
		})
		CNI_CONST_V(reset_all, [](bitset_t &val) {
			return val.reset();
		})
		CNI_CONST_V(reset, [](bitset_t &val, std::size_t pos) {
			return val.reset(pos);
		})
		CNI_CONST_V(flip_all, [](bitset_t &val) {
			return val.flip();
		})
		CNI_CONST_V(flip, [](bitset_t &val, std::size_t pos) {
			return val.flip(pos);
		})
		CNI_CONST_V(logic_and, [](const bitset_t &lhs, const bitset_t &rhs) {
			return lhs & rhs;
		})
		CNI_CONST_V(logic_or, [](const bitset_t &lhs, const bitset_t &rhs) {
			return lhs | rhs;
		})
		CNI_CONST_V(logic_xor, [](const bitset_t &lhs, const bitset_t &rhs) {
			return lhs ^ rhs;
		})
		CNI_CONST_V(logic_not, [](const bitset_t &lhs) {
			return ~lhs;
		})
		CNI_CONST_V(shift_left, [](const bitset_t &val, std::size_t pos) {
			return val << pos;
		})
		CNI_CONST_V(shift_right, [](const bitset_t &val, std::size_t pos) {
			return val >> pos;
		})
		CNI_CONST_V(to_hash, [](const bitset_t &val) -> std::size_t {
			return val.to_ullong();
		})
		CNI_CONST_V(to_number, [](const bitset_t &val) -> cs::number {
			return val.to_ulong();
		})
		CNI_CONST_V(to_string, [](const bitset_t &val) {
			return val.to_string();
		})
		CNI_CONST_V(from_number, [](cs::number val) {
			if (val < 0)
				throw cs::lang_error("Bitset value can not under zero.");
			return bitset_t(static_cast<std::uint64_t>(val));
		})
		CNI_CONST(from_string)
	}
}

CNI_ENABLE_TYPE_EXT_V(bitset, bitset_t, cs::bitset)