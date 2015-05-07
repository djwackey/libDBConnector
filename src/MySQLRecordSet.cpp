/////////////////////////////////////////////////////////////////////////////
#include <cstdio>
#include <cstdlib>

#include "Common.h"
#include "MySQLConnect.h"
#include "MySQLRecordSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

wchar g_fieldValue[FIELD_LEN] = {0};

/////////////////////////////////////////////////////////////////////////////
// CMySQLRecordSet

CMySQLRecordSet::CMySQLRecordSet(CMySQLConnect *pMySQLConn)
{
	m_pField  = NULL;
	m_pDBRs   = NULL;

	m_uiRsPointer  = 0;
	m_uiRecordsNum = 0;

	m_ulFieldsNum   = 0;

	m_pDBConn = pMySQLConn->GetMySQLHandle();
}

CMySQLRecordSet::~CMySQLRecordSet()
{
	Close();
}

/////////////////////////////////////////////////////////////////////////////
// IDBRecordSet Operations
bool CMySQLRecordSet::Query(const wchar *sql)
{
	int ret;
#ifdef _UNICODE
	char csql[SQL_LEN] = {0};
	wcstombs(csql, sql, SQL_LEN);
	ret = mysql_real_query(m_pDBConn, csql, mblen(csql, MB_CUR_MAX));
#else
	ret = mysql_real_query(m_pDBConn, sql, (unsigned long)strlen(sql));
#endif
	if (0 != ret)
	{
		printf("[MYSQL] Failed to query data.\n");
		return false;
	}
	
	m_pDBRs = mysql_store_result(m_pDBConn);
	if (NULL == m_pDBRs)
	{
		printf("[MYSQL] Failed to store result.\n");
		return false;
	}

	m_ulFieldsNum  = mysql_num_fields(m_pDBRs);
	m_uiRecordsNum = mysql_num_rows(m_pDBRs);

	m_pField = mysql_fetch_fields(m_pDBRs);
	if (NULL == m_pField)
	{
		printf("[MYSQL] Failed to fetch fields.\n");
		return false;
	}

	m_row = mysql_fetch_row(m_pDBRs);

	m_uiRsPointer = 0;
	
	return true;
}

void CMySQLRecordSet::Close(void)
{
	if (NULL != m_pDBRs)
	{
		mysql_free_result(m_pDBRs);
		m_pDBRs = NULL;
	}
}


bool CMySQLRecordSet::Refresh(uint32 options)
{
	return (bool) ! mysql_refresh(m_pDBConn, options);
}


int CMySQLRecordSet::GetFieldIndex(const wchar *field)
{
	int nFieldIndex = -1;		// no match field

	int ret;
	ulong i;
	for (i=0; i<m_ulFieldsNum; i++)
	{
#ifdef _UNICODE
		wchar wfield[FIELD_LEN] = {0};
		mbstowcs(wfield, m_pField[i].name, FIELD_LEN);
		ret = wstrcmp(wfield, field);
#else
		ret = wstrcmp(m_pField[i].name, field);
#endif
		if (0 == ret)
		{
			nFieldIndex = i;
			break;
		}
	}

	return nFieldIndex;
}


wchar * CMySQLRecordSet::GetFieldValue(const wchar *field)
{
	int index = GetFieldIndex(field);

	if (m_uiRecordsNum < 1)
	{
		printf("[MYSQL] Records Number is below 1.\n");
		return NULL;
	}

#ifdef _UNICODE
	memset(g_fieldValue, 0, FIELD_LEN);
	mbstowcs(g_fieldValue, m_row[index], FIELD_LEN);
	return g_fieldValue;
#else
	return m_row[index];
#endif
}

wchar * CMySQLRecordSet::GetFieldValue(const int index)
{
#ifdef _UNICODE
	memset(g_fieldValue, 0, FIELD_LEN);
	mbstowcs(g_fieldValue, m_row[index], FIELD_LEN);
	return g_fieldValue;
#else
	return m_row[index];
#endif
}

int CMySQLRecordSet::GetFieldValueInt(const wchar *field)
{
	int index = GetFieldIndex(field);

	return atoi(m_row[index]);
}

float CMySQLRecordSet::GetFieldValueFlt(const wchar *field)
{
	int index = GetFieldIndex(field);

	return (float)atof(m_row[index]);
}

