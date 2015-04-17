#pragma once
#include "ObjectPool.h"
#include "MemoryPool.h"
#include "CircularBuffer.h"

#define BUFSIZE	65536

class ClientSession ;
class SessionManager;

enum IOType
{
	IO_NONE,
	IO_SEND,
	IO_RECV,
	IO_RECV_ZERO,
	IO_ACCEPT,
	IO_DISCONNECT
} ;

enum DisconnectReason
{
	DR_NONE,
	DR_ACTIVE,
	DR_ONCONNECT_ERROR,
	DR_IO_REQUEST_ERROR,
	DR_COMPLETION_ERROR,
};

struct OverlappedIOContext
{
	OverlappedIOContext(ClientSession* owner, IOType ioType);

	OVERLAPPED		mOverlapped ;
	ClientSession*	mSessionObject ;
	IOType			mIoType ;
	WSABUF			mWsaBuf;
	
} ;

//DONE: 아래의 OverlappedXXXXContext는 ObjectPool<>을 사용하도록 수정

struct OverlappedSendContext : public OverlappedIOContext, public ObjectPool<OverlappedSendContext>
{
	OverlappedSendContext(ClientSession* owner) : OverlappedIOContext(owner, IO_SEND)
	{
	}
};

struct OverlappedRecvContext : public OverlappedIOContext, public ObjectPool<OverlappedRecvContext>
{
	OverlappedRecvContext(ClientSession* owner) : OverlappedIOContext(owner, IO_RECV)
	{
	}
};

struct OverlappedPreRecvContext : public OverlappedIOContext, public ObjectPool<OverlappedPreRecvContext>
{
	OverlappedPreRecvContext(ClientSession* owner) : OverlappedIOContext(owner, IO_RECV_ZERO)
	{
	}
};

struct OverlappedDisconnectContext : public OverlappedIOContext, public ObjectPool<OverlappedDisconnectContext>
{
	OverlappedDisconnectContext(ClientSession* owner, DisconnectReason dr) 
	: OverlappedIOContext(owner, IO_DISCONNECT), mDisconnectReason(dr)
	{}

	DisconnectReason mDisconnectReason;
};

//얘는 최소한 MAX_CONNECTION개는 만들게 확실하니깐 기본적으로 그만큼 풀을 만들자.
//기본 100개로 하면 메모리 추가 할당 작업이 너무 빈번하게 일어날 듯
struct OverlappedAcceptContext : public OverlappedIOContext, public ObjectPool<OverlappedAcceptContext, MAX_CONNECTION>
{
	OverlappedAcceptContext(ClientSession* owner) : OverlappedIOContext(owner, IO_ACCEPT)
	{}
};


void DeleteIoContext(OverlappedIOContext* context) ;

//DONE: 아래의 ClientSession은 xnew/xdelete사용 가능하도록 클래스 정의 부분 수정
class ClientSession : public PooledAllocatable
{
public:
	ClientSession();
	~ClientSession() {}

	void	SessionReset();

	bool	IsConnected() const { return !!mConnected; }

	bool	PostAccept();
	void	AcceptCompletion();

	bool	PreRecv() ; ///< zero byte recv

	bool	PostRecv();
	void	RecvCompletion(DWORD transferred);

	bool	PostSend();
	void	SendCompletion(DWORD transferred);
	
	void	DisconnectRequest(DisconnectReason dr);
	void	DisconnectCompletion(DisconnectReason dr);
	
	void	AddRef();
	void	ReleaseRef();

	void	SetSocket(SOCKET sock) { mSocket = sock; }
	SOCKET	GetSocket() const { return mSocket;  }

private:
	
	SOCKET			mSocket ;

	SOCKADDR_IN		mClientAddr ;
		
	FastSpinlock	mBufferLock;

	CircularBuffer	mBuffer;

	volatile long	mRefCount;
	volatile long	mConnected;

	friend class SessionManager;
} ;



