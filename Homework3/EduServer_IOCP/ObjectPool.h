#pragma once


#include "Exception.h"
#include "FastSpinlock.h"


template <class TOBJECT, int ALLOC_COUNT = 100>
class ObjectPool : public ClassTypeLock<TOBJECT>
{
public:

	static void* operator new(size_t objSize)
	{
		//DONE: TOBJECT 타입 단위로 lock 잠금
		LockGuard lock;

		if (!mFreeList)
		{
			mFreeList = new uint8_t[sizeof(TOBJECT)*ALLOC_COUNT];

			uint8_t* pNext = mFreeList;
			uint8_t** ppCurr = reinterpret_cast<uint8_t**>(mFreeList);

			for (int i = 0; i < ALLOC_COUNT - 1; ++i)
			{
				
				pNext += sizeof(TOBJECT);
				*ppCurr = pNext;
				ppCurr = reinterpret_cast<uint8_t**>(pNext);
			}

			//마지막이 nullptr로 안 되어 있어서 추가 할당에서 자꾸 오류가 났다..
			*ppCurr = nullptr;
			mTotalAllocCount += ALLOC_COUNT;
		}

		uint8_t* pAvailable = mFreeList;
		mFreeList = *reinterpret_cast<uint8_t**>(pAvailable);
		++mCurrentUseCount;

		return pAvailable;
	}

	static void	operator delete(void* obj)
	{
		//DONE: TOBJECT 타입 단위로 lock 잠금
		LockGuard lock;

		CRASH_ASSERT(mCurrentUseCount > 0);

		--mCurrentUseCount;

		*reinterpret_cast<uint8_t**>(obj) = mFreeList;
		mFreeList = static_cast<uint8_t*>(obj);
	}


private:
	static uint8_t*	mFreeList;
	static int		mTotalAllocCount; ///< for tracing
	static int		mCurrentUseCount; ///< for tracing

};


template <class TOBJECT, int ALLOC_COUNT>
uint8_t* ObjectPool<TOBJECT, ALLOC_COUNT>::mFreeList = nullptr;

template <class TOBJECT, int ALLOC_COUNT>
int ObjectPool<TOBJECT, ALLOC_COUNT>::mTotalAllocCount = 0;

template <class TOBJECT, int ALLOC_COUNT>
int ObjectPool<TOBJECT, ALLOC_COUNT>::mCurrentUseCount = 0;


