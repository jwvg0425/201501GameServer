#pragma once

#include "PacketInterface.h"
#include "Session.h"
#include <queue>

class Player;
class DummyClientSession : public Session, public ObjectPool < DummyClientSession >
{
public:
	DummyClientSession();
	virtual ~DummyClientSession();

	bool PrepareSession();

	bool ConnectRequest();
	void ConnectCompletion();

	bool SendRequest(short packetType, const protobuf::MessageLite& payload);

	virtual void OnReceive(size_t len);
	virtual void OnRelease();

	std::shared_ptr<Player> GetPlayer(){ return mPlayer; }

	void Login();

private:
	
	SOCKADDR_IN mConnectAddr;

	std::shared_ptr<Player> mPlayer;
} ;