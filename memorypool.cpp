#include "memorypool.h"
#include <cstdio>
#include <cstring>


MemoryPool::MemoryPool()
{
	m_free = &m_memory[0];
	for	(int i = 0; i < 9; i++)
	{
		m_memory[i].data.id = i;
		sprintf(m_memory[i].data.name, "data id: %d", i);
		m_memory[i].next = &m_memory[i+1];
	}

	m_memory[9].data.id = 9;
	strcpy(m_memory[9].data.name, "data id: 9");
	m_memory[9].next = nullptr;
}

MemoryPool::~MemoryPool()
{

}

MemoryPool::data_t *MemoryPool::Alloc()
{
	node_t *tmp = m_free;
	m_free = m_free->next;
	return &tmp->data;
}

void MemoryPool::Free(data_t *ptr)
{
	node_t *node = (node_t*)((char*)ptr - sizeof ptr);
	node->next = m_free;
	m_free = node;
}
