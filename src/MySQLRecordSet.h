#ifndef _MYSQL_RECORDSET_H_
#define _MYSQL_RECORDSET_H_

#ifdef WIN32
#include <winsock2.h>
#pragma comment(lib, "lib/mysql/libmysql.lib")
#endif

#include <mysql.h>
#include "IDBRecordSet.h"

class CMySQLConnect;
class CMySQLRecordSet : IDBRecordSet
{
public:
	CMySQLRecordSet(CMySQLConnect *pMySQLConn);
	virtual ~CMySQLRecordSet();
	
public:
	virtual bool Query(const wchar *sql);
	virtual void Close(void);
	virtual bool Refresh(uint32 options);
	
	virtual wchar *GetFieldValue(const wchar *field);
	virtual wchar *GetFieldValue(const int index);
	virtual int	   GetFieldValueInt(const wchar *field);
	virtual float  GetFieldValueFlt(const wchar *field);
	virtual double GetFieldValueDbl(const wchar *field);
	virtual void  *GetBinaryData(const wchar *field);
	
	virtual ulong  GetFieldsCount(void);
	virtual uint64 GetRecordCount(void);
	virtual ulong *GetColumnLength(void);
	virtual FieldTypes GetFieldType(const int index) const;
	virtual FieldTypes GetFieldType(const wchar *field);

	int GetFieldIndex(const wchar *field);
	
	virtual bool IsOpen(void);
	virtual bool IsBOF(void);
	virtual bool IsEOF(void);
	virtual bool IsFieldNull(const int index);
	virtual bool IsFieldNull(const wchar *field);
	
	// RecordSet Navigation
	virtual void MoveFirst(void);
	virtual void MoveLast(void);
	virtual void MoveNext(void);
	virtual void MovePrev(void);
	
//public:
private:
	MYSQL       *m_pDBConn;
	MYSQL_RES   *m_pDBRs;
	MYSQL_FIELD *m_pField;
	MYSQL_ROW   m_row;

	uint64	m_uiRecordsNum;
	uint64	m_uiRsPointer;
	
	ulong m_ulFieldsNum;
};

#endif
