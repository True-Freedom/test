#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H


class MemoryPool
{
public:
	struct data_t
	{
		int id;
		char name[16];
	};
public:
	struct node_t
	{
		node_t *next;
		data_t data;
	};
public:
	MemoryPool();
	~MemoryPool();

public:
	data_t* Alloc();
	void Free(data_t *ptr);

public:
	node_t *m_free;
	node_t m_memory[10];
};

#endif // MEMORYPOOL_H
