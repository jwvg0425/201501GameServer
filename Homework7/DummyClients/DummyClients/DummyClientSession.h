#pragma once

#include "PacketInterface.h"
#include "Session.h"
#include <queue>

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

	void UpdatePlayer(int pid, const std::string& name, float x, float y, float z);

	void UpdatePlayerId(int pid);
	void UpdatePlayerName(const std::string& name);
	void UpdatePlayerPos(float x, float y, float z);

	int GetPlayerId() { return mPlayerId; }

	const std::string& GetPlayerName(){ return mPlayerName; }

	float GetX() { return mX; }
	float GetY() { return mY; }
	float GetZ() { return mZ; }

	void Login();
	void move();
	void chat();

private:
	
	SOCKADDR_IN mConnectAddr;

	int mPlayerId;
	std::string mPlayerName;
	float mX;
	float mY;
	float mZ;
} ;