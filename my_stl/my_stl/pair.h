template <class _Ty1, class _Ty2>
struct pair
{
	typedef pair<_Ty1, _Ty2> _Myt;
	typedef _Ty1 first_type;
	typedef _Ty2 second_type;

	pair()
		: first(), second()
	{	// default construct
	}

	pair(const _Ty1& _Val1, const _Ty2& _Val2)
		: first(_Val1), second(_Val2)
	{	// construct from specified values
	}

	template <class _Other1, class _Other2>
		_Myt& operator=(const pair<_Other1, _Other2>& _Right)
	{	// assign from compatible pair
		first = _Right.first;
		second = _Right.second;
		return (*this);
	}

	_Ty1 first;	// the first stored value
	_Ty2 second;	// the second stored value
};