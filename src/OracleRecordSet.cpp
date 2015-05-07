/////////////////////////////////////////////////////////////////////////////
#include "Common.h"
#include "OracleConnect.h"
#include "OracleRecordSet.h"

/////////////////////////////////////////////////////////////////////////////
// COracleRecordSet

COracleRecordSet::COracleRecordSet(COracleConnect *pOraConn)
{
	m_pOraConn = pOraConn;

	m_phOCIStmt  = NULL;
	m_phOCIError = NULL;

	memset(m_pFetchBuffer, 0, sizeof(m_pFetchBuffer));
	memset(m_pDataLens,    0, sizeof(m_pDataLens));
	memset(m_pIndicators,  0, sizeof(m_pIndicators));

	m_ulRsPointer  = 0;
	m_ulRecordsNum = 0;		// no any record now

	//m_fFieldPointer = 0;
	m_nFieldsNum    = 0;

	m_nSize    = 0;
	m_nOCIType = 0;
}

COracleRecordSet::~COracleRecordSet()
{
	Close();
}

/////////////////////////////////////////////////////////////////////////////
// IDBRecordSet Operations
bool COracleRecordSet::Query(const wchar *sql)
{
	bool ret = m_pOraConn->Execute(sql);
	if (!ret)
	{
		return false;
	}

	/* store current result set */
	m_phOCIStmt = m_pOraConn->GetResultHandle();

	/* store current error result */
	m_phOCIError = m_pOraConn->GetErrorHandle();

	Cleanup();

	m_nFieldsNum = GetFieldsCount();

	ret = BindAllColumns((int) m_nFieldsNum);
	if (!ret)
	{
		return false;
	}

	m_ulRecordsNum = GetRecordsCount();

	MoveFirst();
	//m_ulRsPointer  = 0;

	return true;
}

int COracleRecordSet::GetFieldIndex(const wchar *field)
{
	size_t len = wstrlen(field);

	wchar *pField = (wchar *) malloc(len + 1);
	wstrcpy(pField, field);

	int i;
	for (i=0; i<int(len); i++)
	{
		pField[i] = toupper(pField[i]);
	}

	for (i=0; i<(int)m_vecFieldName.size(); i++)
	{
		if (0 == wstrcmp((wchar *)m_vecFieldName[i].c_str(), pField))
		{
			free(pField);
			return i;
		}
	}

	free(pField);
	return -1;			// no match field
}

uint64 COracleRecordSet::GetRecordsCount(void)
{
	// get records count
	ub4 count = 0;
	sword result = OCIStmtFetch2(m_phOCIStmt, m_phOCIError, 1, OCI_FETCH_LAST, (sb4) 0, OCI_DEFAULT);
	if (result != OCI_SUCCESS)
	{
		return uint64(-1);
	}
/*
	if (result != OCI_NO_DATA)
	{
		printf("[ORACLE] record set have no data.\n");
		return count;
	}
*/
	result = OCIAttrGet(m_phOCIStmt, OCI_HTYPE_STMT, &count, (ub4 *) sizeof(count), OCI_ATTR_ROW_COUNT, m_phOCIError);
	if (result != OCI_SUCCESS)
	{
		return uint64(-1);
	}

	return count;
}

