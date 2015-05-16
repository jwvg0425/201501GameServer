#include "stdafx.h"
#include "Log.h"

#include <iostream>

void ThreadCallHistory::DumpOut(std::ostream& ost)
{
	//DONE: 현재 스레드의 call history를 ost 스트림에 쓰기
	uint64_t count = mCounter < MAX_HISTORY ? mCounter : MAX_HISTORY;

	ost << "===== Call History [Thread:" << mThreadId << "]" << std::endl;


	for (int i = 1; i <= count; i++)
	{
		ost << mHistory[(mCounter - i) % MAX_HISTORY] << std::endl;
	}

	ost << "===== End of Call History" << std::endl << std::endl;
}
	

void ThreadCallElapsedRecord::DumpOut(std::ostream& ost)
{
	uint64_t count = mCounter < MAX_ELAPSED_RECORD ? mCounter : MAX_ELAPSED_RECORD;

	ost << "===== Recent Call Performance [Thread:" << mThreadId << "]" << std::endl;

	for (int i = 1; i <= count; ++i)
	{
		ost << "  FUNC:" << mElapsedFuncSig[(mCounter - i) % MAX_ELAPSED_RECORD] 
			<< "ELAPSED: " << mElapsedTime[(mCounter - i) % MAX_ELAPSED_RECORD] << std::endl;
	}
	ost << "===== End of Recent Call Performance" << std::endl << std::endl;

}


namespace LoggerUtil
{
	LogEvent gLogEvents[MAX_LOG_SIZE];
	__int64 gCurrentLogIndex = 0;

	void EventLogDumpOut(std::ostream& ost)
	{
		//DONE: gLogEvents내용 ost 스트림에 쓰기
		uint64_t count = gCurrentLogIndex < MAX_LOG_SIZE ? gCurrentLogIndex : MAX_LOG_SIZE;

		ost << "===== Log Event " << std::endl;

		for (int i = 1; i <= count; ++i)
		{
			int idx = (gCurrentLogIndex - i) % MAX_LOG_SIZE;
			ost << "[Thread : " << gLogEvents[idx].mThreadId <<
				 "] [Info : " << gLogEvents[idx].mAdditionalInfo <<
				 "] [Message : " << gLogEvents[idx].mMessage << 
				 "]" << std::endl;
		}
		ost << "===== End of Log Event" << std::endl << std::endl;
		
	}
}
