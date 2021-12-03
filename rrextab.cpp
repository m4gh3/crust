#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <map>

struct rrex_key
{
	int a,b;
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
	int reduce = -1;
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
		return s+1+i & 0x3ff; /*s+1+i >= sz ? buf[s+1+i-sz] : buf[s+1+i];*/
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

int rrex_insert(std::vector<rrex_key> &&rrex, int reduce=0 )
{ return rrex_insert(rrex, reduce ); }

struct match_shared_t
{
	int *ret;
	rrex_tree *root;
	struct circ_buf_t *buf;
	std::istream *is;	
};

void match(match_shared_t &m, rrex_tree *next, int reduce, int idx=0 )
{

	//if( next != nullptr )
	//{
		//read from tape:
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
		}
		else
			c = (*m.buf)[idx];	

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
					if( to_check == 1 )
					{
						next = it->second.next;
						reduce = -1;
						idx++;
						goto match_start;
					}
					std::cout << "{ ";
					match(m, it->second.next, -1, idx+1 );
					std::cout << "} ";
				}
				if( it->second.reduce >= 0 )
				{	
					if( idx > m.ret[0] )
					{
						m.ret[0] = idx;
						m.ret[1] = it->second.reduce;
					}
					if( to_check == 1 )
					{
						next = m.root;
						reduce = it->second.reduce;
						goto match_start;
					}
					std::cout << "{ ";
					match(m, m.root, it->second.reduce, idx );
					std::cout << "} ";
				}
				to_check--;
			}
		}

	//}

}

void match(int *ret, rrex_tree *root, rrex_tree *next, circ_buf_t &buf, std::istream &is, int reduce=-1, int idx=0 )
{
	match_shared_t m{ret, root, &buf, &is };
	match(m, root, reduce, idx );
}

int main()
{
	/*std::vector<rrex_key> test = { {'a','z'}, {'A','z'}, {'0','9'}, {' ', 'z'} };
	std::sort(test.begin(), test.end() );
	for(auto i : test )
		i.print();
	std::vector<rrex_key> rrex_expr = {{'a','z'}};
	rrex_insert(rrex_expr, 256 );
	rrex_expr = {{'a','y'}, {'A','z'}};
	rrex_insert(rrex_expr, 257 );
	rrex_insert({{256,256}, {'a','z'}}, 256 );*/
	rrex_insert({{'_','_'}}, 256 );
	rrex_insert({{'a','z'}}, 256 );
	rrex_insert({{'A','Z'}}, 256 );
	rrex_insert({{256,256}, {'a','z'}}, 256 );
	rrex_insert({{256,256}, {'A','Z'}}, 256 );
	rrex_insert({{256,256}, {'0','9'}}, 256 );
	rrex_insert({{256,256}, {'_','_'}}, 256 );
	rrex_insert({{'u','u'}, {'v','v'}, {'a','a'}, {'l','l'}, {' ', ' '}}, 257 );
	rrex_insert({{'r','r'}, {'e','e'}, {'f','f'}, {' ',' '}}, 258 );
	rrex_insert({{'s','s'}, {'t','t'}, {'r','r'}, {'u','u'}, {'c','c'}, {'t','t'}, {' ',' '}}, 259 );
	rrex_insert({{'u','u'}, {'n','n'}, {'i','i'}, {'o','o'}, {'n','n'}, {' ',' '}}, 260 );
	rrex_insert({{'e','e'}, {'n','n'}, {'u','u'}, {'m','m'}, {' ',' '}}, 261 );
	rrex_insert({{'c','c'}, {'o','o'}, {'n','n'}, {'s','s'}, {'t','t'}, {' ',' '}}, 262 );
	std::cout << "rrex_tree sz:" << rrex_tree_size(rrex_main_tree_ptr) << std::endl;
	int ret[2]={-1,-1};
	match(ret, rrex_main_tree_ptr, rrex_main_tree_ptr, matchbuf, std::cin );
	std::cout << "len:" << ret[0] << ", reduce:" << ret[1] << std::endl;
	std::cout << std::endl;
	return 0;
}