double CMySQLRecordSet::GetFieldValueDbl(const wchar *field)
{
	int index = GetFieldIndex(field);

	return atof(m_row[index]);
}

ulong CMySQLRecordSet::GetFieldsCount(void)
{
	return m_ulFieldsNum;
}

FieldTypes CMySQLRecordSet::GetFieldType(const int index) const
{
	FieldTypes fieldTypes = TYPE_STRING;

	switch (m_pField[index].type)
	{
	case MYSQL_TYPE_DECIMAL:
	case MYSQL_TYPE_TINY:
	case MYSQL_TYPE_SHORT:
	case MYSQL_TYPE_INT24:
	case MYSQL_TYPE_NEWDECIMAL:
		fieldTypes = TYPE_INT;
		break;
	case MYSQL_TYPE_LONG:
	case MYSQL_TYPE_LONGLONG:
		fieldTypes = TYPE_LONG;
		break;
	case MYSQL_TYPE_FLOAT:
		fieldTypes = TYPE_FLOAT;
		break;
	case MYSQL_TYPE_DOUBLE:
		fieldTypes = TYPE_DOUBLE;
		break;
	case MYSQL_TYPE_TIMESTAMP:
	case MYSQL_TYPE_DATE:
	case MYSQL_TYPE_TIME:
	case MYSQL_TYPE_DATETIME:
	case MYSQL_TYPE_YEAR:
	case MYSQL_TYPE_NEWDATE:
		fieldTypes = TYPE_DATETIME;
		break;
	case MYSQL_TYPE_VARCHAR:
	case MYSQL_TYPE_VAR_STRING:
	case MYSQL_TYPE_STRING:
		fieldTypes = TYPE_STRING;
		break;
	case MYSQL_TYPE_ENUM:
		fieldTypes = TYPE_ENUM;
		break;
	case MYSQL_TYPE_SET:
		fieldTypes = TYPE_SET;
		break;
	default:
		break;
	}

	return fieldTypes;
}


FieldTypes  CMySQLRecordSet::GetFieldType(const wchar *field)
{
	int index = GetFieldIndex(field);

	//return m_pField[iIndex].type;
	return GetFieldType(index);
}

uint64 CMySQLRecordSet::GetRecordCount(void)
{
	return m_uiRecordsNum;
}

bool CMySQLRecordSet::IsOpen(void)
{
	return (NULL != m_pDBRs->handle) ? true : false;
}

bool CMySQLRecordSet::IsBOF(void)
{
	return (0 == m_uiRsPointer) ? true : false;
}

bool CMySQLRecordSet::IsEOF(void)
{
	return (m_uiRsPointer == m_uiRecordsNum) ? true : false;
	//return mysql_eof(pRs);
}

bool CMySQLRecordSet::IsFieldNull(const int index)
{
	return (NULL == m_row[index]) ? true : false;
}

bool CMySQLRecordSet::IsFieldNull(const wchar *field)
{
	int index = GetFieldIndex(field);

	return (NULL == m_row[index]) ? true : false;
}

void CMySQLRecordSet::MoveFirst(void)
{
	mysql_data_seek(m_pDBRs, 0);
}

void CMySQLRecordSet::MoveLast(void)
{
	mysql_data_seek(m_pDBRs, m_uiRecordsNum);
}

void CMySQLRecordSet::MoveNext(void)
{
	m_uiRsPointer ++;
	if (m_uiRsPointer > m_uiRecordsNum)
	{
		m_uiRsPointer = m_uiRecordsNum;
	}
	mysql_data_seek(m_pDBRs, m_uiRsPointer);

	m_row = mysql_fetch_row(m_pDBRs);
}

void CMySQLRecordSet::MovePrev(void)
{
	m_uiRsPointer --;
	if (m_uiRsPointer < 0)
	{
		m_uiRsPointer = 0;
	}
	mysql_data_seek(m_pDBRs, m_uiRsPointer);

	m_row = mysql_fetch_row(m_pDBRs);
}

ulong* CMySQLRecordSet::GetColumnLength(void)
{
	return mysql_fetch_lengths(m_pDBRs);
}

void* CMySQLRecordSet::GetBinaryData(const wchar *field)
{
	field;
	if (m_uiRecordsNum < 1)
	{
		return NULL;
	}

	//int index = GetFieldIndex(field);

	return (void *) *m_row;
}

