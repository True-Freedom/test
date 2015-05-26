#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H


class MemoryPool
{
public:
	struct thunk_t
	{
		int id;
		char name[16];
	};
public:
	struct node_t
	{
		node_t *next;
		thunk_t data;
	};
public:
	MemoryPool();
	~MemoryPool();

public:
	thunk_t* Alloc();
	void Free(thunk_t *ptr);

public:
	node_t *m_free;
	node_t m_memory[10];
};

#endif // MEMORYPOOL_H
