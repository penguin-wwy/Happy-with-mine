typedef  int	ptrdiff_t;
typedef unsigned int	size_t;

struct input_iterator_tag{};
struct output_iterator_tag{};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

template <class Category, class T, class Distance = ptrdiff_t, 
		class Pointer = T*, class Reference = T&>
struct iterator
{
	typedef Category	iterator_category;
	typedef T			value_type;
	typedef Distance	difference_type;
	typedef Pointer		pointer;
	typedef Reference	reference;
};

template <class Iterator>
struct iterator_traits
{
	typedef typename Iterator::iterator_category	iterator_category;
	typedef typename Iterator::value_type			value_type;
	typedef typename Iterator::difference_type		difference_type;
	typedef typename Iterator::pointer				pointer;
	typedef typename Iterator::reference			reference;
};

template <class T>
struct iterator_traits<T*>
{
	typedef random_access_iterator_tag	iterator_category;
	typedef T							value_type;
	typedef ptrdiff_t					difference_type;
	typedef T*							pointer;
	typedef T&							reference;
};

template <class T>
struct iterator_traits<const T*>
{
	typedef random_access_iterator_tag	iterator_category;
	typedef T							value_type;
	typedef ptrdiff_t					difference_type;
	typedef const T*					pointer;
	typedef const T&					reference;
};

//这个函数可以很方便地决定某个迭代器的category
template <class Iterator>
inline typename iterator_traits<Iterator>::iterator_category 
iterator_category(const Iterator&)
{
	typedef typename iterator_traits<Iterator>::iterator_category	category;
	return category();
}

//这个函数可以很方便的决定某个迭代器的distance type
template <class Iterator>
inline typename iterator_traits<Iterator>::difference_type*
distance_type(const Iterator&)
{
	return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
}

//这个函数可以很方便的决定某个迭代器的value type
//static_cast < type-id > ( expression ) 将expression转换为type-id类型
template <class Iterator>
inline typename iterator_traits<Iterator>::value_type*
value_type(const Iterator&)
{
	return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
}

//以下是整组distance函数
template <class InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
_distance(InputIterator first, InputIterator last, input_iterator_tag)
{
	iterator_traits<InputIterator>::difference_type n = 0;
	while (first != last){
		++first;
		++n;
	}
	return n;
}

template <class RandomAccessIterator>
inline typename iterator_traits<RandomAccessIterator>::difference_type
_distance(RandomAccessIterator first, RandomAccessIterator last, random_access_iterator_tag)
{
	return last - first;
}

template <class InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
distance(InputIterator first, InputIterator last)
{
	typedef typename iterator_traits<InputIterator>::iterator_category category;
	return _distance(first, last, category());
}