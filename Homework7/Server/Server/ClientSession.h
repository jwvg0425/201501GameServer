#pragma once

#include "MemoryPool.h"
#include "Session.h"
#include "Player.h"

class ClientSessionManager;

class ClientSession : public Session, public PooledAllocatable
{
public:
	ClientSession();
	virtual ~ClientSession();

	void SessionReset();

	bool PostAccept();
	void AcceptCompletion();
	
	template <class PKT_TYPE>
	bool ParsePacket(PKT_TYPE& pkt)
	{
		return mRecvBuffer.Read((char*)&pkt, pkt.mSize);
	}

	virtual void OnRead(size_t len);
	virtual void OnDisconnect(DisconnectReason dr);
	virtual void OnRelease();

public:
	std::shared_ptr<Player> mPlayer;

private:
	
	SOCKADDR_IN		mClientAddr ;

	
	friend class ClientSessionManager;
} ;



