#include "IDBConnect.h"
#include <cassert>
#include <cstdio>

int main()
{
    IDBConnect *pDBConn = wDBConnector::Create(_ORACLE_);
    assert(pDBConn);
    if (NULL == pDBConn)
    {
	return 0;
    }

    const char *host   = "192.168.1.1";
    const char *user   = "test";
    const char *passwd = "test";
    const char *db     = "db";
    const int  port    = 3306;
    bool bRetValue = pDBConn->Connect(host, user, passwd, db, port);
    if (!bRetValue)
    {
	wDBConnector:: Destroy(pDBConn);
		
	return 0;
    }
    
    IDBRecordSet *pDBRs = wDBConnector::Create(pDBConn);	// create a recordset instance…
    assert(pDBRs)
    if (NULL == pDBRs)
    {
	wDBConnector:: Destroy(pDBConn);

	return 0;
    }

    const char *sql = "select * from table";
    bRetValue = pDBRs->Query(sql);
    if (!bRetValue)
    {
	goto END;
    }

    char *pName = NULL;
    while (!pDBRs->IsEOF())
    {
	pName = pDBRs->GetFieldValue("Name");
	printf("%s\n", pName);

	pDBRs->MoveNext();
    }

END:
    pDBRs->Close();
    pDBConn->Close();

    wDBConnector::Destroy(pDBRs);
    wDBConnector::Destroy(pDBConn);

    return 0;
}

