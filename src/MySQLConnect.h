#ifndef _MYSQL_CONNECT_H_
#define _MYSQL_CONNECT_H_

#ifdef WIN32
#include <winsock2.h>
#pragma comment(lib, "lib/mysql/libmysql.lib")
#endif

#include <mysql.h>
#include "IDBConnect.h"

//#define USE_OLD_FUNCTIONS

class CMySQLConnect : IDBConnect
{
public:
	CMySQLConnect();
	virtual ~CMySQLConnect();

public:
	virtual bool Connect(const wchar *host, const wchar *user, const wchar *passwd, const wchar *db, const uint32 port);
	virtual void Close(void);
	virtual bool Execute(const wchar *sql);
	virtual bool StorBinData(const wchar *insertsql, const wchar *sql, const wchar *binfilename);
	virtual	bool ReadBinData(const wchar *binfilename);

	virtual bool IsOpen(void);

	//virtual int SetConnOptions(enum mysql_option option, const char * arg);

	virtual bool BeginTrans(void);
	virtual bool RollbackTrans(void);
	virtual bool CommitTrans(void);
	virtual bool EndTrans(void);
	
	virtual int GetLastError(void);
	virtual wchar *GetErrorText(void);
	virtual int GetRealString(const wchar *inbuffer, uint32 length, wchar *&outbuffer);

	MYSQL *GetMySQLHandle(void) const { return m_pMySQL; }

private:
	MYSQL *m_pMySQL;
	MYSQL *m_pConn;

	MYSQL_STMT *m_pStmt;

	bool m_bConnectStatus;
};

#endif
