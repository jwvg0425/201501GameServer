#pragma once
#include "XTL.h"
#include "FastSpinlock.h"

#include "MyPacket.pb.h"
#include "PacketInterface.h"

struct PacketHeader;
class ClientSession;

class BroadcastManager : public std::enable_shared_from_this<BroadcastManager>
{
public:
	BroadcastManager();
	virtual ~BroadcastManager() {}

	void RegisterClient(ClientSession* client);
	void UnregisterClient(ClientSession* client);

	void BroadcastPacket(short packetType, const protobuf::MessageLite& payload);

	//x,y,z 좌표를 기준으로 radius 거리 안에 있는 클라이언트들에게만 패킷 전송
	void BroadcastPacketInRange(short packetType, const protobuf::MessageLite& payload, float x, float y, float z, float radius);

private:
	FastSpinlock mLock;

	xset<ClientSession*>::type mConnectedClientSet;
	int mCurrentConnectedCount;

};

extern std::shared_ptr<BroadcastManager> GBroadcastManager;