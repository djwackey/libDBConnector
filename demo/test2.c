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

    wDBConnector::SetConnectParam(host, user, passwd, db, port);

    const char *sql = "delete from table where id = 1";
    bool bRetValue = wDBConnector::ExecSQL(sql);
    if (!bRetValue)
    {
        printf("failed to execute sql.\n");
        return 0;
    }

    printf("done.\n");
    
    return 0;
}

