#include "iterator.h"
#include "pair.h"
#include "type_traits.h"
#include<string.h>

//equal
template <class InputIterator1, class InputIterator2>
inline bool equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2)
{
	for (; first1 != last1; ++first1, ++first2){
		if (*first1 != *first2){
			return false;
		}
	}
	return true;
}

template <class InputIterator1, class InputIterator2, class BinaryPredicate>
inline bool equal(InputIterator1 first1, InputIterator1 last1, 
	InputIterator2 first2, BinaryPredicate binary_pred)
{
	for (; first1 != last1; ++first1, ++first2){
		if (!binary_pred(*first1, *first1)){
			return false;
		}
	}
	return true;
}

//fill
template <class ForwardIterator, clss T>
void fill(ForwardIterator first, ForwardIterator last, const T& value)
{
	for (; first != last; ++first){
		*first = value;
	}
}

//fill_n
template <class OutputIterator, class Size, class T>
OutputIterator fill_n(OutputIterator first, Size n, const T& value)
{
	for (; n > 0; --n, ++first){
		*first = value;
	}
	return first;
}

//iter_swap
template <class Forwarditerator1, class ForwardIterator2>
inline void iter_swap(Forwarditerator1 a, ForwardIterator2 b)
{
	__iter_swap(a, b, value_type(a));
}

template <class Forwarditerator1, class ForwardIterator2, class T>
inline void __iter_swap(Forwarditerator1 a, ForwardIterator2 b, T*)
{
	T tmp = *a;
	*a = *b;
	*b = tmp;
}

//lexicographical_compare
template <class InputIterator1, class InputIterator2>
bool lexicographical_compare(InputIterator1 first1, InputIterator1 last1, 
	InputIterator2 first2, InputIterator2 last2)
{
	//以下，任何一个序列到达尾端，就结束。否则两序列就相应元素一一进行比较
	for (; first1 != last1 && first2 != last2; ++first1, ++first2){
		if (*first1 < *first2)//第一序列元素小于第二序列的相应元素值
			return true;
		if (*first2 < *first1)//第二序列元素小于第一序列的相应元素值
			return false;
		//如果不符合以上两个条件，表示两值相等
	}
	//进行到这里，如果第一序列到达尾端而第二序列尚有余额，那么第一序列小于第二序列
	return first1 == last1 && first2 != last2;
}

template <class InputIterator1, class InputIterator2, class Compare>
bool lexicographical_compare(InputIterator1 first1, InputIterator1 last1,
	InputIterator2 first2, InputIterator2 last2, Compare comp)
{
	for (; first1 != last1 && first2 != last2; ++first1, ++first2){
		if (comp(*first1, *first2))
			return true;
		if (comp(*first2, *first1))
			return false;
	}
	return first1 == last1 &&first2 != last2;
}

inline bool lexicographical_compare(const unsigned char *first1, const unsigned char *last1,
	const unsigned char *first2, const unsigned char *last2)
{
	const size_t len1 = last1 - first1;
	const size_t len2 = last2 - first2;
	//先比较相同长度的一段。memcmp（）速度极快
	const int result = memcmp(first1, first2, len1 <= len2 ? len1 : len2);
	//如果不相上下，则长度较长者被视为比较大
	return result != 0 ? result < 0 : len1 < len2;
}

//max
template <class T>
inline const T& max(const T& a, const T& b)
{
	return a < b ? b : a;
}

template <class T, class Compare>
inline const T& max(const T&a, const T&b, Compare comp)
{
	return comp(a, b) ? b : a;
}

//min
template <class T>
inline const T& min(const T& a, const T& b)
{
	return b < a ? b : a;
}

template <class T, class Compare>
inline const T& min(const T& a, const T& b, Compare comp)
{
	return comp(b, a) ? b : a;
}

