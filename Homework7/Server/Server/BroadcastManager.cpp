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
		CRASH_ASSERT(false); ///< 이미 존재하는 경우
	
}

void BroadcastManager::UnregisterClient(ClientSession* client)
{
	FastSpinlockGuard criticalSection(mLock);

	if (mConnectedClientSet.erase(client) > 0)
		--mCurrentConnectedCount;
	else
		CRASH_ASSERT(false); ///< 지울게 없는 경우로 쌍이 맞지 않는 경우다
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

void BroadcastManager::BroadcastPacketInRange(short packetType, const protobuf::MessageLite& payload, float x, float y, float z, float radius)
{
	FastSpinlockGuard criticalSection(mLock);

	for (auto it : mConnectedClientSet)
	{
		float xDiff = it->mPlayer->GetX() - x;
		float yDiff = it->mPlayer->GetY() - y;
		float zDiff = it->mPlayer->GetZ() - z;

		if (xDiff*xDiff + yDiff*yDiff + zDiff*zDiff > radius * radius)
		{
			continue;
		}

		if (false == it->PostSend(packetType, payload))
		{
			it->DisconnectRequest(DR_ACTIVE);
		}
	}
}
