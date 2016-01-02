#include<new>

typedef unsigned int	size_t;

//一级适配器
template <int inst>
class __malloc_alloc_template
{
private:
	static void *oom_malloc(size_t);
	static void *oom_realloc(void *, size_t);
	static void(*__malloc_alloc_oom_handler)();

public:
	static void *allocate(size_t n);
	{
		void *result = malloc(n);				//第一级适配器
		//以下无法满足要求时，改用oom_malloc()
		if (0 == result){
			result = oom_malloc(n);
		}
		return result;
	}

	static void deallocate(void *p, size_t /* n */)
	{
		free(p);
	}

	static void *reallocate(void *p, size_t /* old_sz */, size_t new_sz)
	{
		void *result = realloc(p, new_sz);
		//无法满足时，改用oom_realloc
		if (0 == result){
			result = oom_realloc(p, new_sz);
		}
		return result;
	}

	//以下仿真C++的set_new_handler()
	static void(*set_malloc_handler(void(*f)()))()
	{
		void(*old)() = __malloc_alloc_oom_handler;
		__malloc_alloc_oom_handler = f;
		return (old);
	}
};

template <int inst>
void(*__malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0;

template <int inst>
void *__malloc_alloc_template<inst>::oom_malloc(size_t n)
{
	void(*my_malloc_handler)();
	void *result;

	for (;;){											//不断尝试释放、配置、再释放、再配置
		my_malloc_handler = __malloc_alloc_oom_handler;
		if (0 == my_malloc_handler){
			_THROW_BAD_ALLOC;
		}
		(*my_malloc_handler)();
		result = realloc(p, n);
		if (result){
			return (result);
		}
	}
}

typedef __malloc_alloc_template<0> malloc_alloc;

//二级配置器
enum {__ALIGN = 8};							//小型区块的上调边界
enum {__MAX_BYTES = 128};					//小型区块的上限
enum {__NFREELISTS = __MAX_BYTES / __ALIGN};//free-lists的个数

template <bool threads, int inst>
class __default_alloc_template
{
private:
	//ROUND_UP将bytes上调至8的倍数
	static size_t ROUND_UP(size_t bytes)
	{
		return (((bytes) + __ALIGN - 1) & ~(__ALIGN - 1));
	}

private:
	union obj{					//free-lists的节点构造
		union obj *free_list_link;
		char client_data[1];	/* The client sees this. */
	};

private:
	//16个frss-lists
	static obj *volatile free_list[__NFREELISTS];
	//以下函数根据区块大小，决定使用第n号free-list。n从0算起
	static size_t FREELIST_INDEX(size_t bytes)
	{
		return (((bytes) + __ALIGN - 1) / __ALIGN - 1);
	}

	//返回一个大小为n的对象，并可能加入大小为n的其他区块到free list
	static void *refill(size_t n);
	//配置一大块空间，可容纳nobjs个大小为size的区块
	//如果配置nobjs个区块有所不便，nobjs可能会降低
	static char *chunk_alloc(size_t size, int &nobjs);

	//Chunk allocation state
	static char *start_free;		//内存池起始位置，只在chunk_alloc（）中变化
	static char *end_free;			//内存池结合位置，只在chunk_alloc（）中变化
	static size_t heap_size;

public:
	static void *allocate(size_t n);
	static void deallocate(void *p, size_t n);
	static void *reallocate(void *p, size_t old_sz, size_t new_sz);
};

//以下是static data member的定义与初值设定
template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::start_free = 0;

template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::end_free = 0;

template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::heap_size = 0;

template <bool threads, int inst>
char *typename __default_alloc_template<threads, inst>::obj * volatile
__default_alloc_template<threads, inst>::free_list[__NFREELISTS] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

template <class threads, int inst>
void *__default_alloc_template<threads, inst>::allocate(size_t n)
{
	obj *volatile *my_free_list;
	obj *result;

	//大于128一级配置器
	if(n > (size_t)__MAX_BYTES){
		return (malloc_alloc::allocate(n));
	}
	// 寻找16个free lists中适当的一个
	my_free_list = free_list + FREELIST_INDEX(n);
	result = *my_free_list;
	if (0 == result){
		//没找到可用的free list，准备重新填充free list
		void *r = refill(ROUND_UP(n));
		return r;
	}
	//调整free list
	*my_free_list = result->free_list_link;
	return (result);
}

template <class threads, int inst>
void __default_alloc_template<threads, inst>::deallocate(void *p, size_t n)
{
	obj *q = (obj *)p;
	obj *volatile *my_free_list;

	//大于128
	if (n > (size_t)__MAX_BYTES){
		malloc_alloc::deallocate(p, n);
		return;
	}
	//寻找对应的free list
	my_free_list = free_list + FREELIST_INDEX(n);
	//调整free list，收回区块
	q->free_list_link = *my_free_list;
	*my_free_list = q;
}

template <class threads, int inst>
void *__default_alloc_template<threads, inst>::refill(size_t n)
{
	int nobjs = 20;
	//调用chunk_alloc()，尝试取得onbjs个区块作为free list的新节点
	//注意参数nobjs是pass by reference
	char *chunk = chunk_alloc(n, nobjs);
	obj *volatile *my_free_list;
	obj *result;
	obj *current_obj, *next_obj;
	int i;

	//如果只获得一个区块，这个区块就分配给调用者，free list无新节点
	if (1 == nobjs){
		return(chunk);
	}

	//否则准备调整free list，纳入新节点
	my_free_list = free_list + FREELIST_INDEX(n);

	//以下在chunk空间内建立free list
	result = (obj *)chunk;
	//以下引导free list指向新配置的空间（取自内存池）
	*my_free_list = next_obj = (obj *)(chunk + n);
	//以下将free list的各节点串接起来
	for (i = 1;; i++){
		current_obj = next_obj;
		next_obj = (obj *)((char *)next_obj + n);
		if (nobjs - 1 == i){
			current_obj->free_list_link = 0;
			break;
		}
		else{
			current_obj->free_list_link = next_obj;
		}
	}
	return (result);
}

//内存池
template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::chunk_alloc(size_t size /* 每块内存大小 */, int& nobjs/* 内存块数量 */)
{
	char *result;
	size_t total_bytes = size * nobjs;				//内存总需求
	size_t bytes_left = end_free - start_free;		//内存池剩余空间
	
	if (bytes_left >= total_bytes){
		//内存池剩余空间完全满足需求量
		result = start_free;
		start_free += total_bytes;
		return (result);
	}
	else if (bytes_left >= size){
		//内存池剩余空间不能完全满足需求量，但足够供应一个（含以上）的区块
		nobjs = bytes_left / size;
		total_bytes = size * nobjs;
		result = start_free;
		start_free += total_bytes;
		return (result);
	}
	else{
		//内存池剩余空间连一个区块的大小都无法提供
		size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
		//以下试着让内存池中的残余零头还有利用价值
		if (bytes_left > 0){
			//内存池还有一些零头，先配给适当的free list
			//首先寻找适当的free list
			obj *volatile *my_free_list = free_list + FREELIST_INDEX(bytes_left);
			//调整free list，将内存池中的残余空间编入
			((obj *)start_free)->free_list_link = *my_free_list;
			*my_free_list = (obj *)start_free;
		}

		//配置heap空间，用来补充内存池
		start_free = (char *)malloc(bytes_to_get);
		if (0 == start_free){
			//heap空间不足，配置失败
			int i;
			obj *volatile *my_free_list;
			obj *volatile *p;
			//试着检视我们手上拥有的东西，这不会造成伤害。我们不打算尝试配置
			//拥有较小的区块，因为那在多进程机器上容易导致灾难
			//以下寻找适当的free list
			//所谓适当是指“尚有未用区块，且区块足够大”之free list
			for (i = size; i <= __MAX_BYTES; i += __ALIGN){
				my_free_list = free_list + FREELIST_INDEX(i);
				p = *my_free_list;
				if (0 != p){
					//free list内尚有未用区块
					//调整free list以释出未用区块
					*my_free_list = p->free_list_link;
					start_free = (char *)p;
					end_free = start_free + i;
					//递归调用自己，为了修正nobjs
					return (chunk_alloc(size, nobjs));
					//注意，任何残余零头终将被编入适当的free-list中备用
				}
			}
			end_free = 0;			//如果出现意外（山穷水尽，到处都没有内存可用了）
			//调用第一级配置器，看看out-of-memory机制能否尽点力
			start_free = (char *)malloc_alloc::allocate(bytes_to_get);
			//这会抛出异常(exception)，或内存不足的情况获得改善
		}
		heap_size += bytes_to_get;
		end_free = start_free + bytes_to_get;
		//递归调用自己，为了修正nobjs
		return (chunk_alloc(size, nobjs));
	}
}

//接口
template <class T, class Alloc>
class simple_alloc
{
public:
	static T *allocate(size_t n)
	{
		return 0 == n ? 0 : (T *)Alloc::allocate(n * sizeof(T));
	}

	static T *allocate(void)
	{
		return (T *)Alloc::allocate(sizeof(T));
	}

	static void deallocate(T *p, size_t n)
	{
		if (0 != n)
		{
			Alloc::deallocate(p, n * sizeof(T));
		}
	}

	static void deallocate(T *p)
	{
		Alloc::deallocate(p, sizeof(T));
	}
};

//基本的内存处理


template <class InputIterator, class ForwardIterator>
ForwardIterator uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result)
{

}

template <class ForwardIterator, class T>
void uninitialized_fill(ForwardIterator first, forwardIterator last, const T& x)
{

}

template <class ForwardIterator, class Size, class T>
ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n, const T& x)
{

}