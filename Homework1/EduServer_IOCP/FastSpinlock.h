#pragma once

class FastSpinlock
{
public:
	FastSpinlock();
	~FastSpinlock();

	FastSpinlock(const FastSpinlock& rhs) = delete;
	FastSpinlock& operator=(const FastSpinlock& rhs) = delete;

	void EnterLock();
	void LeaveLock();
	
private:
	volatile long mLockFlag;
};

class FastSpinlockGuard
{
public:
	FastSpinlockGuard(FastSpinlock& lock) : mLock(lock)
	{
		mLock.EnterLock();
	}

	~FastSpinlockGuard()
	{
		mLock.LeaveLock();
	}

private:
	FastSpinlock& mLock;
};

template <class TargetClass>
class ClassTypeLock
{
public:
	struct LockGuard
	{
		LockGuard()
		{
			TargetClass::mLock.EnterLock();
		}

		~LockGuard()
		{
			TargetClass::mLock.LeaveLock();
		}

	};

private:
	static FastSpinlock mLock;
	
	//friend struct LockGuard;
};

template <class TargetClass>
FastSpinlock ClassTypeLock<TargetClass>::mLock;