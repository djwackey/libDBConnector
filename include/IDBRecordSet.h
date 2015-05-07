#ifndef _DB_RECORDSET_H_
#define _DB_RECORDSET_H_

#ifdef _UNICODE
typedef wchar_t _wTCHAR;
#else
typedef char _wTCHAR;
#endif

typedef unsigned __int64 uint64;
typedef unsigned int	 uint32;
typedef unsigned long    ulong;

#define REFRESH_GRANT		1	/* Refresh grant tables */
#define REFRESH_LOG			2	/* Start on new log file */
#define REFRESH_TABLES		4	/* close all tables */
#define REFRESH_HOSTS		8	/* Flush host cache */
#define REFRESH_STATUS		16	/* Flush status variables */
#define REFRESH_THREADS		32	/* Flush thread cache */
#define REFRESH_SLAVE       64  /* Reset master info and restart slave thread */
#define REFRESH_MASTER     128  /* Remove all bin logs in the index and truncate the index */

/* The following can't be set with mysql_refresh() */
#define REFRESH_READ_LOCK	16384	/* Lock tables for read */
#define REFRESH_FAST		32768	/* Intern flag */

/* RESET (remove all queries) from query cache */
#define REFRESH_QUERY_CACHE		65536
#define REFRESH_QUERY_CACHE_FREE 0x20000L /* pack query cache */
#define REFRESH_DES_KEY_FILE	0x40000L
#define REFRESH_USER_RESOURCES	0x80000L

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
	virtual bool Query(const _wTCHAR *sql) = 0;
	virtual void Close(void) = 0;
	virtual bool Refresh(uint32 nOptions) = 0;

	virtual _wTCHAR *GetFieldValue(const _wTCHAR *field) = 0;
	virtual _wTCHAR *GetFieldValue(const int index) = 0;
	virtual int    GetFieldValueInt(const _wTCHAR *field) = 0;
	virtual float  GetFieldValueFlt(const _wTCHAR *field) = 0;
	virtual double GetFieldValueDbl(const _wTCHAR *field) = 0;
	virtual void  *GetBinaryData(const _wTCHAR *field) = 0;
	
	virtual ulong  GetFieldsCount(void) = 0;
	virtual uint64 GetRecordCount(void) = 0;
	virtual ulong *GetColumnLength(void) = 0;
	virtual FieldTypes GetFieldType(const int index) const = 0;
	virtual FieldTypes GetFieldType(const _wTCHAR *field) = 0;

	virtual bool IsOpen(void) = 0;
	virtual bool IsBOF(void) = 0;
	virtual bool IsEOF(void) = 0;
	virtual bool IsFieldNull(const int index) = 0;
	virtual bool IsFieldNull(const _wTCHAR *field) = 0;

	// RecordSet Navigation
	virtual void MoveFirst(void) = 0;
	virtual void MoveLast(void) = 0;
	virtual void MoveNext(void) = 0;
	virtual void MovePrevious(void) = 0;
};

#endif