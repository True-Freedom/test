#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H


class MemoryPool
{
public:
	struct node
	{
		int data;
		node *next;
	};
public:
	MemoryPool();
	~MemoryPool();

public:
	node* Alloc();
	void Free(node *ptr);

public:
	node *m_free;
	node m_memory[10];
};

#endif // MEMORYPOOL_H
