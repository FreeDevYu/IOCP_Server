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
		_overlappedPool.Construct(overlappedCount); // Overlapped ��ü Ǯ �ʱ�ȭ
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