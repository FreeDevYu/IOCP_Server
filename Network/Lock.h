#pragma once
#include <boost/thread/mutex.hpp>
#include "pch.h"

template<class LOCK>
class LockWorker
{
public:
	LockWorker() {}
	virtual ~LockWorker() {}

protected:
	LOCK _mutex;


private:
	// 복사를 제한한다.
	LockWorker(const LockWorker& rhs);
	LockWorker& operator=(const LockWorker& rhs);

public:
	void LockOn() { _mutex.lock(); }
	void LockOff() { _mutex.unlock(); }
};


typedef LockWorker<boost::mutex> DefaultLock;

//windows플랫폼 전용
class SRWLOCKWorker
{
public:
	SRWLOCKWorker() { InitializeSRWLock(&_lock); }
	~SRWLOCKWorker() {}

protected:
	SRWLOCK _lock;

private:
	SRWLOCKWorker(const SRWLOCKWorker& rhs);
	SRWLOCKWorker& operator=(const SRWLOCKWorker& rhs);

public:
	void ReadLockOn() 
	{
		AcquireSRWLockShared(&_lock);
	}

	void ReadLockOff()
	{
		ReleaseSRWLockShared(&_lock);

	}

	void WriteLockOn() 
	{ 
		AcquireSRWLockExclusive(&_lock);
	}

	void WriteLockOff()
	{
		ReleaseSRWLockExclusive(&_lock);
	}
};
