#pragma once
#include <assert.h>

#define Dyn_FirstAlloc          1024
#define Dyn_SubsequentAlloc     1024

#define Dyn_MemoryAlignment     8


template <typename T>
class DynArray
{
public:
	DynArray();
	~DynArray();

	bool AddItem(T i);
	bool InsertEmpty(unsigned int Pos, unsigned int Count=1, bool ZeroOut=TRUE);
	bool InsertItems(unsigned int Pos, T* pData, unsigned int Count=1);
	void DeleteItems(unsigned int Pos, unsigned int Count=1);

	T* m_Items;
	unsigned int m_ItemCount;

protected:
	unsigned int m_Allocated;
};


#define INITDYNARRAY \
	if (!m_Items) \
	{ \
		m_Items = static_cast<T*>(_aligned_malloc(Dyn_FirstAlloc*sizeof(T), Dyn_MemoryAlignment)); \
		if (!m_Items) \
			return false; \
		m_Allocated = Dyn_FirstAlloc; \
	} \


template <typename T>
DynArray<T>::DynArray()
{
	m_Items = NULL;
	m_ItemCount = m_Allocated = 0;
}

template <typename T>
DynArray<T>::~DynArray()
{
	if (m_Items)
		_aligned_free(m_Items);
}

template <typename T>
bool DynArray<T>::AddItem(T i)
{
	INITDYNARRAY

	if (m_ItemCount==m_Allocated)
	{
		m_Items = static_cast<T*>(_aligned_realloc(m_Items, (m_Allocated+Dyn_SubsequentAlloc)*sizeof(T), Dyn_MemoryAlignment));
		if (!m_Items)
			return false;

		m_Allocated += Dyn_SubsequentAlloc;
	}

	m_Items[m_ItemCount++] = i;
	return true;
}

template <typename T>
bool DynArray<T>::InsertEmpty(unsigned int Pos, unsigned int Count=1, bool ZeroOut=TRUE)
{
	if (!Count)
		return true;

	INITDYNARRAY

	if (m_ItemCount<m_Allocated+Count)
	{
		const unsigned int Add = (Count+Dyn_SubsequentAlloc-1) & ~(Dyn_SubsequentAlloc-1);
		m_Items = static_cast<T*>(_aligned_realloc(m_Items, (m_Allocated+Add)*sizeof(T), Dyn_MemoryAlignment));
		if (!m_Items)
			return false;

		m_Allocated += Add;
	}

	for (int a=((int)(m_ItemCount))-1; a>=(int)Pos; a--)
		m_Items[a+Count] = m_Items[a];

	if (ZeroOut)
		ZeroMemory(&m_Items[Pos], Count*sizeof(T));

	m_ItemCount += Count;
	return true;
}

template <typename T>
bool DynArray<T>::InsertItems(unsigned int Pos, T* pData, unsigned int Count=1)
{
	assert(pData);

	if (!InsertEmpty(Pos, Count, false))
		return false;

	memcpy_s(m_Items[Pos], sizeof(T)*Count, pData, sizeof(T)*Count);
	return true;
}

template <typename T>
void DynArray<T>::DeleteItems(unsigned int Pos, unsigned int Count=1)
{
	if (!Count)
		return;

	for (unsigned int a=Pos; a<m_ItemCount-Count; a++)
		m_Items[a] = m_Items[a+Count];

	m_ItemCount -= Count;
	return true;
}
