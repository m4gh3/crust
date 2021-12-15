#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <cstdint>

#define GLOW_DEBUG 0

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
	#if GLOW_DEBUG
		bool glow=false;
	#endif
	std::map<rrex_key, rrex_data > *next = nullptr;
};

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

extern circ_buf_t<char, 10> matchbuf;

extern rrex_tree rrex_main_tree;
extern rrex_tree* rrex_main_tree_ptr;
extern int rrex_insert(std::vector<rrex_key> &rrex, int64_t reduce=0, bool glow=false );
extern int rrex_tree_size(rrex_tree *root);
extern int rrex_insert(std::vector<rrex_key> &&rrex, int64_t reduce=0, bool glow=false );

struct match_shared_t
{
	int64_t *ret;
	rrex_tree *root;
	struct circ_buf_t<char, 10 > *buf;
	struct circ_buf_t<int64_t, 3 > *redbuf;
	std::istream *is;
	std::ostream *os;
	int offset;
};

const int64_t DO_CALLBACK=0x80000000;
const int64_t DO_RECURSION=0x40000000;
const int64_t REDMASK=0x3fffffff;

extern void lang_callbacks(int64_t, match_shared_t &, circ_buf_t<int64_t, 3 >&, void *&, void *& );
extern void match(match_shared_t &m, rrex_tree *next, int idx=0 );
extern void *match(rrex_tree *root, int64_t *ret, circ_buf_t<char, 10 > &buf, circ_buf_t<int64_t, 3 > &redbuf, void *lval, std::istream &is, std::ostream &os, int idx=0 );

