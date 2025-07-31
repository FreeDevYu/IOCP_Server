#pragma once
#include <boost/thread/mutex.hpp>


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
