#include<new>

typedef unsigned int	size_t;

//һ��������
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
		void *result = malloc(n);				//��һ��������
		//�����޷�����Ҫ��ʱ������oom_malloc()
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
		//�޷�����ʱ������oom_realloc
		if (0 == result){
			result = oom_realloc(p, new_sz);
		}
		return result;
	}

	//���·���C++��set_new_handler()
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

	for (;;){											//���ϳ����ͷš����á����ͷš�������
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

//����������
enum {__ALIGN = 8};							//С��������ϵ��߽�
enum {__MAX_BYTES = 128};					//С�����������
enum {__NFREELISTS = __MAX_BYTES / __ALIGN};//free-lists�ĸ���

template <bool threads, int inst>
class __default_alloc_template
{
private:
	//ROUND_UP��bytes�ϵ���8�ı���
	static size_t ROUND_UP(size_t bytes)
	{
		return (((bytes) + __ALIGN - 1) & ~(__ALIGN - 1));
	}

private:
	union obj{					//free-lists�Ľڵ㹹��
		union obj *free_list_link;
		char client_data[1];	/* The client sees this. */
	};

private:
	//16��frss-lists
	static obj *volatile free_list[__NFREELISTS];
	//���º������������С������ʹ�õ�n��free-list��n��0����
	static size_t FREELIST_INDEX(size_t bytes)
	{
		return (((bytes) + __ALIGN - 1) / __ALIGN - 1);
	}

	//����һ����СΪn�Ķ��󣬲����ܼ����СΪn���������鵽free list
	static void *refill(size_t n);
	//����һ���ռ䣬������nobjs����СΪsize������
	//�������nobjs�������������㣬nobjs���ܻή��
	static char *chunk_alloc(size_t size, int &nobjs);

	//Chunk allocation state
	static char *start_free;		//�ڴ����ʼλ�ã�ֻ��chunk_alloc�����б仯
	static char *end_free;			//�ڴ�ؽ��λ�ã�ֻ��chunk_alloc�����б仯
	static size_t heap_size;

public:
	static void *allocate(size_t n);
	static void deallocate(void *p, size_t n);
	static void *reallocate(void *p, size_t old_sz, size_t new_sz);
};

//������static data member�Ķ������ֵ�趨
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

	//����128һ��������
	if(n > (size_t)__MAX_BYTES){
		return (malloc_alloc::allocate(n));
	}
	// Ѱ��16��free lists���ʵ���һ��
	my_free_list = free_list + FREELIST_INDEX(n);
	result = *my_free_list;
	if (0 == result){
		//û�ҵ����õ�free list��׼���������free list
		void *r = refill(ROUND_UP(n));
		return r;
	}
	//����free list
	*my_free_list = result->free_list_link;
	return (result);
}

template <class threads, int inst>
void __default_alloc_template<threads, inst>::deallocate(void *p, size_t n)
{
	obj *q = (obj *)p;
	obj *volatile *my_free_list;

	//����128
	if (n > (size_t)__MAX_BYTES){
		malloc_alloc::deallocate(p, n);
		return;
	}
	//Ѱ�Ҷ�Ӧ��free list
	my_free_list = free_list + FREELIST_INDEX(n);
	//����free list���ջ�����
	q->free_list_link = *my_free_list;
	*my_free_list = q;
}

template <class threads, int inst>
void *__default_alloc_template<threads, inst>::refill(size_t n)
{
	int nobjs = 20;
	//����chunk_alloc()������ȡ��onbjs��������Ϊfree list���½ڵ�
	//ע�����nobjs��pass by reference
	char *chunk = chunk_alloc(n, nobjs);
	obj *volatile *my_free_list;
	obj *result;
	obj *current_obj, *next_obj;
	int i;

	//���ֻ���һ�����飬�������ͷ���������ߣ�free list���½ڵ�
	if (1 == nobjs){
		return(chunk);
	}

	//����׼������free list�������½ڵ�
	my_free_list = free_list + FREELIST_INDEX(n);

	//������chunk�ռ��ڽ���free list
	result = (obj *)chunk;
	//��������free listָ�������õĿռ䣨ȡ���ڴ�أ�
	*my_free_list = next_obj = (obj *)(chunk + n);
	//���½�free list�ĸ��ڵ㴮������
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

//�ڴ��
template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::chunk_alloc(size_t size /* ÿ���ڴ��С */, int& nobjs/* �ڴ������ */)
{
	char *result;
	size_t total_bytes = size * nobjs;				//�ڴ�������
	size_t bytes_left = end_free - start_free;		//�ڴ��ʣ��ռ�
	
	if (bytes_left >= total_bytes){
		//�ڴ��ʣ��ռ���ȫ����������
		result = start_free;
		start_free += total_bytes;
		return (result);
	}
	else if (bytes_left >= size){
		//�ڴ��ʣ��ռ䲻����ȫ���������������㹻��Ӧһ���������ϣ�������
		nobjs = bytes_left / size;
		total_bytes = size * nobjs;
		result = start_free;
		start_free += total_bytes;
		return (result);
	}
	else{
		//�ڴ��ʣ��ռ���һ������Ĵ�С���޷��ṩ
		size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
		//�����������ڴ���еĲ�����ͷ�������ü�ֵ
		if (bytes_left > 0){
			//�ڴ�ػ���һЩ��ͷ��������ʵ���free list
			//����Ѱ���ʵ���free list
			obj *volatile *my_free_list = free_list + FREELIST_INDEX(bytes_left);
			//����free list�����ڴ���еĲ���ռ����
			((obj *)start_free)->free_list_link = *my_free_list;
			*my_free_list = (obj *)start_free;
		}

		//����heap�ռ䣬���������ڴ��
		start_free = (char *)malloc(bytes_to_get);
		if (0 == start_free){
			//heap�ռ䲻�㣬����ʧ��
			int i;
			obj *volatile *my_free_list;
			obj *volatile *p;
			//���ż�����������ӵ�еĶ������ⲻ������˺������ǲ����㳢������
			//ӵ�н�С�����飬��Ϊ���ڶ���̻��������׵�������
			//����Ѱ���ʵ���free list
			//��ν�ʵ���ָ������δ�����飬�������㹻��֮free list
			for (i = size; i <= __MAX_BYTES; i += __ALIGN){
				my_free_list = free_list + FREELIST_INDEX(i);
				p = *my_free_list;
				if (0 != p){
					//free list������δ������
					//����free list���ͳ�δ������
					*my_free_list = p->free_list_link;
					start_free = (char *)p;
					end_free = start_free + i;
					//�ݹ�����Լ���Ϊ������nobjs
					return (chunk_alloc(size, nobjs));
					//ע�⣬�κβ�����ͷ�ս��������ʵ���free-list�б���
				}
			}
			end_free = 0;			//����������⣨ɽ��ˮ����������û���ڴ�����ˣ�
			//���õ�һ��������������out-of-memory�����ܷ񾡵���
			start_free = (char *)malloc_alloc::allocate(bytes_to_get);
			//����׳��쳣(exception)�����ڴ治��������ø���
		}
		heap_size += bytes_to_get;
		end_free = start_free + bytes_to_get;
		//�ݹ�����Լ���Ϊ������nobjs
		return (chunk_alloc(size, nobjs));
	}
}

//�ӿ�
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

//�������ڴ洦��


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