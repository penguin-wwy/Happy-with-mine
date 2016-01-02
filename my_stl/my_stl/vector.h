#include<xmemory0>
#include "iterator.h"
using namespace std;

template <class T, class Alloc = allocator<T>>
class vector
{
public:
	typedef T			value_type;
	typedef value_type*	pointer;
	typedef value_type* iterator;
	typedef value_type& reference;
	typedef size_t		size_type;
	typedef ptrdiff_t	difference_type;

protected:
	typedef _Is_simple_alloc

	vector();
	~vector();

private:

};

