#include "typestr.hpp"

typestr::typestr()
{
	_error = false;
	raw_data = "";
}

typestr::typestr(char type_code, std::string s )
{
	raw_data = type_code + s + (char)_end;
}

void typestr::add_op(char type_code, std::vector<typestr> op_params )
{
	if( (_type <= type_code) && (type_code <= _fun ) )
	{
		raw_data = (char)_end + raw_data;
		for(size_t i=op_params.size(); i > 0; i--  )
			raw_data = op_params[i-1].raw_data + raw_data;
	}
	raw_data = type_code + raw_data;
}

typestr typestr::check_op(typestr &op )
{
	bool in_name[2] = { false, false };
	size_t i=0, j=0;
	typestr retval;
	while( i < op.raw_data.size() )
	{
			
		if(!(
			in_name[0] && (in_name[0] = op.raw_data[i++] != _end) ||
			in_name[1] && (in_name[1] = raw_data[j++] != _end)
		))
		{

			if(!(  ( in_name[0] = (op.raw_data[i] == _name ) ) || ( in_name[1] = (raw_data[j] == _name) )  ))
			{
				if( op.raw_data[i] != raw_data[j] )
				{
					retval._error = true;
					return retval;
				}
				i++; j++;
			}
		}
	}
	retval.raw_data = raw_data.substr(j);
	return retval;
}

bool typestr::empty() const
{ return raw_data.empty(); }

bool typestr::error() const
{ return _error; }

std::string typestr::get_raw_data() const
{ return raw_data; }


int main()
{
	typestr tpstream(typestr::_type, "int" );
	tpstream.add_op( typestr::typeop::_fun, {typestr(typestr::_type, "float"), typestr(typestr::_type, "int") } );
	std::cout << tpstream.get_raw_data() << std::endl;
	typestr funcall; funcall.add_op( typestr::typeop::_fun, {typestr(typestr::_type, "float"), typestr(typestr::_name, "f"), typestr(typestr::_type, "int") } );
	std::cout << tpstream.check_op(funcall).get_raw_data() << std::endl;
	return 0;
}
