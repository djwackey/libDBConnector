/////////////////////////////////////////////////////////////////////////////
#include "Common.h"
#include "OracleConnect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COraConn

COracleConnect::COracleConnect()
{
	m_bConnectStatus = false;			// disconnect

	m_phOCIEnv   = NULL;
	m_phOCISrv   = NULL;
	m_phOCIError = NULL;
	m_phOCISess  = NULL;
	m_phOCISvc   = NULL;
	m_phOCIStmt  = NULL;

	//m_phClob	= NULL;
	m_phBlob	= NULL;
	m_phInsertBind	= NULL;
	m_phBind	= NULL;
	m_phDefine	= NULL;

	memset(m_szErrorText, 0, sizeof(m_szErrorText));
}

COracleConnect::~COracleConnect()
{
	Close();
}

/////////////////////////////////////////////////////////////////////////////
// COracleConnect Operations

bool COracleConnect::Connect(const wchar *host, const wchar *user, const wchar *passwd, const wchar *db, const uint32 port)
{
	/*--------reserved-------*/
	db;
	port;
	/*----------end----------*/

	// prerequisites
	assert(NULL != host);
	assert(NULL != user);
	assert(NULL != passwd);

	sword result;
	result = OCIInitialize((ub4) OCI_DEFAULT, (dvoid *)0, (dvoid * (*)(dvoid *, size_t)) 0, (dvoid * (*)(dvoid *, dvoid *, size_t))0, (void (*)(dvoid *, dvoid *)) 0 );
	if (result != OCI_SUCCESS)
	{
		return (m_bConnectStatus = false);
	}

	// allocate an environment handle
	result = OCIEnvCreate(&m_phOCIEnv, OCI_THREADED | OCI_OBJECT, (dvoid *) 0, 0, 0, 0, (size_t) 0, (dvoid **) 0);
	if (result != OCI_SUCCESS)
	{
		return (m_bConnectStatus = false);
	}

	// allocate a server handle
	result = OCIHandleAlloc((dvoid *) m_phOCIEnv, (dvoid **) &m_phOCISrv, OCI_HTYPE_SERVER, 0, (dvoid **) 0);
	if (result != OCI_SUCCESS)
	{
		return (m_bConnectStatus = false);
	}

	// allocate an error handle
	result = OCIHandleAlloc((dvoid *) m_phOCIEnv, (dvoid **) &m_phOCIError, OCI_HTYPE_ERROR, 0, (dvoid **) 0);
	if (result != OCI_SUCCESS)
	{
		return (m_bConnectStatus = false);
	}

	// allocate an statement handle
	result = OCIHandleAlloc((dvoid *) m_phOCIEnv, (dvoid **) &m_phOCIStmt, (ub4) OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0);
	if (result != OCI_SUCCESS)
	{
		return (m_bConnectStatus = false);
	}

	// create a server context
#ifdef _UNICODE
	char chost[PARAM_LEN] = {0};
	wcstombs(chost, host, PARAM_LEN);
	result = OCIServerAttach(m_phOCISrv, m_phOCIError, (text *) chost, (ub4) mblen(chost, MB_CUR_MAX), OCI_DEFAULT);
#else
	result = OCIServerAttach(m_phOCISrv, m_phOCIError, (text *) host, (ub4) strlen(host), OCI_DEFAULT);
#endif
	if ( result != OCI_SUCCESS )
	{
		return (m_bConnectStatus = false);
	}

	// allocate a service handle
	result = OCIHandleAlloc((dvoid *) m_phOCIEnv, (dvoid **) &m_phOCISvc, OCI_HTYPE_SVCCTX, 0, (dvoid **) 0);
	if (result != OCI_SUCCESS)
	{
		return (m_bConnectStatus = false);
	}

	// set the server attribute in the service context handle
	result = OCIAttrSet((dvoid *) m_phOCISvc, OCI_HTYPE_SVCCTX, (dvoid *) m_phOCISrv, (ub4) 0, OCI_ATTR_SERVER, m_phOCIError);
	if (result != OCI_SUCCESS)
	{
		return (m_bConnectStatus = false);
	}

	// allocate a user session handle
	result = OCIHandleAlloc((dvoid *) m_phOCIEnv, (dvoid **) &m_phOCISess, OCI_HTYPE_SESSION, 0, (dvoid **) 0);
	if (result != OCI_SUCCESS)
	{
		return (m_bConnectStatus = false);
	}
/*
	result = OCIDescriptorAlloc((dvoid *) m_phOCIEnv, (dvoid **) &m_phClob, (ub4)OCI_DTYPE_LOB, (size_t) 0, (dvoid **) 0);
	if (result != OCI_SUCCESS)
	{
		return 0;
	}
*
	/* allocate the lob locator variables */
	if (OCIDescriptorAlloc((dvoid *) m_phOCIEnv, (dvoid **) &m_phBlob, (ub4)OCI_DTYPE_LOB, (size_t) 0, (dvoid **) 0))
	{
		return (m_bConnectStatus = false);
	}
	
	// set username and password attributes in user session handle
#ifdef _UNICODE
	char cuser[PARAM_LEN] = {0};
	wcstombs(cuser, user, PARAM_LEN);
	result = OCIAttrSet((dvoid *) m_phOCISess, OCI_HTYPE_SESSION, (dvoid *) cuser, (ub4) mblen(cuser, MB_CUR_MAX), OCI_ATTR_USERNAME, m_phOCIError);
#else
	result = OCIAttrSet((dvoid *) m_phOCISess, OCI_HTYPE_SESSION, (dvoid *) user, (ub4) strlen(user), OCI_ATTR_USERNAME, m_phOCIError);
#endif
	if (result != OCI_SUCCESS)
	{
		return (m_bConnectStatus = false);
	}
	
#ifdef _UNICODE
	char cpswd[PARAM_LEN] = {0};
	wcstombs(cpswd, passwd, PARAM_LEN);
	result = OCIAttrSet((dvoid *) m_phOCISess, OCI_HTYPE_SESSION, (dvoid *) cpswd, (ub4) mblen(cpswd, MB_CUR_MAX), OCI_ATTR_PASSWORD, m_phOCIError);
#else
	result = OCIAttrSet((dvoid *) m_phOCISess, OCI_HTYPE_SESSION, (dvoid *) passwd, (ub4) strlen(passwd), OCI_ATTR_PASSWORD, m_phOCIError);
#endif
	if (result != OCI_SUCCESS)
	{
		return (m_bConnectStatus = false);
	}

	// start the session
	result = OCISessionBegin(m_phOCISvc, m_phOCIError, m_phOCISess, OCI_CRED_RDBMS, OCI_DEFAULT);
	if (result != OCI_SUCCESS)
	{
		return (m_bConnectStatus = false);
	}

	// set the user session attribute in the service context handle
	result = OCIAttrSet((dvoid *) m_phOCISvc, OCI_HTYPE_SVCCTX, (dvoid *) m_phOCISess, (ub4) 0, OCI_ATTR_SESSION, m_phOCIError);
	if (result != OCI_SUCCESS)
	{
		return (m_bConnectStatus = false);
	}
	
	return (m_bConnectStatus = true);
}