//mismatch
template <class InputIterator1, class InputIterator2>
pair<InputIterator1, InputIterator2> mismatch(InputIterator1 first1, InputIterator1 last1,
	InputIterator2 first2)
{
	//如果序列一走完，就结束
	//如果序列一和序列二的对应元素相等，就结束
	//显然，序列一的元素个数必须多过序列二的元素个数，否则结果无法预期
	while (first1 != last1 && *first1 == *first2){
		++first1;
		++first2;
	}
	return pai<InputIterator1, InputIterator2>(first1, first2);
}

template <class InputIterator1, class InputIterator2, class BinaryPredicate>
pair<InputIterator1, InputIterator2> mismatch(InputIterator1 first1, InputIterator1 last1,
	InputIterator2 first2, BinaryPredicate binary_pred)
{
	while (first1 != last1 && binary_pred(*first1, *first2)){
		++first1;
		++first2;
	}
	return pair<InputIterator1, InputIterator2>(first1, first2);
}

//swap
template <class T>
inline void swap(T& a, T& b)
{
	T tmp = a;
	a = b;
	b = tmp;
}

//copy强化效率
//完全泛化版本
template <class InputIterator, class OutputIterator>
inline OutputIterator copy(InputIterator first, InputIterator last,
	OutputIterator result)
{
	return __copy_dispatch<InputIterator, OutputIterator>()
		(first, last, result);
}

//特殊版本（1）重载形式
inline char *copy(const char *first, const char *last,
	char *result)
{
	memmove(result, first, last - first);
	return result + (last - first);
}

//特殊版本（2）
inline wchar_t *copy(const wchar_t *first, const wchar_t *last,
	wchar_t *result)
{
	memmove(result, first, sizeof(wchar_t) * (last - first));
	return result + (last - first);
}

//完全泛化版本
template <class InputIterator, class OutputIterator>
struct __copy_dispatch
{
	OutputIterator operator()(InputIterator first, InputIterator last,
		OutputIterator result){
		return __copy(first, last, result, iterator_category(first));
	}
};

//偏特化版本1
template <class T>
struct __copy_dispatch<T*, T*>
{
	T *operator()(T *first, T *last, T *result)
	{
		typedef typename __type_traits<T>::has_trivial_assignment_operator t;
		return 
	}
};

//偏特化版本2
template <class T>
struct __copy_dispatch<const T*, T*>
{
	T *operator()(const T *first, const T *last, T *reslut)
	{
		typedef typename __type_traits<T>::has_trivial_assignment_operator t;
		return 
	}
};

//InputIterator版本
template <class InputIterator, class OutputIterator>
inline OutputIterator __copy(InputIterator first, InputIterator last,
	OutputIterator result, input_iterator_tag)
{
	//以迭代器等同与否，决定循环是否继续，速度慢
	for (; first != last; ++result, ++first){
		*result = *first;						//assignment operator
	}
	return result;
}

//RandomAccessIterator版本
template <class RandomAccessIterator, class OutputIterator>
inline OutputIterator __copy(RandomAccessIterator first, RandomAccessIterator last,
	OutputIterator result, random_access_iterator_tag)
{
	//又划分出一个函数，为的是其它地方也能用到
	return __copy_d(first, last, result, distance_type(first));
}

template <class RandomAccessIterator, class OutputIterator, class Distance>
inline OutputIterator __copy_d(RandomAccessIterator first, RandomAccessIterator last,
	OutputIterator result, Distance*)
{
	//以n决定循环的执行次数，速度快
	for (Distance n = last - first; n > 0; --n, ++result, ++first)
		*result = *first;							//assignment operator
	return result;
}

//以下版本适用于指针所指之对象具备trivial assignment operator
template <class T>
inline T *__copy_t(const T *first, const T *last, T *result
	__true_type)
{
	memmove(result, first, sizeof(T) * (last - first));
	return result + (last - first);
}

//以下版本适用于指针所指对象具备non-trivial assignment operator
template <class T>
inline T *__copy_t(const T *first, const T *last, T *result
	__false_type)
{
	//原生指针是一种RandomAccessIterator，所以交给__copy_d()来完成
	return __copy_d(first, last, result, (ptrdiff_t*)0);
}