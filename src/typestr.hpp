#include <iostream>
#include <vector>
#include <string>

class typestr
{
	public:

		enum typeop
		{
			_end,
			_ref, _bpref,
			_const, _ptr,
			_name, _type, _struct, _fun
		};

		typestr();

		typestr(char type_code, std::string s );

		void add_op(char type_code, std::vector<typestr> op_params );	

		typestr check_op(typestr &op );

		bool empty() const;
		bool error() const;

		std::string get_raw_data() const;	

	private:	
		bool _error;
		std::string raw_data;

};

