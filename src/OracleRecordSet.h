#ifndef _ORACLE_RECORDSET_H_
#define _ORACLE_RECORDSET_H_

#include "IDBRecordSet.h"

#include <string>
#include <vector>

using namespace std;

#define CHAR_SIZE 1

class COracleConnect;
class COracleRecordSet : IDBRecordSet
{
public:
	COracleRecordSet(COracleConnect *pOraConn);
	virtual ~COracleRecordSet();

public:
	virtual bool Query(const wchar *sql);
	virtual void Close(void);
	virtual bool Refresh(uint32 options);

	virtual wchar *GetFieldValue(const wchar *field);
	virtual wchar *GetFieldValue(const int index);
	virtual int    GetFieldValueInt(const wchar *field);
	virtual float  GetFieldValueFlt(const wchar *field);
	virtual double GetFieldValueDbl(const wchar *field);
	virtual void  *GetBinaryData(const wchar *field);

	virtual ulong  GetFieldsCount(void);
	virtual uint64 GetRecordCount(void);
	virtual ulong *GetColumnLength(void);
	virtual FieldTypes GetFieldType(const int index) const;
	virtual FieldTypes GetFieldType(const wchar *field);

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

protected:
	int    GetFieldIndex(const wchar *field);
	uint64 GetRecordsCount(void);
	bool   BindAllColumns(int nFieldsNum);
	int    ConvertFieldValueType(int index);
	void   Cleanup(void);

private:
	COracleConnect *m_pOraConn;
	OCIStmt		*m_phOCIStmt;
	OCIError	*m_phOCIError;		// error handle

	uint64 m_ulRecordsNum;
	uint64 m_ulRsPointer;

	ub4 m_nFieldsNum;

	//float m_fFieldPointer;

	int	m_nValue;
	float m_fValue;
	double m_dValue;

	int m_nSize;

	//DataTypesEnum m_colType;		// as it will be returned

	//ub2 *m_pDataLens;				
	//sb2 *m_pIndicators;			

	ub2 m_pDataLens[512];			// an array with data lengths (for text columns)
	ub2	m_nOCIType;
	sb2 m_pIndicators[5];			// an array with indicator:0 - ok; -1 - null

	char m_pFetchBuffer[100][1024];

	// a vector with fields values
	vector<signed short>	m_vecIndicators;
	vector<string>			m_vecFieldName;
	vector<ub2>				m_vecOCIType;
	vector<int>				m_vecColSize;
};

#endif