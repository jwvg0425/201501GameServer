#include "stdafx.h"
#include "Exception.h"
#include "ThreadLocal.h"
#include "DBHelper.h"

//DONE: DbHelper의 static 멤버변수 초기화
SQLHENV DbHelper::mSqlHenv = SQL_NULL_HANDLE;
SQL_CONN* DbHelper::mSqlConnPool = nullptr;
int DbHelper::mDbWorkerThreadCount = 0;


DbHelper::DbHelper()
{
	CRASH_ASSERT(mSqlConnPool[LWorkerThreadId].mUsingNow == false);

	mCurrentSqlHstmt = mSqlConnPool[LWorkerThreadId].mSqlHstmt;
	mCurrentResultCol = 1;
	mCurrentBindParam = 1;
	CRASH_ASSERT(mCurrentSqlHstmt != nullptr);

	mSqlConnPool[LWorkerThreadId].mUsingNow = true;
}

DbHelper::~DbHelper()
{
	//DONE: SQLFreeStmt를 이용하여 현재 SQLHSTMT 해제(unbind, 파라미터리셋, close 순서로)
	///# 아래 것들은 일일이 에러체크하고 리턴시킬 필요없다. 소멸자니까 그냥 로그만 남겨도 OK. 그리고 Free가 안되는 상황이라면 사실상 서버 내려야겠지?
	SQLRETURN ret = SQLFreeStmt(mCurrentSqlHstmt, SQL_UNBIND);
	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return;
	}

	ret = SQLFreeStmt(mCurrentSqlHstmt, SQL_RESET_PARAMS);
	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return;
	}

	ret = SQLFreeStmt(mCurrentSqlHstmt, SQL_CLOSE);
	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return;
	}

	mSqlConnPool[LWorkerThreadId].mUsingNow = false;
}

bool DbHelper::Initialize(const wchar_t* connInfoStr, int workerThreadCount)
{
	//DONE: mSqlConnPool, mDbWorkerThreadCount를 워커스레스 수에 맞추어 초기화
	mDbWorkerThreadCount = workerThreadCount;
	mSqlConnPool = new SQL_CONN[workerThreadCount];


	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &mSqlHenv))
	{
		printf_s("DbHelper Initialize SQLAllocHandle failed\n");
		return false;
	}

	if (SQL_SUCCESS != SQLSetEnvAttr(mSqlHenv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER))
	{
		printf_s("DbHelper Initialize SQLSetEnvAttr failed\n");
		return false;
	}
		

	/// 스레드별로 SQL connection을 풀링하는 방식. 즉, 스레드마다 SQL서버로의 연결을 갖는다.
	for (int i = 0; i < mDbWorkerThreadCount; ++i)
	{
		//DONE: SQLAllocHandle을 이용하여 SQL_CONN의 mSqlHdbc 핸들 사용가능하도록 처리
		if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, mSqlHenv, &mSqlConnPool[i].mSqlHdbc))
		{
			printf_s("DBHelper Initialize SQLAllocHandle failed\n");
			return false;
		}
		
		SQLSMALLINT resultLen = 0;
		
		//DONE: SQLDriverConnect를 이용하여 SQL서버에 연결하고 그 핸들을 SQL_CONN의 mSqlHdbc에 할당
		SQLRETURN ret = SQLDriverConnect(mSqlConnPool[i].mSqlHdbc, NULL, (SQLWCHAR*)connInfoStr, SQL_NTS,
			nullptr, 0, nullptr, SQL_DRIVER_NOPROMPT); // =  SQLDriverConnect(...);

		///# SQLRETURN ret = SQLDriverConnect(mSqlConnPool[i].mSqlHdbc, NULL, (SQLWCHAR*)connInfoStr, (SQLSMALLINT)wcslen(connInfoStr), NULL, 0, &resultLen, SQL_DRIVER_NOPROMPT);


		if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
		{
			SQLWCHAR sqlState[1024] = { 0, } ;
			SQLINTEGER nativeError = 0;
			SQLWCHAR msgText[1024] = { 0, } ;
			SQLSMALLINT textLen = 0 ;

			SQLGetDiagRec(SQL_HANDLE_DBC, mSqlConnPool[i].mSqlHdbc, 1, sqlState, &nativeError, msgText, 1024, &textLen);

			wprintf_s(L"DbHelper Initialize SQLDriverConnect failed: %s \n", msgText);

			return false;
		}

		//DONE: SQLAllocHandle를 이용하여 SQL_CONN의 mSqlHstmt 핸들 사용가능하도록 처리
		if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, mSqlConnPool[i].mSqlHdbc, &mSqlConnPool[i].mSqlHstmt))
		{
			printf_s("DBHelper Initialize SQLAllocHandle failed\n");
			return false;
		}
	}

	return true;
}


