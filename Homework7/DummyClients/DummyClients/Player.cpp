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
	mChatNum = 0;
	mIsLogin = false;
}

using namespace MyPacket;

void Player::OnTick()
{
	if (!IsValid())
		return;

	if (rand() % 2 == 0)
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
	if (!mIsLogin)
		return;

	MyPacket::MoveRequest moveRequest;
	moveRequest.set_playerid(mPlayerId);
	int xRate, yRate, zRate;
	const float speed = 0.3f;

	xRate = rand() % 10000;
	yRate = rand() % (10000 - xRate);
	zRate = rand() % (10000 - xRate - yRate);

	if (rand() % 2)
		xRate = -xRate;

	if (rand() % 2)
		yRate = -yRate;

	if (rand() % 2)
		zRate = -zRate;

	mPosX = mPosX + speed*xRate*0.0001f;
	mPosY = mPosY + speed*yRate*0.0001f;
	mPosZ = mPosZ + speed*zRate*0.0001f;

	if (mPosX < 0.0f) mPosX = 0.0f;
	if (mPosX > 50.0f) mPosX = 50.0f;
	if (mPosY < 0.0f) mPosY = 0.0f;
	if (mPosY > 50.0f) mPosY = 50.0f;
	if (mPosZ < 0.0f) mPosZ = 0.0f;
	if (mPosZ > 50.0f) mPosZ = 50.0f;

	moveRequest.mutable_playerpos()->set_x(mPosX);
	moveRequest.mutable_playerpos()->set_y(mPosY);
	moveRequest.mutable_playerpos()->set_z(mPosZ);

	mSession->SendRequest(MyPacket::PKT_CS_MOVE, moveRequest);
}

void Player::Chat()
{
	if (!mIsLogin)
		return;

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

void Player::IncreaseChatNum()
{
	if (InterlockedIncrement(&mChatNum) == 100)
	{
		MyPacket::LogoutRequest logoutRequest;
		logoutRequest.set_playerid(mPlayerId);

		mIsLogin = false;
		mSession->SendRequest(MyPacket::PKT_CS_LOGOUT, logoutRequest);
	}
}

void Player::Login()
{
	mIsLogin = true;
}
