#include "stdafx.h"
#include "ThreadLocal.h"
#include "Exception.h"
#include "SyncExecutable.h"
#include "Timer.h"



Timer::Timer()
{
	LTickCount = GetTickCount64();
}


void Timer::PushTimerJob(SyncExecutablePtr owner, const TimerTask& task, uint32_t after)
{
	CRASH_ASSERT(LThreadType == THREAD_IO_WORKER);

	//DONE: mTimerJobQueue�� TimerJobElement�� push..
	//����ִ� �������κ��� after��ŭ �ڿ� ����ǵ���.
	mTimerJobQueue.push(TimerJobElement(owner, task, LTickCount + after));
}


void Timer::DoTimerJob()
{
	/// thread tick update
	LTickCount = GetTickCount64();

	while (!mTimerJobQueue.empty())
	{
		//�� ���� ������!
		//�̻��ϰ� �Ŵ� lock�� ������ lock�� ��� �ٸ�.
		//lock�� �ɰ� �۾� �����߿� �ٸ� ������鿡�� �۾��� push�ϸ� ���� ���̸鼭 crash�� ����.
		//�� ã�ƺ��� STLAllocator���� ���� �޸� Ǯ�� �̿��ؼ� �޸𸮸� ������
		//push�� �� vector�� �޸� ���Ӽ��� �����ϱ� ���� �޸� �� �����ϴ� ���ҵ� ��ġ���� �ٲ� �� �ִ�
		//�޸� ��ġ �ű� -> �����ڴ� ���� ��ġ�� ����Ű�� ���� -> ����!
		//����ü �� ������ 1���� �۾� ���� ���ϴٰ� ���ڱ� ������ 1�� mJobElement�� ������ 2���� �۾� �� �����ϴ� mJobElement�� �ٲ �ߴµ�
		//�޸𸮰� �� ���� �޸� Ǯ���� ���Ҵ��� �� ������ 1�� �޸� �ٸ� ��� �̵� ->
		//������ 2�� �������� ������ �޸� ��� �̵� -> ���� ���� ������ 1���� ���� �޸� �ּҷ� ��(�� �� �Ҵ��س��� ��Ȱ���ϴϱ�)
		//������ 1�� mJobElement�� ����Ű�� �ּҰ� ������ 2�� �޸� ���� � �ּҷ� �� �� ����
		//�Ƹ� �̷� ���� ������ �׷� ���ذ� �� ���� ������ �߻����� �ʾҳ� ����
		//���� �����ڰ� �ƴ϶� �� ���縦 �̿��ϸ� �ƹ� ���� ���� ��.
		TimerJobElement timerJobElem = mTimerJobQueue.top();

		if (LTickCount < timerJobElem.mExecutionTick)
			break;

		timerJobElem.mOwner->EnterLock();
		
		timerJobElem.mTask();

		timerJobElem.mOwner->LeaveLock();
		
		mTimerJobQueue.pop();
	}


}

