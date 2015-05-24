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

private:
	FastSpinlock mLock;

	xset<ClientSession*>::type mConnectedClientSet;
	int mCurrentConnectedCount;

};

extern std::shared_ptr<BroadcastManager> GBroadcastManager;