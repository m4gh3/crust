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

int rrex_insert(std::vector<rrex_key> &rrex, int64_t reduce=0 )
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
	//void *(**lang_callbacks)(match_shared_t &, void *, void *);
};

#define DO_CALLBACK 	0x80000000 
#define DO_RECURSION 	0x40000000
#define REDMASK		0x3fffffff

void *lang_callbacks(int64_t, match_shared_t &, void *, void *);

void match(match_shared_t &m, rrex_tree *next, int64_t lreduce, int64_t rreduce, int idx=0, int offset=0 )
{	
	match_start:

		int64_t c;
		if( lreduce >= 0 )
			c = lreduce;
		else if( rreduce >= 0 )
			c = rreduce;
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
						if( lreduce >= 0 )
							lreduce = -1;
						else
							rreduce = -1;
						goto match_start;
					}
					std::cout << "{ ";
					if( rreduce >= 0 )
						match(m, it->second.next, rreduce, -1, idx, offset );
					else
						match(m, it->second.next, -1, -1, idx, offset );
					std::cout << "} ";
				}
				if( it->second.reduce >= 0 )
				{
					if( idx+offset > m.ret[0] )
					{
						m.ret[0] = idx;
						m.ret[1] = it->second.reduce;
					}
					if( to_check == 1 )
					{
						next = m.root;
						if(rreduce >= 0)
						{
							lreduce = rreduce;
							rreduce = it->second.reduce;
						}
						else
							lreduce = it->second.reduce;
						goto match_start;
					}
					std::cout << "{ ";
					if( rreduce >=0 )
						match(m, m.root, rreduce, it->second.reduce, idx, offset );
					else
						match(m, m.root, it->second.reduce, -1, idx, offset );
					std::cout << "} ";
				}
				to_check--;
			}
		}
}

void *make_num(match_shared_t &m, void *pval, void *sval )
{
	int *retval = new int(0);
	for(int i=0; i < m.ret[0]; i++ )
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

void *match(rrex_tree *root, int64_t *ret, circ_buf_t &buf, std::istream &is, int64_t lreduce=-1, int idx=0 )
{
	//void *(*lang_callbacks[])(match_shared_t &, void *, void * ) = {make_num, make_sum};
	void *lval = NULL; void *rval = NULL;
	int64_t rreduce = -1;
	int64_t last_good;
	//int64_t ret[2] = {0,0};
	match_shared_t m{ret, root, &buf, &is /*, lang_callbacks*/ };
	while( lreduce != -1 )
	{
		last_good = lreduce /*& REDMASK*/;
		ret[0] = 0; ret[1] = -1;
		match(m, root, lreduce, rreduce, idx, rreduce >= 0 );
		//std::cout << "\nlen:" << ret[0] << ", reduce:" << (ret[1] & REDMASK) << ' ' << (ret[1]&DO_CALLBACK ? 'c' : ' ') << (ret[1]&DO_RECURSION ? 'r' : ' ') << std::endl;
		lreduce = ret[1];
		if( lreduce >= 0 )
		{
			if( lreduce & DO_CALLBACK )
				lval = lang_callbacks(lreduce & REDMASK, m, lval, rval );
			buf.pop_head(m.ret[0]);
			ret[0] = 0; ret[1] = -1;
			rreduce = -1;
			if( lreduce & DO_RECURSION )
			{
				rval = match(root, ret, buf, is, lreduce & REDMASK, 0 );
				rreduce = ret[1];
			}
			lreduce &= REDMASK;
		}
	}
	ret[1] = last_good; //but this is useless
	return lval;
}

//#define ID (DO_CALLBACK | 0)
//#define ID 256
/*#define START 256
#define L_SUM 257
#define R_NUM 258
#define R_PROD 259
#define L_NUM 260
#define L_PROD 261
#define SUM 262*/

enum
{
	START = 256,
	L_SUM,
	LL_PROD,
	R_NUM,
	R_PROD,
	L_NUM,
	L_PROD,
	SUM	
};

void *lang_callbacks(int64_t reduce, match_shared_t &m, void *lval, void *rval)
{
	switch(reduce)
	{
		case R_NUM:
		case L_NUM:
			return make_num(m, lval, rval );
		case SUM:
			return make_sum(m, lval, rval );
	}
	return NULL;
}

int main()
{
	rrex_insert({{START,START}, {'0','9'}}, L_NUM | DO_CALLBACK );
	rrex_insert({{L_SUM,LL_PROD}, {'0','9'}}, R_NUM | DO_CALLBACK );
	//rrex_insert({{LL_PROD,LL_PROD}, {'0','9'}}, R_NUM | DO_CALLBACK );
	rrex_insert({{L_NUM | DO_CALLBACK, L_NUM | DO_CALLBACK}, {'0','9'}}, L_NUM | DO_CALLBACK );
	rrex_insert({{R_NUM | DO_CALLBACK, R_NUM | DO_CALLBACK}, {'0','9'}}, R_NUM | DO_CALLBACK );
	rrex_insert({{L_NUM, SUM}, {'+','+'}}, L_SUM | DO_RECURSION );
	rrex_insert({{L_NUM, L_PROD}, {'*','*'}}, LL_PROD | DO_RECURSION );
	rrex_insert({{L_SUM,L_SUM}, {R_NUM,R_NUM}}, SUM | DO_CALLBACK );
	std::cout << "rrex_tree sz:" << rrex_tree_size(rrex_main_tree_ptr) << std::endl;
	int64_t ret[3]={0,-1};
	//match(ret, rrex_main_tree_ptr, rrex_main_tree_ptr, matchbuf, std::cin, START );
	match(rrex_main_tree_ptr, ret, matchbuf, std::cin, START );
	//std::cout << "\nlen:" << ret[0] << ", reduce:" << ret[1] << std::endl;
	std::cout << std::endl;
	return 0;
}
