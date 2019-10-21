#include "StdAfx.h"
#include "NtlPacketCQ.h"


const char * s_packetName_CQ[] =
{
	DECLARE_PACKET_NAME( CQ_NOTIFY_SERVER_BEGIN ),

	DECLARE_PACKET_NAME( CQ_CHECK_AUTH_KEY_REQ ),

	DECLARE_PACKET_NAME( CQ_CHARACTER_INFO_REQ ),
	DECLARE_PACKET_NAME( CQ_CHARACTER_ADD_REQ ),
	DECLARE_PACKET_NAME( CQ_CHARACTER_DEL_REQ ),

	DECLARE_PACKET_NAME( CQ_CHARACTER_EXIT ),
	DECLARE_PACKET_NAME( CQ_CHARACTER_LOAD_REQ ),
	DECLARE_PACKET_NAME( CQ_CHAR_SERVER_RESET_REQ ),

	DECLARE_PACKET_NAME( CQ_CONNECT_WAIT_CHECK_REQ ),
	DECLARE_PACKET_NAME( CQ_CONNECT_WAIT_CANCEL_REQ ),

	DECLARE_PACKET_NAME( CQ_CHARACTER_RENAME_REQ ),
};


//------------------------------------------------------------------
//
//------------------------------------------------------------------
const char * NtlGetPacketName_CQ(WORD wOpCode)
{
	if( wOpCode < CQ_OPCODE_BEGIN )
	{
		return "NOT DEFINED PACKET NAME : OPCODE LOW";
	}

	if( wOpCode > CQ_OPCODE_END )
	{
		return "NOT DEFINED PACKET NAME : OPCODE HIGH";
	}

	int nIndex = wOpCode - CQ_OPCODE_BEGIN;
	if( nIndex >= _countof( s_packetName_CQ) )
	{
		return "OPCODE BUFFER OVERFLOW";
	}

	return s_packetName_CQ[ nIndex ];
}