void COracleConnect::Close(void)
{
	if (m_bConnectStatus)
	{
		m_bConnectStatus = false;

		// just in case switch server to blocking mode
		//sword result;
		//if (m_phOCISrv)
		//{
			//ub1 attrValue = 1;
			//result = OCIAttrSet(m_phOCISrv, OCI_HTYPE_SERVER, &attrValue, sizeof(attrValue), OCI_ATTR_NONBLOCKING_MODE, m_phOCIError);
		//}
		// end session
		if (m_phOCISess)
		{
			OCISessionEnd(m_phOCISvc, m_phOCIError, m_phOCISess, OCI_DEFAULT);
		}

		// detach from the oracle server
		OCIServerDetach(m_phOCISrv, m_phOCIError, OCI_DEFAULT);
		
		// free handles
		if (m_phOCISess)
		{
			sword result;

			if (NULL != m_phOCISess)
			{
				result = OCIHandleFree(m_phOCISess, OCI_HTYPE_STMT);
				if (result == OCI_SUCCESS)
				{
					m_phOCISess = NULL;
				}
			}

			if (NULL != m_phOCISrv)
			{
				result = OCIHandleFree(m_phOCISrv, OCI_HTYPE_STMT);
				if (result == OCI_SUCCESS)
				{
					m_phOCISrv = NULL;
				}
			}

			if (NULL != m_phOCIError)
			{
				result = OCIHandleFree(m_phOCIError, OCI_HTYPE_STMT);
				if (result == OCI_SUCCESS)
				{
					m_phOCIError = NULL;
				}
			}
			
			if (NULL != m_phOCIStmt)
			{
				result = OCIHandleFree(m_phOCIStmt, OCI_HTYPE_STMT);
				if (result == OCI_SUCCESS)
				{
					m_phOCIStmt = NULL;
				}
			}

			//OCIHandleFree((dvoid *) m_phClob, OCI_DTYPE_LOB);
			//OCIHandleFree((dvoid *) m_phBlob, OCI_DTYPE_LOB);
			//OCIHandleFree((dvoid *) m_phInsertBind, OCI_HTYPE_BIND);
			//OCIHandleFree((dvoid *) m_phBind, OCI_HTYPE_BIND);
			//OCIHandleFree((dvoid *) m_phDefine, OCI_HTYPE_DEFINE);

			m_phOCISess  = NULL;
			m_phOCIError = NULL;
			m_phOCISrv   = NULL;

			//m_phClob = NULL;
			//m_phBlob = NULL;
			//m_phInsertBind = NULL;
			//m_phBind = NULL;
			//m_phDefine = NULL;
		}

		if (m_phOCIEnv)
		{
			OCIHandleFree((dvoid *) m_phOCIEnv, OCI_HTYPE_ENV);
			m_phOCIEnv = NULL;
		}

		//OCITerminate(OCI_DEFAULT);
	}
}

