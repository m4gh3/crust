#pragma once

#include <any>
#include <iostream>
#include <deque>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>

class LazyfiedOstream
{
	public:
		LazyfiedOstream(std::ostream &out);
		LazyfiedOstream &operator<<(std::any &ref);	
		LazyfiedOstream &try_flush_refs();	
	private:
		std::ostream *ostr;
		std::deque<std::any *> refs;
};

