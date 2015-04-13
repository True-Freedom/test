#include "memorypool.h"


MemoryPool::MemoryPool()
{
	m_free = &m_memory[0];
	for	(int i = 0; i < 9; i++)
	{
		m_memory[i].data = i;
		m_memory[i].next = &m_memory[i+1];
	}

	m_memory[9].data = 9;
	m_memory[9].next = nullptr;
}

MemoryPool::~MemoryPool()
{

}

MemoryPool::node* MemoryPool::Alloc()
{
	node *tmp = m_free;
	m_free = m_free->next;
	return tmp;
}

void MemoryPool::Free(node *ptr)
{
	ptr->next = m_free;
	m_free = ptr;
}
