#include "StdAfx.h"
#include "NtlPacketPT.h"


const char * s_packetName_PT[] =
{
	DECLARE_PACKET_NAME( PT_HEARTBEAT ),
	DECLARE_PACKET_NAME( PT_REPEAT_NOTICE_REPLACE ),
	DECLARE_PACKET_NAME( PT_REPEAT_NOTICE_DELETE ),
	DECLARE_PACKET_NAME( PT_REALTIME_NOTICE ),

	DECLARE_PACKET_NAME( PT_GM_GROUP_AUTH_NFY ),
	DECLARE_PACKET_NAME( PT_GM_GROUP_AUTH_DEL_NFY ),
	DECLARE_PACKET_NAME( PT_CHARACTER_WATCH_REQ ),
	DECLARE_PACKET_NAME( PT_CHARACTER_WATCH_END_NFY),
	DECLARE_PACKET_NAME( PT_PETITION_CHAT_START_REQ ),
	DECLARE_PACKET_NAME( PT_PETITION_CHAT_GM_SAY_REQ ),
	DECLARE_PACKET_NAME( PT_PETITION_CHAT_USER_SAY_RES ),
	DECLARE_PACKET_NAME( PT_PETITION_CHAT_GM_END_NFY ),
	DECLARE_PACKET_NAME( PT_PETITION_FINISH_NFY ),
	DECLARE_PACKET_NAME( PT_PETITION_USER_CANCEL_RES ),
	DECLARE_PACKET_NAME( PT_PETITION_USER_INSERT_RES ),
	DECLARE_PACKET_NAME( PT_PETITION_CONTENT_MODIFY_RES ),
	DECLARE_PACKET_NAME( PT_SERVER_STATUS_REQ ),
};


//------------------------------------------------------------------
//
//------------------------------------------------------------------
const char * NtlGetPacketName_PT(WORD wOpCode)
{
	if( wOpCode < PT_OPCODE_BEGIN )
	{
		return "NOT DEFINED PACKET NAME : OPCODE LOW";
	}

	if( wOpCode > PT_OPCODE_END )
	{
		return "NOT DEFINED PACKET NAME : OPCODE HIGH";
	}

	int nIndex = wOpCode - PT_OPCODE_BEGIN;
	if( nIndex >= _countof( s_packetName_PT) )
	{
		return "OPCODE BUFFER OVERFLOW";
	}

	return s_packetName_PT[ nIndex ];
}