bool COracleRecordSet::BindAllColumns(int nFieldsNum)
{
	sword result;
	int i;
	for (i=0; i<nFieldsNum; i++)
	{
		// get next column info
		OCIParam *hParam    = NULL;
		text	 *pColName  = NULL;
		ub4		 nNameLen = 0;
		ub4		 nMaxSize = 0;

		result = OCIParamGet(m_phOCIStmt, OCI_HTYPE_STMT, m_phOCIError, reinterpret_cast <void **> (&hParam), i+1);	// first is 1
		if (result != OCI_SUCCESS)
		{
			return false;
		}

		// column name
		result = OCIAttrGet(hParam, OCI_DTYPE_PARAM, (text **)&pColName, &nNameLen, OCI_ATTR_NAME, m_phOCIError);
		if (result != OCI_SUCCESS)
		{
			return false;
		}

		std::string strColName((char *)pColName, nNameLen);
		m_vecFieldName.push_back(strColName);

		// OCI Data Type
		result = OCIAttrGet(hParam, OCI_DTYPE_PARAM, &m_nOCIType, NULL, OCI_ATTR_DATA_TYPE, m_phOCIError);
		if (result != OCI_SUCCESS)
		{
			return false;
		}

		// maximum data size to in bytes
		result = OCIAttrGet(hParam, OCI_DTYPE_PARAM, &nMaxSize, NULL, OCI_ATTR_DATA_SIZE, m_phOCIError);
		if (result != OCI_SUCCESS)
		{
			return false;
		}

		// ignore result code
		if (hParam)
		{
			OCIDescriptorFree(hParam, OCI_DTYPE_PARAM);
		}

		switch (m_nOCIType)
		{
		case SQLT_INT:			// integer
		case SQLT_LNG:			// long
		case SQLT_UIN:			// unsigned int

		case SQLT_NUM:			// numeric
		case SQLT_FLT:			// float
		case SQLT_VNU:			// numeric with length
		case SQLT_PDN:			// packed decimal
			m_nOCIType = SQLT_VNU;
			m_nSize = sizeof(OCINumber);
			break;

		case SQLT_DAT:			// date
		case SQLT_ODT:			// oci data   -   should not appear?
			m_nOCIType = SQLT_ODT;
			m_nSize = sizeof(OCIDate);
			break;

		case SQLT_CHR:			// character string
		case SQLT_STR:			// zero-terminated string
		case SQLT_VCS:			// variable - character string
		case SQLT_AFC:			// ansi fixed char
		case SQLT_AVC:			// ansi var char
		case SQLT_VST:			// oci string type
			m_nOCIType = SQLT_STR;
			m_nSize = (nMaxSize + 1) * CHAR_SIZE;	// + 1 for terminating zero!
			break;
		default:
			;
		}

		m_vecOCIType.push_back(m_nOCIType);
		m_vecColSize.push_back(m_nSize);

		OCIDefine *hDefine = NULL;

		if (m_vecOCIType[i] == SQLT_STR)
		{	
			// only text columns requires length
			result = OCIDefineByPos(m_phOCIStmt, &hDefine, m_phOCIError, i+1, m_pFetchBuffer[i], m_vecColSize[i], m_vecOCIType[i], m_pIndicators, m_pDataLens, NULL, OCI_DEFAULT);
		}
		else
		{
			result = OCIDefineByPos(m_phOCIStmt, &hDefine, m_phOCIError, i+1, m_pFetchBuffer[i], m_vecColSize[i], m_vecOCIType[i], m_pIndicators, NULL, NULL, OCI_DEFAULT);
		}

		if (result != OCI_SUCCESS)
		{
			return false;
		}
	}

	return true;
}


void COracleRecordSet::Close(void)
{
	/*
	sword result;
	if (NULL != m_phOCIStmt)
	{
		result = OCIHandleFree(m_phOCIStmt, OCI_HTYPE_STMT);
		if (result == OCI_SUCCESS)
		{
			m_phOCIStmt = NULL;
		}
	}
	*/

	Cleanup();
}


bool COracleRecordSet::Refresh(uint32 options)
{
	options;
	return true;
}

int COracleRecordSet::ConvertFieldValueType(int index)
{
	if (m_vecOCIType[index] == SQLT_VNU)
	{
		//sword result = ::OCINumberToInt(m_phOCIError, reinterpret_cast <OCINumber *> (m_pFetchBuffer[index]), sizeof(long), OCI_NUMBER_SIGNED, &m_lValue);
		sword result = ::OCINumberToReal(m_phOCIError, reinterpret_cast <OCINumber *> (m_pFetchBuffer[index]), sizeof(double), &m_dValue);
		if (result != OCI_SUCCESS)
		{
			return 0;
		}

		//_ltoa_s(m_dValue, m_pFetchBuffer[index], 10);
		sprintf(m_pFetchBuffer[index], "%lf", m_dValue);
	}
	else if (m_vecOCIType[index] == SQLT_ODT)
	{
		OCIDate * pOCIDate = reinterpret_cast <OCIDate *> (m_pFetchBuffer[index]);
		sprintf(m_pFetchBuffer[index], "%04d-%02d-%02d %02d:%02d:%02d", pOCIDate->OCIDateYYYY, 
																		pOCIDate->OCIDateMM, 
																		pOCIDate->OCIDateDD,
																		pOCIDate->OCIDateTime.OCITimeHH,
																		pOCIDate->OCIDateTime.OCITimeMI,
																		pOCIDate->OCIDateTime.OCITimeSS
																		);
	}

	return 1;
}

wchar * COracleRecordSet::GetFieldValue(const wchar *field)
{	
	int index = GetFieldIndex(field);
	if (-1 == index)
	{
		printf("[ORACLE] Failed to Get Field's Index.\n");
		return NULL;
	}
	
	if ( !ConvertFieldValueType(index) )
	{
		return NULL;
	}

	return ( reinterpret_cast <wchar *> (m_pFetchBuffer[index]) );
}

