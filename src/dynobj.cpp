#include <string>

#include <unordered_map>
#include <map>

#include <iostream>

class dyn_obj
{

	public:

		enum class dyn_obj_type
		{
			OBJ,
			DYN_PTR,
			INT,
			STR
		} type;	

		dyn_obj()
		{
			value.obj.by_str = new std::unordered_map<std::string, dyn_obj >;
			value.obj.by_num = new std::map<int, dyn_obj >;
			type = dyn_obj_type::OBJ;
		}
		
		dyn_obj(dyn_obj *p)
		{
			value.ptr = p;
			type = dyn_obj_type::DYN_PTR;
		}

		dyn_obj(int i)
		{
			value.i = i;
			type = dyn_obj_type::INT;
		}
	
		dyn_obj(const std::string &s)
		{
			new (&value.str) std::string(s);
			type = dyn_obj_type::STR;
		}

		dyn_obj &operator=(const dyn_obj &dobj)
		{
			//this->~dyn_obj();
			type = dobj.type;
			switch( dobj.type )
			{
				case dyn_obj_type::OBJ:
					//deep copy
					value.obj.by_str = new std::unordered_map<std::string, dyn_obj >;
					for( auto &pair : *dobj.value.obj.by_str )
						(*value.obj.by_str)[pair.first] = pair.second;
					for( auto &pair : *dobj.value.obj.by_num )
						(*value.obj.by_num)[pair.first] = pair.second;
					break;
				case dyn_obj_type::DYN_PTR:
					value.ptr = dobj.value.ptr;
					break;
				case dyn_obj_type::INT:
					value.i = dobj.value.i;
					break;
				case dyn_obj_type::STR:
					value.str = dobj.value.str;
					break;
			}
			return *this;
		}
		
		dyn_obj &operator=(dyn_obj *ptr)
		{ return (*this)=dyn_obj(ptr); }

		dyn_obj &operator=(const int i)
		{ return (*this)=dyn_obj(i); }

		dyn_obj &operator=(const std::string &s)
		{ return (*this)=dyn_obj(s); }	
		
		~dyn_obj()
		{
			if( type == dyn_obj_type::OBJ )
				delete value.obj.by_str;
			if( type == dyn_obj_type::STR )
				value.str.std::string::~string();
		}

		bool is_obj()
		{ return type == dyn_obj_type::OBJ; }

		bool is_ptr()
		{ return type == dyn_obj_type::DYN_PTR; }

		bool is_int()
		{ return type == dyn_obj_type::INT; }


		bool is_str()
		{ return type == dyn_obj_type::STR; }	

		dyn_obj *get_ptr()
		{
			if( type == dyn_obj_type::DYN_PTR )
				return value.ptr;
			else
				return NULL;
		}

		int get_int()
		{
			if( type == dyn_obj_type::INT )
				return value.i;
			else
				return 0;
		}
	
		std::string get_str()
		{
			if( type == dyn_obj_type::STR )
				return value.str;
			else
				return "";
		}
	
		dyn_obj &operator[](const std::string &key)
		{ return (*value.obj.by_str)[key]; }

		dyn_obj &operator[](const int key)
		{ return (*value.obj.by_num)[key]; }

	private:

		union value_union
		{
			value_union() {}
			~value_union(){}
			struct
			{
				std::unordered_map<std::string, dyn_obj > *by_str;
				std::map<int, dyn_obj > *by_num;
			} obj;
			dyn_obj *ptr;
			int i;
			std::string str;
		} value;
	
};

int main()
{
	dyn_obj d;
	dyn_obj subobj; subobj = new dyn_obj;
	(*subobj.get_ptr())["birth_year"] = 2000;
	(*subobj.get_ptr())["continent"] = "Europe";
	(*subobj.get_ptr())[0] = "Lol";
	d["name"] = "m4gh3";
	d["info"] = subobj;
	std::cout << d["name"].get_str() << std::endl;
	std::cout << (*d["info"].get_ptr())["birth_year"].get_int() << std::endl;
	return 0;
}
