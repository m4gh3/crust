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
	std::map<rrex_key, rrex_data > *next = nullptr;
};

int last_reduce=1;

typedef std::map<rrex_key, rrex_data > rrex_tree;

struct circ_buf_t
{
	char buf[1024];
	int s=0, e=1, sz = 1024;
	int sz_ = 0;
	void push_back(int k)
	{
		buf[e] = k;
		e++; e &= 0x3ff;
		/*if ( e == sz )
			e = 0;*/
		sz_++;
	}
	int pop_back()
	{
		int k = buf[e];
		/*if( e == 0 )
			e=sz-1;*/
		//else
			e--; e &= 0x3ff;
		sz_--;
		return k;
	}
	void push_head(int k)
	{
		buf[s] = k;
		//if( s == 0 )
		//	s=sz-1;
		//else
			s--; s &= 0x3ff;
		sz_++;
	}
	int pop_head()
	{
		int k = buf[s];
		s++; s &= 0x3ff;
		//if( s == sz )
		//	s = 0;
		sz_--;
		return k;
	}
	bool is_empty()
	{ return e == (s+1 & 0x3ff);/*( e == s+1 || ( e == 0 && s==sz-1 ) );*/ }
	bool is_full()
	{ return ( s == e ); }
	int back()
	{
		/*if( e == 0 )
			return buf[sz-1];
		else
			return buf[e-1];*/
		return buf[e-1 & 0x3ff];
	}
	int operator[](size_t i)
	{
		return buf[s+1+i & 0x3ff]; /*s+1+i >= sz ? buf[s+1+i-sz] : buf[s+1+i];*/
	}
	size_t size()
	{ return sz_; }
};


circ_buf_t matchbuf;

rrex_tree rrex_main_tree;
rrex_tree *rrex_main_tree_ptr = &rrex_main_tree;

int rrex_insert(std::vector<rrex_key> &rrex, int reduce=0 )
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

// range_beg, range_end, next_offset, reduce_to
// 32bit, 32bit, 32bit, 32bit

int flatten_tree(rrex_tree *root, int *buf )
{
	if( root == nullptr )
		return 0;
	//buf[0] = root->size();
	//int jmpaddr = buf[0]+1;
	int offset=0;
	for(auto it = root->begin(); it != root->end(); it++ )
	{
		
	}
}

int rrex_insert(std::vector<rrex_key> &&rrex, int64_t reduce=0 )
{ return rrex_insert(rrex, reduce ); }

struct match_shared_t
{
	int64_t *ret;
	rrex_tree *root;
	struct circ_buf_t *buf;
	std::istream *is;
	void *(**lang_callbacks)(match_shared_t &, void *, void *);
};

