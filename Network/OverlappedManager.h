#pragma once
#include "pch.h"
#include "ObjectPool.h"
#include "NetworkDefine.h"

namespace Network
{
	class OverlappedManager
	{
	public:
		OverlappedManager();
		~OverlappedManager();

	private:
		Utility::LockObjectPool<CustomOverlapped> _overlappedPool; // Overlapped °´Ã¼ Ç®

	public:
		void Initialize(int overlappedCount);
		CustomOverlapped* Pop(Network::OperationType operationType);
		void Push(CustomOverlapped* overlapped);
	
	};
}