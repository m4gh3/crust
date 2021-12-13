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

struct lang_val
{
	int64_t prec_token;
	std::vector<std::string> value;
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
		case IDENT:
		{
			m.redbuf->push_head(((lang_val *)lval)->prec_token);
			((lang_val *)lval)->value = {""};
			std::string *retval = &(((lang_val *)lval)->value)[0];
			for(int i=0; i < m.ret[0]-m.offset; i++ )
				(*retval) += (*m.buf)[i];
			std::cout << " result:" << (*retval) << std::endl;
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
	rrex_insert({{L_COMMA,L_COMMA}, {' ',' '}}, L_COMMA );
	rrex_insert({{L_COMMA,L_COMMA}, {'\t','\t'}}, L_COMMA );
	rrex_insert({{L_ARROW,L_ARROW}, {' ',' '}}, L_ARROW );
	rrex_insert({{L_ARROW,L_ARROW}, {'\t','\t'}}, L_ARROW );
	rrex_insert({{L_CHAIN | DO_CALLBACK | DO_RECURSION, L_CHAIN | DO_CALLBACK | DO_RECURSION}, {' ',' '}}, L_CHAIN | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{L_CHAIN | DO_CALLBACK | DO_RECURSION, L_CHAIN | DO_CALLBACK | DO_RECURSION}, {'\t','\t'}}, L_CHAIN | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{L_CHAIN | DO_CALLBACK | DO_RECURSION, L_CHAIN | DO_CALLBACK | DO_RECURSION}, {'-','-'},{'>','>'}}, L_ARROW | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{START,L_COMMA}, {'a','z'}}, IDENT | DO_CALLBACK );
	rrex_insert({{START,L_COMMA}, {'A','Z'}}, IDENT | DO_CALLBACK );
	rrex_insert({{START,L_COMMA}, {'_','_'}}, IDENT | DO_CALLBACK );
	rrex_insert({{START,L_COMMA}, {'\'','\''}}, IDENT | DO_CALLBACK );
	rrex_insert({{IDENT | DO_CALLBACK,IDENT | DO_CALLBACK}, {'a','z'}}, IDENT | DO_CALLBACK );
	rrex_insert({{IDENT | DO_CALLBACK,IDENT | DO_CALLBACK}, {'A','Z'}}, IDENT | DO_CALLBACK );
	rrex_insert({{IDENT | DO_CALLBACK}, {'_','_'}}, IDENT | DO_CALLBACK );
	rrex_insert({{START,L_CHAIN}, {IDENT,IDENT}, {',',','} }, L_COMMA | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{START,L_CHAIN}, {L_COMMA,L_COMMA}, {IDENT,IDENT} }, COMMA | DO_CALLBACK );
	rrex_insert({{START,START}, {IDENT,CHAIN}, {' ',' '} }, L_CHAIN | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{START,START}, {IDENT,CHAIN}, {'\t','\t'} }, L_CHAIN | DO_CALLBACK | DO_RECURSION );
	//rrex_insert({{START,START}, {IDENT,COMMA}, {' ',' '} }, L_CHAIN | DO_CALLBACK | DO_RECURSION );
	//rrex_insert({{START,START}, {IDENT,COMMA}, {'\t','\t'} }, L_CHAIN | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{START,START}, {L_CHAIN,L_CHAIN}, {IDENT,COMMA}}, CHAIN | DO_CALLBACK );
	rrex_insert({{START,START}, {L_ARROW,L_ARROW}, {IDENT,IDENT}, {'\n','\n'}}, ARROW | DO_CALLBACK );
	std::cout << "rrex_tree sz:" << rrex_tree_size(rrex_main_tree_ptr) << std::endl;
	std::cout << "START = " << START <<std::endl;
	int64_t ret[3]={0,-1};
	circ_buf_t<int64_t, 3 > redbuf; redbuf.push_head(START);
	match(rrex_main_tree_ptr, ret, matchbuf, redbuf, new lang_val{START, {}}, std::cin );
	std::cout << std::endl;	
	return 0;
}
