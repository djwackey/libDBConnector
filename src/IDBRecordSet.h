#ifndef _DB_RECORDSET_H_
#define _DB_RECORDSET_H_

#ifdef _UNICODE
typedef wchar_t wchar;
#else
typedef char wchar;
#endif

typedef unsigned long long	uint64;
typedef unsigned int		uint32;
typedef unsigned long		ulong;

enum FieldTypes { 
	TYPE_INT,
	TYPE_LONG,
	TYPE_FLOAT,
	TYPE_DOUBLE,
	TYPE_DATETIME,
	TYPE_ENUM,
	TYPE_SET,
	TYPE_STRING
};

class IDBRecordSet
{
public:
	IDBRecordSet();
	virtual ~IDBRecordSet();

public:
	virtual bool Query(const wchar *sql) = 0;
	virtual void Close(void) = 0;
	virtual bool Refresh(uint32 options) = 0;

	virtual wchar *GetFieldValue(const wchar *field) = 0;
	virtual wchar *GetFieldValue(const int index) = 0;
	virtual int    GetFieldValueInt(const wchar *field) = 0;
	virtual float  GetFieldValueFlt(const wchar *field) = 0;
	virtual double GetFieldValueDbl(const wchar *field) = 0;
	virtual void  *GetBinaryData(const wchar *field) = 0;
	
	virtual ulong  GetFieldsCount(void) = 0;
	virtual uint64 GetRecordCount(void) = 0;
	virtual ulong *GetColumnLength(void) = 0;
	virtual FieldTypes GetFieldType(const int index) const = 0;
	virtual FieldTypes GetFieldType(const wchar *field) = 0;

	virtual bool IsOpen(void) = 0;
	virtual bool IsBOF(void) = 0;
	virtual bool IsEOF(void) = 0;
	virtual bool IsFieldNull(const int index) = 0;
	virtual bool IsFieldNull(const wchar *field) = 0;

	// RecordSet Navigation
	virtual void MoveFirst(void) = 0;
	virtual void MoveLast(void) = 0;
	virtual void MoveNext(void) = 0;
	virtual void MovePrev(void) = 0;
};

#endif

