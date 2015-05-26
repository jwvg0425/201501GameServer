#include "stdafx.h"
#include "Log.h"
#include "MyPacket.pb.h"
#include "PacketInterface.h"
#include "Player.h"
#include "ClientSession.h"
#include "BroadcastManager.h"

//@{ Handler Helper

/// �ϴ� ���� ũ�� ��Ŷ�� ���

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
	/// ��Ŷ �Ľ��ϰ� ó��
	protobuf::io::ArrayInputStream arrayInputStream(mRecvBuffer.GetBufferStart(), mRecvBuffer.GetContiguiousBytes());
	protobuf::io::CodedInputStream codedInputStream(&arrayInputStream);

	PacketHeader packetheader;

	while (codedInputStream.ReadRaw(&packetheader, HEADER_SIZE))
	{
		const void* payloadPos = nullptr;
		int payloadSize = 0;

		codedInputStream.GetDirectBufferPointer(&payloadPos, &payloadSize);

		if (payloadSize < packetheader.mSize) ///< ��Ŷ ��ü ������ üũ
			break;

		if (packetheader.mType >= MAX_PKT_TYPE || packetheader.mType <= 0)
		{
			DisconnectRequest(DR_ACTIVE);
			break;;
		}

		/// payload �б�
		protobuf::io::ArrayInputStream payloadArrayStream(payloadPos, packetheader.mSize);
		protobuf::io::CodedInputStream payloadInputStream(&payloadArrayStream);

		/// packet dispatch...
		HandlerTable[packetheader.mType](this, packetheader, payloadInputStream);

		/// ���� ��ŭ ���� �� ���ۿ��� ����
		codedInputStream.Skip(packetheader.mSize); ///< readraw���� ��� ũ�⸸ŭ �̸� �����߱⶧��
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
	
	/// �׽�Ʈ�� ���� 10ms�Ŀ� �ε��ϵ��� ����
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

	/// ������ ���� �׽�Ʈ�� ���� DB�� ������Ʈ�ϰ� �뺸�ϵ��� ����.
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

	/// chatting�� ��� ���⼭ �ٷ� ���
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