wchar * COracleRecordSet::GetFieldValue(const int index)
{
	if ( !ConvertFieldValueType(index) )
	{
		return NULL;
	}

	return ( reinterpret_cast <wchar *> (m_pFetchBuffer[index]) );
}

int COracleRecordSet::GetFieldValueInt(const wchar *field)
{
	int index = GetFieldIndex(field);
	if (-1 == index)
	{
		return int(0);
	}

	if (m_vecOCIType[index] == SQLT_VNU)
	{
		//sword result = ::OCINumberToInt(m_phOCIError, reinterpret_cast <OCINumber *> (m_pFetchBuffer[index]), sizeof(long), OCI_NUMBER_SIGNED, &m_lValue);
		sword result = ::OCINumberToReal(m_phOCIError, reinterpret_cast <OCINumber *> (m_pFetchBuffer[index]), sizeof(double), &m_nValue);
		if (result != OCI_SUCCESS)
		{
			return int(0);
		}
	}

	return m_nValue;
}

float COracleRecordSet::GetFieldValueFlt(const wchar *field)
{
	int index = GetFieldIndex(field);
	if (-1 == index)
	{
		return float(0.0);
	}

	if (m_vecOCIType[index] == SQLT_VNU)
	{
		sword result = ::OCINumberToReal(m_phOCIError, reinterpret_cast <OCINumber *> (m_pFetchBuffer[index]), sizeof(double), &m_fValue);
		if (result != OCI_SUCCESS)
		{
			return float(0.0);
		}
	}

	return m_fValue;
}

double COracleRecordSet::GetFieldValueDbl(const wchar *field)
{
	int index = GetFieldIndex(field);
	if (-1 == index)
	{
		return double(0.0);
	}

	if (m_vecOCIType[index] == SQLT_VNU)
	{
		sword result = ::OCINumberToReal(m_phOCIError, reinterpret_cast <OCINumber *> (m_pFetchBuffer[index]), sizeof(double), &m_dValue);
		if (result != OCI_SUCCESS)
		{
			return double(0.0);
		}
	}

	return m_dValue;
}

void COracleRecordSet::Cleanup()
{
	// set all to null to be save to call cleanup more than once for a single instance
	//if ( !m_vecIndicators.empty() )
	{
		m_vecIndicators.clear();
	}
	
	//if ( !m_vecFieldName.empty() )
	{
		m_vecFieldName.clear();
	}
	
	//if ( !m_vecOCIType.empty() )
	{
		m_vecOCIType.clear();
	}
	
	//if ( !m_vecColSize.empty() )
	{
		m_vecColSize.clear();
	}
}

ulong COracleRecordSet::GetFieldsCount(void)
{
	ub4 count = 0;
	sword result = OCIAttrGet(m_phOCIStmt, OCI_HTYPE_STMT, &count, NULL, OCI_ATTR_PARAM_COUNT, m_phOCIError);
	if (result != OCI_SUCCESS)
	{
		count =  result;
	}

	return count;
}

