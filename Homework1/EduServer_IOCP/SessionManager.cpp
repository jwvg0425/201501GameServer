#include "stdafx.h"
#include "ClientSession.h"
#include "SessionManager.h"

SessionManager* GSessionManager = nullptr;

ClientSession* SessionManager::CreateClientSession(SOCKET sock)
{
	ClientSession* client = new ClientSession(sock);

	{
		FastSpinlockGuard lock(mLock);

		mClientList.insert(ClientList::value_type(sock, client));
	}

	return client;
}


void SessionManager::DeleteClientSession(ClientSession* client)
{
	{
		FastSpinlockGuard lock(mLock);

		mClientList.erase(client->mSocket);
	}
	
	delete client;
}