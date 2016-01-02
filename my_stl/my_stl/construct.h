#include<new.h>
#include "type_traits.h"

template <class T1, class T2>
inline void construct(T1 *p, const T2& value)
{
	new (p)T1(value);		//placement new;调用T1::T1(value)
}

//destroy第一个版本，接受一个指针
template <class T>
inline void destroy(T *pointer)
{
	pointer->~T();
}

//destroy第二个版本，接受两个迭代器，设法找出元素的数值型别
//进而利用__type_traits<>求取最适当措施
template <class ForwardIterator>
inline void destroy(ForwardIterator first, ForwardIterator last)
{
	__destroy(first, last, value_type(first));
}

//判断元素的数值型别
template <class ForwardIterator, class T>
inline void __destroy(ForwardIterator first, ForwardIterator last, T*)
{
	typedef typename __type_traits<T>::has_trivial_destructor	trivial_destructor;
	__destroy_aux(first, last, trivial_destructor());
}

template <class ForwardIterator>
inline void __destory_aux(ForwardIterator first, ForwardIterator last, __false_type)
{
	for (; first < last; ++first){
		destroy(&*first);
	}
}

template <class ForwardIterator>
inline void __destroy_aux(ForwardIterator, ForwardIterator, __true_type) {}

inline void destroy(char *, char *){}
inline void destroy(wchar_t *, wchar_t *){}

