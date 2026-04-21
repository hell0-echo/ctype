#ifndef _TYPEGAME_TG_OBJECTPOOL_H_
#define _TYPEGAME_TG_OBJECTPOOL_H_

#include <array>
#include <bitset>
#include <cstddef>
#include <new>
#include <type_traits>

template<typename T, std::size_t Capacity>
class TgObjectPool
{
public:
	TgObjectPool() = default;

	TgObjectPool(const TgObjectPool &) = delete;
	TgObjectPool &operator=(const TgObjectPool &) = delete;

	T *acquire()
	{
		for (std::size_t i = 0; i < Capacity; ++i)
		{
			if (!m_used.test(i))
			{
				m_used.set(i);
				void *pMem = &m_storage[i];
				T *pObj = new (pMem) T();
				m_activeCount++;
				return pObj;
			}
		}
		return nullptr;
	}

	void release(T *pObject)
	{
		if (nullptr == pObject)
			return;
		const std::ptrdiff_t diff = reinterpret_cast<std::byte *>(pObject)
			- reinterpret_cast<std::byte *>(&m_storage[0]);
		const std::size_t stride = sizeof(std::aligned_storage_t<sizeof(T), alignof(T)>);
		if (diff < 0 || diff % static_cast<std::ptrdiff_t>(stride) != 0)
			return;
		const std::size_t index = static_cast<std::size_t>(diff / static_cast<std::ptrdiff_t>(stride));
		if (index >= Capacity)
			return;
		if (!m_used.test(index))
			return;
		pObject->~T();
		m_used.reset(index);
		m_activeCount--;
	}

	void clear()
	{
		for (std::size_t i = 0; i < Capacity; ++i)
		{
			if (m_used.test(i))
			{
				ptrAt(i)->~T();
				m_used.reset(i);
			}
		}
		m_activeCount = 0;
	}

	std::size_t activeCount() const
	{
		return m_activeCount;
	}

	static constexpr std::size_t capacity()
	{
		return Capacity;
	}

private:
	using StorageCell = std::aligned_storage_t<sizeof(T), alignof(T)>;

	T *ptrAt(std::size_t index)
	{
		return reinterpret_cast<T *>(&m_storage[index]);
	}

	std::array<StorageCell, Capacity> m_storage{};
	std::bitset<Capacity> m_used{};
	std::size_t m_activeCount = 0;
};

#endif
