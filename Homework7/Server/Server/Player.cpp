#include "stdafx.h"
#include "Timer.h"
#include "ClientSession.h"
#include "Player.h"
#include "PlayerDBContext.h"
#include "DBManager.h"
#include "GrandCentralExecuter.h"

#include "MyPacket.pb.h"
#include "PacketInterface.h"
#include "BroadcastManager.h"



Player::Player(ClientSession* session) : mSession(session)
{
	PlayerReset();
}

Player::~Player()
{
}

void Player::PlayerReset()
{
	mPlayerName = L"";
	mComment = L"";
	mPlayerId = -1;
	mIsValid = false;
	mPosX = mPosY = mPosZ = 0;
}

using namespace MyPacket;

void Player::RequestLoad(int pid)
{
 	LoadPlayerDataContext* context = new LoadPlayerDataContext(mSession, pid);
 	GDatabaseManager->PostDatabsaseRequest(context);
}

void Player::ResponseLoad(int pid, float x, float y, float z, bool valid, wchar_t* name, wchar_t* comment)
{
	mPlayerId = pid;
	mPosX = x;
	mPosY = y;
	mPosZ = z;
	mIsValid = valid;

	mPlayerName = name;
	mComment = comment;

	//TODO: 아래는 나중에 로그로...
	wprintf_s(L"PID[%d], X[%f] Y[%f] Z[%f] NAME[%s] COMMENT[%s]\n", mPlayerId, mPosX, mPosY, mPosZ, mPlayerName.c_str(), mComment.c_str());
	
	DoSyncAfter(HEART_BEAT, GetSharedFromThis<Player>(), &Player::OnTick);

	LoginResult outPacket;

	outPacket.set_playerid(mPlayerId);
	outPacket.mutable_playerpos()->set_x(mPosX);
	outPacket.mutable_playerpos()->set_y(mPosY);
	outPacket.mutable_playerpos()->set_z(mPosZ);

	std::string strName;
	strName.assign(mPlayerName.begin(), mPlayerName.end());

	outPacket.set_playername(strName);
	mSession->PostSend(PKT_SC_LOGIN, outPacket);
}

void Player::RequestUpdatePosition(float x, float y, float z)
{
	UpdatePlayerPositionContext* context = new UpdatePlayerPositionContext(mSession, mPlayerId);
	context->mPosX = x;
	context->mPosY = y;
	context->mPosZ = z;
	GDatabaseManager->PostDatabsaseRequest(context);
}

void Player::ResponseUpdatePosition(float x, float y, float z)
{
	mPosX = x;
	mPosY = y;
	mPosZ = z;

	MoveResult outPacket;

	outPacket.set_playerid(mPlayerId);
	outPacket.mutable_playerpos()->set_x(mPosX);
	outPacket.mutable_playerpos()->set_y(mPosY);
	outPacket.mutable_playerpos()->set_z(mPosZ);

	GBroadcastManager->BroadcastPacket(PKT_SC_MOVE, outPacket);
}

void Player::RequestUpdateComment(const wchar_t* comment)
{
	UpdatePlayerCommentContext* context = new UpdatePlayerCommentContext(mSession, mPlayerId);
	context->SetNewComment(comment);
	GDatabaseManager->PostDatabsaseRequest(context);
}

void Player::ResponseUpdateComment(const wchar_t* comment)
{
	mComment = comment;
}

void Player::RequestUpdateValidation(bool isValid)
{
	UpdatePlayerValidContext* context = new UpdatePlayerValidContext(mSession, mPlayerId);
	context->mIsValid = isValid;
	GDatabaseManager->PostDatabsaseRequest(context);
}

void Player::ResponseUpdateValidation(bool isValid)
{
	mIsValid = isValid;
}


void Player::TestCreatePlayerData(const wchar_t* newName)
{
	CreatePlayerDataContext* context = new CreatePlayerDataContext(mSession);
	context->SetNewName(newName);
	GDatabaseManager->PostDatabsaseRequest(context);
}

void Player::TestDeletePlayerData(int playerId)
{
	DeletePlayerDataContext* context = new DeletePlayerDataContext(mSession, playerId);
	GDatabaseManager->PostDatabsaseRequest(context);
}


void Player::OnTick()
{
	if (!IsValid())
		return;

	DoSyncAfter(HEART_BEAT, GetSharedFromThis<Player>(), &Player::OnTick);
}