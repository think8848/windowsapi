;--- include file created by h2incx v0.99.19 (copyright 2005-2006 japheth)
;--- source file: D:\Microsoft SDK\Include\Sql.h, last modified: 7/25/2001 6:21
;--- cmdline used for creation: -i -o ASM -d3 sql.h

ifndef __SQL
__SQL	EQU	<>
ifndef ODBCVER
ODBCVER	EQU	0351h
endif 
ifndef __SQLTYPES
	include sqltypes.inc
endif 
ifdef __cplusplus
;extern "C"
;{
endif 
SQL_NULL_DATA	EQU	- 1
SQL_DATA_AT_EXEC	EQU	- 2
SQL_SUCCESS	EQU	0
SQL_SUCCESS_WITH_INFO	EQU	1
if (ODBCVER ge 0300h)
SQL_NO_DATA	EQU	100
endif 
SQL_ERROR	EQU	- 1
SQL_INVALID_HANDLE	EQU	- 2
SQL_STILL_EXECUTING	EQU	2
SQL_NEED_DATA	EQU	99
SQL_SUCCEEDED macro rc
	exitm <( ( ( rc )  and  (  not  1 ) ) == 0 ) >
	endm
SQL_NTS	EQU	- 3
SQL_NTSL	EQU	- 3
SQL_MAX_MESSAGE_LENGTH	EQU	512
if (ODBCVER ge 0300h)
SQL_DATE_LEN	EQU	10
SQL_TIME_LEN	EQU	8
SQL_TIMESTAMP_LEN	EQU	19
endif 
if (ODBCVER ge 0300h)
SQL_HANDLE_ENV	EQU	1
SQL_HANDLE_DBC	EQU	2
SQL_HANDLE_STMT	EQU	3
SQL_HANDLE_DESC	EQU	4
endif 
if (ODBCVER ge 0300h)
SQL_ATTR_OUTPUT_NTS	EQU	10001
endif 
if (ODBCVER ge 0300h)
SQL_ATTR_AUTO_IPD	EQU	10001
SQL_ATTR_METADATA_ID	EQU	10014
endif 
if (ODBCVER ge 0300h)
SQL_ATTR_APP_ROW_DESC	EQU	10010
SQL_ATTR_APP_PARAM_DESC	EQU	10011
SQL_ATTR_IMP_ROW_DESC	EQU	10012
SQL_ATTR_IMP_PARAM_DESC	EQU	10013
SQL_ATTR_CURSOR_SCROLLABLE	EQU	- 1
SQL_ATTR_CURSOR_SENSITIVITY	EQU	- 2
endif 
if (ODBCVER ge 0300h)
SQL_NONSCROLLABLE	EQU	0
SQL_SCROLLABLE	EQU	1
endif 
if (ODBCVER ge 0300h)
SQL_DESC_COUNT	EQU	1001
SQL_DESC_TYPE	EQU	1002
SQL_DESC_LENGTH	EQU	1003
SQL_DESC_OCTET_LENGTH_PTR	EQU	1004
SQL_DESC_PRECISION	EQU	1005
SQL_DESC_SCALE	EQU	1006
SQL_DESC_DATETIME_INTERVAL_CODE	EQU	1007
SQL_DESC_NULLABLE	EQU	1008
SQL_DESC_INDICATOR_PTR	EQU	1009
SQL_DESC_DATA_PTR	EQU	1010
SQL_DESC_NAME	EQU	1011
SQL_DESC_UNNAMED	EQU	1012
SQL_DESC_OCTET_LENGTH	EQU	1013
SQL_DESC_ALLOC_TYPE	EQU	1099
endif 
if (ODBCVER ge 0300h)
SQL_DIAG_RETURNCODE	EQU	1
SQL_DIAG_NUMBER	EQU	2
SQL_DIAG_ROW_COUNT	EQU	3
SQL_DIAG_SQLSTATE	EQU	4
SQL_DIAG_NATIVE	EQU	5
SQL_DIAG_MESSAGE_TEXT	EQU	6
SQL_DIAG_DYNAMIC_FUNCTION	EQU	7
SQL_DIAG_CLASS_ORIGIN	EQU	8
SQL_DIAG_SUBCLASS_ORIGIN	EQU	9
SQL_DIAG_CONNECTION_NAME	EQU	10
SQL_DIAG_SERVER_NAME	EQU	11
SQL_DIAG_DYNAMIC_FUNCTION_CODE	EQU	12
endif 
if (ODBCVER ge 0300h)
SQL_DIAG_ALTER_DOMAIN	EQU	3
SQL_DIAG_ALTER_TABLE	EQU	4
SQL_DIAG_CALL	EQU	7
SQL_DIAG_CREATE_ASSERTION	EQU	6
SQL_DIAG_CREATE_CHARACTER_SET	EQU	8
SQL_DIAG_CREATE_COLLATION	EQU	10
SQL_DIAG_CREATE_DOMAIN	EQU	23
SQL_DIAG_CREATE_INDEX	EQU	- 1
SQL_DIAG_CREATE_SCHEMA	EQU	64
SQL_DIAG_CREATE_TABLE	EQU	77
SQL_DIAG_CREATE_TRANSLATION	EQU	79
SQL_DIAG_CREATE_VIEW	EQU	84
SQL_DIAG_DELETE_WHERE	EQU	19
SQL_DIAG_DROP_ASSERTION	EQU	24
SQL_DIAG_DROP_CHARACTER_SET	EQU	25
SQL_DIAG_DROP_COLLATION	EQU	26
SQL_DIAG_DROP_DOMAIN	EQU	27
SQL_DIAG_DROP_INDEX	EQU	- 2
SQL_DIAG_DROP_SCHEMA	EQU	31
SQL_DIAG_DROP_TABLE	EQU	32
SQL_DIAG_DROP_TRANSLATION	EQU	33
SQL_DIAG_DROP_VIEW	EQU	36
SQL_DIAG_DYNAMIC_DELETE_CURSOR	EQU	38
SQL_DIAG_DYNAMIC_UPDATE_CURSOR	EQU	81
SQL_DIAG_GRANT	EQU	48
SQL_DIAG_INSERT	EQU	50
SQL_DIAG_REVOKE	EQU	59
SQL_DIAG_SELECT_CURSOR	EQU	85
SQL_DIAG_UNKNOWN_STATEMENT	EQU	0
SQL_DIAG_UPDATE_WHERE	EQU	82
endif 
SQL_UNKNOWN_TYPE	EQU	0
SQL_CHAR	EQU	1
SQL_NUMERIC	EQU	2
SQL_DECIMAL	EQU	3
SQL_INTEGER	EQU	4
SQL_SMALLINT	EQU	5
SQL_FLOAT	EQU	6
SQL_REAL	EQU	7
SQL_DOUBLE	EQU	8
if (ODBCVER ge 0300h)
SQL_DATETIME	EQU	9
endif 
SQL_VARCHAR	EQU	12
if (ODBCVER ge 0300h)
SQL_TYPE_DATE	EQU	91
SQL_TYPE_TIME	EQU	92
SQL_TYPE_TIMESTAMP	EQU	93
endif 
if (ODBCVER ge 0300h)
SQL_UNSPECIFIED	EQU	0
SQL_INSENSITIVE	EQU	1
SQL_SENSITIVE	EQU	2
endif 
SQL_ALL_TYPES	EQU	0
if (ODBCVER ge 0300h)
SQL_DEFAULT	EQU	99
endif 
if (ODBCVER ge 0300h)
SQL_ARD_TYPE	EQU	- 99
endif 
if (ODBCVER ge 0300h)
SQL_CODE_DATE	EQU	1
SQL_CODE_TIME	EQU	2
SQL_CODE_TIMESTAMP	EQU	3
endif 
if (ODBCVER ge 0300h)
SQL_FALSE	EQU	0
SQL_TRUE	EQU	1
endif 
SQL_NO_NULLS	EQU	0
SQL_NULLABLE	EQU	1
SQL_NULLABLE_UNKNOWN	EQU	2
if (ODBCVER ge 0300h)
SQL_PRED_NONE	EQU	0
SQL_PRED_CHAR	EQU	1
SQL_PRED_BASIC	EQU	2
endif 
if (ODBCVER ge 0300h)
SQL_NAMED	EQU	0
SQL_UNNAMED	EQU	1
endif 
if (ODBCVER ge 0300h)
SQL_DESC_ALLOC_AUTO	EQU	1
SQL_DESC_ALLOC_USER	EQU	2
endif 
SQL_CLOSE	EQU	0
SQL_DROP	EQU	1
SQL_UNBIND	EQU	2
SQL_RESET_PARAMS	EQU	3
SQL_FETCH_NEXT	EQU	1
SQL_FETCH_FIRST	EQU	2
SQL_FETCH_LAST	EQU	3
SQL_FETCH_PRIOR	EQU	4
SQL_FETCH_ABSOLUTE	EQU	5
SQL_FETCH_RELATIVE	EQU	6
SQL_COMMIT	EQU	0
SQL_ROLLBACK	EQU	1
SQL_NULL_HENV	EQU	0
SQL_NULL_HDBC	EQU	0
SQL_NULL_HSTMT	EQU	0
if (ODBCVER ge 0300h)
SQL_NULL_HDESC	EQU	0
endif 
if (ODBCVER ge 0300h)
SQL_NULL_HANDLE	EQU	0
endif 
SQL_SCOPE_CURROW	EQU	0
SQL_SCOPE_TRANSACTION	EQU	1
SQL_SCOPE_SESSION	EQU	2
SQL_PC_UNKNOWN	EQU	0
if (ODBCVER ge 0300h)
SQL_PC_NON_PSEUDO	EQU	1
endif 
SQL_PC_PSEUDO	EQU	2
if (ODBCVER ge 0300h)
SQL_ROW_IDENTIFIER	EQU	1
endif 
SQL_INDEX_UNIQUE	EQU	0
SQL_INDEX_ALL	EQU	1
SQL_INDEX_CLUSTERED	EQU	1
SQL_INDEX_HASHED	EQU	2
SQL_INDEX_OTHER	EQU	3
SQL_API_SQLALLOCCONNECT	EQU	1
SQL_API_SQLALLOCENV	EQU	2
if (ODBCVER ge 0300h)
SQL_API_SQLALLOCHANDLE	EQU	1001
endif 
SQL_API_SQLALLOCSTMT	EQU	3
SQL_API_SQLBINDCOL	EQU	4
if (ODBCVER ge 0300h)
SQL_API_SQLBINDPARAM	EQU	1002
endif 
SQL_API_SQLCANCEL	EQU	5
if (ODBCVER ge 0300h)
SQL_API_SQLCLOSECURSOR	EQU	1003
SQL_API_SQLCOLATTRIBUTE	EQU	6
endif 
SQL_API_SQLCOLUMNS	EQU	40
SQL_API_SQLCONNECT	EQU	7
if (ODBCVER ge 0300h)
SQL_API_SQLCOPYDESC	EQU	1004
endif 
SQL_API_SQLDATASOURCES	EQU	57
SQL_API_SQLDESCRIBECOL	EQU	8
SQL_API_SQLDISCONNECT	EQU	9
if (ODBCVER ge 0300h)
SQL_API_SQLENDTRAN	EQU	1005
endif 
SQL_API_SQLERROR	EQU	10
SQL_API_SQLEXECDIRECT	EQU	11
SQL_API_SQLEXECUTE	EQU	12
SQL_API_SQLFETCH	EQU	13
if (ODBCVER ge 0300h)
SQL_API_SQLFETCHSCROLL	EQU	1021
endif 
SQL_API_SQLFREECONNECT	EQU	14
SQL_API_SQLFREEENV	EQU	15
if (ODBCVER ge 0300h)
SQL_API_SQLFREEHANDLE	EQU	1006
endif 
SQL_API_SQLFREESTMT	EQU	16
if (ODBCVER ge 0300h)
SQL_API_SQLGETCONNECTATTR	EQU	1007
endif 
SQL_API_SQLGETCONNECTOPTION	EQU	42
SQL_API_SQLGETCURSORNAME	EQU	17
SQL_API_SQLGETDATA	EQU	43
if (ODBCVER ge 0300h)
SQL_API_SQLGETDESCFIELD	EQU	1008
SQL_API_SQLGETDESCREC	EQU	1009
SQL_API_SQLGETDIAGFIELD	EQU	1010
SQL_API_SQLGETDIAGREC	EQU	1011
SQL_API_SQLGETENVATTR	EQU	1012
endif 
SQL_API_SQLGETFUNCTIONS	EQU	44
SQL_API_SQLGETINFO	EQU	45
if (ODBCVER ge 0300h)
SQL_API_SQLGETSTMTATTR	EQU	1014
endif 
SQL_API_SQLGETSTMTOPTION	EQU	46
SQL_API_SQLGETTYPEINFO	EQU	47
SQL_API_SQLNUMRESULTCOLS	EQU	18
SQL_API_SQLPARAMDATA	EQU	48
SQL_API_SQLPREPARE	EQU	19
SQL_API_SQLPUTDATA	EQU	49
SQL_API_SQLROWCOUNT	EQU	20
if (ODBCVER ge 0300h)
SQL_API_SQLSETCONNECTATTR	EQU	1016
endif 
SQL_API_SQLSETCONNECTOPTION	EQU	50
SQL_API_SQLSETCURSORNAME	EQU	21
if (ODBCVER ge 0300h)
SQL_API_SQLSETDESCFIELD	EQU	1017
SQL_API_SQLSETDESCREC	EQU	1018
SQL_API_SQLSETENVATTR	EQU	1019
endif 
SQL_API_SQLSETPARAM	EQU	22
if (ODBCVER ge 0300h)
SQL_API_SQLSETSTMTATTR	EQU	1020
endif 
SQL_API_SQLSETSTMTOPTION	EQU	51
SQL_API_SQLSPECIALCOLUMNS	EQU	52
SQL_API_SQLSTATISTICS	EQU	53
SQL_API_SQLTABLES	EQU	54
SQL_API_SQLTRANSACT	EQU	23
if (ODBCVER ge 0300h)
SQL_MAX_DRIVER_CONNECTIONS	EQU	0
SQL_MAXIMUM_DRIVER_CONNECTIONS	EQU	<SQL_MAX_DRIVER_CONNECTIONS>
SQL_MAX_CONCURRENT_ACTIVITIES	EQU	1
SQL_MAXIMUM_CONCURRENT_ACTIVITIES	EQU	<SQL_MAX_CONCURRENT_ACTIVITIES>
endif 
SQL_DATA_SOURCE_NAME	EQU	2
SQL_FETCH_DIRECTION	EQU	8
SQL_SERVER_NAME	EQU	13
SQL_SEARCH_PATTERN_ESCAPE	EQU	14
SQL_DBMS_NAME	EQU	17
SQL_DBMS_VER	EQU	18
SQL_ACCESSIBLE_TABLES	EQU	19
SQL_ACCESSIBLE_PROCEDURES	EQU	20
SQL_CURSOR_COMMIT_BEHAVIOR	EQU	23
SQL_DATA_SOURCE_READ_ONLY	EQU	25
SQL_DEFAULT_TXN_ISOLATION	EQU	26
SQL_IDENTIFIER_CASE	EQU	28
SQL_IDENTIFIER_QUOTE_CHAR	EQU	29
SQL_MAX_COLUMN_NAME_LEN	EQU	30
SQL_MAXIMUM_COLUMN_NAME_LENGTH	EQU	<SQL_MAX_COLUMN_NAME_LEN>
SQL_MAX_CURSOR_NAME_LEN	EQU	31
SQL_MAXIMUM_CURSOR_NAME_LENGTH	EQU	<SQL_MAX_CURSOR_NAME_LEN>
SQL_MAX_SCHEMA_NAME_LEN	EQU	32
SQL_MAXIMUM_SCHEMA_NAME_LENGTH	EQU	<SQL_MAX_SCHEMA_NAME_LEN>
SQL_MAX_CATALOG_NAME_LEN	EQU	34
SQL_MAXIMUM_CATALOG_NAME_LENGTH	EQU	<SQL_MAX_CATALOG_NAME_LEN>
SQL_MAX_TABLE_NAME_LEN	EQU	35
SQL_SCROLL_CONCURRENCY	EQU	43
SQL_TXN_CAPABLE	EQU	46
SQL_TRANSACTION_CAPABLE	EQU	<SQL_TXN_CAPABLE>
SQL_USER_NAME	EQU	47
SQL_TXN_ISOLATION_OPTION	EQU	72
SQL_TRANSACTION_ISOLATION_OPTION	EQU	<SQL_TXN_ISOLATION_OPTION>
SQL_INTEGRITY	EQU	73
SQL_GETDATA_EXTENSIONS	EQU	81
SQL_NULL_COLLATION	EQU	85
SQL_ALTER_TABLE	EQU	86
SQL_ORDER_BY_COLUMNS_IN_SELECT	EQU	90
SQL_SPECIAL_CHARACTERS	EQU	94
SQL_MAX_COLUMNS_IN_GROUP_BY	EQU	97
SQL_MAXIMUM_COLUMNS_IN_GROUP_BY	EQU	<SQL_MAX_COLUMNS_IN_GROUP_BY>
SQL_MAX_COLUMNS_IN_INDEX	EQU	98
SQL_MAXIMUM_COLUMNS_IN_INDEX	EQU	<SQL_MAX_COLUMNS_IN_INDEX>
SQL_MAX_COLUMNS_IN_ORDER_BY	EQU	99
SQL_MAXIMUM_COLUMNS_IN_ORDER_BY	EQU	<SQL_MAX_COLUMNS_IN_ORDER_BY>
SQL_MAX_COLUMNS_IN_SELECT	EQU	100
SQL_MAXIMUM_COLUMNS_IN_SELECT	EQU	<SQL_MAX_COLUMNS_IN_SELECT>
SQL_MAX_COLUMNS_IN_TABLE	EQU	101
SQL_MAX_INDEX_SIZE	EQU	102
SQL_MAXIMUM_INDEX_SIZE	EQU	<SQL_MAX_INDEX_SIZE>
SQL_MAX_ROW_SIZE	EQU	104
SQL_MAXIMUM_ROW_SIZE	EQU	<SQL_MAX_ROW_SIZE>
SQL_MAX_STATEMENT_LEN	EQU	105
SQL_MAXIMUM_STATEMENT_LENGTH	EQU	<SQL_MAX_STATEMENT_LEN>
SQL_MAX_TABLES_IN_SELECT	EQU	106
SQL_MAXIMUM_TABLES_IN_SELECT	EQU	<SQL_MAX_TABLES_IN_SELECT>
SQL_MAX_USER_NAME_LEN	EQU	107
SQL_MAXIMUM_USER_NAME_LENGTH	EQU	<SQL_MAX_USER_NAME_LEN>
if (ODBCVER ge 0300h)
SQL_OJ_CAPABILITIES	EQU	115
SQL_OUTER_JOIN_CAPABILITIES	EQU	<SQL_OJ_CAPABILITIES>
endif 
if (ODBCVER ge 0300h)
SQL_XOPEN_CLI_YEAR	EQU	10000
SQL_CURSOR_SENSITIVITY	EQU	10001
SQL_DESCRIBE_PARAMETER	EQU	10002
SQL_CATALOG_NAME	EQU	10003
SQL_COLLATION_SEQ	EQU	10004
SQL_MAX_IDENTIFIER_LEN	EQU	10005
SQL_MAXIMUM_IDENTIFIER_LENGTH	EQU	<SQL_MAX_IDENTIFIER_LEN>
endif 
if (ODBCVER ge 0200h)
SQL_AT_ADD_COLUMN	EQU	00000001h
SQL_AT_DROP_COLUMN	EQU	00000002h
endif 
if (ODBCVER ge 0300h)
SQL_AT_ADD_CONSTRAINT	EQU	00000008h
endif 
if (ODBCVER ge 0300h)
SQL_AM_NONE	EQU	0
SQL_AM_CONNECTION	EQU	1
SQL_AM_STATEMENT	EQU	2
endif 
SQL_CB_DELETE	EQU	0
SQL_CB_CLOSE	EQU	1
SQL_CB_PRESERVE	EQU	2
SQL_FD_FETCH_NEXT	EQU	00000001h
SQL_FD_FETCH_FIRST	EQU	00000002h
SQL_FD_FETCH_LAST	EQU	00000004h
SQL_FD_FETCH_PRIOR	EQU	00000008h
SQL_FD_FETCH_ABSOLUTE	EQU	00000010h
SQL_FD_FETCH_RELATIVE	EQU	00000020h
SQL_GD_ANY_COLUMN	EQU	00000001h
SQL_GD_ANY_ORDER	EQU	00000002h
SQL_IC_UPPER	EQU	1
SQL_IC_LOWER	EQU	2
SQL_IC_SENSITIVE	EQU	3
SQL_IC_MIXED	EQU	4
if (ODBCVER ge 0201h)
SQL_OJ_LEFT	EQU	00000001h
SQL_OJ_RIGHT	EQU	00000002h
SQL_OJ_FULL	EQU	00000004h
SQL_OJ_NESTED	EQU	00000008h
SQL_OJ_NOT_ORDERED	EQU	00000010h
SQL_OJ_INNER	EQU	00000020h
SQL_OJ_ALL_COMPARISON_OPS	EQU	00000040h
endif 
SQL_SCCO_READ_ONLY	EQU	00000001h
SQL_SCCO_LOCK	EQU	00000002h
SQL_SCCO_OPT_ROWVER	EQU	00000004h
SQL_SCCO_OPT_VALUES	EQU	00000008h
SQL_TC_NONE	EQU	0
SQL_TC_DML	EQU	1
SQL_TC_ALL	EQU	2
SQL_TC_DDL_COMMIT	EQU	3
SQL_TC_DDL_IGNORE	EQU	4
SQL_TXN_READ_UNCOMMITTED	EQU	00000001h
SQL_TRANSACTION_READ_UNCOMMITTED	EQU	<SQL_TXN_READ_UNCOMMITTED>
SQL_TXN_READ_COMMITTED	EQU	00000002h
SQL_TRANSACTION_READ_COMMITTED	EQU	<SQL_TXN_READ_COMMITTED>
SQL_TXN_REPEATABLE_READ	EQU	00000004h
SQL_TRANSACTION_REPEATABLE_READ	EQU	<SQL_TXN_REPEATABLE_READ>
SQL_TXN_SERIALIZABLE	EQU	00000008h
SQL_TRANSACTION_SERIALIZABLE	EQU	<SQL_TXN_SERIALIZABLE>
SQL_NC_HIGH	EQU	0
SQL_NC_LOW	EQU	1
ifndef RC_INVOKED
SQLAllocConnect proto :SQLHENV, :ptr SQLHDBC
SQLAllocEnv proto :ptr SQLHENV
if (ODBCVER ge 0300h)
SQLAllocHandle proto :SQLSMALLINT, :SQLHANDLE, :ptr SQLHANDLE
endif 
SQLAllocStmt proto :SQLHDBC, :ptr SQLHSTMT
SQLBindCol proto :SQLHSTMT, :SQLUSMALLINT, :SQLSMALLINT, :SQLPOINTER, :SQLLEN, :ptr SQLLEN
if (ODBCVER ge 0300h)
SQLBindParam proto :SQLHSTMT, :SQLUSMALLINT, :SQLSMALLINT, :SQLSMALLINT, :SQLULEN, :SQLSMALLINT, :SQLPOINTER, :ptr SQLLEN
endif 
SQLCancel proto :SQLHSTMT
if (ODBCVER ge 0300h)
SQLCloseCursor proto :SQLHSTMT
ifdef _WIN64
SQLColAttribute proto :SQLHSTMT, :SQLUSMALLINT, :SQLUSMALLINT, :SQLPOINTER, :SQLSMALLINT, :ptr SQLSMALLINT, :ptr SQLLEN
else 
SQLColAttribute proto :SQLHSTMT, :SQLUSMALLINT, :SQLUSMALLINT, :SQLPOINTER, :SQLSMALLINT, :ptr SQLSMALLINT, :SQLPOINTER
endif 
endif 
SQLColumns proto :SQLHSTMT, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLCHAR, :SQLSMALLINT
SQLConnect proto :SQLHDBC, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLCHAR, :SQLSMALLINT
if (ODBCVER ge 0300h)
SQLCopyDesc proto :SQLHDESC, :SQLHDESC
endif 
SQLDataSources proto :SQLHENV, :SQLUSMALLINT, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLSMALLINT, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLSMALLINT
SQLDescribeCol proto :SQLHSTMT, :SQLUSMALLINT, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLSMALLINT, :ptr SQLSMALLINT, :ptr SQLULEN, :ptr SQLSMALLINT, :ptr SQLSMALLINT
SQLDisconnect proto :SQLHDBC
if (ODBCVER ge 0300h)
SQLEndTran proto :SQLSMALLINT, :SQLHANDLE, :SQLSMALLINT
endif 
SQLError proto :SQLHENV, :SQLHDBC, :SQLHSTMT, :ptr SQLCHAR, :ptr SQLINTEGER, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLSMALLINT
SQLExecDirect proto :SQLHSTMT, :ptr SQLCHAR, :SQLINTEGER
SQLExecute proto :SQLHSTMT
SQLFetch proto :SQLHSTMT
if (ODBCVER ge 0300h)
SQLFetchScroll proto :SQLHSTMT, :SQLSMALLINT, :SQLLEN
endif 
SQLFreeConnect proto :SQLHDBC
SQLFreeEnv proto :SQLHENV
if (ODBCVER ge 0300h)
SQLFreeHandle proto :SQLSMALLINT, :SQLHANDLE
endif 
SQLFreeStmt proto :SQLHSTMT, :SQLUSMALLINT
if (ODBCVER ge 0300h)
SQLGetConnectAttr proto :SQLHDBC, :SQLINTEGER, :SQLPOINTER, :SQLINTEGER, :ptr SQLINTEGER
endif 
SQLGetConnectOption proto :SQLHDBC, :SQLUSMALLINT, :SQLPOINTER
SQLGetCursorName proto :SQLHSTMT, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLSMALLINT
SQLGetData proto :SQLHSTMT, :SQLUSMALLINT, :SQLSMALLINT, :SQLPOINTER, :SQLLEN, :ptr SQLLEN
if (ODBCVER ge 0300h)
SQLGetDescField proto :SQLHDESC, :SQLSMALLINT, :SQLSMALLINT, :SQLPOINTER, :SQLINTEGER, :ptr SQLINTEGER
SQLGetDescRec proto :SQLHDESC, :SQLSMALLINT, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLSMALLINT, :ptr SQLSMALLINT, :ptr SQLSMALLINT, :ptr SQLLEN, :ptr SQLSMALLINT, :ptr SQLSMALLINT, :ptr SQLSMALLINT
SQLGetDiagField proto :SQLSMALLINT, :SQLHANDLE, :SQLSMALLINT, :SQLSMALLINT, :SQLPOINTER, :SQLSMALLINT, :ptr SQLSMALLINT
SQLGetDiagRec proto :SQLSMALLINT, :SQLHANDLE, :SQLSMALLINT, :ptr SQLCHAR, :ptr SQLINTEGER, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLSMALLINT
SQLGetEnvAttr proto :SQLHENV, :SQLINTEGER, :SQLPOINTER, :SQLINTEGER, :ptr SQLINTEGER
endif 
SQLGetFunctions proto :SQLHDBC, :SQLUSMALLINT, :ptr SQLUSMALLINT
SQLGetInfo proto :SQLHDBC, :SQLUSMALLINT, :SQLPOINTER, :SQLSMALLINT, :ptr SQLSMALLINT
if (ODBCVER ge 0300h)
SQLGetStmtAttr proto :SQLHSTMT, :SQLINTEGER, :SQLPOINTER, :SQLINTEGER, :ptr SQLINTEGER
endif 
SQLGetStmtOption proto :SQLHSTMT, :SQLUSMALLINT, :SQLPOINTER
SQLGetTypeInfo proto :SQLHSTMT, :SQLSMALLINT
SQLNumResultCols proto :SQLHSTMT, :ptr SQLSMALLINT
SQLParamData proto :SQLHSTMT, :ptr SQLPOINTER
SQLPrepare proto :SQLHSTMT, :ptr SQLCHAR, :SQLINTEGER
SQLPutData proto :SQLHSTMT, :SQLPOINTER, :SQLLEN
SQLRowCount proto :SQLHSTMT, :ptr SQLLEN
if (ODBCVER ge 0300h)
SQLSetConnectAttr proto :SQLHDBC, :SQLINTEGER, :SQLPOINTER, :SQLINTEGER
endif 
SQLSetConnectOption proto :SQLHDBC, :SQLUSMALLINT, :SQLULEN
SQLSetCursorName proto :SQLHSTMT, :ptr SQLCHAR, :SQLSMALLINT
if (ODBCVER ge 0300h)
SQLSetDescField proto :SQLHDESC, :SQLSMALLINT, :SQLSMALLINT, :SQLPOINTER, :SQLINTEGER
SQLSetDescRec proto :SQLHDESC, :SQLSMALLINT, :SQLSMALLINT, :SQLSMALLINT, :SQLLEN, :SQLSMALLINT, :SQLSMALLINT, :SQLPOINTER, :ptr SQLLEN, :ptr SQLLEN
SQLSetEnvAttr proto :SQLHENV, :SQLINTEGER, :SQLPOINTER, :SQLINTEGER
endif 
SQLSetParam proto :SQLHSTMT, :SQLUSMALLINT, :SQLSMALLINT, :SQLSMALLINT, :SQLULEN, :SQLSMALLINT, :SQLPOINTER, :ptr SQLLEN
if (ODBCVER ge 0300h)
SQLSetStmtAttr proto :SQLHSTMT, :SQLINTEGER, :SQLPOINTER, :SQLINTEGER
endif 
SQLSetStmtOption proto :SQLHSTMT, :SQLUSMALLINT, :SQLULEN
SQLSpecialColumns proto :SQLHSTMT, :SQLUSMALLINT, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLCHAR, :SQLSMALLINT, :SQLUSMALLINT, :SQLUSMALLINT
SQLStatistics proto :SQLHSTMT, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLCHAR, :SQLSMALLINT, :SQLUSMALLINT, :SQLUSMALLINT
SQLTables proto :SQLHSTMT, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLCHAR, :SQLSMALLINT, :ptr SQLCHAR, :SQLSMALLINT
SQLTransact proto :SQLHENV, :SQLHDBC, :SQLUSMALLINT
endif 
ifdef __cplusplus
;}
endif 
endif 

;--- errors: 0
;--- end of file ---