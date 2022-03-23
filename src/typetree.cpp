#include "typetree.hpp"

std::unordered_map<std::string, TypeTree > base_types;

TypeTree *get_type(TypeTree *ptr, uintptr_t op )
{
	auto f = ptr->ops.find(op);
	if( f != ptr->ops.end() )
		return f->second;
	else
		return NULL;
}

TypeTree *get_type_back(TypeTree *t, uintptr_t op )
{
	if( t->parent == NULL || op != t->back_op )
		return NULL;
	else
		return t->parent;
}

TypeTree *get_type(std::string base_type_name, const std::vector<uintptr_t> &ops )
{
	TypeTree *ptr = &base_types[base_type_name];
	for(auto e : ops )
	{
		auto f = ptr->ops.find(e);
		if( f != ptr->ops.end() )
			ptr = f->second;
		else
			return NULL;
	}
	return ptr;
}

TypeTree *add_type(TypeTree *ptr, uintptr_t op )
{
	auto f = ptr->ops.find(op);
	if( f != ptr->ops.end() )
		return NULL;
	else
	{
		auto p = ptr->ops[op] = new TypeTree;
		p->back_op = op;
		p->parent = ptr;
		return p;
	}
}

