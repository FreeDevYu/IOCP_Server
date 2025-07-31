#include "OverlappedManager.h"



namespace Network
{
	OverlappedManager::OverlappedManager()
	{

	}

	OverlappedManager::~OverlappedManager()
	{

	}

	void OverlappedManager::Initialize(int overlappedCount)
	{
		_overlappedPool.Construct(overlappedCount); // Overlapped 객체 풀 초기화
	}

	CustomOverlapped* OverlappedManager::Pop(Network::OperationType operationType)
	{
		auto result = _overlappedPool.Pop();
		result->OperationType = operationType;
		return result;
	}

	void OverlappedManager::Push(CustomOverlapped* overlapped)
	{
		overlapped->Clear();
		_overlappedPool.Push(overlapped);
	}


}