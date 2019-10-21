#ifndef _NTL_PATHENGINE_DEF_H_
#define _NTL_PATHENGINE_DEF_H_

//////////////////////////////////////////////////////////////////////////
// NTL PathEngine Define.
// Note : this version for develop.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// NTL PathEngine Log file define
//////////////////////////////////////////////////////////////////////////

#define NTLPE_LOG_FILENAME					("PathEngineLog.txt")

//////////////////////////////////////////////////////////////////////////
// NTL files define
//////////////////////////////////////////////////////////////////////////

#define NTLPE_FILE_WORLDINFO				("worldinfo")

//////////////////////////////////////////////////////////////////////////
// NTL PathEngine Data File Define
//////////////////////////////////////////////////////////////////////////

#define NTLPE_DATA_EXTENSION_BINARY			(".mf")
#define NTLPE_DATA_EXTENSION_XML			(".xml")
#define NTLPE_DATA_EXTENSION_SCRIPT			(".txt")

#define NTLPE_DATA_EXTENSION_PRE_COLLISION	(".cf")
#define NTLPE_DATA_EXTENSION_PRE_PATHFIND	(".pf")

//////////////////////////////////////////////////////////////////////////
// NTL PathEngine Data File Format
//////////////////////////////////////////////////////////////////////////

#define NTLPE_DATA_FORMAT_TOK				("tok")
#define NTLPE_DATA_FORMAT_XML				("xml")

//////////////////////////////////////////////////////////////////////////
// NTL PathEngine filename convension
//////////////////////////////////////////////////////////////////////////

#define NTLPE_FEDERATION_APPEND_HEADER		("_h")
#define NTLPE_FEDERATION_APPEND_TILE		("_f")

//////////////////////////////////////////////////////////////////////////
// NTL PathEngine character length
//////////////////////////////////////////////////////////////////////////

#define NTLPE_MAX_FILENAME_LENGTH			(32)

//////////////////////////////////////////////////////////////////////////
// NTL define Index
//////////////////////////////////////////////////////////////////////////

#define NTLPE_INVALID_WORLDINDEX			(0xFFFFFFFF)
#define NTLPE_INVALID_FIELDINDEX			(0xFFFFFFFF)
#define NTLPE_INVALID_TILEINDEX				(0xFFFFFFFF)

//////////////////////////////////////////////////////////////////////////
// NTL PathEngine Default Value
//////////////////////////////////////////////////////////////////////////

#define NTLPE_DEFAULT_SEAMLESSRANGE			(100000)		//( 1km )

#define NTLPE_DEFAULT_HORIZ_RANGE			(1000)
#define NTLPE_DEFAULT_VERT_RANGE			(100000)

//////////////////////////////////////////////////////////////////////////
// NTL PathEngine Coordination translate
//////////////////////////////////////////////////////////////////////////
#define NTLPE_PATH_RATIO					(100.f)
#define NTLPE_TO_PATHCOORD( v )				((v) / NTLPE_PATH_RATIO )
#define NTLPE_TO_WORLDCOORD( v )			((v) * NTLPE_PATH_RATIO )

//////////////////////////////////////////////////////////////////////////
// NTL PathEngine data mode (Data format)
//////////////////////////////////////////////////////////////////////////

#define NTLPE_DATA_BINARY
// #define NTL_PATHENGINE_DATA_XML

#ifdef NTLPE_DATA_BINARY
#define NTLPE_DATA_EXTENSION	NTLPE_DATA_EXTENSION_BINARY
#define NTLPE_DATA_FORMAT		NTLPE_DATA_FORMAT_TOK
#else
#define NTLPE_DATA_EXTENSION	NTLPE_DATA_EXTENSION_XML
#define NTLPE_DATA_FORMAT		NTLPE_DATA_FORMAT_XML
#endif

#endif