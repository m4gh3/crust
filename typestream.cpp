#include <vector>

class typegraph
{
	public:
		enum class typeop
		{
			_uval, _huval, _hhuval,
			_sval, _hsval, _hhsval,
			_ref, _bpref,
			_const, _ptr,
			_struct, _fun
		};
		void add_op(int type_code, std::vector<typegraph> op_params );
		typegraph check_op(int type_code, std::vector<typegraph> op_params );
	private:
		std::vector<typeop> raw_type_data;
		std::vector<std::vector<typegraph>> type_params_children;
};
