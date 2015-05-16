#pragma once

#include <iostream>
#include "Exception.h"
#include "ThreadLocal.h"

class ThreadCallHistory
{
public:
	ThreadCallHistory(int tid) : mThreadId(tid)
	{
		memset(mHistory, 0, sizeof(mHistory));
	}

	inline void Append(const char* funsig)
	{
		mHistory[mCounter++ % MAX_HISTORY] = funsig;
	}

	void DumpOut(std::ostream& ost = std::cout);

private:
	enum
	{
		MAX_HISTORY = 1024
	};

	uint64_t	mCounter = 0;
	int			mThreadId = -1;
	const char*	mHistory[MAX_HISTORY];
};


#define TRACE_THIS	\
	__if_exists (this)	\
	{	\
		LRecentThisPointer = (void*)this;	\
	}	\
	if (LThreadType != THREAD_MAIN)	\
			{	\
		/*DONE: 스레드로컬에 함수 호출(__FUNCSIG__) 기록남기기*/ \
		LThreadCallHistory->Append( __FUNCSIG__); \
	}	
	


class ThreadCallElapsedRecord
{
public:
	ThreadCallElapsedRecord(int tid) : mThreadId(tid)
	{
		memset(mElapsedFuncSig, 0, sizeof(mElapsedFuncSig));
		memset(mElapsedTime, 0, sizeof(mElapsedTime));
	}

	inline void Append(const char* funcsig, int64_t elapsed)
	{
		//DONE: mElapsedFuncSig, mElapsedTime에 정보(funcsig, elapsed) 남기기
		mElapsedFuncSig[mCounter % MAX_ELAPSED_RECORD] = funcsig;
		mElapsedTime[mCounter % MAX_ELAPSED_RECORD] = elapsed;

		mCounter++;
	}

	void DumpOut(std::ostream& ost = std::cout);

private:
	enum
	{
		MAX_ELAPSED_RECORD = 512
	};

	uint64_t	mCounter = 0;
	int			mThreadId = -1;
	const char*	mElapsedFuncSig[MAX_ELAPSED_RECORD];
	int64_t		mElapsedTime[MAX_ELAPSED_RECORD];
};

class ScopeElapsedCheck
{
public:
	ScopeElapsedCheck(const char* funcsig) : mFuncSig(funcsig)
	{
		/* FYI
		 * 10~16 ms 해상도로 체크하려면 GetTickCount 사용
		 * 1 us 해상도로 체크하려면  QueryPerformanceCounter 사용
		*/ 
		mStartTick = GetTickCount64();
	}

	~ScopeElapsedCheck()
	{
		if (LThreadType != THREAD_MAIN)
		{
			//DONE: LThreadCallElapsedRecord에 함수 수행 시간 남기기
			LThreadCallElapsedRecord->Append(mFuncSig, GetTickCount64() - mStartTick);
		}
	}

private:

	const char*	mFuncSig;
	int64_t	mStartTick = 0;
};

#define TRACE_PERF	\
	ScopeElapsedCheck __scope_elapsed_check__(__FUNCSIG__);


namespace LoggerUtil
{

	struct LogEvent
	{
		int mThreadId = -1;
		int	mAdditionalInfo = 0;
		const char* mMessage = nullptr; 
	};

	#define MAX_LOG_SIZE  65536   ///< Must be a power of 2

	extern LogEvent gLogEvents[MAX_LOG_SIZE];
	extern __int64 gCurrentLogIndex;

	inline void EventLog(const char* msg, int info)
	{
		__int64 index = _InterlockedIncrement64(&gCurrentLogIndex) - 1;
		
		//DONE: gLogEvents에 LogEvent정보 남기기
		//배열 인덱스 안 터지게 순환하면서 저장. 근데 이러면 선후 관계 헷갈릴 것 같은데 상관없을려나
		//출력할 때 인덱스를 적절한 위치에서 시작하게 하는 군.. 상관 없다고 합니다
		gLogEvents[index % MAX_LOG_SIZE].mMessage = msg;
		gLogEvents[index % MAX_LOG_SIZE].mAdditionalInfo = info;
		gLogEvents[index % MAX_LOG_SIZE].mThreadId = LWorkerThreadId;
	}

	void EventLogDumpOut(std::ostream& ost = std::cout);
}

#define EVENT_LOG(x, info) LoggerUtil::EventLog(x, info)

