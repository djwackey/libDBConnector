/*
*
* Title : DataBase Connection Interface Header File
* Author: wackey
* E-Mail: worcy_kiddy@126.com
* Version:1.1
* Describe:
*
* ChangeLog:
* 2011.09.14 - fixed connect database slowly(return a value need a long time),if a error host parameters infilling.
             - refer function:connect(), ConnDBServer().
			 - add namespace "wDBConnector"
*/

#ifndef _DB_CONNECT_H_
#define _DB_CONNECT_H_

#include "IDBRecordSet.h"

#define _CRT_SECURE_NO_WARNINGS
#define EXPORT __declspec(dllexport)

enum DB_TYPE {_MYSQL_, _ORACLE_};

class IDBConnect
{
// Construction and Destruction
public:
	IDBConnect();
	virtual ~IDBConnect();

public:
	virtual bool Connect( const _wTCHAR *host, 
						  const _wTCHAR *user, 
						  const _wTCHAR *passwd, 
						  const _wTCHAR *db, 
						  const uint32 port) = 0;
	virtual void Close(void) = 0;
	virtual bool Execute(const _wTCHAR *sql) = 0;
	virtual bool StorBinData(const _wTCHAR *insert_sql, const _wTCHAR *sql, const _wTCHAR * binfilename) = 0;
	virtual	bool ReadBinData(const _wTCHAR * binfilename) = 0;

	// connection status
	virtual bool IsOpen(void) = 0;

	// reserved
	//virtual int SetConnOptions(enum mysql_option option, const char * arg) = 0;

	// transaction
	virtual bool BeginTrans(void) = 0;
	virtual bool RollbackTrans(void) = 0;
	virtual bool CommitTrans(void) = 0;
	virtual bool EndTrans(void) = 0;

	virtual int GetLastError(void) = 0;
	virtual int GetRealString(const _wTCHAR *inBuffer, uint32 length, _wTCHAR *&outBuffer) = 0;
};

/* public interface function */
namespace wDBConnector
{
EXPORT    void SetDBType(DB_TYPE dbType);
EXPORT DB_TYPE GetDBType();
	
EXPORT IDBConnect*	 Create(DB_TYPE dbType);
EXPORT IDBRecordSet* Create(IDBConnect *pDBConn);

EXPORT void Destroy(IDBConnect *&pDBConn);
EXPORT void Destroy(IDBRecordSet *&pDBRs);

// 调用Connect函数前，一定要先执行SetDBType函数，
// 否则将启用默认数据库连接方式(ORACLE)
EXPORT IDBConnect * Connect(const _wTCHAR *host, 
		   					const _wTCHAR *user, 
						    const _wTCHAR *passwd, 
						    const _wTCHAR *db, 
						    const uint32 port);

// 适用于短连接，调用ExecSQL函数前，一定要先执行SetConnectParam函数
// ExecSQL函数执行一次SQL语句即结束
EXPORT void SetConnectParam(const _wTCHAR *host, 
							const _wTCHAR *user, 
							const _wTCHAR *passwd, 
							const _wTCHAR *db, 
							const uint32 port);
EXPORT bool ExecSQL(const _wTCHAR *sql);

// 以下函数 等于 调用连接对象指针或记录集对象指针的关闭函数 + 调用命名域的销毁函数
// pDBConn->Close();               or pDBRs->Close();
// wDBConnector::Destroy(pDBConn); or wDBConnector::Destroy(pDBRs);
EXPORT void Close(IDBConnect *&pDBConn);
EXPORT void Close(IDBRecordSet *&pDBRs);

EXPORT int  GetErrorCode();
}

#endif