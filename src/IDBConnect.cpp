/////////////////////////////////////////////////////////////////////////////
#include "Common.h"
#include "IDBConnect.h"

#include "MySQLConnect.h"
#include "MySQLRecordSet.h"
//#include "OracleConnect.h"
//#include "OracleRecordSet.h"

/////////////////////////////////////////////////////////////////////////////
// IDBConnect

namespace wDBConnector
{
	wchar g_host[PARAM_LEN] = {0};
	wchar g_user[PARAM_LEN] = {0};
	wchar g_passwd[PARAM_LEN] = {0};
	wchar g_db[PARAM_LEN] = {0};
	uint32 g_port = 0;

	DB_TYPE g_dbType = _ORACLE_;


void SetDBType(DB_TYPE dbType)
{
	g_dbType = dbType;
}

DB_TYPE GetDBType()
{
	return g_dbType;
}

IDBConnect * Connect(const wchar *host, const wchar *user, const wchar *passwd, const wchar *db, const uint32 port)
{
	IDBConnect *pDBConn = NULL;
	do {
		pDBConn = Create(g_dbType);
		if (!pDBConn)
		{
			break;
		}

		bool bRetValue = pDBConn->Connect(host, user, passwd, db, port);
		if (!bRetValue)
		{
			Close(pDBConn);
			break;
		}
	} while (true);
	return pDBConn;
}

void SetConnectParam(const wchar *host, const wchar *user, const wchar *passwd, const wchar *db, const uint32 port)
{
	assert(host);
	assert(user);
	assert(passwd);
	assert(db);
	assert(port > 0);

	wstrcpy(g_host,   host);
	wstrcpy(g_user,   user);
	wstrcpy(g_passwd, passwd);
	wstrcpy(g_db,     db);

	g_port = port;
}

bool ExecSQL(const wchar *sql)
{
	IDBConnect *pDBConn = Connect(g_host, g_user, g_passwd, g_db, g_port); 
	if (!pDBConn)
	{
		return false;
	}

	bool bRetValue = pDBConn->Execute(sql);

	Close(pDBConn);

	return bRetValue;
}


IDBConnect * Create(DB_TYPE dbType)
{
	IDBConnect *pDBConn = NULL;

	SetDBType(dbType);
	switch (dbType)
	{
	case _MYSQL_:
		{
			pDBConn = (IDBConnect *) new CMySQLConnect;
			break;
		}
	case _ORACLE_:
		{
			//pDBConn = (IDBConnect *) new COracleConnect;
			break;
		}
	default:
		break;
	}

	return pDBConn;
}

void Destroy(IDBConnect *&pDBConn)
{
	delete pDBConn;
	pDBConn = NULL;
}

void Close(IDBConnect *&pDBConn)
{
	if (NULL != pDBConn)
	{
		Destroy(pDBConn);
	}
}


IDBRecordSet * Create(IDBConnect *pDBConn)
{
	IDBRecordSet *pDBRs = NULL;

	assert(pDBConn);
	if (NULL == pDBConn)
	{
		goto END;
	}

	switch (g_dbType)
	{
	case _MYSQL_:
		{
			pDBRs = (IDBRecordSet *) new CMySQLRecordSet( (CMySQLConnect *) pDBConn );
			break;
		}
	case _ORACLE_:
		{
			//pDBRs = (IDBRecordSet *) new COracleRecordSet( (COracleConnect *) pDBConn );
			break;
		}
	default:
		break;
	}

END:
	return pDBRs;
}

void Destroy(IDBRecordSet *&pDBRs)
{
	delete pDBRs;
	pDBRs = NULL;
}

void Close(IDBRecordSet *&pDBRs)
{
	if (NULL != pDBRs)
	{
		Destroy(pDBRs);
	}
}

int GetErrorCode(IDBConnect *pDBConn)
{
	if (NULL == pDBConn)
		return 0;

	return pDBConn->GetLastError();
}

}
//	end of namespace IDBConnector

IDBConnect::IDBConnect()
{

}

IDBConnect::~IDBConnect()
{
}

/////////////////////////////////////////////////////////////////////////////
// IDBConnect Operations