void *match(match_shared_t &m, rrex_tree *next, int64_t reduce, int idx=0, int nonterminals=0 )
{
	//if( next != nullptr )
	//{
		//read from tape:
	void *pval = NULL, *sval = NULL;
	match_start:

		int c;
		if( reduce >= 0 )
			c = reduce;
		else if( idx == m.buf->size() )
		{
			if( m.buf->is_full() )
				exit(EXIT_FAILURE);
			m.buf->push_back(m.is->get());
			c = m.buf->back();
			idx++;
		}
		else
			c = (*m.buf)[idx++];	
		
		std::cout << '(' << c << ',' << idx << ',' << m.buf->size() << ')';

		int to_check = 0;

		for( auto it = next->lower_bound({c,c}); it != next->end(); it++ )
			if( it->first.a <= c && c <= it->first.b )
				to_check++;

		for( auto it = next->lower_bound({c,c}); it != next->end() && to_check; it++ )
		{
			if( it->first.a <= c && c <= it->first.b )
			{
				if( it->second.next != nullptr )
				{
					std::cout << 'n';
					if( to_check == 1 && (it->second.reduce < 0) )
					{
						std::cout << 'o';
						next = it->second.next;
						reduce = -1;
						//std::cout << '+';
						//idx++;
						goto match_start;
					}
					std::cout << "{ ";
					match(m, it->second.next, -1, idx/*+1*/ );
					std::cout << "} ";
				}
				if( it->second.reduce >= 0 )
				{
					std::cout << 'r';	
					if( idx > m.ret[0] )
					{
						std::cout << 'm';
						m.ret[0] = idx;
						m.ret[1] = it->second.reduce;
						if( nonterminals == 2 )
							nonterminals = 1;
					}
					if( to_check == 1 )
					{
						std::cout << 'o';
						next = m.root;
						reduce = it->second.reduce;
						goto match_start;
					}
					std::cout << "{ ";
					match(m, m.root, it->second.reduce, idx );
					std::cout << "} ";
				}
				std::cout << '-';
				to_check--;
			}
		}
		/*if( nonterminals & (m.ret[1] >= 0) )
		{
			reduce = m.ret[1];
			if( m.ret[1] & 0x2000000000000000 )
				sval = match(m, m.root, m.ret[1], idx, true );
			if( reduce & 0x4000000000000000 )
			{
				//do callback
				pval = (m.lang_callbacks[m.ret[1] & 0xbfffffffffffffff ])(m, pval, sval );
				sval = NULL;
			}
			if( reduce != m.ret[1] )
			{
				reduce = m.ret[1];
				nonterminals = 2;
				goto match_start;
			}
		}*/

	return pval;
	//}

}

// p -> p*n
// p -> n
// s -> s+p //now the problem is how you actually forbid that rule from happening?
// s -> p   // see? you don't want to apply that rule after s+ right?

void *print_id(match_shared_t &m, void *pval, void *sval )
{
	std::cout << (*m.buf)[0] << std::endl;
	return NULL;
}

void match(int64_t *ret, rrex_tree *root, rrex_tree *next, circ_buf_t &buf, std::istream &is, int64_t reduce=-1, int idx=0 )
{
	void *(*lang_callbacks[])(match_shared_t &, void *, void * ) = {print_id};
	match_shared_t m{ret, root, &buf, &is, lang_callbacks };
	match(m, root, reduce, idx );
}


#define DO_CALLBACK 0x4000000000000000 
#define DO_RECURSION 0x2000000000000000

//#define ID (DO_CALLBACK | 0)
#define ID 256

int main()
{
	rrex_insert({{'_','_'}}, ID );
	rrex_insert({{'a','z'}}, ID );
	rrex_insert({{'A','Z'}}, ID );
	rrex_insert({{ID, ID}, {'a','z'}}, ID );
	rrex_insert({{ID, ID}, {'A','Z'}}, ID );
	rrex_insert({{ID, ID}, {'0','9'}}, ID );
	rrex_insert({{ID, ID}, {'_','_'}}, ID );
	rrex_insert({{'u','u'}, {'v','v'}, {'a','a'}, {'l','l'}, {' ', ' '}}, 257 );
	rrex_insert({{'r','r'}, {'e','e'}, {'f','f'}, {' ',' '}}, 258 );
	rrex_insert({{'s','s'}, {'t','t'}, {'r','r'}, {'u','u'}, {'c','c'}, {'t','t'}, {' ',' '}}, 259 );
	rrex_insert({{'u','u'}, {'n','n'}, {'i','i'}, {'o','o'}, {'n','n'}, {' ',' '}}, 260 );
	rrex_insert({{'e','e'}, {'n','n'}, {'u','u'}, {'m','m'}, {' ',' '}}, 261 );
	rrex_insert({{'c','c'}, {'o','o'}, {'n','n'}, {'s','s'}, {'t','t'}, {' ',' '}}, 262 );
	std::cout << "rrex_tree sz:" << rrex_tree_size(rrex_main_tree_ptr) << std::endl;
	int64_t ret[2]={-1,-1};
	match(ret, rrex_main_tree_ptr, rrex_main_tree_ptr, matchbuf, std::cin );
	std::cout << "\nlen:" << ret[0] << ", reduce:" << ret[1] << std::endl;
	std::cout << std::endl;
	return 0;
}
