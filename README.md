## libDBConnector ##
### 介绍 ###
libDBConnector程序库使用C/C++语言开发，通过MySQL C API 和Oralce OCI方式访问MySQL和Oracle数据库，且程序库可以跨平台使用，保留统一访问接口，用户只需设置创建连接对象的参数，即可改变数据库连接，同时保证同一应用程序可同时访问MySQL和Oralce数据库，并可同时连接多个数据库。
### 程序架构 ###
![组件图](http://my.csdn.net/uploads/201204/17/1334648782_2983.png)
### 功能说明 ###
公共接口文件

IDBConnect.h
<pre>
#ifndef _DB_CONNECT_H_  
#define _DB_CONNECT_H_  
  
#include "IDBRecordSet.h"  
  
#define EXPORT __declspec(dllexport)  
  
enum DB_TYPE {_MYSQL_, _ORACLE_};  
  
class IDBConnect  
{  
// Construction and Destruction  
public:  
    IDBConnect();  
    virtual ~IDBConnect();  
  
public:  
    virtual bool Connect( const wchar *host,   
                          const wchar *user,   
                          const wchar *passwd,   
                          const wchar *db,   
                          const uint32 port) = 0;  
    virtual void Close(void) = 0;  
    virtual bool Execute(const wchar *sql) = 0;  
    virtual bool StorBinData(const wchar *insertsql, const wchar *sql, const wchar *binfilename) = 0;  
    virtual bool ReadBinData(const wchar *binfilename) = 0;  
  
    // connection status  
    virtual bool IsOpen(void) = 0;  
  
    // reserved  
    //virtual int SetConnOptions(enum mysql_option option, const char *arg) = 0;  
  
    // transaction  
    virtual bool BeginTrans(void) = 0;  
    virtual bool RollbackTrans(void) = 0;  
    virtual bool CommitTrans(void) = 0;  
    virtual bool EndTrans(void) = 0;  
  
    virtual int GetLastError(void) = 0;  
    virtual int GetRealString(const wchar *inbuffer, uint32 length, wchar *&outbuffer) = 0;  
};  
  
/* public interface function */  
namespace wDBConnector  
{  
EXPORT    void SetDBType(DB_TYPE dbType);  
EXPORT DB_TYPE GetDBType();  
      
EXPORT IDBConnect*   Create(DB_TYPE dbType);  
EXPORT IDBRecordSet* Create(IDBConnect *pDBConn);  
  
EXPORT void Destroy(IDBConnect *&pDBConn);  
EXPORT void Destroy(IDBRecordSet *&pDBRs);  
  
// 调用Connect函数前，一定要先执行SetDBType函数，  
// 否则将启用默认数据库连接方式(ORACLE)  
EXPORT IDBConnect * Connect(const wchar *host,   
                            const wchar *user,   
                            const wchar *passwd,   
                            const wchar *db,   
                            const uint32 port);  
  
// 适用于短连接，调用ExecSQL函数前，一定要先执行SetConnectParam函数  
EXPORT void SetConnectParam(const wchar *host,   
                            const wchar *user,   
                            const wchar *passwd,   
                            const wchar *db,   
                            const uint32 port);  
EXPORT bool ExecSQL(const wchar *sql);  
  
EXPORT void Close(IDBConnect *&pDBConn);  
EXPORT void Close(IDBRecordSet *&pDBRs);  
}  
  
#endif
</pre>
IDBRecordSet.h
<pre>
#ifndef _DB_RECORDSET_H_  
#define _DB_RECORDSET_H_  
  
#ifdef _UNICODE  
typedef wchar_t wchar;  
#else  
typedef char wchar;  
#endif  
  
typedef unsigned __int64 uint64;  
typedef unsigned int     uint32;  
typedef unsigned long    ulong;  
  
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
</pre>
### 历史版本 ###
v1.0

- 修正了当填入错误的连接参数进行数据库连接时，获取返回值需要很长时间的错误。

    涉及函数：Connect();

v1.1

- 增加Unicode支持

- 改为命名域方式访问接口

- 增加存储二进制文件功能

### ORACLE客户端配置说明 ###
**windows:**

访问www.oracle.com网站，下载instantclient-basic-win32-11.2压缩包，解压到C:\根目录（其它目录亦可）下，重命名文件夹为instantclient。

下载instantclient-sqlplus-win32-11.2压缩包，解压到C:\instantclient（推荐）。

**设置系统环境变量：**

NLS_LANG=SIMPLIFIED CHINESE_CHINA.ZHS16GBK

TNS_ADMIN=C:\instantclient

Path=C:\instantclient

创建tnsnames.ora文件，拷贝到C:\instantclient目录下，文件内容如下：
<pre>
ORCL =
   (DESCRIPTION =
     (ADDRESS_LIST =
       (ADDRESS = (PROTOCOL = TCP)(HOST = localhost)(PORT = 1521))
     )
     (CONNECT_DATA =
       (SERVICE_NAME = orcl)
     )

   )
</pre>
Host填写目的数据库服务器IP地址，本例为localhost（本机数据库服务器）

**linux:**

**第一步 执行命令：**

\# rpm -ivh oracle-instantclient11.2-basic-11.2.0.2.0.i386.rpm

\# rpm -ivh oracle-instantclient11.2-sqlplus-11.2.0.2.0.i386.rpm

**第二步 设置环境变量：**

1.进入root目录

2.ls -a 显示隐藏的.bashrc文件

3.Vim编辑.bashrc文件，增加如下内容：

export LD_LIBRARY_PATH=/usr/local/lib:/usr/lib/oracle/11.2/client/lib

export ORACLE_HOME=/usr/lib/oracle/11.2

export TNS_ADMIN=$ORACLE_HOME:/network/admin

NLS_LANG="Simplified Chinese_China.ZHS16GBK"

**That's OK.**

**测试连通性：**

打开命令终端

输入sqlplus /nolog命令

输入conn scott/tiger@orcl，如果出现“已连接”字样，则证明数据库客户端配置成功。

### 使用例程 ###
**步骤一：包含头文件**

<pre>#include "IDBConnect.h"</pre>

**步骤二：创建连接对象指针**

<pre>IDBConnect *pDBConn = wDBConnector::Create(_ORACLE_);</pre>

输入参数：_ORACLE或_MYSQL_

返回值：连接对象指针非空，则证明创建成功！

**步骤三：连接数据库**

<pre>bool bRetValue = pDBConn->Connect(host, user, passwd, db, port);</pre>

输入参数：

	host   -> tnsnames.ora中的连接字符串，例如：orcl_192.168.1.1

    user   -> 用户名

    passwd -> 密码

    db     -> 数据库名称（若是连接ORACLE，可忽略此参数项）

    port   -> 数据库端口号

返回值：true - 成功 false - 失败

**步骤四：创建记录集对象指针**

<pre>IDBRecordSet *pDBRs = wDBConnector::Create(pDBConn);</pre>

输入参数：连接对象指针

返回值：记录集对象指针非空，则证明创建成功！

**步骤五：执行SQL语句**

<pre>bool bRetValue = pDBRs->Query(sql);</pre>

输入参数：SQL语句

返回值：true - 成功 false - 失败

（注意：连接ORACLE数据库时，SQL语句不能加分号结尾。）

**步骤六：获取结果集数据**
<pre>
char *pName = NULL;

while (!pDBRs->IsEOF())

{

    pName = pDBRs->GetFieldValue(“Name”);

    printf(“%s\n”, pName);

    pDBRs->MoveNext();

}
</pre>
输入参数：数据表的字段名称

返回值：某条记录字段的数据值

**步骤七：关闭记录集对象**

<pre>pDBRs->Close();</pre>

**步骤八：关闭连接对象**

<pre>pDBConn->Close();</pre>

**步骤九：销毁记录集对象指针**

<pre>wDBConnector::Destroy(pDBRs);</pre>

**步骤十：销毁连接对象指针**

<pre>wDBConnector::Destroy(pDBConn);</pre>

（注意：通常wDBConnector::Create和wDBConnector::Destroy是对应出现的，否则将发生内存泄露。）
<pre><code>
#include "IDBConnect.h"
#include &lt;cassert&gt;
#include &lt;cstdio&gt;
int main()
{
    IDBConnect *pDBConn = wDBConnector::Create(_ORACLE_);
    assert(NULL != pDBConn);
    if (NULL == pDBConn)
    {
        printf("Failed to Create Connection Object.\n");
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
        wDBConnector::Destroy(pDBConn);
        return 0;
    }
  
    // create a recordset instance…
    IDBRecordSet *pDBRs = wDBConnector::Create(pDBConn);
    assert(NULL != pDBRs)
    if (NULL == pDBRs)
    {
        printf("Failed to Create Record Set Object.\n");
        wDBConnector::Close(pDBConn);
        return 0;
    }

    const char *sql = "select * from table";
    do
    {
        bRetValue = pDBRs->Query(sql);
        if (!bRetValue)
        {
            printf("Failed to Query Data.\n");
            break;
        }
  
        char *pName = NULL;
        while (!pDBRs->IsEOF())
        {
            pName = pDBRs->GetFieldValue("Name");
            printf("%s\n", pName);
  
            pDBRs->MoveNext();
        }
    } while (false);

    wDBConnector::Close(pDBRs);
    wDBConnector::Close(pDBConn);
    return 0;
}
</code></pre>