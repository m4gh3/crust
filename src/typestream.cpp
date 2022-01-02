#include <iostream>
#include <vector>
#include <string>

class typestream
{
	public:
		enum typeop
		{
			_end,
			_ref, _bpref,
			_const, _ptr,
			_name, _type, _struct, _fun
		};
		typestream()
		{
			error = false;
			raw_type_data = "";
		}
		typestream(char type_code, std::string s )
		{
			raw_type_data = type_code + s + (char)_end;
		}
		void add_op(char type_code, std::vector<typestream> op_params )
		{
			if( (_type <= type_code) && (type_code <= _fun ) )
			{
				raw_type_data = (char)_end + raw_type_data;
				for(size_t i=op_params.size(); i > 0; i--  )
					raw_type_data = op_params[i-1].raw_type_data + raw_type_data;
			}
			raw_type_data = type_code + raw_type_data;
		}
		typestream check_op(typestream &op )
		{
			bool in_name[2] = { false, false };
			size_t i=0, j=0;
			typestream retval;
			while( i < op.raw_type_data.size() )
			{
				
				if(!(
					in_name[0] && (in_name[0] = op.raw_type_data[i++] != _end) ||
					in_name[1] && (in_name[1] = raw_type_data[j++] != _end)
				))
				{

					if(!(  ( in_name[0] = (op.raw_type_data[i] == _name ) ) || ( in_name[1] = (raw_type_data[j] == _name) )  ))
					{
						if( op.raw_type_data[i] != raw_type_data[j] )
						{
							retval.error = true;
							return retval;
						}
						i++; j++;
					}
				}
			}
			retval.raw_type_data = raw_type_data.substr(j);
			return retval;
		}
	//private:	
		bool error = false;
		std::string raw_type_data;
};

int main()
{
	typestream tpstream(typestream::_type, "int" );
	tpstream.add_op( typestream::typeop::_fun, {typestream(typestream::_type, "float"), typestream(typestream::_type, "int") } );
	std::cout << tpstream.raw_type_data << std::endl;
	typestream funcall; funcall.add_op( typestream::typeop::_fun, {typestream(typestream::_type, "float"), typestream(typestream::_name, "f"), typestream(typestream::_type, "int") } );
	std::cout << tpstream.check_op(funcall).raw_type_data << std::endl;
	return 0;
}
