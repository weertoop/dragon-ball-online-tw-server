#include "StdAfx.h"
#include "NtlPacketQG.h"


const char * s_packetName_QG[] =
{
	DECLARE_PACKET_NAME( QG_HEARTBEAT ),
	DECLARE_PACKET_NAME( QG_LOAD_PC_DATA_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_CREATE_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_MOVE_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_MOVE_STACK_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_UPDATE_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_DELETE_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_EQUIP_REPAIR_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_REPAIR_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_DUR_UPDATE_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_USE_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_UPGRADE_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_IDENTIFY_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_CREATE_EX_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_DELETE_EX_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_AUTO_EQUIP_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_AUTO_EQUIP_ROLLBACK_RES ),

	DECLARE_PACKET_NAME( QG_ITEM_PICK_RES ),
	DECLARE_PACKET_NAME( QG_ZENNY_PICK_RES ),

	DECLARE_PACKET_NAME( QG_SKILL_RP_BONUS_SETTING_RES ),
	DECLARE_PACKET_NAME( QG_SKILL_LOAD_RES ),
	DECLARE_PACKET_NAME( QG_BUFF_LOAD_RES ),
	DECLARE_PACKET_NAME( QG_HTB_SKILL_LOAD_RES ),

	DECLARE_PACKET_NAME( QG_LOAD_PC_BANK_DATA_RES ),
	DECLARE_PACKET_NAME( QG_BANK_MOVE_RES ),
	DECLARE_PACKET_NAME( QG_BANK_MOVE_STACK_RES ),
	DECLARE_PACKET_NAME( QG_BANK_ZENNY_RES ),
	DECLARE_PACKET_NAME( QG_BANK_BUY_RES ),
	DECLARE_PACKET_NAME( QG_BANK_ITEM_DELETE_RES ),

	DECLARE_PACKET_NAME( QG_PC_UPDATE_POSITION_RES ),
	DECLARE_PACKET_NAME( QG_PC_UPDATE_EXP_RES ),
	DECLARE_PACKET_NAME( QG_PC_UPDATE_LPEPRP_RES ),
	DECLARE_PACKET_NAME( QG_PC_UPDATE_LEVEL_RES ),
	DECLARE_PACKET_NAME( QG_SAVE_PC_DATA_RES ),
	DECLARE_PACKET_NAME( QG_SAVE_SKILL_DATA_RES ),
	DECLARE_PACKET_NAME( QG_SAVE_HTB_DATA_RES ),
	DECLARE_PACKET_NAME( QG_SAVE_BUFF_DATA_RES ),

	DECLARE_PACKET_NAME( QG_PC_EXIT_RES ),
	//DECLARE_PACKET_NAME( QG_MAKE_AUTH_KEY_RES ),

	DECLARE_PACKET_NAME( QG_SHOP_BUY_RES ),
	DECLARE_PACKET_NAME( QG_SHOP_SELL_RES ),
	
	DECLARE_PACKET_NAME( QG_PARTY_ACTIVATE_CHARM_RES ),
	DECLARE_PACKET_NAME( QG_PARTY_LOOTING_ITEM_RES ),
	DECLARE_PACKET_NAME( QG_PARTY_LOOTING_ZENNY_RES ),

	DECLARE_PACKET_NAME( QG_PC_UPDATE_BIND_RES ),
	DECLARE_PACKET_NAME( QG_CHAR_CONVERT_CLASS_RES ),
	DECLARE_PACKET_NAME( QG_CHAR_PUNISH_RES ),
	DECLARE_PACKET_NAME( QG_CHAR_AWAY_RES ),
	DECLARE_PACKET_NAME( QG_CHAR_KEY_UPDATE_RES ),

	DECLARE_PACKET_NAME( QG_UPDATE_CHAR_ZENNY_RES ),

	DECLARE_PACKET_NAME( QG_QUEST_ITEM_CREATE_RES ),
	DECLARE_PACKET_NAME( QG_QUEST_ITEM_DELETE_RES ),
	DECLARE_PACKET_NAME( QG_QUEST_ITEM_MOVE_RES ),
	DECLARE_PACKET_NAME( QG_QUEST_PROGRESS_DATA_CREATE_RES ),
	DECLARE_PACKET_NAME( QG_QUEST_PROGRESS_DATA_DELETE_RES ),
	DECLARE_PACKET_NAME( QG_QUEST_COMPLETE_DATA_UPDATE_RES ),
	DECLARE_PACKET_NAME( QG_QUEST_DATA_RESET_RES ),
	DECLARE_PACKET_NAME( QG_QUEST_STATE_UPDATE_RES ),
	DECLARE_PACKET_NAME( QG_QUEST_TSP_UPDATE_RES ),
	DECLARE_PACKET_NAME( QG_QUEST_EXCEPTION_TIMER_UPDATE_RES ),
	DECLARE_PACKET_NAME( QG_QUEST_SSM_UPDATE_RES ),
	DECLARE_PACKET_NAME( QG_QUEST_SERVER_EVENT_UPDATE_RES ),
	DECLARE_PACKET_NAME( QG_QUEST_EXC_CLIENT_GROUP_RES ),
	DECLARE_PACKET_NAME( QG_QUEST_INFO_UPDATE_RES ),

	DECLARE_PACKET_NAME( QG_QUICK_SLOT_UPDATE_RES ),
	DECLARE_PACKET_NAME( QG_QUICK_SLOT_DEL_RES ),

	DECLARE_PACKET_NAME( QG_SAVE_SPAWNED_SUMMON_PET_DATA_RES ),
	DECLARE_PACKET_NAME( QG_SAVE_SPAWNED_ITEM_PET_DATA_RES ),
	DECLARE_PACKET_NAME( QG_LOAD_SUMMON_PET_DATA_RES ),
	DECLARE_PACKET_NAME( QG_LOAD_ITEM_PET_DATA_RES ),
	DECLARE_PACKET_NAME( QG_LOAD_PET_BUFF_RES ),
	DECLARE_PACKET_NAME( QG_DELETE_ALL_TEMPORARY_PET_DATA_RES ),

	DECLARE_PACKET_NAME( QG_TRADE_RES ),
	DECLARE_PACKET_NAME( QG_SCHEDULE_INFO ),
	DECLARE_PACKET_NAME( QG_TUTORIAL_HINT_UPDATE_RES ),

	DECLARE_PACKET_NAME( QG_BALL_ITEM_PICK_RES ),
	DECLARE_PACKET_NAME( QG_BALL_ITEM_PARTY_PICK_RES ),
	DECLARE_PACKET_NAME( QG_BALL_ITEM_REWARD_RES ),

	DECLARE_PACKET_NAME( QG_PRIVATESHOP_ITEM_BUYING_RES ),	
	DECLARE_PACKET_NAME( QG_PRIVATESHOP_ITEM_INSERT_RES ), 	
	DECLARE_PACKET_NAME( QG_PRIVATESHOP_ITEM_DELETE_RES ), 	
	DECLARE_PACKET_NAME( QG_PRIVATESHOP_ITEM_UPDATE_RES ), 	
	DECLARE_PACKET_NAME( QG_PRIVATESHOP_SHOP_LOADING_RES ),	
	DECLARE_PACKET_NAME( QG_PRIVATESHOP_ITEM_DATA_INFO ),	
	DECLARE_PACKET_NAME( QG_PRIVATESHOP_CREATE_RES ),		
	DECLARE_PACKET_NAME( QG_PRIVATESHOP_UPDATE_RES ),		

	DECLARE_PACKET_NAME( QG_RANKBATTLE_SCORE_UPDATE_RES ),

	DECLARE_PACKET_NAME( QG_TMQ_DAYRECORD_RESET_RES ),
	DECLARE_PACKET_NAME( QG_TMQ_DAYRECORD_UPDATE_RES ),
	DECLARE_PACKET_NAME( QG_TMQ_DAYRECORD_LIST_RES ),
	DECLARE_PACKET_NAME( QG_TMQ_DAYRECORD_RES ),

	DECLARE_PACKET_NAME( QG_MAIL_START_RES ),
	DECLARE_PACKET_NAME( QG_MAIL_SEND_RES ),
	DECLARE_PACKET_NAME( QG_MAIL_READ_RES ),
	DECLARE_PACKET_NAME( QG_MAIL_DEL_RES ),
	DECLARE_PACKET_NAME( QG_MAIL_RETURN_RES ),
	DECLARE_PACKET_NAME( QG_MAIL_RELOAD_RES ),
	DECLARE_PACKET_NAME( QG_MAIL_LOAD_RES ),
	DECLARE_PACKET_NAME( QG_MAIL_ITEM_RECEIVE_RES ),
	DECLARE_PACKET_NAME( QG_MAIL_LOCK_RES ),
	DECLARE_PACKET_NAME( QG_MAIL_EVENT_SEND_RES ),
	DECLARE_PACKET_NAME( QG_MAIL_MULTI_DEL_RES ),	

	DECLARE_PACKET_NAME( QG_PORTAL_START_RES ),
	DECLARE_PACKET_NAME( QG_PORTAL_ADD_RES ),

	DECLARE_PACKET_NAME( QG_WAR_FOG_UPDATE_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_UPGRADE_ALL_RES ),

	DECLARE_PACKET_NAME( QG_GUILD_BANK_LOAD_RES ),
	DECLARE_PACKET_NAME( QG_GUILD_BANK_MOVE_RES ),
	DECLARE_PACKET_NAME( QG_GUILD_BANK_MOVE_STACK_RES ),
	DECLARE_PACKET_NAME( QG_GUILD_BANK_ZENNY_RES ),

	DECLARE_PACKET_NAME( QG_SHOP_ITEM_IDENTIFY_RES ),

	DECLARE_PACKET_NAME( QG_BUDOKAI_DATA_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_INIT_DATA_NFY ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_INIT_DATA_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_UPDATE_STATE_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_UPDATE_MATCH_STATE_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_UPDATE_CLEAR_RES ),
	DECLARE_PACKET_NAME( QG_RANKPOINT_RESET_RES ),
	DECLARE_PACKET_NAME( QG_RANKBATTLE_ALLOW_RES ),

	DECLARE_PACKET_NAME( QG_BUDOKAI_INDIVIDUAL_ALLOW_REGISTER_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_JOIN_INDIVIDUAL_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_LEAVE_INDIVIDUAL_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_INDIVIDUAL_SELECTION_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_INDIVIDUAL_LIST_START ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_INDIVIDUAL_LIST_DATA ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_INDIVIDUAL_LIST_END ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_TOURNAMENT_INDIVIDUAL_ADD_ENTRY_LIST_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_TOURNAMENT_INDIVIDUAL_ENTRY_LIST_START ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_TOURNAMENT_INDIVIDUAL_ENTRY_LIST_DATA ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_TOURNAMENT_INDIVIDUAL_ENTRY_LIST_END ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_TOURNAMENT_INDIVIDUAL_ADD_MATCH_RESULT_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_TOURNAMENT_INDIVIDUAL_MATCH_RESULT_RES ),

	DECLARE_PACKET_NAME( QG_BUDOKAI_TEAM_ALLOW_REGISTER_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_JOIN_TEAM_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_LEAVE_TEAM_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_LEAVE_TEAM_MEMBER_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_TEAM_SELECTION_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_TEAM_LIST_START ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_TEAM_LIST_DATA ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_TEAM_LIST_END ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_TOURNAMENT_TEAM_ADD_ENTRY_LIST_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_TOURNAMENT_TEAM_ENTRY_LIST_START ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_TOURNAMENT_TEAM_ENTRY_LIST_DATA ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_TOURNAMENT_TEAM_ENTRY_LIST_END ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_TOURNAMENT_TEAM_ADD_MATCH_RESULT_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_TOURNAMENT_TEAM_MATCH_RESULT_RES ),

	DECLARE_PACKET_NAME( QG_BUDOKAI_JOIN_INFO_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_JOIN_STATE_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_HISTORY_WRITE_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_HISTORY_WINNER_PLAYER_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_JOIN_STATE_LIST_RES ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_JOIN_STATE_LIST_DATA ),
	DECLARE_PACKET_NAME( QG_BUDOKAI_SET_OPEN_TIME_RES ),

	DECLARE_PACKET_NAME( QG_MATCH_REWARD_RES ),

	DECLARE_PACKET_NAME( QG_SCOUTER_ITEM_SELL_RES ),
	DECLARE_PACKET_NAME( QG_SCOUTER_CHIP_REMOVE_ALL_RES ),

	DECLARE_PACKET_NAME( QG_SHOP_EVENTITEM_BUY_RES ),

	DECLARE_PACKET_NAME( QG_SHOP_GAMBLE_BUY_RES ),

	DECLARE_PACKET_NAME( QG_BUDOKAI_UPDATE_MUDOSA_POINT_RES ),
	
	DECLARE_PACKET_NAME( QG_ITEM_REPLACE_RES ),

	DECLARE_PACKET_NAME( QG_UPDATE_MARKING_RES ),

	DECLARE_PACKET_NAME( QG_SKILL_ADD_RES ),
	DECLARE_PACKET_NAME( QG_SKILL_UPGRADE_RES ),
	DECLARE_PACKET_NAME( QG_HTB_SKILL_ADD_RES ),
	DECLARE_PACKET_NAME( QG_SKILL_BUY_RES ),
	DECLARE_PACKET_NAME( QG_SKILL_INIT_RES ),
	
	DECLARE_PACKET_NAME( QG_RECIPE_REG_RES ),
	DECLARE_PACKET_NAME( QG_HOIPOIMIX_JOB_SET_RES ),
	DECLARE_PACKET_NAME( QG_HOIPOIMIX_JOB_RESET_RES ),
	DECLARE_PACKET_NAME( QG_HOIPOIMIX_ITEM_MAKE_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_STACK_UPDATE_RES ),

	DECLARE_PACKET_NAME( QG_DOJO_BANK_HISTORY_RES ),
	DECLARE_PACKET_NAME( QG_DOJO_BANK_ZENNY_UPDATE_RES ),
	DECLARE_PACKET_NAME( QG_WORLD_SCHEDULE_SET_RES ),

	DECLARE_PACKET_NAME( QG_SWITCH_CHILD_ADULT_RES ),
	DECLARE_PACKET_NAME( QG_SET_CHANGE_CLASS_AUTHORITY_RES ),
	DECLARE_PACKET_NAME( QG_ITEM_CHANGE_ATTRIBUTE_RES ),

	DECLARE_PACKET_NAME( QG_CHANGE_NETP_RES ),

	DECLARE_PACKET_NAME( QG_ITEM_CHANGE_DURATIONTIME_RES ),
	DECLARE_PACKET_NAME( QG_SHOP_NETPYITEM_BUY_RES ),

    DECLARE_PACKET_NAME( QG_GM_COMMAND_LOG_RES ),

	DECLARE_PACKET_NAME( QG_DURATION_ITEM_BUY_RES ),
	DECLARE_PACKET_NAME( QG_DURATION_RENEW_RES ),

	DECLARE_PACKET_NAME( QG_CASHITEM_INFO_RES ),
	DECLARE_PACKET_NAME( QG_CASHITEM_MOVE_RES ),
	DECLARE_PACKET_NAME( QG_CASHITEM_DEL_RES ),
	DECLARE_PACKET_NAME( QG_CASHITEM_UNPACK_RES ),
	DECLARE_PACKET_NAME( QG_CASHITEM_BUY_RES ),

	DECLARE_PACKET_NAME( QG_GMT_UPDATE_RES ),

	DECLARE_PACKET_NAME( QG_PC_DATA_LOAD_RES ),
	DECLARE_PACKET_NAME( QG_PC_ITEM_LOAD_RES ),
	DECLARE_PACKET_NAME( QG_PC_SKILL_LOAD_RES ),		
	DECLARE_PACKET_NAME( QG_PC_BUFF_LOAD_RES ),
	DECLARE_PACKET_NAME( QG_PC_HTB_LOAD_RES ),
	DECLARE_PACKET_NAME( QG_PC_QUEST_ITEM_LOAD_RES ),		
	DECLARE_PACKET_NAME( QG_PC_QUEST_COMPLETE_LOAD_RES ),
	DECLARE_PACKET_NAME( QG_PC_QUEST_PROGRESS_LOAD_RES ),	
	DECLARE_PACKET_NAME( QG_PC_QUICK_SLOT_LOAD_RES ),	
	DECLARE_PACKET_NAME( QG_PC_SHORTCUT_LOAD_RES ),
	DECLARE_PACKET_NAME( QG_PC_ITEM_RECIPE_LOAD_RES ),
	DECLARE_PACKET_NAME( QG_PC_WAR_FOG_RES ),
	DECLARE_PACKET_NAME( QG_PC_GMT_LOAD_RES ),

};


//------------------------------------------------------------------
//
//------------------------------------------------------------------
const char * NtlGetPacketName_QG(WORD wOpCode)
{
	if( wOpCode < QG_OPCODE_BEGIN )
	{
		return "NOT DEFINED PACKET NAME : OPCODE LOW";
	}

	if( wOpCode > QG_OPCODE_END )
	{
		return "NOT DEFINED PACKET NAME : OPCODE HIGH";
	}

	int nIndex = wOpCode - QG_OPCODE_BEGIN;
	if( nIndex >= _countof( s_packetName_QG) )
	{
		return "OPCODE BUFFER OVERFLOW";
	}

	return s_packetName_QG[ nIndex ];
}



