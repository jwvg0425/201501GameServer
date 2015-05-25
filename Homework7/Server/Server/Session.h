#pragma once
#include "CircularBuffer.h"
#include "OverlappedIOContext.h"

#include "MyPacket.pb.h"
#include "PacketInterface.h"

class Session
{
public:
	Session(size_t sendBufSize, size_t recvBufSize);
	virtual ~Session() {}

	bool IsConnected() const { return !!mConnected; }

	void DisconnectRequest(DisconnectReason dr) ;

	bool PreRecv(); ///< zero byte recv
	bool PostRecv() ;

	bool PostSend(short packetType, const google::protobuf::MessageLite& payload);
	bool FlushSend() ;

	void DisconnectCompletion(DisconnectReason dr) ;
	void SendCompletion(DWORD transferred) ;
	void RecvCompletion(DWORD transferred) ;

	void AddRef();
	void ReleaseRef();

	virtual void OnRead(size_t len) {}
	virtual void OnDisconnect(DisconnectReason dr) {}
	virtual void OnRelease() {}

	void	SetSocket(SOCKET sock) { mSocket = sock; }
	SOCKET	GetSocket() const { return mSocket; }

protected:

	SOCKET			mSocket;

	CircularBuffer	mRecvBuffer;
	CircularBuffer	mSendBuffer;
	FastSpinlock	mSendBufferLock;
	int				mSendPendingCount;

	volatile long	mRefCount;
	volatile long	mConnected;

	bool			mIsRequest = false;

};


extern __declspec(thread) std::deque<Session*>* LSendRequestSessionList;