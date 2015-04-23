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
	/// �� ���� �Ű� �Ƚᵵ �Ǵ� ���� �׳� �н�
	if ( mLockOrder != LO_DONT_CARE)
		LLockOrderChecker->Push(this);

	while (true)
	{
		/// �ٸ����� writelock Ǯ���ٶ����� ��ٸ���.
		while (mLockFlag & LF_WRITE_MASK)
			YieldProcessor();

		if ((InterlockedAdd(&mLockFlag, LF_WRITE_FLAG) & LF_WRITE_MASK) == LF_WRITE_FLAG)
		{
			/// �ٸ����� readlock Ǯ���ٶ����� ��ٸ���.
			// �׳� �����ϸ� writelock�� ��� ���¿� ���� �� ���� �� ����
			// ���⼭ LF_WRITE_FLAG ���� �ö� ���¿��� read lock �� ���� ������ ��������ν�
			// write ��û�� ��� ���¿� ������ �� �����ϴ� ��
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
	
	/// �� ���� �Ű� �Ƚᵵ �Ǵ� ���� �׳� �н�
	if (mLockOrder != LO_DONT_CARE)
		LLockOrderChecker->Pop(this);
}

void FastSpinlock::EnterReadLock()
{
	if (mLockOrder != LO_DONT_CARE)
		LLockOrderChecker->Push(this);

	while (true)
	{
		/// �ٸ����� writelock Ǯ���ٶ����� ��ٸ���.
		while (mLockFlag & LF_WRITE_MASK)
			YieldProcessor();

		//DONE: Readlock ���� ���� (mLockFlag�� ��� ó���ϸ� �Ǵ���?)
		// if ( readlock�� ������ )
		//return;
		// else
		// mLockFlag ����

		//read lock �ɸ� ��� - LF_WRITE_MASK�� ��ġ�� �� �ִٸ� �̰� write�� ���� ����
		if ((InterlockedIncrement(&mLockFlag) & LF_WRITE_MASK) == 0)
		{
			return;
		}
		
		//read lock �� �����Ƿ� �÷ȴ��� ��
		InterlockedDecrement(&mLockFlag);
		
	}
}

void FastSpinlock::LeaveReadLock()
{
	//DONE: mLockFlag ó�� 
	InterlockedDecrement(&mLockFlag);

	if (mLockOrder != LO_DONT_CARE)
		LLockOrderChecker->Pop(this);
}