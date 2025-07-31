#pragma once
#include <boost/pool/object_pool.hpp>
#include "Lock.h"

namespace Utility
{
	template <typename T>
	class LockObjectPool : public DefaultLock
	{
	public:
		LockObjectPool<T>()
		{

		}

		~LockObjectPool<T>()
		{

		}

	private:
		boost::object_pool<T> _objectPool; // 내부적으로는 객체풀이 아니라 메모리풀이다.
		int _currentCount;
		int _sizeOffset;

	public:

		void Construct(int sizeOffset)
		{
			_currentCount = 0;
			_sizeOffset = sizeOffset;
			_objectPool.set_next_size(sizeOffset);
		}

		T* Pop()
		{
			LockOn();

			_currentCount++;

			T* result = _objectPool.construct();
			if (result == nullptr)
			{
				_objectPool.set_next_size(_sizeOffset);
				result = _objectPool.construct();
			}


			LockOff();

			return result;
		}

		template <typename Arg1>
		T* Pop(Arg1& arg1)
		{
			LockOn();

			_currentCount++;

			T* result = _objectPool.construct(arg1);
			if (result == nullptr)
			{
				_objectPool.set_next_size(_sizeOffset);
				result = _objectPool.construct();
			}


			LockOff();

			return result;
		}

		void Push(T* object)
		{
			if (object == nullptr)
			{
				return;
			}

			LockOn();

			_currentCount--;
			_objectPool.destroy(object);
			object = NULL;

			LockOff();
		}
	};
}