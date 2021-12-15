#include "rrextab.hpp"

enum
{
	START=256,
	L_ARROW,
	ARROW,
	L_CHAIN,
	L_COMMA,
	L_OR,
	IDENT,
	OR,
	COMMA,
	CHAIN
};

std::map<std::string, int64_t> idents = {{"DO_RECURSION", DO_RECURSION}, {"DO_CALLBACK",DO_CALLBACK} };

struct lang_val
{
	int64_t prec_token;
	struct tagged_value_t
	{
		int type;
		struct value_t
		{
			value_t() {}
			value_t(const std::string &s) : str(s)
			{}
			std::string str;
			int64_t r[2];
		} value;
		tagged_value_t(std::string _str ) : value(_str)
		{ type = 1; }
		tagged_value_t(uint64_t a, uint64_t b)
		{
			type = 0;
			value.r[0] = a;
			value.r[1] = b;
		}
		tagged_value_t(const tagged_value_t& val)
		{
			type = val.type;
			if(val.type)
				value.str = val.value.str;
			else
			{
				value.r[0] = val.value.r[0];
				value.r[1] = val.value.r[1];
			}
		}
		tagged_value_t() : tagged_value_t(0,0)
		{}
		tagged_value_t& operator=(const lang_val::tagged_value_t& val)
		{
			if(type)
				value.str="";
			type = val.type;
			if(val.type)
				value.str = val.value.str;
			else
			{
				value.r[0] = val.value.r[0];
				value.r[1] = val.value.r[1];
			}
			return *this;
		}
	};
	std::vector<tagged_value_t> value;
};

void cast_ident(lang_val *val)
{
	if( val->value[0].type )
	{
		uint64_t c = val->value[0].value.str[0];
		val->value[0].~tagged_value_t();
		val->value[0].value.r[0] =
		val->value[0].value.r[1] = c;
		val->value[0].type = 0;
	}
}

void lang_callbacks(int64_t reduce, match_shared_t &m, circ_buf_t<int64_t, 3 > &next_redbuf, void *&lval, void *&rval )
{
	switch(reduce)
	{
		case L_ARROW:
		{
			m.redbuf->push_head(START);
			rval = new lang_val{L_ARROW, {}};
			//std::cout << "L_ARROW" << std::endl;
		}
		break;
		case L_CHAIN:
		{
			m.redbuf->push_head(START);
			rval = new lang_val{L_CHAIN, {}};
		}
		break;
		case L_COMMA:
		{
			m.redbuf->push_head(((lang_val *)lval)->prec_token);
			rval = new lang_val{L_COMMA, {}};
		}
		break;
		case L_OR:
		{
			m.redbuf->push_head(((lang_val *)lval)->prec_token);
			rval = new lang_val{L_OR, {}};
			//std::cout << "L_OR" << std::endl;
		}
		break;
		case IDENT:
		{
			std::string identstr;
			m.redbuf->push_head(((lang_val *)lval)->prec_token);
			if((*m.buf)[0] == '\'')
			{
				for(int i=1; i < m.ret[0]-m.offset+1; i++ )
				{
					if((*m.buf)[i] == '\'' )
					{
						m.ret[0]++;
						goto end_fetch_str;
					}
					identstr += (*m.buf)[i];
				}
				for(char c; (c = m.is->get()) != '\''; )
					identstr.push_back(c);
				end_fetch_str:
				((lang_val *)lval)->value.push_back(lang_val::tagged_value_t(identstr));
				//std::cout << "IDENT = '" << identstr << "'" << std::endl;
			}
			else
			{
				//TODO: optimize and remove loop
				for(int i=0; i < m.ret[0]-m.offset; i++ )
					identstr += (*m.buf)[i];
				auto ident_data = idents.find(identstr);
				if( ident_data == idents.end() )
					exit(EXIT_FAILURE);
				((lang_val *)lval)->value.push_back(lang_val::tagged_value_t(ident_data->second, ident_data->second));
				//std::cout << std::hex << "IDENT = 0x" << ident_data->second << std::endl; 
			}
		}
		break;
		case OR:
		{
			m.redbuf->push_head(((lang_val *)lval)->prec_token);
			cast_ident((lang_val *)lval);
			cast_ident((lang_val *)rval);
			((lang_val *)lval)->value[0].value.r[0] =
				( ((lang_val *)lval)->value[0].value.r[1] |=
				((lang_val *)rval)->value[0].value.r[0] );
			//std::cout << std::hex << "OR = 0x" <<  ((lang_val *)lval)->value[0].value.r[0] << std::endl; 
			delete (lang_val *)rval;
		}
		break;
		case COMMA:
		{
			m.redbuf->push_head(((lang_val *)lval)->prec_token);
			cast_ident((lang_val *)lval);
			cast_ident((lang_val *)rval);
			((lang_val *)lval)->value[0].value.r[1] = ((lang_val *)rval)->value[0].value.r[0];
			//std::cout << "COMMA" << std::endl;
			delete (lang_val *)rval;
		}
		break;
		case CHAIN:
		{	
			m.redbuf->push_head(START);
			((lang_val *)lval)->value.push_back( ((lang_val *)rval)->value[0] );
			delete (lang_val *)rval;
			//std::cout << "CHAIN" << std::endl;
		}
		break;
		case ARROW:
		{
			//don't m.redbuf->push_head(START);
			cast_ident((lang_val *)rval);
			std::cout << "rrex_insert({";
			for(int i=0; i < ((lang_val *)lval)->value.size()-1; i++ )
			{
				auto &e = ((lang_val *)lval)->value[i];
				if( e.type )
					for( auto c : e.value.str )
						std::cout << "{'" << c << "','" << c << "},";
				else
					std::cout << "{0x" << std::hex << e.value.r[0] << ",0x" << e.value.r[1] << "},";
			}
			{
				auto &e = ((lang_val *)lval)->value.back();
				if( e.type )
				{
					for(int i=0; i < e.value.str.size()-1; i++ )
					{
						auto &c = e.value.str[i];
						std::cout << "{'" << c << "','" << c << "'},";
					}
					{
						auto &c = e.value.str.back();
						std::cout << "{'" << c << "','" << c << "'}";
					}
				}
				else
					std::cout << "{0x" << std::hex << e.value.r[0] << ",0x" << e.value.r[1] << "}";
			}
			std::cout << "}, 0x" << std::hex << ((lang_val *)rval)->value[0].value.r[0] << " );";
			delete (lang_val *)rval;
			((lang_val *)lval)->value = {};
		}
		break;
	}
}

