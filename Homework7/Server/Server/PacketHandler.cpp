#include "stdafx.h"
#include "Log.h"
#include "MyPacket.pb.h"
#include "PacketInterface.h"
#include "Player.h"
#include "ClientSession.h"
#include "BroadcastManager.h"

//@{ Handler Helper

/// 일단 고정 크기 패킷만 취급

typedef void(*HandlerFunc)(ClientSession* session, PacketHeader& pktBase, protobuf::io::CodedInputStream& payloadStream);

static HandlerFunc HandlerTable[MAX_PKT_TYPE];

static void DefaultHandler(ClientSession* session, PacketHeader& pktBase, protobuf::io::CodedInputStream& payloadStream)
{
	LoggerUtil::EventLog("invalid packet handler", session->mPlayer->GetPlayerId());
	session->DisconnectRequest(DR_ACTIVE);
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
	static void Handler_##PKT_TYPE(ClientSession* session, PacketHeader& pktBase, protobuf::io::CodedInputStream& payloadStream); \
	static RegisterHandler _register_##PKT_TYPE(PKT_TYPE, Handler_##PKT_TYPE); \
	static void Handler_##PKT_TYPE(ClientSession* session, PacketHeader& pktBase, protobuf::io::CodedInputStream& payloadStream)

//@}


void ClientSession::OnRead(size_t len)
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

		if (payloadSize < packetheader.mSize) ///< 패킷 본체 사이즈 체크
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

/////////////////////////////////////////////////////////

using namespace MyPacket;

REGISTER_HANDLER(PKT_CS_LOGIN)
{
	LoginRequest inPacket;
	if (false == inPacket.ParseFromCodedStream(&payloadStream))
	{
		LoggerUtil::EventLog("packet parsing error", PKT_CS_LOGIN);
		return;
	}
	
	/// 테스트를 위해 10ms후에 로딩하도록 ㄱㄱ
	DoSyncAfter(10, session->mPlayer, &Player::RequestLoad, inPacket.playerid());

}

REGISTER_HANDLER(PKT_CS_MOVE)
{
	MoveRequest inPacket;
	if (false == inPacket.ParseFromCodedStream(&payloadStream))
	{
		LoggerUtil::EventLog("packet parsing error", PKT_CS_MOVE);
		return;
	}

	if (inPacket.playerid() != session->mPlayer->GetPlayerId())
	{
		LoggerUtil::EventLog("PKT_CS_MOVE: invalid player ID", session->mPlayer->GetPlayerId());
		return;
	}

	const Position& pos = inPacket.playerpos();

	/// 지금은 성능 테스트를 위해 DB에 업데이트하고 통보하도록 하자.
	session->mPlayer->DoSync(&Player::RequestUpdatePosition, pos.x(), pos.y(), pos.z());
}

REGISTER_HANDLER(PKT_CS_CHAT)
{
	ChatRequest inPacket;
	if (false == inPacket.ParseFromCodedStream(&payloadStream))
	{
		LoggerUtil::EventLog("packet parsing error", PKT_CS_CHAT);
		return;
	}

	if (inPacket.playerid() != session->mPlayer->GetPlayerId())
	{
		LoggerUtil::EventLog("PKT_CS_CHAT: invalid player ID", session->mPlayer->GetPlayerId());
		return;
	}

	/// chatting의 경우 여기서 바로 방송
	ChatResult outPacket;
	std::string name;

	name.assign(session->mPlayer->GetPlayerName().begin(), session->mPlayer->GetPlayerName().end());
		
	outPacket.set_playername(name);
	*outPacket.mutable_playermessage() = inPacket.playermessage();
	GBroadcastManager->BroadcastPacket(PKT_SC_CHAT, outPacket);
}

REGISTER_HANDLER(PKT_CS_LOGOUT)
{
	LogoutRequest inPacket;

	if (false == inPacket.ParseFromCodedStream(&payloadStream))
	{
		LoggerUtil::EventLog("packet parsing error", PKT_CS_LOGOUT);
		return;
	}

	if (inPacket.playerid() != session->mPlayer->GetPlayerId())
	{
		LoggerUtil::EventLog("PKT_CS_LOGOUT : invalid player ID", session->mPlayer->GetPlayerId());
		return;
	}

	LogoutResult outPacket;
	outPacket.set_playerid(session->mPlayer->GetPlayerId());
	GBroadcastManager->BroadcastPacket(PKT_SC_LOGOUT, outPacket);
}