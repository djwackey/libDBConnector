#ifndef _COMMON_H_
#define _COMMON_H_

#include <wchar.h>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <stdlib.h>

using namespace std;

#define PARAM_LEN	32
#define SQL_LEN		128
#define FIELD_LEN	128
#define BUFFER_LEN	512
#define MAX_LEN     204800


#define REFRESH_GRANT		1	/* Refresh grant tables */
#define REFRESH_LOG		2	/* Start on new log file */
#define REFRESH_TABLES		4	/* close all tables */
#define REFRESH_HOSTS		8	/* Flush host cache */
#define REFRESH_STATUS		16	/* Flush status variables */
#define REFRESH_THREADS		32	/* Flush thread cache */
#define REFRESH_SLAVE       	64	/* Reset master info and restart slave thread */
#define REFRESH_MASTER     	128	/* Remove all bin logs in the index and truncate the index */

/* The following can't be set with mysql_refresh() */
#define REFRESH_READ_LOCK	16384	/* Lock tables for read */
#define REFRESH_FAST		32768	/* Intern flag */

#define REFRESH_QUERY_CACHE		65536
#define REFRESH_QUERY_CACHE_FREE 0x20000L /* pack query cache */
#define REFRESH_DES_KEY_FILE	0x40000L
#define REFRESH_USER_RESOURCES	0x80000L

#ifdef WIN32
#pragma warning(disable : 4996)

#include <tchar.h>
#endif

#ifdef _UNICODE
#define wstrcpy wcscpy  
#define wstrlen wcslen
#define wstrcmp wcscmp
#else
#define wstrcpy strcpy
#define wstrlen strlen
#define wstrcmp strcmp
#endif
#endif
