#include <unordered_map>
#include <vector>
#include <cstdint>
#include <string>
#include <iostream>

enum TypeOps
{
	NONE,
	PTR,
	ARR,
	CALL
};

struct TypeTree
{
	std::unordered_map<uintptr_t, TypeTree* > ops;
	uintptr_t back_op = TypeOps::NONE;
	TypeTree *parent = NULL;
};

extern std::unordered_map<std::string, TypeTree > base_types;

TypeTree *get_type(TypeTree *ptr, uintptr_t op );
TypeTree *get_type_back(TypeTree *t, uintptr_t op );
TypeTree *get_type(std::string base_type_name, const std::vector<uintptr_t> &ops );
TypeTree *add_type(TypeTree *ptr, uintptr_t op );