bool COracleConnect::Execute(const wchar *sql)
{
	assert(NULL != sql);

	sword result;
#ifdef _UNICODE
	char csql[SQL_LEN] = {0};
	wcstombs(csql, sql, SQL_LEN);
	result = OCIStmtPrepare(m_phOCIStmt, m_phOCIError, (text *) csql, (ub4) mblen(csql, MB_CUR_MAX), OCI_NTV_SYNTAX, OCI_DEFAULT);
#else
	result = OCIStmtPrepare(m_phOCIStmt, m_phOCIError, (text *) sql, (ub4) strlen(sql), OCI_NTV_SYNTAX, OCI_DEFAULT);
#endif
	if (result != OCI_SUCCESS)
	{
		return false;
	}

	ub2 stmt_type = 0;
	result = OCIAttrGet(m_phOCIStmt, OCI_HTYPE_STMT, &stmt_type, NULL, OCI_ATTR_STMT_TYPE, m_phOCIError);
	if (result != OCI_SUCCESS)
	{
		return false;
	}

	ub4 iters = (stmt_type == OCI_STMT_SELECT) ? 0 : 1;

	ub4 mode = (stmt_type == OCI_STMT_SELECT) ? OCI_STMT_SCROLLABLE_READONLY | OCI_COMMIT_ON_SUCCESS : OCI_DEFAULT | OCI_COMMIT_ON_SUCCESS;
	//ub4 mode = OCI_COMMIT_ON_SUCCESS;

	result = OCIStmtExecute(m_phOCISvc, m_phOCIStmt, m_phOCIError, iters, 0, NULL, NULL, mode);
	if (result != OCI_SUCCESS)
	{
		return false;
	}

	return true;
}

