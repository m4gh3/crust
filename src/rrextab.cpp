#include "rrextab.hpp"

rrex_tree rrex_main_tree;
rrex_tree *rrex_main_tree_ptr = &rrex_main_tree;
circ_buf_t<char, 10> matchbuf;

int rrex_insert(std::vector<rrex_key> &rrex, int64_t reduce, bool glow )
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
		#if GLOW_DEBUG
			data.glow |= glow;
		#endif
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

int rrex_insert(std::vector<rrex_key> &&rrex, int64_t reduce, bool glow )
{ return rrex_insert(rrex, reduce, glow ); }

void match(match_shared_t &m, rrex_tree *next, int idx )
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
			}
			else
				c = (*m.buf)[idx-m.offset];
			idx++;
		}	

		int to_check = 0;

		for( auto it = next->lower_bound({c,c}); it != next->end(); it++ )
			if( it->first.a <= c && c <= it->first.b )
				to_check++;

		for( auto it = next->lower_bound({c,c}); it != next->end() && to_check; it++ )
		{
			if( it->first.a <= c && c <= it->first.b )
			{
				#if GLOW_DEBUG
					if( it->second.glow )
						std::cout << "\033[48;2;255;165;0mGLOW\033[0m" << std::endl;
						//std::cout << "GLOW" << std::endl;
				#endif
				if( it->second.next != nullptr )
				{
					if( to_check == 1 && (it->second.reduce < 0) )
					{
						next = it->second.next;
						goto match_start;
					}
					match(m, it->second.next, idx );
				}
				if( it->second.reduce >= 0 )
				{
					if( idx > m.ret[0] )
					{
						m.ret[0] = idx;
						m.ret[1] = it->second.reduce;
					}
					m.redbuf->push_back(it->second.reduce);
					if( to_check == 1 )
					{
						next = m.root;
						goto match_start;
					}
					match(m, m.root, idx );
				}
				to_check--;
			}
		}
}


void *match(rrex_tree *root, int64_t *ret, circ_buf_t<char, 10 > &buf, circ_buf_t<int64_t, 3 > &redbuf, void *lval, std::istream &is, int idx )
{
	void *rval = NULL;
	int64_t last_good=-1;
	circ_buf_t<int64_t, 3 > next_redbuf;
	match_shared_t m{ret, root, &buf, &redbuf, &is };
	while( true )
	{
		ret[0] = 0; ret[1] = -1;
		m.offset = redbuf.size();
		match(m, root, idx );
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


struct lang_val
{
	int64_t prec_token;
	int value;
};

