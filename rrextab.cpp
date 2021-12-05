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
		sz_++;
	}
	int pop_back()
	{
		int k = buf[e];
		e--; e &= 0x3ff;
		sz_--;
		return k;
	}
	void push_head(int k)
	{
		buf[s] = k;
		s--; s &= 0x3ff;
		sz_++;
	}
	int pop_head(int n)
	{
		int k = buf[s];
		s+=n; s &= 0x3ff;
		sz_-=n;
		return k;
	}
	bool is_empty()
	{ return e == (s+1 & 0x3ff); }
	bool is_full()
	{ return ( s == e ); }
	int back()
	{
		return buf[e-1 & 0x3ff];
	}
	int operator[](size_t i)
	{
		return buf[s+1+i & 0x3ff];
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

/*int flatten_tree(rrex_tree *root, int *buf )
{
	if( root == nullptr )
		return 0;
	//buf[0] = root->size();
	//int jmpaddr = buf[0]+1;
	int offset=0;
	for(auto it = root->begin(); it != root->end(); it++ )
	{
		
	}
}*/

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

#define DO_CALLBACK 	0x40000000 
#define DO_RECURSION 	0x20000000
#define REDMASK		0x1fffffff

void *match(match_shared_t &m, rrex_tree *next, int64_t reduce, int idx=0, int mlen=0, int nonterminals=0 )
{
	//if( next != nullptr )
	//{
		//read from tape:
	void *pval = NULL, *sval = NULL;
	match_start:

		int64_t c;
		if( reduce >= 0 )
			c = reduce;
		else if( idx == m.buf->size() )
		{
			if( m.buf->is_full() )
				exit(EXIT_FAILURE);
			m.buf->push_back(m.is->get());
			c = m.buf->back();
			idx++;
			mlen++;
		}
		else
		{
			c = (*m.buf)[idx++];
			mlen++;
		}

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
					if( to_check == 1 && (it->second.reduce < 0) )
					{
						next = it->second.next;
						reduce = -1;
						goto match_start;
					}
					std::cout << "{ ";
					match(m, it->second.next, -1, idx, mlen );
					std::cout << "} ";
				}
				if( it->second.reduce >= 0 )
				{
					if( mlen > m.ret[0] )
					{
						m.ret[0] = mlen;
						m.ret[1] = it->second.reduce;
						m.ret[2] = idx;
						if( nonterminals == 2 )
							nonterminals = 1;
					}
					if( to_check == 1 )
					{
						next = m.root;
						reduce = it->second.reduce;
						goto match_start;
					}
					std::cout << "{ ";
					match(m, m.root, it->second.reduce, idx, mlen );
					std::cout << "} ";
				}
				to_check--;
			}
		}
		if( nonterminals & (m.ret[1] >= 0) )
		{
			reduce = m.ret[1];
			if( (reduce & DO_CALLBACK) && ( m.lang_callbacks[((reduce & REDMASK)-256<<1)+0] != NULL ) )
				pval = (m.lang_callbacks[((reduce & REDMASK)-256<<1)+0])(m, pval, sval );
			m.buf->pop_head(m.ret[2]);
			idx = 0;
			if( m.ret[1] & DO_RECURSION )
				sval = match(m, m.root, m.ret[1] & REDMASK, 0, mlen, true );
			if( (reduce & DO_CALLBACK) && ( m.lang_callbacks[((reduce & REDMASK)-256<<1)+1] != NULL ) )
			{
				//do callback
				pval = (m.lang_callbacks[((reduce & REDMASK)-256<<1)+1])(m, pval, sval );
				sval = NULL;
			}
			if( reduce != (m.ret[1] & REDMASK) )
			{
				reduce = m.ret[1] & REDMASK;
				next = m.root; //?
				nonterminals = 2;
				goto match_start;
			}
		}

	return pval;
	//}

}

void *print_id(match_shared_t &m, void *pval, void *sval )
{
	std::cout << "identfier: ";
	for(int i=0; i < m.ret[2]; i++ )
		std::cout << (char)(*m.buf)[i];
	std::cout << std::endl;
	return NULL;
}

void *make_num(match_shared_t &m, void *pval, void *sval )
{
	int *retval = new int(0);
	for(int i=0; i < m.ret[2]; i++ )
		((*retval) *= 10)+=((*m.buf)[i]-'0');
	std::cout << (*retval) << std::endl;
	return retval;
}

void *make_sum(match_shared_t &m, void *pval, void *sval ) 
{
	(*(int *)pval) += (*(int *)sval);
	std::cout << (*(int *)pval) << std::endl;
	return pval;
}

void match(int64_t *ret, rrex_tree *root, rrex_tree *next, circ_buf_t &buf, std::istream &is, int64_t reduce=-1, int idx=0 )
{
	void *(*lang_callbacks[])(match_shared_t &, void *, void * ) = {NULL, NULL, make_num, NULL, NULL, make_sum };
	match_shared_t m{ret, root, &buf, &is, lang_callbacks };
	match(m, root, reduce, idx, 0, 1 );
}

//#define ID (DO_CALLBACK | 0)
//#define ID 256
#define START 256
#define NUM 257
#define SUM 258

int main()
{
	rrex_insert({{START,START}, {'0','9'}}, DO_CALLBACK | NUM );
	rrex_insert({{DO_CALLBACK | NUM, DO_CALLBACK | NUM}, {'0','9'}}, DO_CALLBACK | NUM );
	rrex_insert({{NUM,SUM}, {'+','+'} }, DO_RECURSION | START );
	rrex_insert({{NUM,NUM}, {'+','+'}, {NUM,NUM}}, DO_RECURSION | SUM );
	std::cout << "rrex_tree sz:" << rrex_tree_size(rrex_main_tree_ptr) << std::endl;
	int64_t ret[3]={-1,-1,-1};
	match(ret, rrex_main_tree_ptr, rrex_main_tree_ptr, matchbuf, std::cin, START );
	std::cout << "\nlen:" << ret[0] << ", reduce:" << ret[1] << std::endl;
	std::cout << std::endl;
	return 0;
}
