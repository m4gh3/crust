#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <cstdint>

struct rrex_key
{
	int64_t a,b;
	//if rrex_key X is included in or equal to rrex_key Y then X <= Y
	bool operator<(const struct rrex_key s) const
	{
		if(b<s.b)
			return true;
		if(b>s.b)
			return false;
		if(a>s.a)
			return true;
		return false;
	}
	bool operator==(const struct rrex_key s) const
	{ return (a == s.a) && (b == s.b); }
	void print()
	{ std::cout << "('" << a << "','" << b << "')"; }
};

struct rrex_data
{
	int64_t reduce = -1;
	bool glow=false;
	std::map<rrex_key, rrex_data > *next = nullptr;
};

int last_reduce=1;

typedef std::map<rrex_key, rrex_data > rrex_tree;

template<typename T, unsigned int pow >  struct circ_buf_t
{
	T buf[1<<pow];
	int s=0, e=1, sz = 1<<pow;
	int sz_ = 0;
	void push_back(T k)
	{
		buf[e] = k;
		e++; e &= (sz-1);
		sz_++;
	}
	T pop_back()
	{
		e--; e &= (sz-1);
		T k = buf[e];
		sz_--;
		return k;
	}
	void push_head(T k)
	{
		buf[s] = k;
		s--; s &= (sz-1);
		sz_++;
	}
	T pop_head(int n)
	{
		s+=n; s &= (sz-1);
		T k = buf[(s+1)&(sz-1)];
		sz_-=n;
		return k;
	}
	bool is_empty()
	{ return e == (s+1 & (sz-1)); }
	bool is_full()
	{ return ( s == e ); }
	T back()
	{
		return buf[e-1 & (sz-1)];
	}
	void clear()
	{ s=0; e=1; sz_ = 0; }
	T operator[](size_t i)
	{
		return buf[s+1+i & (sz-1)];
	}
	size_t size()
	{ return sz_; }
};


circ_buf_t<char, 10> matchbuf;

rrex_tree rrex_main_tree;
rrex_tree *rrex_main_tree_ptr = &rrex_main_tree;

int rrex_insert(std::vector<rrex_key> &rrex, int64_t reduce=0, bool glow=false )
{

	std::map<rrex_key, rrex_data > **next = &rrex_main_tree_ptr;

	for(int i=0; i < rrex.size(); i++ )
	{

		auto &e = rrex[i];

		if( *next == nullptr )
			*next = new std::map<rrex_key, rrex_data >;

		auto &data = (**next)[e];

		if( i == rrex.size()-1 )
			data.reduce = reduce;
		data.glow |= glow;
	
		next = &data.next;

	}

	return reduce;

}

int rrex_tree_size(rrex_tree *root)
{
	if( root == nullptr )
		return 0;
	int size = root->size()*4+1;
	for(auto it = root->begin(); it != root->end(); it++ )
		size += rrex_tree_size(it->second.next);
	return size;
}

int rrex_insert(std::vector<rrex_key> &&rrex, int64_t reduce=0, bool glow=false )
{ return rrex_insert(rrex, reduce, glow ); }

struct match_shared_t
{
	int64_t *ret;
	rrex_tree *root;
	struct circ_buf_t<char, 10 > *buf;
	struct circ_buf_t<int64_t, 3 > *redbuf;
	std::istream *is;
	int offset;
};

#define DO_CALLBACK 	0x80000000 
#define DO_RECURSION 	0x40000000
#define REDMASK		0x3fffffff

void lang_callbacks(int64_t, match_shared_t &, circ_buf_t<int64_t, 3 >&, void *&, void *& );

void match(match_shared_t &m, rrex_tree *next, int idx=0 )
{	
	match_start:

		int64_t c;
		if( m.redbuf->size() > m.offset )
			c = m.redbuf->pop_back();
		else
		{
			if( idx < m.offset )
				c = (*m.redbuf)[idx];
			else if( idx-m.offset == m.buf->size() )
			{
				if( m.buf->is_full() )
					exit(EXIT_FAILURE);
				m.buf->push_back(m.is->get());
				c = m.buf->back();
				//idx++;
			}
			else
				c = (*m.buf)[idx-m.offset/*++*/];
			idx++;
		}

		std::cout << "c=" << c << ',';

		int to_check = 0;

		for( auto it = next->lower_bound({c,c}); it != next->end(); it++ )
			if( it->first.a <= c && c <= it->first.b )
				to_check++;

		for( auto it = next->lower_bound({c,c}); it != next->end() && to_check; it++ )
		{
			if(it->second.glow)
				std::cout << "g, ";
			if( it->first.a <= c && c <= it->first.b )
			{
				if( it->second.next != nullptr )
				{
					if( to_check == 1 && (it->second.reduce < 0) )
					{
						next = it->second.next;
						goto match_start;
					}
					std::cout << "{ ";
						match(m, it->second.next, idx );
					std::cout << "} ";
				}
				if( it->second.reduce >= 0 )
				{
					if( idx/*+m.offset*/ > m.ret[0] )
					{
						m.ret[0] = idx;
						m.ret[1] = it->second.reduce;
					}
					std::cout << "m, ";
					m.redbuf->push_head(it->second.reduce);
					if( to_check == 1 )
					{
						next = m.root;
						goto match_start;
					}
					std::cout << "{ ";
						match(m, m.root, idx );
					std::cout << "} ";
				}
				to_check--;
			}
		}
}


