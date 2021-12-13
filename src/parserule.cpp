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
		union value_t
		{
			std::string str;
			int64_t r[2];
			~value_t(){}
		} value;
		~tagged_value_t()
		{
			if(type)
				value.str.std::string::~string();
		}		
	};
	std::vector<tagged_value_t> value;
};

void lang_callbacks(int64_t reduce, match_shared_t &m, circ_buf_t<int64_t, 3 > &next_redbuf, void *&lval, void *&rval )
{
	switch(reduce)
	{
		case L_ARROW:
		{
			m.redbuf->push_head(START);
			rval = new lang_val{L_ARROW, {}};
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
			std::cout << "L_OR" << std::endl;
		}
		break;
		case IDENT:
		{
			std::string identstr;
			m.redbuf->push_head(((lang_val *)lval)->prec_token);
			if((*m.buf)[0] == '\'')
			{
				for(int i=1; i < m.ret[0]-m.offset; i++ )
				{
					if((*m.buf)[i] == '\'' )
						goto end_fetch_str;
					identstr += (*m.buf)[i];
				}
				for(char c; (c = m.is->get()) != '\''; )
					identstr.push_back(c);
				end_fetch_str:
				((lang_val *)lval)->value.push_back({0,{.str=identstr}});
			}
			else
			{
				//TODO: optimize and remove loop
				for(int i=0; i < m.ret[0]-m.offset; i++ )
					identstr += (*m.buf)[i];
				auto ident_data = idents.find(identstr);
				if( ident_data == idents.end() )
					exit(EXIT_FAILURE);
				((lang_val *)lval)->value.push_back({0,{.r={ident_data->second, ident_data->second}}});
			}
		}
		break;
		case OR:
		{
			m.redbuf->push_head(((lang_val *)lval)->prec_token);
			if( ((lang_val *)lval)->value[0].type )
			{
				((lang_val *)lval)->value[0].value.r[0] =
					((lang_val *)lval)->value[0].value.r[1] =
					((lang_val *)lval)->value[0].value.str[0];
				((lang_val *)lval)->value[0].type = 0;
			}
			if( ((lang_val *)rval)->value[0].type )
			{
				((lang_val *)rval)->value[0].value.r[0] =
					((lang_val *)rval)->value[0].value.r[1] =
					((lang_val *)rval)->value[0].value.str[0];
				((lang_val *)rval)->value[0].type = 0;
			}
			delete (lang_val *)rval;
		}
		break;
		case COMMA:
		{
			m.redbuf->push_head(((lang_val *)lval)->prec_token);
			((lang_val *)lval)->value[0] += "," + ((lang_val *)rval)->value[0];
			delete (lang_val *)rval;
			std::cout << " result:" << ((lang_val *)lval)->value[0] << std::endl;
		}
		break;
		case CHAIN:
		{	
			m.redbuf->push_head(START);
			((lang_val *)lval)->value.push_back( ((lang_val *)rval)->value[0] );
			delete (lang_val *)rval;
			std::cout << " result:";
			for(auto &str : ((lang_val *)lval)->value )
				std::cout << ' ' << str;
			std::cout << std::endl;
		}
		break;
		case ARROW:
		{
			//m.redbuf->push_head(START);
			std::cout << "result:";
			for(auto &str : ((lang_val *)lval)->value )
				std::cout << ' ' << str;
			std::cout << " ->" << ((lang_val *)rval)->value[0] << std::endl;
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
	rrex_insert({{L_OR,L_OR}, {' ',' '}}, L_OR );
	rrex_insert({{L_OR,L_OR}, {'\t','\t'}}, L_OR );
	rrex_insert({{L_COMMA,L_COMMA}, {' ',' '}}, L_COMMA );
	rrex_insert({{L_COMMA,L_COMMA}, {'\t','\t'}}, L_COMMA );
	rrex_insert({{L_ARROW,L_ARROW}, {' ',' '}}, L_ARROW );
	rrex_insert({{L_ARROW,L_ARROW}, {'\t','\t'}}, L_ARROW );

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
	rrex_insert({{START,START}, {L_ARROW,L_ARROW}, {IDENT,OR}, {'\n','\n'}}, ARROW | DO_CALLBACK );
	std::cout << "rrex_tree sz:" << rrex_tree_size(rrex_main_tree_ptr) << std::endl;
	std::cout << "START = " << START <<std::endl;
	int64_t ret[3]={0,-1};
	circ_buf_t<int64_t, 3 > redbuf; redbuf.push_head(START);
	match(rrex_main_tree_ptr, ret, matchbuf, redbuf, new lang_val{START, {}}, std::cin );
	std::cout << std::endl;	
	return 0;
}
