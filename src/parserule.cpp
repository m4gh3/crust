#include "rrextab.hpp"


std::map<std::string, int64_t> idents = {{"DO_RECURSION", DO_RECURSION}, {"DO_CALLBACK",DO_CALLBACK} };



int main()
{
		std::cout << "rrex_tree sz:" << rrex_tree_size(rrex_main_tree_ptr) << std::endl;
	//std::cout << "START = " << START <<std::endl;
		std::cout << std::endl;	
	return 0;
}
