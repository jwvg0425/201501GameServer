#include "stdafx.h"
#include "Exception.h"
#include "BroadcastManager.h"
#include "ClientSession.h"




std::shared_ptr<BroadcastManager> GBroadcastManager;

BroadcastManager::BroadcastManager() : mCurrentConnectedCount(0), mLock(LO_ECONOMLY_CLASS)
{

}

void BroadcastManager::RegisterClient(ClientSession* client)
{
	FastSpinlockGuard criticalSection(mLock);

	auto result = mConnectedClientSet.insert(client);

	if (result.second) 
		++mCurrentConnectedCount;
	else
		CRASH_ASSERT(false); ///< �̹� �����ϴ� ���
	
}

void BroadcastManager::UnregisterClient(ClientSession* client)
{
	FastSpinlockGuard criticalSection(mLock);

	if (mConnectedClientSet.erase(client) > 0)
		--mCurrentConnectedCount;
	else
		CRASH_ASSERT(false); ///< ����� ���� ���� ���� ���� �ʴ� ����
}

void BroadcastManager::BroadcastPacket(short packetType, const protobuf::MessageLite& payload)
{
	FastSpinlockGuard criticalSection(mLock);

	for (auto it : mConnectedClientSet)
	{
		if ( false == it->PostSend(packetType, payload) )
		{
			it->DisconnectRequest(DR_ACTIVE);
		}
	}
}
