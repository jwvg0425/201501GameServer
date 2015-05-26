#include "stdafx.h"
#include "Exception.h"
#include "Log.h"
#include "PacketInterface.h"
#include "DummyClientSession.h"
#include "Player.h"


#include "MyPacket.pb.h"


//@{ Handler Helper

typedef void(*HandlerFunc)(DummyClientSession* session, PacketHeader& pktBase, protobuf::io::CodedInputStream& payloadStream);

static HandlerFunc HandlerTable[MAX_PKT_TYPE];

static void DefaultHandler(DummyClientSession* session, PacketHeader& pktBase, protobuf::io::CodedInputStream& payloadStream)
{
	
	printf_s("Default Handler...PKT ID: %d\n", pktBase.mType);
}

struct InitializeHandlers
{
	InitializeHandlers()
	{
		for (int i = 0; i < MAX_PKT_TYPE; ++i)
			HandlerTable[i] = DefaultHandler;
	}
} _init_handlers_;

struct RegisterHandler
{
	RegisterHandler(int pktType, HandlerFunc handler)
	{
		HandlerTable[pktType] = handler;
	}
};

#define REGISTER_HANDLER(PKT_TYPE)	\
	static void Handler_##PKT_TYPE(DummyClientSession* session, PacketHeader& pktBase, protobuf::io::CodedInputStream& payloadStream); \
	static RegisterHandler _register_##PKT_TYPE(PKT_TYPE, Handler_##PKT_TYPE); \
	static void Handler_##PKT_TYPE(DummyClientSession* session, PacketHeader& pktBase, protobuf::io::CodedInputStream& payloadStream)


//@}

void DummyClientSession::OnReceive(size_t len)
{
	
	/// 패킷 파싱하고 처리
	protobuf::io::ArrayInputStream arrayInputStream(mRecvBuffer.GetBufferStart(), mRecvBuffer.GetContiguiousBytes());
	protobuf::io::CodedInputStream codedInputStream(&arrayInputStream);

	PacketHeader packetheader;

	while (codedInputStream.ReadRaw(&packetheader, HEADER_SIZE))
	{
		const void* payloadPos = nullptr;
		int payloadSize = 0;

		codedInputStream.GetDirectBufferPointer(&payloadPos, &payloadSize);

		if ( payloadSize < packetheader.mSize ) ///< 패킷 본체 사이즈 체크
			break;

		if (packetheader.mType >= MAX_PKT_TYPE || packetheader.mType <= 0)
		{
			DisconnectRequest(DR_ACTIVE);
			break;;
		}

		/// payload 읽기
		protobuf::io::ArrayInputStream payloadArrayStream(payloadPos, packetheader.mSize);
		protobuf::io::CodedInputStream payloadInputStream(&payloadArrayStream);

		/// packet dispatch...
		HandlerTable[packetheader.mType](this, packetheader, payloadInputStream);
	
		/// 읽은 만큼 전진 및 버퍼에서 제거
		codedInputStream.Skip(packetheader.mSize); ///< readraw에서 헤더 크기만큼 미리 전진했기때문
		mRecvBuffer.Remove(HEADER_SIZE + packetheader.mSize);

	}
}

/////////////////////////////////////////////////////////////

using namespace MyPacket;

REGISTER_HANDLER(PKT_SC_LOGIN)
{
	LoginResult loginResult;
	if (false == loginResult.ParseFromCodedStream(&payloadStream))
	{
		session->DisconnectRequest(DR_ACTIVE);
		return;
	}

	const Position& pos = loginResult.playerpos();

	session->GetPlayer()->SetPlayerId(loginResult.playerid());
	session->GetPlayer()->SetPlayerName(loginResult.playername());
	session->GetPlayer()->SetPlayerPos(pos.x(), pos.y(), pos.z());
	session->GetPlayer()->Login();

	printf_s("LOGIN SUCCESS: ID[%d], NAME[%s], POS[%f, %f, %f]\n", loginResult.playerid(), loginResult.playername().c_str(), pos.x(), pos.y(), pos.z());

	DoSyncAfter(1000, session->GetPlayer(), &Player::OnTick);
}

REGISTER_HANDLER(PKT_SC_MOVE)
{
	MoveResult moveResult;
	if (false == moveResult.ParseFromCodedStream(&payloadStream))
	{
		session->DisconnectRequest(DR_ACTIVE);
		return;
	}
	auto player = session->GetPlayer();

	printf_s("player [%d] move to POS[%f, %f, %f]\n", player->GetPlayerId(), player->GetX(), player->GetY(), player->GetZ());

	const Position& pos = moveResult.playerpos();

	player->SetPlayerPos(pos.x(), pos.y(), pos.z());
}

REGISTER_HANDLER(PKT_SC_CHAT)
{
	ChatResult chatResult;

	if (false == chatResult.ParseFromCodedStream(&payloadStream))
	{
		session->DisconnectRequest(DR_ACTIVE);
		return;
	}

	printf_s("[%s] listen [%s] says [%s]\n", session->GetPlayer()->GetPlayerName(), chatResult.playername().c_str(), chatResult.playermessage().c_str());
	session->GetPlayer()->IncreaseChatNum();
}

REGISTER_HANDLER(PKT_SC_LOGOUT)
{
	LogoutResult logoutResult;

	if (false == logoutResult.ParseFromCodedStream(&payloadStream))
	{
		session->DisconnectRequest(DR_ACTIVE);
		return;
	}

	if (logoutResult.playerid() == session->GetPlayer()->GetPlayerId())
	{
		session->DisconnectRequest(DR_LOGOUT);
	}

	printf_s("player [%d] logout\n", logoutResult.playerid());
}