int main()
{
	rrex_insert({{START,ARROW}, {' ',' '}}, START );
	rrex_insert({{START,ARROW}, {'\t','\t'}}, START );
	rrex_insert({{L_OR | DO_CALLBACK | DO_RECURSION,L_OR | DO_CALLBACK | DO_RECURSION}, {' ',' '}}, L_OR | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{L_OR | DO_CALLBACK | DO_RECURSION,L_OR | DO_CALLBACK | DO_RECURSION}, {'\t','\t'}}, L_OR | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{L_COMMA | DO_CALLBACK | DO_RECURSION,L_COMMA | DO_CALLBACK | DO_RECURSION}, {' ',' '}}, L_COMMA | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{L_COMMA | DO_CALLBACK | DO_RECURSION,L_COMMA | DO_CALLBACK | DO_RECURSION}, {'\t','\t'}}, L_COMMA | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{L_ARROW | DO_CALLBACK | DO_RECURSION,L_ARROW | DO_CALLBACK | DO_RECURSION}, {' ',' '}}, L_ARROW | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{L_ARROW | DO_CALLBACK | DO_RECURSION,L_ARROW | DO_CALLBACK | DO_RECURSION}, {'\t','\t'}}, L_ARROW | DO_CALLBACK | DO_RECURSION );

	rrex_insert({{L_CHAIN | DO_CALLBACK | DO_RECURSION, L_CHAIN | DO_CALLBACK | DO_RECURSION}, {' ',' '}}, L_CHAIN | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{L_CHAIN | DO_CALLBACK | DO_RECURSION, L_CHAIN | DO_CALLBACK | DO_RECURSION}, {'\t','\t'}}, L_CHAIN | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{L_CHAIN | DO_CALLBACK | DO_RECURSION, L_CHAIN | DO_CALLBACK | DO_RECURSION}, {'-','-'},{'>','>'}}, L_ARROW | DO_CALLBACK | DO_RECURSION );

	rrex_insert({{START,L_OR}, {'a','z'}}, IDENT | DO_CALLBACK );
	rrex_insert({{START,L_OR}, {'A','Z'}}, IDENT | DO_CALLBACK );
	rrex_insert({{START,L_OR}, {'_','_'}}, IDENT | DO_CALLBACK );
	rrex_insert({{START,L_OR}, {'\'','\''}}, IDENT | DO_CALLBACK );
	rrex_insert({{IDENT | DO_CALLBACK,IDENT | DO_CALLBACK}, {'a','z'}}, IDENT | DO_CALLBACK );
	rrex_insert({{IDENT | DO_CALLBACK,IDENT | DO_CALLBACK}, {'A','Z'}}, IDENT | DO_CALLBACK );
	rrex_insert({{IDENT | DO_CALLBACK,IDENT | DO_CALLBACK}, {'_','_'}}, IDENT | DO_CALLBACK );

	rrex_insert({{START,L_COMMA}, {IDENT,OR}, {'|','|'} }, L_OR | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{START,L_COMMA}, {L_OR,L_OR}, {IDENT,IDENT} }, OR | DO_CALLBACK );

	rrex_insert({{START,L_CHAIN}, {IDENT,OR}, {',',','} }, L_COMMA | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{START,L_CHAIN}, {L_COMMA,L_COMMA}, {IDENT,OR} }, COMMA | DO_CALLBACK );

	rrex_insert({{START,START}, {IDENT,CHAIN}, {' ',' '} }, L_CHAIN | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{START,START}, {IDENT,CHAIN}, {'\t','\t'} }, L_CHAIN | DO_CALLBACK | DO_RECURSION );

	rrex_insert({{START,START}, {L_CHAIN,L_CHAIN}, {IDENT,COMMA}}, CHAIN | DO_CALLBACK );
	rrex_insert({{START,START}, {L_ARROW,L_ARROW}, {IDENT,OR}, {'\n','\n'}}, ARROW | DO_CALLBACK, true );
	std::cout << "rrex_tree sz:" << rrex_tree_size(rrex_main_tree_ptr) << std::endl;
	//std::cout << "START = " << START <<std::endl;
	int64_t ret[3]={0,-1};
	circ_buf_t<int64_t, 3 > redbuf; redbuf.push_head(START);
	match(rrex_main_tree_ptr, ret, matchbuf, redbuf, new lang_val{START, {}}, std::cin );
	std::cout << std::endl;	
	return 0;
}
