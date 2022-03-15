#include "lazyfied_ostream.hpp"

#define output_func_entry(type) { typeid(type), base_print_func<type> }

template<typename T> static void base_print_func(std::ostream &out, std::any *a )
{ out << std::any_cast<T>(*a); }

static std::unordered_map<std::type_index, void (*)(std::ostream &, std::any *a)> output_func_table = std::unordered_map<std::type_index, void (*)(std::ostream &, std::any *a)>{
	output_func_entry(size_t),
	output_func_entry(int),
	output_func_entry(float),
	output_func_entry(std::string),
	output_func_entry(const char *)
};

LazyfiedOstream::LazyfiedOstream(std::ostream &out)
{ ostr = &out; }

LazyfiedOstream &LazyfiedOstream::operator<<(std::any &ref)
{
	refs.push_back(&ref);
	return try_flush_refs();
}

LazyfiedOstream &LazyfiedOstream::try_flush_refs()
{
	while( refs.size() && refs[0]->has_value() )
	{
		output_func_table[refs[0]->type()](*ostr, refs[0] );
		refs.pop_front();
	}
	return *this;
}