void *match(rrex_tree *root, int64_t *ret, circ_buf_t<char, 10 > &buf, circ_buf_t<int64_t, 3 > &redbuf, void *lval, std::istream &is, int idx=0 )
{
	//void *lval = NULL;
	void *rval = NULL;
	int64_t last_good=-1;
	circ_buf_t<int64_t, 3 > next_redbuf;
	match_shared_t m{ret, root, &buf, &redbuf, &is };
	while( /*redbuf.size()*/ true )
	{
		//last_good = redbuf[0];
		ret[0] = 0; ret[1] = -1;
		m.offset = redbuf.size();
		match(m, root, idx );
		std::cout << "\nlen:" << ret[0] << ", reduce:" << (ret[1] & REDMASK) << ' ' << (ret[1]&DO_CALLBACK ? 'c' : ' ') << (ret[1]&DO_RECURSION ? 'r' : ' ') << std::endl;
		redbuf.clear();

		if( ret[1] >= 0 )
		{
			last_good = ret[1];
			redbuf.push_back(last_good & REDMASK);
			if( last_good & DO_CALLBACK )
				lang_callbacks(last_good & REDMASK, m, next_redbuf, lval, rval );
			buf.pop_head(m.ret[0]-m.offset);
			ret[0] = 0; ret[1] = -1;
			if( last_good & DO_RECURSION )
			{
				next_redbuf.push_back(last_good & REDMASK);
				rval = match(root, ret, buf, next_redbuf, rval, is );
				redbuf.push_back(ret[1]);
			}
		}
		else
			break;
	}
	ret[1] = last_good & REDMASK;
	return lval;
}


enum
{
	START = 256,
	L_PAR,
	L_SUM,
	L_PROD,
	NUM,
	PAR,
	PROD,
	SUM	
};

struct lang_val
{
	int64_t prec_token;
	int value;
};


void lang_callbacks(int64_t reduce, match_shared_t &m, circ_buf_t<int64_t, 3 > &next_redbuf, void *&lval, void *&rval )
{
	switch(reduce)
	{
		case L_PAR:
			{
				//m.redbuf->push_head(PAR);
				m.redbuf->push_head(((lang_val *)lval)->prec_token);
				rval = new lang_val{START, 0};
			}
			break;
		case L_SUM:
			{
				m.redbuf->push_head(START);
				rval = new lang_val{L_SUM, 0};
			}
			break;
		case L_PROD:
			{
				m.redbuf->push_head(((lang_val *)lval)->prec_token);
				rval = new lang_val{L_PROD, 0};
			}
			break;
		case NUM:
			{
				if( lval != NULL )
				{
					m.redbuf->push_head(((lang_val *)lval)->prec_token);
					((lang_val *)lval)->value = 0;
				}
				else
				{
					lval = new lang_val{START, 0 };
					m.redbuf->push_head(START);
				}
				int *retval = &(((lang_val *)lval)->value);
				for(int i=0; i < m.ret[0]-m.offset; i++ )
					((*retval) *= 10)+=((*m.buf)[i]-'0');
				std::cout << (*retval) << std::endl;
			}
			break;
		case PAR:
				m.redbuf->push_head(((lang_val *)lval)->prec_token);
				((lang_val *)lval)->value = ((lang_val *)rval)->value;

			break;
		case PROD:
			{
				m.redbuf->push_head(((lang_val *)lval)->prec_token);
				((lang_val *)lval)->value *= ((lang_val *)rval)->value;
				delete (lang_val *)rval;
				std::cout << ((lang_val *)lval)->value;
			}
			break;
		case SUM:
			{
				m.redbuf->push_head(START);
				((lang_val *)lval)->value += ((lang_val *)rval)->value;
				delete (lang_val *)rval;
				std::cout << ((lang_val *)lval)->value;
			}
			break;
	}
}

int main()
{
	rrex_insert({{START,L_PROD}, {'0','9'}}, NUM | DO_CALLBACK );
	rrex_insert({{START,L_PROD}, {'(','('}}, L_PAR | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{START,L_PROD}, {L_PAR,L_PAR}, {')',')'}}, PAR | DO_CALLBACK );
	rrex_insert({{NUM | DO_CALLBACK, NUM | DO_CALLBACK}, {'0','9'}}, NUM | DO_CALLBACK, true );
	rrex_insert({{START,START}, {NUM, SUM}, {'+','+'}}, L_SUM | DO_CALLBACK | DO_RECURSION, true );
	rrex_insert({{START,L_SUM}, {NUM,PROD}, {'*','*'}}, L_PROD | DO_CALLBACK | DO_RECURSION );
	rrex_insert({{START,START}, {L_SUM,L_SUM}, {NUM,PROD}}, SUM | DO_CALLBACK );
	rrex_insert({{START,L_SUM}, {L_PROD, L_PROD}, {NUM,NUM}}, PROD | DO_CALLBACK );
	std::cout << "rrex_tree sz:" << rrex_tree_size(rrex_main_tree_ptr) << std::endl;
	int64_t ret[3]={0,-1};
	circ_buf_t<int64_t, 3 > redbuf; redbuf.push_head(START);
	match(rrex_main_tree_ptr, ret, matchbuf, redbuf, NULL, std::cin );
	std::cout << std::endl;
	return 0;
}
