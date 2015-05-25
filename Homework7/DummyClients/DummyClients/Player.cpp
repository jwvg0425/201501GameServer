#include "stdafx.h"
#include "Timer.h"
#include "Player.h"

#include "MyPacket.pb.h"
#include "PacketInterface.h"
#include "ThreadLocal.h"

#include "DummyClientSession.h"



Player::Player(DummyClientSession* session) : mSession(session)
{
	PlayerReset();
}

Player::~Player()
{
}

void Player::PlayerReset()
{
	mPlayerName = "";
	mComment = "";
	mPlayerId = -1;
	mIsValid = false;
	mPosX = mPosY = mPosZ = 0;
}

using namespace MyPacket;

void Player::OnTick()
{
	if (!IsValid())
		return;

	if (rand() % 20 == 0)
	{
		Chat();
	}
	else
	{
		Move();
	}

	DoSyncAfter(100, GetSharedFromThis<Player>(), &Player::OnTick);
}

void Player::Move()
{
	MyPacket::MoveRequest moveRequest;
	moveRequest.set_playerid(mPlayerId);
	int xRate, yRate, zRate;
	const float speed = 3.0f;

	xRate = rand() % 10000;
	yRate = rand() % (10000 - xRate);
	zRate = rand() % (10000 - xRate - yRate);

	mPosX = mPosX + speed*xRate*0.0001f;
	mPosY = mPosY + speed*yRate*0.0001f;
	mPosZ = mPosZ + speed*zRate*0.0001f;

	moveRequest.mutable_playerpos()->set_x(mPosX);
	moveRequest.mutable_playerpos()->set_y(mPosY);
	moveRequest.mutable_playerpos()->set_z(mPosZ);

	mSession->SendRequest(MyPacket::PKT_CS_MOVE, moveRequest);
}

void Player::Chat()
{
	MyPacket::ChatRequest chatRequest;
	chatRequest.set_playerid(mPlayerId);

	switch (rand() % 5)
	{
	case 0:
		*chatRequest.mutable_playermessage() = "test message 1";
		break;
	case 1:
		*chatRequest.mutable_playermessage() = "hello i'm fine thank you";
		break;
	case 2:
		*chatRequest.mutable_playermessage() = "Why do Java Programmers wear glasses?";
		break;
	case 3:
		*chatRequest.mutable_playermessage() = "Because they don't see #";
		break;
	case 4:
		*chatRequest.mutable_playermessage() = "Do you know kimchi?";
		break;
	}

	mSession->SendRequest(MyPacket::PKT_CS_CHAT, chatRequest);
}