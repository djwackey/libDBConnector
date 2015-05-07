#ifndef _ORACLE_CONNECT_H_
#define _ORACLE_CONNECT_H_

#include <oci.h>
#pragma comment(lib, "oci")

#include "IDBConnect.h"

class COracleConnect : IDBConnect
{
public:
	COracleConnect();
	virtual ~COracleConnect();
	
public:
	virtual bool Connect(const wchar *host, const wchar *user, const wchar *passwd, const wchar *db, const uint32 port);
	virtual void Close(void);
	virtual bool Execute(const wchar *sql);
	virtual bool StorBinData(const wchar *insertsql, const wchar *sql, const wchar *binfilename);
	virtual	bool ReadBinData(const wchar *binfilename);

	//virtual int SetConnectOptions(enum mysql_option option, const char * arg);

	virtual bool IsOpen(void);

	virtual bool BeginTrans(void);
	virtual bool RollbackTrans(void);
	virtual bool CommitTrans(void);
	virtual bool EndTrans(void);

	virtual int GetLastError(void);
	virtual wchar *GetErrorText(void);
	virtual int GetRealString(const wchar *inbuffer, uint32 length, wchar *&outbuffer);

	OCIStmt  * GetResultHandle(void) const { return m_phOCIStmt; };
	OCIError * GetErrorHandle(void)  const { return m_phOCIError; };

protected:
	sb4		   SelectLocator(text *sqlstmt);
	ub4		   FileLength(FILE *fp);
	void	   StreamWriteLob(OCILobLocator *lobl, FILE *fp, ub4 filelen);
	void	   StreamReadLob(OCILobLocator *lobl, FILE *fp);
	
private:
	OCIEnv		*m_phOCIEnv;		// environment handle
	OCIServer	*m_phOCISrv;		// server handle
	OCIError	*m_phOCIError;		// error handle
	OCISession	*m_phOCISess;		// user session handle
	OCISvcCtx	*m_phOCISvc;		// server context
	OCIStmt		*m_phOCIStmt;
	//OCILobLocator *m_phClob;
	OCILobLocator *m_phBlob;
	OCIBind		  *m_phInsertBind;
	OCIBind       *m_phBind;
	OCIDefine     *m_phDefine;

	OraText m_szErrorText[100];

	bool	m_bConnectStatus;
};

#endif