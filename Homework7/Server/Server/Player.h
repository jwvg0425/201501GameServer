#pragma once
#include "ContentsConfig.h"
#include "SyncExecutable.h"

class ClientSession;


class Player : public SyncExecutable
{
public:
	Player(ClientSession* session);
	~Player();

	bool IsValid() { return mPlayerId > 0; }
	int  GetPlayerId() { return mPlayerId;  }
	
	void RequestLoad(int pid);
	void ResponseLoad(int pid, float x, float y, float z, bool valid, wchar_t* name, wchar_t* comment);

	void RequestUpdatePosition(float x, float y, float z);
	void ResponseUpdatePosition(float x, float y, float z);

	void RequestUpdateComment(const wchar_t* comment);
	void ResponseUpdateComment(const wchar_t* comment);

	void RequestUpdateValidation(bool isValid);
	void ResponseUpdateValidation(bool isValid);

	void OnTick(); ///< 로그인후 1초마다 불리는 기능

	const std::wstring& GetPlayerName() { return mPlayerName; }

	float GetX(){ return mPosX; }
	float GetY(){ return mPosY; }
	float GetZ(){ return mPosZ; }



private:

	void PlayerReset();

	//TODO: 테스트용, 요놈들은 사실상 외부로 빼야 함
	void TestCreatePlayerData(const wchar_t* newName);
	void TestDeletePlayerData(int playerId);

private:

	int		mPlayerId;
	float	mPosX;
	float	mPosY;
	float	mPosZ;
	bool	mIsValid;
	std::wstring	mPlayerName;
	std::wstring	mComment;
	bool	mIsLogin;

	ClientSession* const mSession;
	friend class ClientSession;
};