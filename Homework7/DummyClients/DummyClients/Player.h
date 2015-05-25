#pragma once
#include "SyncExecutable.h"

class DummyClientSession;


class Player : public SyncExecutable
{
public:
	Player(DummyClientSession* session);
	~Player();

	bool IsValid() { return mPlayerId > 0; }

	void SetPlayerId(int pid){ mPlayerId = pid; }
	int  GetPlayerId() { return mPlayerId;  }

	void OnTick(); ///< �α����� 1�ʸ��� �Ҹ��� ���

	void SetPlayerName(const std::string& name) { mPlayerName = name; }
	const std::string& GetPlayerName() { return mPlayerName; }

	
	void SetPlayerPos(float x, float y, float z)
	{
		mPosX = x;
		mPosY = y;
		mPosZ = z;
	}

	float GetX(){ return mPosX; }
	float GetY(){ return mPosY; }
	float GetZ(){ return mPosZ; }

	void Move();
	void Chat();

private:

	void PlayerReset();

	//TODO: �׽�Ʈ��, ������ ��ǻ� �ܺη� ���� ��
	void TestCreatePlayerData(const wchar_t* newName);
	void TestDeletePlayerData(int playerId);

private:

	int		mPlayerId;
	float	mPosX;
	float	mPosY;
	float	mPosZ;
	bool	mIsValid;
	std::string	mPlayerName;
	std::string	mComment;

	DummyClientSession* const mSession;
	friend class DummyClientSession;
};