FieldTypes COracleRecordSet::GetFieldType(const int index) const
{
	FieldTypes emFieldTypes = TYPE_STRING;

	switch ( m_vecOCIType[index] )
	{
	case SQLT_NUM:                          /* (ORANET TYPE) oracle numeric */
	case SQLT_UIN:                                 /* unsigned integer */
	case SQLT_INT:                                 /* (ORANET TYPE) integer */
		emFieldTypes = TYPE_INT;
		break;

	case SQLT_FLT:                   /* (ORANET TYPE) Floating point number */
		emFieldTypes = TYPE_FLOAT;
		break;
	case SQLT_STR:								/* zero terminated string */
	case SQLT_VST:								/* OCIString type */
	case SQLT_VCS:                              /* Variable character string */
	case SQLT_CHR:								/* (ORANET TYPE) character string */
		emFieldTypes = TYPE_STRING;
		break;
	//case SQLT_VNU:                        /* NUM with preceding length byte */
	//	break;
	case SQLT_PDN:                  /* (ORANET TYPE) Packed Decimal Numeric */
	case SQLT_LNG:                                                  /* long */
		emFieldTypes = TYPE_LONG;
		break;
	
	//case SQLT_NON:                      /* Null/empty PCC Descriptor entry */
	//case SQLT_RID:                                                /* rowid */
 //
	//SQLT_VBI  15                                 /* binary in VCS format */
	// SQLT_BFLOAT 21                                /* Native Binary float*/
	//SQLT_BDOUBLE 22                             /* NAtive binary double */
	//SQLT_BIN  23                                  /* binary data(DTYBIN) */
	//SQLT_LBI  24                                          /* long binary */
 //
	//SQLT_SLS  91                        /* Display sign leading separate */
	//SQLT_LVC  94                                  /* Longer longs (char) */
	//SQLT_LVB  95                                   /* Longer long binary */
	//SQLT_AFC  96                                      /* Ansi fixed char */
	//SQLT_AVC  97                                        /* Ansi Var char */
	//SQLT_IBFLOAT  100                           /* binary float canonical */
	//SQLT_IBDOUBLE 101                          /* binary double canonical */
	//SQLT_CUR  102                                   /* cursor  type */
	//SQLT_RDD  104                                   /* rowid descriptor */
	//SQLT_LAB  105                                   /* label type */
	//SQLT_OSL  106                                   /* oslabel type */

	//SQLT_NTY  108                                  /* named object type */
	//SQLT_REF  110                                  /* ref type */
	//SQLT_CLOB 112                                  /* character lob */
	//SQLT_BLOB 113                                  /* binary lob */
	//SQLT_BFILEE 114                                /* binary file lob */
	//SQLT_CFILEE 115                                /* character file lob */
	case SQLT_RSET:                                  /* result set type */
		emFieldTypes = TYPE_SET;
		break;
	//SQLT_NCO									/* named collection type (varray or nested table) */
 
	case SQLT_DAT:                              /* date in oracle format */
	case SQLT_ODT:								/* OCIDate type */

	/* datetimes and intervals */
	case SQLT_DATE:								/* ANSI Date */
	case SQLT_TIME:								/* TIME */
	case SQLT_TIME_TZ:							/* TIME WITH TIME ZONE */
	case SQLT_TIMESTAMP:						/* TIMESTAMP */
	case SQLT_TIMESTAMP_TZ:						/* TIMESTAMP WITH TIME ZONE */
	case SQLT_INTERVAL_YM:						/* INTERVAL YEAR TO MONTH */
	case SQLT_INTERVAL_DS:						/* INTERVAL DAY TO SECOND */
	case SQLT_TIMESTAMP_LTZ:					/* TIMESTAMP WITH LOCAL TZ */
		emFieldTypes = TYPE_DATETIME;
		break;

	//SQLT_PNTY   241              /* pl/sql representation of named types */
	default:
		break;
	}
	
	return emFieldTypes;
}

FieldTypes  COracleRecordSet::GetFieldType(const wchar *field)
{
	int index = GetFieldIndex(field);

	return GetFieldType(index);
}

uint64 COracleRecordSet::GetRecordCount(void)
{
	return m_ulRecordsNum;
}

bool COracleRecordSet::IsOpen(void)
{
	return (NULL != m_phOCIStmt) ? true : false;
}

bool COracleRecordSet::IsBOF(void)
{
	return (0 == m_ulRsPointer) ? true : false;
}

bool COracleRecordSet::IsEOF(void)
{
	return (m_ulRsPointer == m_ulRecordsNum) ? true : false;
}

bool COracleRecordSet::IsFieldNull(const int index)
{
	return (m_vecIndicators[index] == -1) ? true : false;
}

bool COracleRecordSet::IsFieldNull(const wchar *field)
{
	int index = GetFieldIndex(field);

	return (m_vecIndicators[index] == -1) ? true : false;
}

void COracleRecordSet::MoveFirst(void)
{
	m_ulRsPointer = 0;

	OCIStmtFetch2(m_phOCIStmt, m_phOCIError, 1, OCI_FETCH_FIRST, (sb4) 0, OCI_DEFAULT);
}

void COracleRecordSet::MoveLast(void)
{
	m_ulRsPointer = m_ulRecordsNum;

	OCIStmtFetch2(m_phOCIStmt, m_phOCIError, 1, OCI_FETCH_LAST, (sb4) 0, OCI_DEFAULT);
}

void COracleRecordSet::MoveNext(void)
{
	m_ulRsPointer ++;
	if (m_ulRsPointer > m_ulRecordsNum)
	{
		m_ulRsPointer = m_ulRecordsNum;
	}

	OCIStmtFetch2(m_phOCIStmt, m_phOCIError, 1, OCI_FETCH_NEXT, (sb4) 0, OCI_DEFAULT);
}

void COracleRecordSet::MovePrev(void)
{
	m_ulRsPointer --;
	if (m_ulRsPointer < 0)
	{
		m_ulRsPointer = 0;
	}

	OCIStmtFetch2(m_phOCIStmt, m_phOCIError, 1, OCI_FETCH_PRIOR, (sb4) 0, OCI_DEFAULT);
}

ulong* COracleRecordSet::GetColumnLength(void)
{
	return NULL;
}

void * COracleRecordSet::GetBinaryData(const wchar *field)
{
	field;
	return NULL;
}