bool COracleConnect::StorBinData(const wchar *insert_sql, const wchar *sql, const wchar *binfilename)
{
	int colc = 1;
	sword result;
#ifdef _UNICODE
	char cinsertsql[SQL_LEN] = {0};
	wcstombs(cinsertsql, insert_sql, SQL_LEN);
	result = OCIStmtPrepare(m_phOCIStmt, m_phOCIError, (text *) cinsertsql, (ub4) mblen(cinsertsql, MB_CUR_MAX), OCI_NTV_SYNTAX, OCI_DEFAULT);
#else
	result = OCIStmtPrepare(m_phOCIStmt, m_phOCIError, (OraText *) insert_sql, (ub4) strlen((wchar *) insert_sql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
#endif
	if (result != OCI_SUCCESS)
	{
		printf("FAILED: OCIStmtPrepare() insstmt\n");
		return false;
	}

	if (OCIBindByPos(m_phOCIStmt, &m_phInsertBind, m_phOCIError, (ub4) 1,
		(dvoid *) &colc, (sb4) sizeof(colc), SQLT_INT,
		(dvoid *) 0, (ub2 *)0, (ub2 *)0,
		(ub4) 0, (ub4 *) 0, (ub4) OCI_DEFAULT))
	{
		printf("FAILED: OCIBindByPos()\n");
		return false;
	}

	if (OCIStmtExecute(m_phOCISvc, m_phOCIStmt, m_phOCIError, (ub4) 1, (ub4) 0,
		(CONST OCISnapshot *) 0, (OCISnapshot *) 0,
		(ub4) OCI_DEFAULT))
	{
		printf("FAILED: OCIStmtExecute() insstmt\n");
		return false;
	}

	OCITransCommit(m_phOCISvc, m_phOCIError, (ub4)0);

#ifdef _UNICODE
	char csql[SQL_LEN] = {0};
	wcstombs(csql, sql, SQL_LEN);
	result = SelectLocator((text *)csql);
#else
	result = SelectLocator((text *)sql);
#endif
	if ( OCI_SUCCESS != result )
	{
		return false;
	}
	
#ifdef _UNICODE
	char cfilename[PARAM_LEN] = {0};
	wcstombs(cfilename, binfilename, PARAM_LEN);
	FILE *fp = fopen(cfilename, (const char *) "rb");
#else
	FILE *fp = fopen(binfilename, (const char *) "rb");
#endif
	if (NULL != fp)
	{
		printf("ERROR: Failed to open file(s).\n");
		return false;
	}

	ub4 binfilelen = FileLength(fp);

	StreamWriteLob(m_phBlob, fp, binfilelen);
	OCITransCommit(m_phOCISvc, m_phOCIError, (ub4)0);

	fclose(fp);

	return true;
}

bool COracleConnect::ReadBinData(const wchar *binfilename)
{
#ifdef _UNICODE
	char cfilename[PARAM_LEN] = {0};
	wcstombs(cfilename, binfilename, PARAM_LEN);
	FILE *fp = fopen(cfilename, (const char *) "rb");
#else
	FILE *fp = fopen(binfilename, (const char *) "rb");
#endif
	if (NULL != fp)
	{
		printf("ERROR: Failed to open file(s).\n");
		return false;
	}

	StreamReadLob(m_phBlob, fp);

	fclose(fp);

	return true;
}

bool COracleConnect::IsOpen(void)
{
	return m_bConnectStatus;
}

/*
int COracleConn::SetConnOptions(enum mysql_option option, const char * arg)
{
	return 1;
}
*/

sb4 COracleConnect::SelectLocator(text *sqlstmt)
{
	int colc = 1;
	if (OCIStmtPrepare(m_phOCIStmt, m_phOCIError, sqlstmt, (ub4) strlen((char *)sqlstmt), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT))
	{
		printf("FAILED: OCIStmtPrepare() sqlstmt\n");
		return OCI_ERROR;
	}

	if (OCIBindByPos(m_phOCIStmt, &m_phBind, m_phOCIError, (ub4) 1,
		(dvoid *) &colc, (sb4) sizeof(colc), SQLT_INT,
		(dvoid *) 0, (ub2 *)0, (ub2 *)0,
		(ub4) 0, (ub4 *) 0, (ub4) OCI_DEFAULT))
	{
		printf("FAILED: OCIBindByPos()\n");
		return OCI_ERROR;
	}

	if (OCIDefineByPos(m_phOCIStmt, &m_phDefine, m_phOCIError, (ub4) 2,
		(dvoid *) &m_phBlob, (sb4) -1, (ub2) SQLT_BLOB,
		(dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) OCI_DEFAULT))
	{
		printf("FAILED: OCIDefineByPos()\n");
		return OCI_ERROR;
	}

	/* execute the select and fetch one row */
	if (OCIStmtExecute(m_phOCISvc, m_phOCIStmt, m_phOCIError, (ub4) 1, (ub4) 0,
		(CONST OCISnapshot*) 0, (OCISnapshot*) 0,
		(ub4) OCI_DEFAULT))
	{
		printf("FAILED: OCIStmtExecute() sqlstmt\n");
		return OCI_ERROR;
	}

	return OCI_SUCCESS;
}

/* ----------------------------------------------------------------- */
/* get the length of the input file.                                 */
/* ----------------------------------------------------------------- */
ub4 COracleConnect::FileLength(FILE *fp)
{
	fseek(fp, 0, SEEK_END);
	return (ub4) (ftell(fp));
}

/* ----------------------------------------------------------------- */
/* Read operating system files into local buffers and then write the */
/* buffers to lobs.                                                  */
/* ----------------------------------------------------------------- */
void COracleConnect::StreamWriteLob(OCILobLocator *lobl, FILE *fp, ub4 filelen)
{
	ub4   offset = 1;
	ub4   loblen = 0;
	ub1   bufp[MAX_LEN];
	ub4   amtp = filelen;
	ub1   piece;
	sword retval;
	//int   readval;
	//ub4   len = 0;
	ub4   nbytes;
	ub4   remainder = filelen;

	printf("--> To do streamed write lob, amount = %d\n", filelen);

	OCILobGetLength(m_phOCISvc, m_phOCIError, lobl, &loblen);
	printf("Before stream write, LOB length = %d\n\n", loblen);

	fseek(fp, 0, 0);

	if (filelen > MAX_LEN)
		nbytes = MAX_LEN;
	else
		nbytes = filelen;

	if (fread((void *)bufp, (size_t)nbytes, 1, fp) != 1)
	{
		printf("ERROR: read file.\n");
		return;
	}

	remainder -= nbytes;

	if (remainder == 0)       /* exactly one piece in the file */
	{
		printf("Only one piece, no need for stream write.\n");
		retval = OCILobWrite(m_phOCISvc, m_phOCIError, lobl, &amtp, offset, (dvoid *) bufp,
			(ub4) nbytes, OCI_ONE_PIECE, (dvoid *)0,
			(sb4 (*)(dvoid *, dvoid *, ub4 *, ub1 *)) 0,
			(ub2) 0, (ub1) SQLCS_IMPLICIT);
		if (retval != OCI_SUCCESS)
		{
			printf("ERROR: OCILobWrite(), retval = %d\n", retval);
			return;
		}
	}
	else                     /* more than one piece */
	{
		if (OCILobWrite(m_phOCISvc, m_phOCIError, lobl, &amtp, offset, (dvoid *) bufp,
			(ub4) MAX_LEN, OCI_FIRST_PIECE, (dvoid *)0,
			(sb4 (*)(dvoid *, dvoid *, ub4 *, ub1 *)) 0,
			(ub2) 0, (ub1) SQLCS_IMPLICIT) != OCI_NEED_DATA)
		{
			printf("ERROR: OCILobWrite().\n");
			return;
		}

		piece = OCI_NEXT_PIECE;

		do
		{
			if (remainder > MAX_LEN)
				nbytes = MAX_LEN;
			else
			{
				nbytes = remainder;
				piece = OCI_LAST_PIECE;
			}

			if (fread((void *)bufp, (size_t)nbytes, 1, fp) != 1)
			{
				printf("ERROR: read file.\n");
				piece = OCI_LAST_PIECE;
			}

			retval = OCILobWrite(m_phOCISvc, m_phOCIError, lobl, &amtp, offset, (dvoid *) bufp,
				(ub4) nbytes, piece, (dvoid *)0,
				(sb4 (*)(dvoid *, dvoid *, ub4 *, ub1 *)) 0,
				(ub2) 0, (ub1) SQLCS_IMPLICIT);
			remainder -= nbytes;

		} while (retval == OCI_NEED_DATA && !feof(fp));
	}

	if (retval != OCI_SUCCESS)
	{
		printf("Error: stream writing LOB.\n");
		return;
	}

	OCILobGetLength(m_phOCISvc, m_phOCIError, lobl, &loblen);
	printf("After stream write, LOB length = %d\n\n", loblen);

	return;
}

/* ----------------------------------------------------------------- */
/* Read lobs using stream mode into local buffers and then write     */
/* them to operating system files.                                   */
/* ----------------------------------------------------------------- */
void COracleConnect::StreamReadLob(OCILobLocator *lobl, FILE *fp)
{
	ub4   offset = 1;
	ub4   loblen = 0;
	ub1   bufp[MAX_LEN];
	ub4   amtp = 0;
	sword retval;
	ub4   piece = 0;
	ub4   remainder;            /* the number of bytes for the last piece */

	OCILobGetLength(m_phOCISvc, m_phOCIError, lobl, &loblen);
	amtp = loblen;

	printf("--> To stream read LOB, loblen = %d.\n", loblen);

	memset(bufp, '\0', MAX_LEN);

	retval = OCILobRead(m_phOCISvc, m_phOCIError, lobl, &amtp, offset, (dvoid *) bufp,
		(loblen < MAX_LEN ? loblen : MAX_LEN), (dvoid *)0,
		(sb4 (*)(dvoid *, const dvoid *, ub4, ub1)) 0,
		(ub2) 0, (ub1) SQLCS_IMPLICIT);

	switch (retval)
	{
	case OCI_SUCCESS:             /* only one piece */
		printf("stream read %d th piece\n", ++piece);
		fwrite((void *)bufp, (size_t)loblen, 1, fp);
		break;
	case OCI_ERROR:
		//report_error();
		break;	
	case OCI_NEED_DATA:           /* there are 2 or more pieces */

		remainder = loblen;

		fwrite((void *)bufp, MAX_LEN, 1, fp); /* full buffer to write */

		do
		{
			memset(bufp, '\0', MAX_LEN);
			amtp = 0;

			remainder -= MAX_LEN;

			retval = OCILobRead(m_phOCISvc, m_phOCIError, lobl, &amtp, offset, (dvoid *) bufp,
				(ub4) MAX_LEN, (dvoid *)0,
				(sb4 (*)(dvoid *, const dvoid *, ub4, ub1)) 0,
				(ub2) 0, (ub1) SQLCS_IMPLICIT);

			/* the amount read returned is undefined for FIRST, NEXT pieces */
			printf("stream read %d th piece, amtp = %d\n", ++piece, amtp);

			if (remainder < MAX_LEN)     /* last piece not a full buffer piece */
				fwrite((void *)bufp, (size_t)remainder, 1, fp);
			else
				fwrite((void *)bufp, MAX_LEN, 1, fp);

		} while (retval == OCI_NEED_DATA);
		break;
	default:
		printf("Unexpected ERROR: OCILobRead() LOB.\n");
		break;
	}

	return;
}

bool COracleConnect::BeginTrans(void)
{
	return (OCITransPrepare(m_phOCISvc, m_phOCIError, OCI_DEFAULT) == OCI_SUCCESS) ? true : false;
}

bool COracleConnect::CommitTrans(void)
{
	return (OCITransCommit(m_phOCISvc, m_phOCIError, OCI_DEFAULT) == OCI_SUCCESS) ? true : false;
}

bool COracleConnect::RollbackTrans(void)
{
	return (OCITransRollback(m_phOCISvc, m_phOCIError, OCI_DEFAULT) == OCI_SUCCESS) ? true : false;
}

bool COracleConnect::EndTrans(void)
{
	return (OCITransDetach(m_phOCISvc, m_phOCIError, OCI_DEFAULT) == OCI_SUCCESS) ? true : false;
}

int COracleConnect::GetLastError(void)
{
	OraText *pSqlState = NULL;
	sb4		sbErrCode  = 0;
	ub4		ubType	   = OCI_HTYPE_ERROR;

	OCIErrorGet(m_phOCIError, 1, pSqlState, &sbErrCode, m_szErrorText, sizeof(m_szErrorText), ubType);
	printf("[ORALCE] Error Code: %d, Error Text: %s\n", sbErrCode, m_szErrorText);
	return sbErrCode;
}

wchar *COracleConnect::GetErrorText(void)
{
	GetLastError();
	return (wchar *)m_szErrorText;
}

int COracleConnect::GetRealString(const wchar *inbuffer, uint32 length, wchar * &outbuffer)
{
	inbuffer;
	length;
	outbuffer;
	return 0;
}