void DbHelper::Finalize()
{
	for (int i = 0; i < mDbWorkerThreadCount; ++i)
	{
		SQL_CONN* currConn = &mSqlConnPool[i];
		if (currConn->mSqlHstmt)
			SQLFreeHandle(SQL_HANDLE_STMT, currConn->mSqlHstmt);

		if (currConn->mSqlHdbc)
			SQLFreeHandle(SQL_HANDLE_DBC, currConn->mSqlHdbc);
	}

	delete[] mSqlConnPool;


}

bool DbHelper::Execute(const wchar_t* sqlstmt)
{
	//DONE: mCurrentSqlHstmt핸들 사용하여 sqlstmt를 수행.  
	SQLRETURN ret = SQLExecDirect(mCurrentSqlHstmt, (SQLWCHAR*)sqlstmt, SQL_NTS); //= SQLExecDirect(...);
	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return false;
	}

	return true;
}

bool DbHelper::FetchRow()
{
	//DONE: mCurrentSqlHstmt가 들고 있는 내용 fetch
	SQLRETURN ret = SQLFetch(mCurrentSqlHstmt);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		if (SQL_NO_DATA != ret)
		{
			PrintSqlStmtError();
		}
		
		return false;
	}

	return true;
}



bool DbHelper::BindParamInt(int* param)
{
	//DONE: int형 파라미터 바인딩
	SQLRETURN ret = SQLBindParameter(mCurrentSqlHstmt, mCurrentBindParam++, SQL_PARAM_INPUT,
		SQL_C_LONG, SQL_INTEGER, 10 /* 자릿수 말하는 인자인가? 뭔지 잘 모르겠다*/, 0, param, 0, nullptr); // = SQLBindParameter(...);
	///# 맞다 ㅎㅎ 

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return false;
	}

	return true;
}

bool DbHelper::BindParamFloat(float* param)
{
	SQLRETURN ret = SQLBindParameter(mCurrentSqlHstmt, mCurrentBindParam++, SQL_PARAM_INPUT,
		SQL_C_FLOAT, SQL_REAL, 15, 0, param, 0, NULL);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return false;
	}

	return true;
}

bool DbHelper::BindParamBool(bool* param)
{
	//DONE: bool형 파라미터 바인딩
	SQLRETURN ret = SQLBindParameter(mCurrentSqlHstmt, mCurrentBindParam++, SQL_PARAM_INPUT,
		SQL_C_BIT, SQL_BIT, 1, 0, param, 0, nullptr); // = SQLBindParameter(...);

	///# 불은 1바이트로 처리: SQLRETURN ret = SQLBindParameter(mCurrentSqlHstmt, mCurrentBindParam++, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 3, 0, param, 0, NULL);
	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return false;
	}

	return true;
}

bool DbHelper::BindParamText(const wchar_t* text)
{

	//DONE: 유니코드 문자열 바인딩
	SQLRETURN ret = SQLBindParameter(mCurrentSqlHstmt, mCurrentBindParam++, SQL_PARAM_INPUT,
		SQL_C_WCHAR, SQL_WVARCHAR, wcslen(text), 0, (SQLPOINTER)text, 0, nullptr); // = SQLBindParameter(...);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return false;
	}

	return true;
}


void DbHelper::BindResultColumnInt(int* r)
{
	SQLLEN len = 0;
	SQLRETURN ret = SQLBindCol(mCurrentSqlHstmt, mCurrentResultCol++, SQL_C_LONG, r, 4, &len);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
	}
}
void DbHelper::BindResultColumnFloat(float* r)
{
	SQLLEN len = 0;
	//DONE: float형 결과 컬럼 바인딩
	SQLRETURN ret = SQLBindCol(mCurrentSqlHstmt, mCurrentResultCol++, SQL_C_FLOAT, r, sizeof(float), &len);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
	}
}

void DbHelper::BindResultColumnBool(bool* r)
{
	SQLLEN len = 0;
	SQLRETURN ret = SQLBindCol(mCurrentSqlHstmt, mCurrentResultCol++, SQL_C_TINYINT, r, 1, &len);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
	}
}
void DbHelper::BindResultColumnText(wchar_t* text, size_t count)
{
	SQLLEN len = 0;
	//DONE: wchar_t*형 결과 컬럼 바인딩
	SQLRETURN ret = SQLBindCol(mCurrentSqlHstmt, mCurrentResultCol++, SQL_C_WCHAR, text, sizeof(wchar_t)*count, &len);;

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
	}
}


void DbHelper::PrintSqlStmtError()
{
	SQLWCHAR sqlState[1024] = { 0, };
	SQLINTEGER nativeError = 0;
	SQLWCHAR msgText[1024] = { 0, };
	SQLSMALLINT textLen = 0;

	SQLGetDiagRec(SQL_HANDLE_STMT, mCurrentSqlHstmt, 1, sqlState, &nativeError, msgText, 1024, &textLen);

	wprintf_s(L"DbHelper SQL Statement Error: %ls \n", msgText);
}