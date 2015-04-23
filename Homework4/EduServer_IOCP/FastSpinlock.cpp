#include "stdafx.h"
#include "Exception.h"
#include "FastSpinlock.h"
#include "LockOrderChecker.h"
#include "ThreadLocal.h"

FastSpinlock::FastSpinlock(const int lockOrder) : mLockFlag(0), mLockOrder(lockOrder)
{
}


FastSpinlock::~FastSpinlock()
{
}


void FastSpinlock::EnterWriteLock()
{
	/// 락 순서 신경 안써도 되는 경우는 그냥 패스
	if ( mLockOrder != LO_DONT_CARE)
		LLockOrderChecker->Push(this);

	while (true)
	{
		/// 다른놈이 writelock 풀어줄때까지 기다린다.
		while (mLockFlag & LF_WRITE_MASK)
			YieldProcessor();

		if ((InterlockedAdd(&mLockFlag, LF_WRITE_FLAG) & LF_WRITE_MASK) == LF_WRITE_FLAG)
		{
			/// 다른놈이 readlock 풀어줄때까지 기다린다.
			// 그냥 경쟁하면 writelock이 기아 상태에 빠질 수 있을 것 같다
			// 여기서 LF_WRITE_FLAG 값이 올라간 상태에서 read lock 다 빠질 때까지 대기함으로써
			// write 요청이 기아 상태에 빠지는 걸 방지하는 듯
			while (mLockFlag & LF_READ_MASK)
				YieldProcessor();
			
			return;
		}

		InterlockedAdd(&mLockFlag, -LF_WRITE_FLAG);
	}

}

void FastSpinlock::LeaveWriteLock()
{
	
	InterlockedAdd(&mLockFlag, -LF_WRITE_FLAG);
	
	/// 락 순서 신경 안써도 되는 경우는 그냥 패스
	if (mLockOrder != LO_DONT_CARE)
		LLockOrderChecker->Pop(this);
}

void FastSpinlock::EnterReadLock()
{
	if (mLockOrder != LO_DONT_CARE)
		LLockOrderChecker->Push(this);

	while (true)
	{
		/// 다른놈이 writelock 풀어줄때까지 기다린다.
		while (mLockFlag & LF_WRITE_MASK)
			YieldProcessor();

		//DONE: Readlock 진입 구현 (mLockFlag를 어떻게 처리하면 되는지?)
		// if ( readlock을 얻으면 )
		//return;
		// else
		// mLockFlag 원복

		//read lock 걸린 경우 - LF_WRITE_MASK랑 겹치는 게 있다면 이건 write가 먼저 들어간거
		if ((InterlockedIncrement(&mLockFlag) & LF_WRITE_MASK) == 0)
		{
			return;
		}
		
		//read lock 못 들어갔으므로 올렸던거 뺌
		InterlockedDecrement(&mLockFlag);
		
	}
}

void FastSpinlock::LeaveReadLock()
{
	//DONE: mLockFlag 처리 
	InterlockedDecrement(&mLockFlag);

	if (mLockOrder != LO_DONT_CARE)
		LLockOrderChecker->Pop(this);
}