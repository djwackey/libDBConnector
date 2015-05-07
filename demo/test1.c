#include "IDBConnect.h"
#include <cassert>
#include <cstdio>
int main()
{
    wDBConnector::SetDBType(_ORACLE_);

    const char *host   = "192.168.1.1";
    const char *user   = "test";
    const char *passwd = "test";
    const char *db     = "db";
    const int  port    = 3306;

    IDBConnect *pDBConn = wDBConnector::Connect(host, user, passwd, db, port);
    assert(pDBConn);
    if (NULL == pDBConn)
    {
	return 0;
    }

    const char *sql = "delete from table where id = 1";
    bool bRetValue = pDBConn->Execute(sql);
    if (!bRetValue)
    {
        return 0;
    }

    pDBConn->Close();
    wDBConnector::Destroy(pDBConn);
    
    return 0;
}

