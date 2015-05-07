/////////////////////////////////////////////////////////////////////////////
#include <cstdio>
#include <cassert>
#include "Common.h"
#include "MySQLConnect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMySQLConn

CMySQLConnect::CMySQLConnect()
{
	mysql_library_init(NULL, NULL, NULL);
	
	m_pMySQL = mysql_init(NULL);
	//mysql_init(&m_pMySQL);

	m_bConnectStatus = false;		// disconnection
}

CMySQLConnect::~CMySQLConnect()
{
	Close();

	mysql_library_end();
}

/////////////////////////////////////////////////////////////////////////////
// IDBConnect Operations

bool CMySQLConnect::Connect(const wchar *host, const wchar *user, const wchar *passwd, const wchar *db, const uint32 port)
{
	assert(NULL != host);
	assert(NULL != user);
	assert(NULL != passwd);
	assert(NULL != db);
	assert(port > 0);

#ifdef _UNICODE
	char chost[PARAM_LEN] = {0};
	char cuser[PARAM_LEN] = {0};
	char cpswd[PARAM_LEN] = {0};
	char cdb[PARAM_LEN]   = {0};
	wcstombs(chost, host,   PARAM_LEN);
	wcstombs(cuser, user,   PARAM_LEN);
	wcstombs(cpswd, passwd, PARAM_LEN);
	wcstombs(cdb,   db,     PARAM_LEN);
	m_pConn = mysql_real_connect(m_pMySQL, chost, cuser, cpswd, cdb, port, NULL, 0);
#else
	m_pConn = mysql_real_connect(m_pMySQL, host, user, passwd, db, port, NULL, 0);
#endif
	if (m_pConn == NULL)
	{
		printf("[MYSQL] Failed to Connect Database.\n");
		return (m_bConnectStatus = false);
	}

	if ( mysql_set_character_set(m_pMySQL, "gbk") )
	{
		printf("[MYSQL] Failed to set Character set.\n");
		return false;
	}

	return (m_bConnectStatus = true);
}

void CMySQLConnect::Close(void)
{
	if (NULL != m_pMySQL)
	{
		m_bConnectStatus = false;

		mysql_close(m_pMySQL);
		//m_pMySQL = NULL;
	}
}

bool CMySQLConnect::Execute(const wchar *sql)
{
	assert(NULL != sql);

	int ret;
#ifdef _UNICODE
	char csql[SQL_LEN] = {0};
	wcstombs(csql, sql, SQL_LEN);
	ret = mysql_real_query(m_pMySQL, csql, mblen(csql, MB_CUR_MAX));
#else
	ret = mysql_real_query(m_pMySQL, sql, (unsigned long)strlen(sql));
#endif

	return (bool) !ret;
}

bool CMySQLConnect::StorBinData(const wchar *insert_sql, const wchar *sql, const wchar *binfilename)
{
	insert_sql;
	sql;
	binfilename;
	return false;
}
	
bool CMySQLConnect::ReadBinData(const wchar *binfilename)
{
	binfilename;
	return false;
}

/*
int CMySQLConnect::SetConnOptions(enum mysql_option option, const char * arg)
{
	return mysql_options(m_pMySQL, option, arg);
}
*/

bool CMySQLConnect::IsOpen(void)
{
	if (m_bConnectStatus)
	{
		return (bool) ! mysql_ping(m_pMySQL);
	}
	
	return false;
}

bool CMySQLConnect::BeginTrans(void)
{
	return (bool) ! mysql_autocommit(m_pMySQL, 0);	// cancel AutoCommit mode...
}

bool CMySQLConnect::CommitTrans(void)
{
	return (bool) ! mysql_commit(m_pMySQL);
}

bool CMySQLConnect::RollbackTrans(void)
{
	return (bool) ! mysql_rollback(m_pMySQL);
}

bool CMySQLConnect::EndTrans(void)
{
	return (bool) ! mysql_autocommit(m_pMySQL, 1);
}

int CMySQLConnect::GetLastError(void)
{
	return mysql_errno(m_pConn);
}

wchar *CMySQLConnect::GetErrorText(void)
{
	return (wchar *)mysql_error(m_pConn);
}

int CMySQLConnect::GetRealString(const wchar *inbuffer, uint32 length, wchar *&outbuffer)
{
	int ret;
#ifdef _UNICODE
	char cIntBuffer[BUFFER_LEN] = {0};
	char cOutBuffer[BUFFER_LEN] = {0};
	wchar_t wOutBuffer[BUFFER_LEN] = {0};
	wcstombs(cIntBuffer, inbuffer, BUFFER_LEN);
	ret = mysql_real_escape_string(m_pMySQL, cOutBuffer, cIntBuffer, length);

	mbstowcs(wOutBuffer, cOutBuffer, BUFFER_LEN);
	wstrcpy(outBuffer, wOutBuffer);
#else
	ret = mysql_real_escape_string(m_pMySQL, outbuffer, inbuffer, length);
#endif
 
	return ret;
}

