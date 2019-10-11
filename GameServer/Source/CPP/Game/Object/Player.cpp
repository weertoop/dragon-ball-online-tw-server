﻿#include <Game\Object\Player.h>
#include <WorldSession.h>
#include <World.h>
#include <mysqlconn_wrapper.h>
#include <Logger.h>
#include <Packet\Game\PacketGU.h>
#include <Game\Maps\MapManager.h>
#include <Game\Object\Mob.h>
#include <Game\Object\Npc.h>
#include <Game\World\WorldSession.h>
#include <ctime>
//----------------------------------------
//	Player Constructor, init all info
//----------------------------------------
Player::Player(WorldSession* session) : Object()
{
	attackCount = 0;
	WorldId = 0;
	WorldTableID = 0;
	m_session = session;
	charid = INVALID_CHARACTERID;
	objList.clear();
	moveDirection = 0;
	m_name = "";
	isLowLifeSend = false;
	m_currMap = nullptr;
	cur_target = 0;
	m_objectType = eOBJTYPE::OBJTYPE_PC;
	cur_obj_tagert = NULL;
	isMovingToCharServer = false;
	rpBallTimer = 0;
}


//----------------------------------------
//	Player Destructor, Remove it from world and clean the list
//----------------------------------------
Player::~Player()
{
	CleanupsBeforeDelete();
	objList.clear();
	for (auto it = DropList.begin(); it != DropList.end(); it++)
	{
		if (it->second != NULL)
			delete it->second;
	}
	DropList.clear();
	cur_obj_tagert = nullptr;
	m_session = nullptr;
}
//----------------------------------------
//	Remove from World
//----------------------------------------
void Player::CleanupsBeforeDelete()
{
	RemoveFromWorld();
}
//----------------------------------------
//	Add to world
//----------------------------------------
void Player::AddToWorld()
{
	///- Do not add/remove the player from the object storage
	///- It will crash when updating the ObjectAccessor
	///- The player should only be added when logging in
	Object::AddToWorld();
}
//----------------------------------------
//	Remove from world
//----------------------------------------
void Player::RemoveFromWorld()
{
	// cleanup
	if (IsInWorld())
	{
		///- Release charmed creatures, unsummon totems and remove pets/guardians
		ClearListAndReference();
	}
	Object::RemoveFromWorld();
}
//----------------------------------------
//	Remove all entity from our list and remove us from any list
//----------------------------------------
void Player::ClearListAndReference()
{
	mutexPlayer.lock();
	for (auto it = objList.begin(); it != objList.end();)
	{
		if (it->second != NULL)
		{
			if (it->second->GetTypeId() == OBJTYPE_PC)
			{
				Player* plr = static_cast<Player*>(it->second);
				plr->RemoveFromList(*this);
			}
			it++;
		}
	}
	objList.clear();
	mutexPlayer.unlock();
}
//----------------------------------------
//	Remove from world
//----------------------------------------
void Player::SendPacket(char* data, size_t size)
{
	if (GetSession() != NULL)
	{
		GetSession()->SendPacket(data, size);
	}
}
//------------------------------------------
//		SAVE OUR PLAYER
//------------------------------------------
void Player::SavePlayer()
{	
		GetInventoryManager()->SaveInventory();
		//skillManager.SaveSkill(*this);
		sDB.SavePlayerPositionAndWorldID(GetVectorPosition(), GetVectorOriantation(), GetWorldID(), GetWorldTableID(), GetCharacterID());
		sDB.SavePlayerLife(GetPcProfile()->dwCurLP, GetPcProfile()->wCurEP, GetCharacterID());
		sDB.SaveTitleMarket( GetPcProfile()->sMarking.dwCode, GetCharacterID());
		// do we need to save attributes as we load them at start ?
	
}
//------------------------------------------
//		SETTER
//------------------------------------------

void Player::SetMoveDirection(BYTE movementDirection)
{
	GetState()->sCharStateDetail.sCharStateMoving.byMoveDirection = movementDirection;
}
void Player::SetSession(WorldSession* s)
{
	m_session = s;
}
void Player::SetName(char* name)
{ 
	m_name = name;
}
void Player::SetIsFighting(bool val)
{ 
	isfighting = val;
}
void Player::SetFlying(bool val)
{
	isFlying = val;
}
void Player::SetWorldID(WORLDID id) 
{ 
	WorldId = id;
}
void Player::SetWorldTableID(TBLIDX id) 
{ 
	WorldTableID = id;
}
void Player::SetMap(Map* m)
{
	m_currMap = m;
	WorldId = m->GetId();
}
void Player::ResetMap()
{
	//WorldId = 0;
	m_currMap = nullptr;
}
void Player::SetTarget(HOBJECT hSerial)
{
	cur_target = hSerial;
	cur_obj_tagert = GetFromList(hSerial);
}
void Player::SetMyClass(ePC_CLASS _class)
{
	myClass = _class;
	//GetAttributesManager()->PlayerClassID = _class;
}
void Player::SetMoveDestinationLocation(sVECTOR3 pos)
{
	m_destination_pos = pos;
}
//----------------------------------------
//		GETTER
//----------------------------------------
DWORD Player::GetPowerLevel()
{
	return Dbo_CalculatePowerLevel(GetPcProfile()->avatarAttribute.byLastStr, GetPcProfile()->avatarAttribute.byLastCon, GetPcProfile()->avatarAttribute.byLastFoc, GetPcProfile()->avatarAttribute.byLastDex,
	GetPcProfile()->avatarAttribute.byLastSol, GetPcProfile()->avatarAttribute.byLastEng, GetPcProfile()->avatarAttribute.wLastPhysicalOffence, GetPcProfile()->avatarAttribute.wLastPhysicalDefence, GetPcProfile()->avatarAttribute.wLastEnergyOffence,
	GetPcProfile()->avatarAttribute.wLastEnergyDefence, GetPcProfile()->avatarAttribute.wLastAttackRate, GetPcProfile()->avatarAttribute.wLastDodgeRate, GetPcProfile()->avatarAttribute.wLastCurseSuccessRate,
	GetPcProfile()->avatarAttribute.wLastCurseToleranceRate, GetPcProfile()->avatarAttribute.wLastPhysicalCriticalRate, GetPcProfile()->avatarAttribute.wLastEnergyCriticalRate, GetPcProfile()->avatarAttribute.wLastAttackRate,
	GetPcProfile()->avatarAttribute.wLastMaxLP, GetPcProfile()->avatarAttribute.wLastMaxEP, GetPcProfile()->dwCurLP, GetPcProfile()->wCurEP, 0, GetPcProfile()->byLevel, 0);
}
bool Player::GetIsEmergency()
{
	float current = static_cast<float>(GetPcProfile()->dwCurLP);
	float max = static_cast<float>(GetPcProfile()->avatarAttribute.wBaseMaxLP);
	float percent = current / max;

	return (percent <= 0.25f) ? true : false;
}
sVECTOR3 Player::GetMoveDestinationVector()
{
	return m_destination_pos;
}
InventoryManager	*Player::GetInventoryManager()
{
	return &inventoryManager;
}
AttributesManager *Player::GetAttributesManager()
{
	return &characterManager;
}
ePC_CLASS Player::GetMyClass() const
{
	return myClass;
}
HOBJECT Player::GetTarget() const
{
	return cur_target;
}
WorldSession* Player::GetSession() const
{
	return m_session;
}
CHARACTERID	Player::GetCharacterID()
{
	return charid;
}
sPC_PROFILE	*Player::GetPcProfile()
{
	return characterManager.GetPlayerProfile();
}
MapReference &Player::GetMapRef()
{
	return m_mapRef;
}
sITEM_BRIEF *Player::GetCharEquippedItems()
{
	return inventoryManager.GetEquippedItems();
}
std::string Player::GetGuildName()
{ 
	return GuildName; 
}
eCHARSTATE Player::GetCharEState()
{ 
	return (eCHARSTATE)GetState()->sCharStateBase.byStateID;
}
BYTE Player::GetMoveDirection()
{ 
	return GetState()->sCharStateDetail.sCharStateMoving.byMoveDirection;
}
std::string Player::GetName() 
{ 
	return m_name; 
}
PORTALID Player::GetPortalIdFromList(HOBJECT handle)
{
	mutexPlayer.lock();
	for (auto it = objList.begin(); it != objList.end();)
	{
		if (it->second != NULL)
		{
			if (handle == it->first && it->second->GetTypeId() == OBJTYPE_NPC)
			{
				Npc* npc = reinterpret_cast<Npc*>(it->second);
				for (auto its = sTBM.GetPortalTable()->Begin(); its != sTBM.GetPortalTable()->End(); its++)
				{
					sPORTAL_TBLDAT* portal = (sPORTAL_TBLDAT*)its->second;
					float dist = NtlGetDistance(portal->vLoc.x, portal->vLoc.z, npc->GetNpcData().curPos.x, npc->GetNpcData().curPos.z);
					if (dist <= 100)
					{
						mutexPlayer.unlock();
						return its->second->tblidx;
					}
				}
			}
		}
		it++;
	}
	mutexPlayer.unlock();
	return 255;
}
//----------------------------------------
//	Create player, fill struct and data with database
//----------------------------------------
bool Player::Create(CHARACTERID id)
{
	if (LoadFromDB(id) == false)
	{
		sLog.outError("Player::Create: Error LoadFromDB(id) return false");
		return false;
	}
	if (characterManager.LoadAttributes(charid, this) == false)
	{
		sLog.outError("Player::Create: Error LoadAttributes(charid, this) return false");
		return false;
	}
	if (skillManager.LoadSkill(charid) == false)
	{
		sLog.outError("Player::Create: Error LoadSkill(charid) return false");
		return false;
	}
	if (inventoryManager.LoadInventory(charid, this) == false)
	{
		sLog.outError("Player::Create: Error LoadInventory(charid) return false");
		return false;
	}
	characterManager.LoadAttributesFromItems(inventoryManager.GetEquippedItems());
	characterManager.CreatePlayerBaseAttributesBitFlag();
	handle = sWorld.AcquireSerialId();
	return true;
}
//----------------------------------------
//	Trigger by Map Tick()
//----------------------------------------
void Player::Update(uint32 _update_diff, uint32 _time)
{
	if (!IsInWorld() && !GetIsDead())
		return;	
	//get SystemTime
/*	std::time_t t = std::time(0);
	std::tm* now = std::localtime(&t);
	int Year = now->tm_year + 1900;
	int Moch = now->tm_mon + 1;
	int Day = now->tm_mday;
	int Hour = now->tm_hour;
	int Minute = now->tm_min;
	int Secound = now->tm_sec;
	//start event
	if (Hour == 14 && Minute == 30 && Secound == 0)
	{
		HandleEvent();
	}*/
	m_position_tmp = m_position;
	m_rotation_tmp = m_rotation;
	DWORD AutoAttackTimmer = GetTickCount() - AutoAttackTmmer;
	if (AutoAttackTimmer >= GetPcProfile()->avatarAttribute.wLastAttackSpeedRate)
	{
		if (GetIsSkillCasting() == false && GetIsFighting() == true && GetCharEState() == eCHARSTATE::CHARSTATE_STANDING)
		{
			Attack();
			attackCount++;		
		}
		AutoAttackTmmer = GetTickCount();
	}
	DWORD RegTimmer = GetTickCount() - RegTmmer;
	if (RegTimmer >= 1200)
	{
		if (GetIsFighting() == false && GetIsDead() == false)
		{		
		Regen();		
		ExecuteLPFood();
		ExecuteEPFood();			
		}		
		characterManager.UpdateAttributes(); // update our attributes all every time thats is good to prevent hack
		CalculeRpBall();			
		RegTmmer = GetTickCount();//Set Time		
		BossEventUpdate();
	}	
	DWORD Affecttimmer = GetTickCount() - AffectTime;
	if (Affecttimmer >= 2000)
	{
		CharAffect();
	}
	DWORD TokenTimer = GetTickCount() - NetPyTimmer;
	if (TokenTimer >= 60000)
	{
		if (countnetpy >= 1)
		{
			NetPYUpdate();
		}
		NetPyTimmer = GetTickCount();//Set Time	
		countnetpy += 1;
	}

	//HandleFreeBattleRange();	
	PowerUpUpdate();
	ExecuteBuffTimmer();	
	CheckPVPArea();
	TranformationRegen();
	CalculeLowLife();	
	rpBallTimer += _time;
	CalculePosition(35);
	// UPDATE DROPPED LIST
	UpdateDropListTimer();
	FindRankPlyer();
	
	if (GetIsSkillCasting() == true)
	{
		DWORD SkillTime = GetTickCount() - SkillCastinTime;
		if (SkillTime >= SkillCastinTimeRemain)
		{
			SkillAcion();
			SetIsSkillCasting(false);
			sLog.outDetail("Skill Annimation can be used now");
		}
	}

}
void Player::SkillAcion()
{
	SetIsFighting(false);
	sUG_CHAR_SKILL_REQ *pCharSkillReq = (sUG_CHAR_SKILL_REQ*)packets.GetPacketBuffer();

	sGU_CHAR_SKILL_RES sSkil;
	sSkil.wPacketSize = sizeof(sGU_CHAR_SKILL_RES) - 2;
	sSkil.wOpCode = GU_CHAR_SKILL_RES;
	sSkil.wResultCode = GAME_SKILL_CANT_CAST_NOW;
	int ObjectType;
	float DemageValue[40];
	float CriticalBonusDemageValue[40];
	int AttackType[40];
	bool bleed = false;
	int skillID = skillManager.getIdAtPos(pCharSkillReq->bySlotIndex);
	SkillTable * skillTable = sTBM.GetSkillTable();
	sSKILL_TBLDAT * skillDataOriginal = reinterpret_cast<sSKILL_TBLDAT*>(skillTable->FindData(skillID));
	if (skillDataOriginal != NULL)
	{
		if (GetIsDead() == false)
		{
			sGU_CHAR_ACTION_SKILL skillRes;
			memset(&skillRes, 0, sizeof(sGU_CHAR_ACTION_SKILL));
			skillRes.wPacketSize = sizeof(sGU_CHAR_ACTION_SKILL) - 2;
			skillRes.wOpCode = GU_CHAR_ACTION_SKILL;
			skillRes.wResultCode = GAME_SKILL_CANT_CAST_NOW;
			BuffTypeSkill pBuffData;
			memset(&pBuffData, 0, sizeof(BuffTypeSkill));
			pBuffData.OpCode = GU_BUFF_REGISTERED;
			pBuffData.size = sizeof(BuffTypeSkill) - 2;
			pBuffData.tblidx = INVALID_TBLIDX;
			int Demagecount = 0;
			//Demage Handle		
			for (int D = 0; D < skillDataOriginal->byApply_Target_Max; D++)
			{
				DemageValue[D] = 0;
				AttackType[D] = 0;
				Object* ObjectInfo = static_cast<Object*>(GetFromList(pCharSkillReq->ahApplyTarget[D]));
				if (ObjectInfo != NULL)
				{
					ObjectType = ObjectInfo->GetTypeId();
				}
				if (ObjectType == OBJTYPE_PC)
				{
					Player* PlayerInfo = static_cast<Player*>(GetFromList(pCharSkillReq->ahApplyTarget[D]));
					if (PlayerInfo != NULL)
					{
						int CriticalRate = 0;
						float attack = 0;
						int Defense = 0;
						//Demage Calculation		
						if (skillDataOriginal->bySkill_Effect_Type[0] == 0 && skillDataOriginal->bySkill_Effect_Type[1] == 255)
						{
							if (skillDataOriginal->bySkill_Type == eSKILL_TYPE::SKILL_TYPE_PHYSICAL)
							{
								CriticalRate = GetPcProfile()->avatarAttribute.wLastPhysicalCriticalRate;
								attack = GetPcProfile()->avatarAttribute.wLastPhysicalOffence + skillDataOriginal->SkillValue[0];
								Defense = PlayerInfo->GetPcProfile()->avatarAttribute.wLastPhysicalDefence;
								CriticalBonusDemageValue[Demagecount] = attack / 100 * GetPcProfile()->avatarAttribute.fPhysicalCriticalDamageBonusRate;
							}
							if (skillDataOriginal->bySkill_Type == eSKILL_TYPE::SKILL_TYPE_ENERGY)
							{
								CriticalRate = GetPcProfile()->avatarAttribute.wLastEnergyCriticalRate;
								attack = GetPcProfile()->avatarAttribute.wLastEnergyOffence + skillDataOriginal->SkillValue[0];
								Defense = PlayerInfo->GetPcProfile()->avatarAttribute.wLastEnergyDefence;
								CriticalBonusDemageValue[Demagecount] = attack / 100 * GetPcProfile()->avatarAttribute.fEnergyCriticalDamageBonusRate;
							}
							if (skillDataOriginal->bySkill_Type == eSKILL_TYPE::SKILL_TYPE_STATE)
							{
								attack = GetPcProfile()->avatarAttribute.wLastEnergyOffence + GetPcProfile()->avatarAttribute.wLastPhysicalOffence + skillDataOriginal->SkillValue[0];
								attack /= 2.5;							
								Defense = PlayerInfo->GetPcProfile()->avatarAttribute.wLastEnergyDefence + PlayerInfo->GetPcProfile()->avatarAttribute.wLastPhysicalDefence;
								CriticalBonusDemageValue[Demagecount] = attack / 100 * GetPcProfile()->avatarAttribute.fPhysicalCriticalDamageBonusRate;
							}
						}
						//Skull Demage in Percent
						else if (skillDataOriginal->bySkill_Effect_Type[0] == 1)
						{
							if (skillDataOriginal->bySkill_Type == eSKILL_TYPE::SKILL_TYPE_PHYSICAL)
							{
								CriticalRate = GetPcProfile()->avatarAttribute.wLastPhysicalCriticalRate;
								attack = GetPcProfile()->avatarAttribute.wLastPhysicalOffence / 100 * skillDataOriginal->SkillValue[0];
								Defense = PlayerInfo->GetPcProfile()->avatarAttribute.wLastPhysicalDefence;
								attack /= 1.5;
								CriticalBonusDemageValue[Demagecount] = attack / 100 * GetPcProfile()->avatarAttribute.fPhysicalCriticalDamageBonusRate;
							}
							if (skillDataOriginal->bySkill_Type == eSKILL_TYPE::SKILL_TYPE_ENERGY)
							{
								CriticalRate = GetPcProfile()->avatarAttribute.wLastEnergyCriticalRate;
								attack = GetPcProfile()->avatarAttribute.wLastEnergyOffence / 100 * skillDataOriginal->SkillValue[0];
								Defense = PlayerInfo->GetPcProfile()->avatarAttribute.wLastEnergyDefence;
								attack /= 1.5;
								CriticalBonusDemageValue[Demagecount] = attack / 100 * GetPcProfile()->avatarAttribute.fEnergyCriticalDamageBonusRate;
							}
							if (skillDataOriginal->bySkill_Type == eSKILL_TYPE::SKILL_TYPE_STATE)
							{
								attack = GetPcProfile()->avatarAttribute.wLastEnergyOffence + GetPcProfile()->avatarAttribute.wLastPhysicalOffence + skillDataOriginal->SkillValue[0];
								attack /= 2.5;							
								Defense = PlayerInfo->GetPcProfile()->avatarAttribute.wLastEnergyDefence + PlayerInfo->GetPcProfile()->avatarAttribute.wLastPhysicalDefence;
								CriticalBonusDemageValue[Demagecount] = attack / 100 * GetPcProfile()->avatarAttribute.fPhysicalCriticalDamageBonusRate;
							}
						}
						else if (skillDataOriginal->bySkill_Effect_Type[0] == 0 && skillDataOriginal->bySkill_Effect_Type[1] == 1)
						{
							if (skillDataOriginal->bySkill_Type == eSKILL_TYPE::SKILL_TYPE_PHYSICAL)
							{
								CriticalRate = GetPcProfile()->avatarAttribute.wLastPhysicalCriticalRate;
								attack = (GetPcProfile()->avatarAttribute.wLastPhysicalOffence + skillDataOriginal->SkillValue[0] / 100 * skillDataOriginal->SkillValue[1]);
								Defense = PlayerInfo->GetPcProfile()->avatarAttribute.wLastPhysicalDefence;
								attack /= 1.5;
								CriticalBonusDemageValue[Demagecount] = attack / 100 * GetPcProfile()->avatarAttribute.fPhysicalCriticalDamageBonusRate;

							}
							if (skillDataOriginal->bySkill_Type == eSKILL_TYPE::SKILL_TYPE_ENERGY)
							{
								CriticalRate = GetPcProfile()->avatarAttribute.wLastEnergyCriticalRate;
								attack = (GetPcProfile()->avatarAttribute.wLastEnergyOffence + skillDataOriginal->SkillValue[0] / 100 * skillDataOriginal->SkillValue[1]);
								Defense = PlayerInfo->GetPcProfile()->avatarAttribute.wLastEnergyDefence;
								attack /= 1.5;
								CriticalBonusDemageValue[Demagecount] = attack / 100 * GetPcProfile()->avatarAttribute.fEnergyCriticalDamageBonusRate;
							}
							if (skillDataOriginal->bySkill_Type == eSKILL_TYPE::SKILL_TYPE_STATE)
							{
								attack = GetPcProfile()->avatarAttribute.wLastEnergyOffence + GetPcProfile()->avatarAttribute.wLastPhysicalOffence + skillDataOriginal->SkillValue[0];
								attack /= 2.5;						
								Defense = PlayerInfo->GetPcProfile()->avatarAttribute.wLastEnergyDefence + PlayerInfo->GetPcProfile()->avatarAttribute.wLastPhysicalDefence;
								CriticalBonusDemageValue[Demagecount] = attack / 100 * GetPcProfile()->avatarAttribute.fPhysicalCriticalDamageBonusRate;
							}
						}
						//Demage
						float TotalAttack = attack + Defense;
						float FinalPercent = attack * 100 / TotalAttack;
						DemageValue[Demagecount] = attack * FinalPercent / 100;
						//DemageValue[Demagecount] / 100 *  GetPcProfile()->avatarAttribute.fPhysicalCriticalDamageBonusRate;
						//HitRate
						int HitRate = GetPcProfile()->avatarAttribute.wLastAttackRate;
						int DodgeRate = PlayerInfo->GetPcProfile()->avatarAttribute.wLastDodgeRate;
						float TotalHitRate = HitRate + DodgeRate;
						float TotalHitRatePercent = HitRate * 100 / TotalHitRate;
						int RandomHit = rand() % 100;
						TotalHitRatePercent *= 1.6;
						//printf("Total Hit Percent %f \n", TotalHitRatePercent);

						int BlockCriticalRate = PlayerInfo->GetPcProfile()->avatarAttribute.fCriticalBlockSuccessRate;
						float TotalCriticRate = CriticalRate + BlockCriticalRate;
						float TotalCriticRatePercent = CriticalRate * 100 / TotalCriticRate;
						int RandomCriticHit = rand() % 100;
						//printf("Total Critic rate Percent %f \n", TotalCriticRatePercent);

						if (RandomHit <= TotalHitRatePercent && TotalHitRatePercent > 0)
						{
							if (RandomCriticHit >= 0 && RandomCriticHit <= TotalCriticRatePercent)
							{
								AttackType[Demagecount] = eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_CRITICAL_HIT;

								DemageValue[Demagecount] *= 2;
								DemageValue[Demagecount] += CriticalBonusDemageValue[Demagecount];
							}
							else
							{
								AttackType[Demagecount] = eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_HIT;
							}
						}
						else
						{
							AttackType[Demagecount] = eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE;
							DemageValue[Demagecount] = 0;
						}
						if (DemageValue[Demagecount] <= 0 || DemageValue[Demagecount] > 1000000000)
						{
							DemageValue[Demagecount] = 60;
						}

						printf("RP Bonus %d \n", pCharSkillReq->byRpBonusType);
						if (pCharSkillReq->byRpBonusType == eDBO_RP_BONUS_TYPE::DBO_RP_BONUS_TYPE_KNOCKDOWN)
						{
							sGU_CHAR_KNOCKDOWN state;

							state.wPacketSize = sizeof(sGU_CHAR_KNOCKDOWN) - 2;
							state.wOpCode = GU_CHAR_KNOCKDOWN;

							state.handle = PlayerInfo->GetHandle();
							AttackType[Demagecount] = eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_KNOCKDOWN;

							sWorld.SendToAll((char*)&state, sizeof(sGU_CHAR_KNOCKDOWN));

							sGU_UPDATE_CHAR_STATE res1;

							res1.wOpCode = GU_UPDATE_CHAR_STATE;
							res1.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;

							res1.handle = PlayerInfo->GetHandle();

							res1.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_KNOCKDOWN;
							res1.sCharState.sCharStateDetail.sCharStateKnockdown.vShift.x = PlayerInfo->GetVectorPosition().x + 5;
							res1.sCharState.sCharStateDetail.sCharStateKnockdown.vShift.y = PlayerInfo->GetVectorPosition().y;
							res1.sCharState.sCharStateDetail.sCharStateKnockdown.vShift.z = PlayerInfo->GetVectorPosition().z;

							res1.sCharState.sCharStateBase.vCurLoc = res1.sCharState.sCharStateDetail.sCharStateKnockdown.vShift;
							
							res1.sCharState.sCharStateBase.vCurDir.x = PlayerInfo->GetVectorOriantation().x;
							res1.sCharState.sCharStateBase.vCurDir.y = PlayerInfo->GetVectorOriantation().y;
							res1.sCharState.sCharStateBase.vCurDir.z = PlayerInfo->GetVectorOriantation().z;

							res1.sCharState.sCharStateBase.aspectState.sAspectStateBase.byAspectStateId = PlayerInfo->GetState()->sCharStateBase.aspectState.sAspectStateBase.byAspectStateId;
							res1.sCharState.sCharStateBase.dwConditionFlag = 0;
							res1.sCharState.sCharStateBase.dwStateTime = 0x01;

							res1.sCharState.sCharStateBase.isFighting = false;
							res1.sCharState.sCharStateBase.isFlying = false;


							sWorld.SendToAll((char*)&res1, sizeof(sGU_UPDATE_CHAR_STATE));
							
						}
						if (pCharSkillReq->byRpBonusType == eDBO_RP_BONUS_TYPE::DBO_RP_BONUS_TYPE_RESULT_PLUS)
						{	
							DemageValue[Demagecount] *= 1.4;
						}

						Demagecount += 1;
					}
				}
				if (ObjectType == OBJTYPE_MOB)
				{
					Mob* MobInfo = static_cast<Mob*>(GetFromList(pCharSkillReq->ahApplyTarget[D]));
					if (MobInfo != NULL && MobInfo->GetIsDead() == false)
					{
						//Demage Calculation
						if (skillDataOriginal->bySkill_Effect_Type[0] == 0)
						{
							if (skillDataOriginal->bySkill_Type == eSKILL_TYPE::SKILL_TYPE_PHYSICAL)
							{
								float attack = GetPcProfile()->avatarAttribute.wLastPhysicalOffence + skillDataOriginal->SkillValue[0];
								int TotalAttack = attack + MobInfo->GetMobData().Basic_physical_defence;
								float FinalPercent = attack * 100 / TotalAttack;
								DemageValue[Demagecount] = attack * FinalPercent / 100;
								CriticalBonusDemageValue[Demagecount] = attack / 100 * GetPcProfile()->avatarAttribute.fPhysicalCriticalDamageBonusRate;
							}
							if (skillDataOriginal->bySkill_Type == eSKILL_TYPE::SKILL_TYPE_ENERGY)
							{
								float attack = GetPcProfile()->avatarAttribute.wLastEnergyOffence + skillDataOriginal->SkillValue[0];
								int TotalAttack = attack + MobInfo->GetMobData().Basic_energy_defence;
								float FinalPercent = attack * 100 / TotalAttack;
								DemageValue[Demagecount] = attack * FinalPercent / 100;
								CriticalBonusDemageValue[Demagecount] = attack / 100 * GetPcProfile()->avatarAttribute.fEnergyCriticalDamageBonusRate;
							}
							if (skillDataOriginal->bySkill_Type == eSKILL_TYPE::SKILL_TYPE_STATE)
							{
								float attack = GetPcProfile()->avatarAttribute.wLastEnergyOffence + GetPcProfile()->avatarAttribute.wLastPhysicalOffence;

								float Defense = MobInfo->GetMobData().Basic_energy_defence + MobInfo->GetMobData().Basic_physical_defence;
								//float attack =  GetPcProfile()->avatarAttribute.wLastEnergyOffence + skillDataOriginal->SkillValue[0];
								int TotalAttack = attack + Defense;
								float FinalPercent = attack * 100 / TotalAttack;
								DemageValue[Demagecount] = attack * FinalPercent / 100;
								CriticalBonusDemageValue[Demagecount] = attack / 100 * GetPcProfile()->avatarAttribute.fPhysicalCriticalDamageBonusRate;
							}
						}
						//Skull Demage in Percent
						else if (skillDataOriginal->bySkill_Effect_Type[0] == 1)
						{

							if (skillDataOriginal->bySkill_Type == eSKILL_TYPE::SKILL_TYPE_PHYSICAL)
							{
								float attack = GetPcProfile()->avatarAttribute.wLastPhysicalOffence / 100 * skillDataOriginal->SkillValue[0];
								int TotalAttack = attack + MobInfo->GetMobData().Basic_physical_defence;
								float FinalPercent = attack * 100 / TotalAttack;
								DemageValue[Demagecount] = attack * FinalPercent / 100;
								CriticalBonusDemageValue[Demagecount] = attack / 100 * GetPcProfile()->avatarAttribute.fPhysicalCriticalDamageBonusRate;
							}
							if (skillDataOriginal->bySkill_Type == eSKILL_TYPE::SKILL_TYPE_ENERGY)
							{
								float attack = GetPcProfile()->avatarAttribute.wLastEnergyOffence / 100 * skillDataOriginal->SkillValue[0];
								int TotalAttack = attack + MobInfo->GetMobData().Basic_energy_defence;
								float FinalPercent = attack * 100 / TotalAttack;
								DemageValue[Demagecount] = attack * FinalPercent / 100;
								CriticalBonusDemageValue[Demagecount] = attack / 100 * GetPcProfile()->avatarAttribute.fEnergyCriticalDamageBonusRate;
							}
							if (skillDataOriginal->bySkill_Type == eSKILL_TYPE::SKILL_TYPE_STATE)
							{
								float attack = GetPcProfile()->avatarAttribute.wLastEnergyOffence + GetPcProfile()->avatarAttribute.wLastPhysicalOffence;

								float Defense = MobInfo->GetMobData().Basic_energy_defence + MobInfo->GetMobData().Basic_physical_defence;
								//float attack =  GetPcProfile()->avatarAttribute.wLastEnergyOffence + skillDataOriginal->SkillValue[0];
								int TotalAttack = attack + Defense;
								float FinalPercent = attack * 100 / TotalAttack;
								DemageValue[Demagecount] = attack * FinalPercent / 100;
								CriticalBonusDemageValue[Demagecount] = attack / 100 * GetPcProfile()->avatarAttribute.fPhysicalCriticalDamageBonusRate;
							}
						}
						//HitRate
						int HitRate = GetPcProfile()->avatarAttribute.wLastAttackRate;
						int DodgeRate = MobInfo->GetMobData().Dodge_rate;
						float TotalHitRate = HitRate + DodgeRate;
						float TotalHitRatePercent = HitRate * 100 / TotalHitRate;
						int RandomHit = rand() % 100;
						//printf("Total Hit Percent %f \n", TotalHitRatePercent);

						int BlockCriticalRate = MobInfo->GetMobData().Block_rate;
						float TotalCriticRate = GetPcProfile()->avatarAttribute.wLastPhysicalCriticalRate + BlockCriticalRate;
						float TotalCriticRatePercent = BlockCriticalRate * 100 / TotalCriticRate;
						int RandomCriticHit = rand() % 100;
						//	printf("Total Critic rate Percent %f \n", TotalCriticRatePercent);

						if (RandomHit <= TotalHitRatePercent && TotalHitRatePercent > 0)
						{
							if (RandomCriticHit >= 0 && RandomCriticHit <= 40)
							{
								AttackType[Demagecount] = eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_CRITICAL_HIT;
								DemageValue[Demagecount] *= 2;
								DemageValue[Demagecount] += CriticalBonusDemageValue[Demagecount];
							}
							else
							{
								AttackType[Demagecount] = eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_HIT;
							}
						}
						else
						{
							AttackType[Demagecount] = eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE;
							DemageValue[Demagecount] = 0;
						}
						if (DemageValue[Demagecount] <= 0 || DemageValue[Demagecount] > 1000000000)
						{
							DemageValue[Demagecount] = 60;
						}
						if (pCharSkillReq->byRpBonusType == eDBO_RP_BONUS_TYPE::DBO_RP_BONUS_TYPE_KNOCKDOWN)
						{
							sGU_CHAR_KNOCKDOWN state;

							state.wPacketSize = sizeof(sGU_CHAR_KNOCKDOWN) - 2;
							state.wOpCode = GU_CHAR_KNOCKDOWN;

							state.handle = MobInfo->GetHandle();
							AttackType[Demagecount] = eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_KNOCKDOWN;

							sWorld.SendToAll((char*)&state, sizeof(sGU_CHAR_KNOCKDOWN));
						}
						if (pCharSkillReq->byRpBonusType == eDBO_RP_BONUS_TYPE::DBO_RP_BONUS_TYPE_RESULT_PLUS)
						{
							DemageValue[Demagecount] *= 1.4;
						}
						Demagecount += 1;
					}
				}
			}
		
			for (int Effect = 0; Effect <= 2; Effect++)
			{
				////Skill Handle
				int count = 0;
				sSYSTEM_EFFECT_TBLDAT * SystemEffectData = NULL;
				SystemEffectData = (sSYSTEM_EFFECT_TBLDAT*)sTBM.GetSystemEffectTable()->FindData(skillDataOriginal->skill_Effect[Effect]);

				if (SystemEffectData != NULL)
				{
					//printf("Skill EffectCode %d \n", SystemEffectData->effectCode);
					switch (SystemEffectData->effectCode)
					{
					case ACTIVE_DIRECT_DAMAGE:
					case ACTIVE_BACKSTAB:
					case ACTIVE_WARP_BACK_ATTACK:
					case ACTIVE_CRITICAL_ATTACK:
					{
						sSkil.wResultCode = GAME_SUCCESS;

						skillRes.wResultCode = GAME_SUCCESS;
						skillRes.handle = GetHandle();
						skillRes.hAppointedTarget = pCharSkillReq->hTarget;
						skillRes.skillId = skillDataOriginal->tblidx;
						skillRes.dwLpEpEventId = skillDataOriginal->tblidx;
						skillRes.byRpBonusType = pCharSkillReq->byRpBonusType;
						skillRes.bIsSkillHarmful = false;

						for (int i = 0; i < skillDataOriginal->byApply_Target_Max; i++)
						{
							if (ObjectType == OBJTYPE_PC)
							{
								Player* PlayerInfo = static_cast<Player*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (PlayerInfo != NULL  && PlayerInfo->GetIsDead() == false)
								{
									if (PlayerInfo->GetAttributesManager()->IsinPVP == true || PlayerInfo->GetAttributesManager()->PlayerInFreeBatle == true)
									{
										skillRes.aSkillResult[count].hTarget = PlayerInfo->GetHandle();
										skillRes.aSkillResult[count].byAttackResult = AttackType[count];

										skillRes.aSkillResult[count].effectResult[Effect].eResultType = DBO_SYSTEM_EFFECT_RESULT_TYPE_DD_DOT;
										skillRes.aSkillResult[count].effectResult[Effect].Value1 = DemageValue[count];
										skillRes.aSkillResult[count].effectResult[Effect].Value2 = 0;
										skillRes.aSkillResult[count].effectResult[Effect].Value3 = 0;
										skillRes.aSkillResult[count].effectResult[Effect].Value4 = 0;
										skillRes.aSkillResult[count].effectResult[Effect].Value5 = 0;
										skillRes.aSkillResult[count].effectResult[Effect].Value6 = 0;
										skillRes.aSkillResult[count].byBlockedAction = 255;
										skillRes.aSkillResult[count].vShift = PlayerInfo->GetVectorPosition();
										skillRes.aSkillResult[count].vShift1 = PlayerInfo->GetVectorPosition();
										skillRes.bySkillResultCount = count + 1;


										PlayerInfo->TakeDamage(skillRes.aSkillResult[count].effectResult[Effect].Value1);
										count = 1;
									}
								}
							}
							if (ObjectType == OBJTYPE_MOB)
							{
								Mob* MobInfo = static_cast<Mob*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (MobInfo != NULL && MobInfo->GetIsDead() == false)
								{

									skillRes.aSkillResult[count].hTarget = MobInfo->GetHandle();
									skillRes.aSkillResult[count].byAttackResult = AttackType[count];

									skillRes.aSkillResult[count].effectResult[Effect].eResultType = DBO_SYSTEM_EFFECT_RESULT_TYPE_DD_DOT;
									skillRes.aSkillResult[count].effectResult[Effect].Value1 = DemageValue[count];
									skillRes.aSkillResult[count].effectResult[Effect].Value2 = 0;
									skillRes.aSkillResult[count].effectResult[Effect].Value3 = 0;
									skillRes.aSkillResult[count].effectResult[Effect].Value4 = 0;
									skillRes.aSkillResult[count].effectResult[Effect].Value5 = 0;
									skillRes.aSkillResult[count].effectResult[Effect].Value6 = 0;
									skillRes.aSkillResult[count].byBlockedAction = 255;
									//skillRes.aSkillResult[count].vShift = MobInfo->GetVectorPosition();
									//skillRes.aSkillResult[count].vShift1 = MobInfo->GetVectorPosition();
									skillRes.bySkillResultCount = count + 1;


									//if (MobInfo->attackers == 0)
									MobInfo->attackers = GetHandle();
									MobInfo->TakeDamage(skillRes.aSkillResult[count].effectResult[Effect].Value1);
									count += 1;
								}
							}
						}
						break;
					}
					case ACTIVE_BLEED:
					{
						sSkil.wResultCode = GAME_SUCCESS;

						skillRes.skillId = skillDataOriginal->tblidx;
						skillRes.wResultCode = GAME_SUCCESS;
						skillRes.byRpBonusType = 0;//Untested
						skillRes.wOpCode = GU_CHAR_ACTION_SKILL;
						skillRes.handle = GetHandle();//My Handle
						skillRes.hAppointedTarget = skillDataOriginal->byAppoint_Target;

						for (int i = 0; i < skillDataOriginal->byApply_Target_Max; i++)
						{
							if (ObjectType == OBJTYPE_PC)
							{
								Player* PlayerInfo = static_cast<Player*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (PlayerInfo != NULL  && PlayerInfo->GetIsDead() == false)
								{
									if (PlayerInfo->GetAttributesManager()->IsinPVP == true || PlayerInfo->GetAttributesManager()->PlayerInFreeBatle == true)
									{
										if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
										{
											bleed = true;
											pBuffData.hHandle = PlayerInfo->GetHandle();
											//pBuffData.slot = 1;
											pBuffData.tblidx = skillDataOriginal->tblidx;
											pBuffData.bySourceType = 0;
											pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs + GetPcProfile()->avatarAttribute.fKeepTimeChangePercent;
											pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs + GetPcProfile()->avatarAttribute.fKeepTimeChangePercent;//Time

											float BleedDemage = DemageValue[i] / 3.5;
											pBuffData.isactive = 1;
											pBuffData.Type = 0;
											pBuffData.BuffInfo[Effect].SystemEffectValue = BleedDemage;
											pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs + GetPcProfile()->avatarAttribute.fKeepTimeChangePercent;
											pBuffData.BuffInfo[Effect].dwSystemEffectValue = BleedDemage;

											if (pBuffData.BuffInfo[Effect].dwSystemEffectValue <= 0 || pBuffData.BuffInfo[Effect].dwSystemEffectValue > 1000000)
											{
												pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
												pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];
											}
											if (pBuffData.hHandle != 0 || pBuffData.hHandle != INVALID_TBLIDX)
											{
												SendPacket((char*)&pBuffData, sizeof(BuffTypeSkill));
												SendToPlayerList((char*)&pBuffData, sizeof(BuffTypeSkill));
											}
											//Handle Buff Time List
											int FreePlace = 0;
											for (int i = 0; i <= 32; i++)
											{
												if (PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
												{
													//printf("Regist new buff \n");
													FreePlace = i;
												}

											}
											PlayerInfo->ExecuteEffectCalculation(pBuffData.tblidx, false);

											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffSlot = 0;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].isAffectPlayer = true;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].EffectType = skillDataOriginal->skill_Effect[Effect];
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].EffectValue[0] = pBuffData.BuffInfo[Effect].SystemEffectValue;
											GetAttributesManager()->sBuffTimeInfo[FreePlace].isMob = false;
										}
										count += 1;
									}
								}
							}
							if (ObjectType == OBJTYPE_MOB)
							{
								Mob* MobInfo = static_cast<Mob*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (MobInfo != NULL)
								{
									if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
									{
										bleed = true;
										pBuffData.hHandle = MobInfo->GetHandle();
										//pBuffData.slot = 1;
										pBuffData.tblidx = skillDataOriginal->tblidx;
										pBuffData.bySourceType = 0;
										pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs + GetPcProfile()->avatarAttribute.fKeepTimeChangePercent;
										pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs + GetPcProfile()->avatarAttribute.fKeepTimeChangePercent;//Time
										float BleedDemage = DemageValue[i] / 3.5;
										pBuffData.isactive = 1;
										pBuffData.Type = 0;
										pBuffData.BuffInfo[Effect].SystemEffectValue = BleedDemage;
										pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs + GetPcProfile()->avatarAttribute.fKeepTimeChangePercent;
										pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect] + GetPcProfile()->avatarAttribute.fDotValueChangePercent;
										if (pBuffData.hHandle != 0 || pBuffData.hHandle != INVALID_TBLIDX)
										{
											SendPacket((char*)&pBuffData, sizeof(BuffTypeSkill));
											SendToPlayerList((char*)&pBuffData, sizeof(BuffTypeSkill));
										}
										//Handle Buff Time List
										int FreePlace = 0;
										for (int i = 0; i <= 32; i++)
										{
											if (GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
											{
												//printf("Regist new buff \n");
												FreePlace = i;
											}

										}
										ExecuteEffectCalculation(pBuffData.tblidx, false);

										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffSlot = 0;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].isAffectPlayer = true;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].EffectType = skillDataOriginal->skill_Effect[Effect];
										GetAttributesManager()->sBuffTimeInfo[FreePlace].EffectValue[0] = pBuffData.BuffInfo[Effect].SystemEffectValue;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].isMob = true;
									}
								}
							}
						}

						break;
					}
					case ACTIVE_POISON:
					case ACTIVE_STOMACHACHE:
					case ACTIVE_BURN:
					{
						sSkil.wResultCode = GAME_SUCCESS;

						skillRes.skillId = skillDataOriginal->tblidx;
						skillRes.wResultCode = GAME_SUCCESS;
						skillRes.byRpBonusType = 0;//Untested
						skillRes.wOpCode = GU_CHAR_ACTION_SKILL;
						skillRes.handle = GetHandle();//My Handle
						skillRes.hAppointedTarget = skillDataOriginal->byAppoint_Target;

						for (int i = 0; i < skillDataOriginal->byApply_Target_Max; i++)
						{
							if (ObjectType == OBJTYPE_PC)
							{
								Player* PlayerInfo = static_cast<Player*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (PlayerInfo != NULL  && PlayerInfo->GetIsDead() == false)
								{
									if (PlayerInfo->GetAttributesManager()->IsinPVP == true || PlayerInfo->GetAttributesManager()->PlayerInFreeBatle == true)
									{
										if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
										{
											bleed = true;
											pBuffData.hHandle = PlayerInfo->GetHandle();
											//pBuffData.slot = 1;
											pBuffData.tblidx = skillDataOriginal->tblidx;
											pBuffData.bySourceType = 0;
											pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs + GetPcProfile()->avatarAttribute.fKeepTimeChangePercent;
											pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs + GetPcProfile()->avatarAttribute.fKeepTimeChangePercent;
											float BleedDemage = DemageValue[i] / 3.5;
											pBuffData.isactive = 1;
											pBuffData.Type = 0;
											pBuffData.BuffInfo[Effect].SystemEffectValue = BleedDemage;
											pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs + GetPcProfile()->avatarAttribute.fKeepTimeChangePercent;
											pBuffData.BuffInfo[Effect].dwSystemEffectValue = BleedDemage;

											if (pBuffData.BuffInfo[Effect].dwSystemEffectValue <= 0 || pBuffData.BuffInfo[Effect].dwSystemEffectValue > 100000)
											{
												pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
												pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];
											}
											if (pBuffData.hHandle != 0 || pBuffData.hHandle != INVALID_TBLIDX)
											{
												SendPacket((char*)&pBuffData, sizeof(BuffTypeSkill));
												SendToPlayerList((char*)&pBuffData, sizeof(BuffTypeSkill));
											}
											//Handle Buff Time List
											int FreePlace = 0;
											for (int i = 0; i <= 32; i++)
											{
												if (PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
												{
													//printf("Regist new buff \n");
													FreePlace = i;
												}

											}
											PlayerInfo->ExecuteEffectCalculation(pBuffData.tblidx, false);

											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffSlot = 0;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].isAffectPlayer = true;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].EffectType = skillDataOriginal->skill_Effect[Effect];
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].EffectValue[0] = pBuffData.BuffInfo[Effect].SystemEffectValue;
										}
										count += 1;
									}
								}
							}
							if (ObjectType == OBJTYPE_MOB)
							{
								Mob* MobInfo = static_cast<Mob*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (MobInfo != NULL)
								{
									if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
									{
										bleed = true;
										pBuffData.hHandle = MobInfo->GetHandle();
										//pBuffData.slot = 1;
										pBuffData.tblidx = skillDataOriginal->tblidx;
										pBuffData.bySourceType = 0;
										pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs + GetPcProfile()->avatarAttribute.fKeepTimeChangePercent;
										pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs + GetPcProfile()->avatarAttribute.fKeepTimeChangePercent;
										float BleedDemage = DemageValue[i] / 3.5;
										pBuffData.isactive = 1;
										pBuffData.Type = 0;
										pBuffData.BuffInfo[Effect].SystemEffectValue = BleedDemage;
										pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs + GetPcProfile()->avatarAttribute.fKeepTimeChangePercent;
										pBuffData.BuffInfo[Effect].dwSystemEffectValue = BleedDemage;
										if (pBuffData.hHandle != 0 || pBuffData.hHandle != INVALID_TBLIDX)
										{
											SendPacket((char*)&pBuffData, sizeof(BuffTypeSkill));
											SendToPlayerList((char*)&pBuffData, sizeof(BuffTypeSkill));
										}
										//Handle Buff Time List
										int FreePlace = 0;
										for (int i = 0; i <= 32; i++)
										{
											if (GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
											{
												//printf("Regist new buff \n");
												FreePlace = i;
											}

										}
										ExecuteEffectCalculation(pBuffData.tblidx, false);

										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffSlot = 0;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].isAffectPlayer = true;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].EffectType = skillDataOriginal->skill_Effect[Effect];
										GetAttributesManager()->sBuffTimeInfo[FreePlace].EffectValue[0] = pBuffData.BuffInfo[Effect].SystemEffectValue;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].isMob = true;
									}
								}
							}
						}

						break;
					}
					case ACTIVE_DIRECT_HEAL:
					{
						sSkil.wResultCode = GAME_SUCCESS;

						skillRes.wResultCode = GAME_SUCCESS;
						skillRes.handle = GetHandle();
						skillRes.hAppointedTarget = pCharSkillReq->hTarget;
						skillRes.skillId = skillDataOriginal->tblidx;
						skillRes.dwLpEpEventId = skillDataOriginal->tblidx;
						skillRes.byRpBonusType = pCharSkillReq->byRpBonusType;
						skillRes.bIsSkillHarmful = false;
						if (pCharSkillReq->hTarget == GetHandle())
						{
							skillRes.aSkillResult[count].hTarget = GetHandle();
							skillRes.aSkillResult[count].byAttackResult = eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_HIT;
							float HealPercent = GetPcProfile()->avatarAttribute.wLastEnergyOffence / 100 * 35;
							skillRes.aSkillResult[count].effectResult[Effect].eResultType = DBO_SYSTEM_EFFECT_RESULT_TYPE_DH_HOT;
							skillRes.aSkillResult[count].effectResult[Effect].Value1 = skillDataOriginal->SkillValue[Effect] + HealPercent;
							skillRes.aSkillResult[count].effectResult[Effect].Value2 = 0;
							skillRes.aSkillResult[count].effectResult[Effect].Value3 = 0;
							skillRes.aSkillResult[count].effectResult[Effect].Value4 = 0;
							skillRes.aSkillResult[count].effectResult[Effect].Value5 = 0;
							skillRes.aSkillResult[count].effectResult[Effect].Value6 = 0;
							skillRes.aSkillResult[count].byBlockedAction = 255;
							skillRes.aSkillResult[count].vShift = GetVectorPosition();
							skillRes.aSkillResult[count].vShift1 = GetVectorOriantation();
							skillRes.bySkillResultCount = count + 1;
							int newlp = GetPcProfile()->dwCurLP;
							if (newlp <= GetPcProfile()->avatarAttribute.wLastMaxLP)
							{
								newlp += skillRes.aSkillResult[count].effectResult[Effect].Value1;
							}
							if (newlp >= GetPcProfile()->avatarAttribute.wLastMaxLP)
							{
								newlp = GetPcProfile()->avatarAttribute.wLastMaxLP;
							}
							sGU_UPDATE_CHAR_LP LPs;
							LPs.wOpCode = GU_UPDATE_CHAR_LP;
							LPs.wPacketSize = sizeof(sGU_UPDATE_CHAR_LP) - 2;
							GetPcProfile()->dwCurLP = newlp;
							LPs.dwLpEpEventId = 0;
							LPs.handle = GetHandle();
							LPs.wCurLP = GetPcProfile()->dwCurLP;
							LPs.wMaxLP = GetPcProfile()->avatarAttribute.wLastMaxLP;
							SendPacket((char*)&LPs, sizeof(sGU_UPDATE_CHAR_LP));
							SendToPlayerList((char*)&LPs, sizeof(sGU_UPDATE_CHAR_LP));
							count = 1;
						}
						for (int i = 0; i < skillDataOriginal->byApply_Target_Max; i++)
						{
							if (ObjectType == OBJTYPE_PC)
							{
								Player* PlayerInfo = static_cast<Player*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (PlayerInfo != NULL &&  GetAttributesManager()->PlayerInFreeBatle == false)
								{
									skillRes.aSkillResult[count].hTarget = PlayerInfo->GetHandle();
									skillRes.aSkillResult[count].byAttackResult = eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_HIT;
									float HealPercent = GetPcProfile()->avatarAttribute.wLastEnergyOffence / 100 * 35;
									skillRes.aSkillResult[count].effectResult[Effect].eResultType = DBO_SYSTEM_EFFECT_RESULT_TYPE_DH_HOT;
									skillRes.aSkillResult[count].effectResult[Effect].Value1 = skillDataOriginal->SkillValue[Effect] + HealPercent;
									skillRes.aSkillResult[count].effectResult[Effect].Value2 = 0;
									skillRes.aSkillResult[count].effectResult[Effect].Value3 = 0;
									skillRes.aSkillResult[count].effectResult[Effect].Value4 = 0;
									skillRes.aSkillResult[count].effectResult[Effect].Value5 = 0;
									skillRes.aSkillResult[count].effectResult[Effect].Value6 = 0;
									skillRes.aSkillResult[count].byBlockedAction = 255;
									skillRes.aSkillResult[count].vShift = PlayerInfo->GetVectorPosition();
									skillRes.aSkillResult[count].vShift1 = PlayerInfo->GetVectorOriantation();
									skillRes.bySkillResultCount = count + 1;
									int newlp = PlayerInfo->GetPcProfile()->dwCurLP;
									if (newlp <= PlayerInfo->GetPcProfile()->avatarAttribute.wLastMaxLP)
									{
										newlp += skillRes.aSkillResult[count].effectResult[Effect].Value1;
									}
									if (newlp >= PlayerInfo->GetPcProfile()->avatarAttribute.wLastMaxLP)
									{
										newlp = PlayerInfo->GetPcProfile()->avatarAttribute.wLastMaxLP;
									}
									sGU_UPDATE_CHAR_LP LPs;
									LPs.wOpCode = GU_UPDATE_CHAR_LP;
									LPs.wPacketSize = sizeof(sGU_UPDATE_CHAR_LP) - 2;
									PlayerInfo->GetPcProfile()->dwCurLP = newlp;
									LPs.dwLpEpEventId = 0;
									LPs.handle = PlayerInfo->GetHandle();
									LPs.wCurLP = PlayerInfo->GetPcProfile()->dwCurLP;
									LPs.wMaxLP = PlayerInfo->GetPcProfile()->avatarAttribute.wLastMaxLP;
									PlayerInfo->SendPacket((char*)&LPs, sizeof(sGU_UPDATE_CHAR_LP));
									PlayerInfo->SendToPlayerList((char*)&LPs, sizeof(sGU_UPDATE_CHAR_LP));
									count = 1;
								}
							}
						}
						break;
					}
					case ACTIVE_SUPER_SAIYAN:
					{
						sSkil.wResultCode = GAME_SUCCESS;

						skillRes.skillId = skillDataOriginal->tblidx;
						skillRes.wResultCode = GAME_SUCCESS;
						skillRes.byRpBonusType = 0;//Untested
						skillRes.wOpCode = GU_CHAR_ACTION_SKILL;
						skillRes.handle = GetHandle();//My Handle
						skillRes.hAppointedTarget = GetHandle();
						GetState()->sCharStateBase.aspectState.sAspectStateBase.byAspectStateId = eASPECTSTATE::ASPECTSTATE_SUPER_SAIYAN;
						GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.bIsEngineOn = false;
						GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.hVehicleItem = INVALID_TBLIDX;
						GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.idVehicleTblidx = INVALID_TBLIDX;
						UpdateAspectState(eASPECTSTATE::ASPECTSTATE_SUPER_SAIYAN);
						break;
					}
					case ACTIVE_PURE_MAJIN:
					{
						sSkil.wResultCode = GAME_SUCCESS;

						skillRes.skillId = skillDataOriginal->tblidx;
						skillRes.wResultCode = GAME_SUCCESS;
						skillRes.byRpBonusType = 0;//Untested
						skillRes.wOpCode = GU_CHAR_ACTION_SKILL;
						skillRes.handle = GetHandle();//My Handle
						skillRes.hAppointedTarget = GetHandle();
						GetState()->sCharStateBase.aspectState.sAspectStateBase.byAspectStateId = eASPECTSTATE::ASPECTSTATE_PURE_MAJIN;
						GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.bIsEngineOn = false;
						GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.hVehicleItem = INVALID_TBLIDX;
						GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.idVehicleTblidx = INVALID_TBLIDX;
						UpdateAspectState(eASPECTSTATE::ASPECTSTATE_PURE_MAJIN);
						break;
					}
					case ACTIVE_KAIOKEN:
					{
						if (GetState()->sCharStateBase.aspectState.sAspectStateDetail.sKaioken.byRepeatingCount >= 100)
						{
							GetState()->sCharStateBase.aspectState.sAspectStateDetail.sKaioken.byRepeatingCount = 0;
							GetState()->sCharStateBase.aspectState.sAspectStateDetail.sKaioken.bySourceGrade = 0;
						}
						if (GetState()->sCharStateBase.aspectState.sAspectStateDetail.sKaioken.byRepeatingCount >= 3 && GetState()->sCharStateBase.aspectState.sAspectStateDetail.sKaioken.byRepeatingCount <= 4)
						{
							//You Get Kaioken Limitacion
						}
						else
						{
							sSkil.wResultCode = GAME_SUCCESS;

							skillRes.skillId = skillDataOriginal->tblidx;
							skillRes.wResultCode = GAME_SUCCESS;
							skillRes.byRpBonusType = 0;//Untested
							skillRes.wOpCode = GU_CHAR_ACTION_SKILL;
							skillRes.handle = GetHandle();//My Handle
							skillRes.hAppointedTarget = GetHandle();
							GetState()->sCharStateBase.aspectState.sAspectStateBase.byAspectStateId = eASPECTSTATE::ASPECTSTATE_KAIOKEN;
							GetState()->sCharStateBase.aspectState.sAspectStateDetail.sKaioken.byRepeatingCount += 1;
							GetState()->sCharStateBase.aspectState.sAspectStateDetail.sKaioken.bySourceGrade = 0;

							UpdateAspectState(eASPECTSTATE::ASPECTSTATE_KAIOKEN);
							//	 GetAttributesManager()->SetLastPhysicalOffence(50);
							// GetAttributesManager()->SetLastEnergyOffence(50);						
						}
						break;
					}
					case ACTIVE_GREAT_NAMEK:
					{
						sSkil.wResultCode = GAME_SUCCESS;

						skillRes.skillId = skillDataOriginal->tblidx;
						skillRes.wResultCode = GAME_SUCCESS;
						skillRes.byRpBonusType = 0;//Untested
						skillRes.wOpCode = GU_CHAR_ACTION_SKILL;
						skillRes.handle = GetHandle();//My Handle
						skillRes.hAppointedTarget = GetHandle();
						GetState()->sCharStateBase.aspectState.sAspectStateBase.byAspectStateId = eASPECTSTATE::ASPECTSTATE_GREAT_NAMEK;
						GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.bIsEngineOn = false;
						GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.hVehicleItem = INVALID_TBLIDX;
						GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.idVehicleTblidx = INVALID_TBLIDX;
						UpdateAspectState(eASPECTSTATE::ASPECTSTATE_GREAT_NAMEK);
						break;
					}
					case ACTIVE_MAX_LP_UP://100% 						
					case ACTIVE_MAX_EP_UP://100% 
					case ACTIVE_MAX_RP_UP://100% 
					case ACTIVE_PHYSICAL_OFFENCE_UP://100% 
					case ACTIVE_ENERGY_OFFENCE_UP://100% 
					case ACTIVE_PHYSICAL_DEFENCE_UP://100% 
					case ACTIVE_ENERGY_DEFENCE_UP://100% 
					case ACTIVE_STR_UP:	//100% 				
					case ACTIVE_CON_UP:	//100% 				
					case ACTIVE_FOC_UP:	//100% 				
					case ACTIVE_DEX_UP:	//100% 				
					case ACTIVE_SOL_UP:	//100% 				
					case ACTIVE_ENG_UP:	//100% 				
					case ACTIVE_MOVE_SPEED_UP://100% 
					case ACTIVE_ATTACK_SPEED_UP://100% 
					case ACTIVE_ATTACK_RATE_UP://100% 
					case ACTIVE_DODGE_RATE_UP://100% 
					case ACTIVE_BLOCK_RATE_UP://100% 
											  //case ACTIVE_LP_REGENERATION://need Handle the effect is here to try do effects in order 
											  //case ACTIVE_EP_REGENERATION://need Handle the effect is here to try do effects in order 
											  //case ACTIVE_RP_CHARGE_SPEED://need Handle the effect is here to try do effects in order 
					case ACTIVE_PHYSICAL_CRITICAL:
					case ACTIVE_ENERGY_CRITICAL:
					case ACTIVE_SKILL_CASTING_TIME_DOWN:
					case ACTIVE_SKILL_COOL_TIME_DOWN:
					{
						sSkil.wResultCode = GAME_SUCCESS;

						skillRes.skillId = skillDataOriginal->tblidx;
						skillRes.wResultCode = GAME_SUCCESS;
						skillRes.byRpBonusType = 0;//Untested
						skillRes.wOpCode = GU_CHAR_ACTION_SKILL;
						skillRes.handle = GetHandle();//My Handle
						skillRes.hAppointedTarget = skillDataOriginal->byAppoint_Target;

						if (pCharSkillReq->hTarget == GetHandle())
						{
							pBuffData.hHandle = GetHandle();
							//pBuffData.slot = 1;
							pBuffData.tblidx = skillDataOriginal->tblidx;
							pBuffData.bySourceType = 0;
							pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs;
							pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs;//Time

							pBuffData.isactive = 1;
							pBuffData.Type = 0;
							pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
							pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs;
							pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];
							//Handle Buff Time List
							int FreePlace = 0;
							for (int i = 0; i <= 32; i++)
							{
								if (GetAttributesManager()->sBuffTimeInfo[i].BuffID == pBuffData.tblidx)
								{
									sGU_BUFF_DROPPED dropbuff;
									dropbuff.wOpCode = GU_BUFF_DROPPED;
									dropbuff.wPacketSize = sizeof(sGU_BUFF_DROPPED) - 2;
									dropbuff.hHandle = GetHandle();
									dropbuff.bySourceType = eDBO_OBJECT_SOURCE::DBO_OBJECT_SOURCE_SKILL;
									dropbuff.Slot = 0;
									dropbuff.tblidx = pBuffData.tblidx;
									dropbuff.unk1 = 0;
									SendPacket((char*)&dropbuff, sizeof(sGU_BUFF_DROPPED));
									SendToPlayerList((char*)&dropbuff, sizeof(sGU_BUFF_DROPPED));
									GetAttributesManager()->sBuffTimeInfo[i].BuffIsActive = false;
									GetAttributesManager()->sBuffTimeInfo[i].BuffEndTime = INVALID_TBLIDX;
									GetAttributesManager()->sBuffTimeInfo[i].BuffTime = INVALID_TBLIDX;
									GetAttributesManager()->sBuffTimeInfo[i].BuffID = INVALID_TBLIDX;
									ExecuteEffectCalculation(pBuffData.tblidx, true);
									//printf("Alardy got that buff \n");
									FreePlace = i;
								}
								//GetFreeSlot
								else if (GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
								{
									//	printf("Regist new buff \n");
									FreePlace = i;
								}

							}
							ExecuteEffectCalculation(pBuffData.tblidx, false);

							GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
							GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
							GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
							GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
							GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
							GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffSlot = 0;
						}
						//Area Buff
						/*	for (int i = 0; i < skillDataOriginal->byApply_Target_Max; i++)
						{
						if (ObjectType == OBJTYPE_PC)
						{
						Player* PlayerInfo = static_cast<Player*>( GetFromList(pCharSkillReq->ahApplyTarget[i]));
						if (PlayerInfo != NULL)
						{
						pBuffData.hHandle = PlayerInfo->GetHandle();
						//pBuffData.slot = 1;
						pBuffData.tblidx = skillDataOriginal->tblidx;
						pBuffData.bySourceType = 0;
						pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs;
						pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs;//Time

						pBuffData.isactive = 1;
						pBuffData.Type = 0;
						pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
						pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs;
						pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];
						//Handle Buff Time List
						int FreePlace = 0;
						for (int i = 0; i <= 32; i++)
						{
						if (PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == pBuffData.tblidx)
						{
						sGU_BUFF_DROPPED dropbuff;
						dropbuff.wOpCode = GU_BUFF_DROPPED;
						dropbuff.wPacketSize = sizeof(sGU_BUFF_DROPPED) - 2;
						dropbuff.hHandle = PlayerInfo->GetHandle();
						dropbuff.bySourceType = eDBO_OBJECT_SOURCE::DBO_OBJECT_SOURCE_SKILL;
						dropbuff.Slot = 0;
						dropbuff.tblidx = pBuffData.tblidx;
						dropbuff.unk1 = 0;
						PlayerInfo->SendPacket((char*)&dropbuff, sizeof(sGU_BUFF_DROPPED));
						PlayerInfo->SendToPlayerList((char*)&dropbuff, sizeof(sGU_BUFF_DROPPED));
						PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffIsActive = false;
						PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffEndTime = INVALID_TBLIDX;
						PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffTime = INVALID_TBLIDX;
						PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID = INVALID_TBLIDX;
						PlayerInfo->ExecuteEffectCalculation(pBuffData.tblidx, true);
						printf("Alardy got that buff \n");
						FreePlace = i;
						}
						//GetFreeSlot
						else if (PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
						{
						printf("Regist new buff \n");
						FreePlace = i;
						}

						}
						PlayerInfo->ExecuteEffectCalculation(pBuffData.tblidx, false);

						PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
						PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
						PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
						PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
						PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
						PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffSlot = 0;
						}
						}
						}*/

						break;
					}
					case ACTIVE_STONE:
					{
						sSkil.wResultCode = GAME_SUCCESS;

						skillRes.skillId = skillDataOriginal->tblidx;
						skillRes.wResultCode = GAME_SUCCESS;
						skillRes.byRpBonusType = 0;//Untested
						skillRes.wOpCode = GU_CHAR_ACTION_SKILL;
						skillRes.handle = GetHandle();//My Handle
						skillRes.hAppointedTarget = GetHandle();

						for (int i = 0; i < skillDataOriginal->byApply_Target_Max; i++)
						{
							//	printf("ahApplyTarget %d \n", pCharSkillReq->ahApplyTarget[i]);
							if (ObjectType == OBJTYPE_PC)
							{
								Player* PlayerInfo = static_cast<Player*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (PlayerInfo != NULL  && PlayerInfo->GetIsDead() == false)
								{
									if (PlayerInfo->GetAttributesManager()->IsinPVP == true || PlayerInfo->GetAttributesManager()->PlayerInFreeBatle == true)
									{
										if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
										{
											pBuffData.hHandle = PlayerInfo->GetHandle();
											pBuffData.tblidx = skillDataOriginal->tblidx;
											pBuffData.bySourceType = 0;
											pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs;
											pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs;//Time

											pBuffData.isactive = 1;
											pBuffData.Type = 0;
											pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
											pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs;
											pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];

											int FreePlace = 0;
											for (int i = 0; i <= 32; i++)
											{
												if (PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
												{
													FreePlace = i;
												}

											}
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
											PlayerInfo->SetState(eCHARSTATE::CHARSTATE_STUNNED);
											sGU_UPDATE_CHAR_STATE state;

											state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
											state.wOpCode = GU_UPDATE_CHAR_STATE;

											state.handle = PlayerInfo->GetHandle();
											state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_STUNNED;
											state.sCharState.sCharStateDetail.sCharStateStunned.byStunType = eDBO_STUN_TYPE::DBO_STUN_TYPE_STONE;

											state.sCharState.sCharStateBase.vCurLoc.x = PlayerInfo->GetVectorPosition().x;
											state.sCharState.sCharStateBase.vCurLoc.y = PlayerInfo->GetVectorPosition().y;
											state.sCharState.sCharStateBase.vCurLoc.z = PlayerInfo->GetVectorPosition().z;
											state.sCharState.sCharStateBase.vCurDir.x = PlayerInfo->GetVectorOriantation().x;
											state.sCharState.sCharStateBase.vCurDir.y = PlayerInfo->GetVectorOriantation().y;
											state.sCharState.sCharStateBase.vCurDir.z = PlayerInfo->GetVectorOriantation().x;
											state.sCharState.sCharStateBase.dwConditionFlag = 0;
											//	res.sCharState.sCharStateBase.bFightMode = false;
											state.sCharState.sCharStateBase.dwStateTime = 2;

											sWorld.SendToAll((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
											//PlayerInfo->SendToPlayerList((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
										}
										count = 1;
										//	printf("count %d \n", count);
									}
								}
							}
							if (ObjectType == OBJTYPE_MOB)
							{
								Mob* MobInfo = static_cast<Mob*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (MobInfo != NULL && MobInfo->GetIsDead() == false)
								{
									if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
									{
										pBuffData.hHandle = MobInfo->GetHandle();

										pBuffData.tblidx = skillDataOriginal->tblidx;
										pBuffData.bySourceType = 0;
										pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs;
										pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs;//Time

										pBuffData.isactive = 1;
										pBuffData.Type = 0;
										pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
										pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs;
										pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];

										int FreePlace = 0;
										for (int i = 0; i <= 32; i++)
										{
											if (GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
											{
												FreePlace = i;
											}

										}
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
										//if (MobInfo->attackers == 0)
										MobInfo->attackers = GetHandle();
										sGU_UPDATE_CHAR_STATE state;

										state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
										state.wOpCode = GU_UPDATE_CHAR_STATE;

										state.handle = MobInfo->GetHandle();
										state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_STUNNED;
										state.sCharState.sCharStateDetail.sCharStateStunned.byStunType = eDBO_STUN_TYPE::DBO_STUN_TYPE_STONE;

										MobInfo->SetState(eCHARSTATE::CHARSTATE_STUNNED);
										state.sCharState.sCharStateBase.vCurLoc.x = MobInfo->GetVectorPosition().x;
										state.sCharState.sCharStateBase.vCurLoc.y = MobInfo->GetVectorPosition().y;
										state.sCharState.sCharStateBase.vCurLoc.z = MobInfo->GetVectorPosition().z;
										state.sCharState.sCharStateBase.vCurDir.x = MobInfo->GetVectorOriantation().x;
										state.sCharState.sCharStateBase.vCurDir.y = MobInfo->GetVectorOriantation().y;
										state.sCharState.sCharStateBase.vCurDir.z = MobInfo->GetVectorOriantation().x;
										state.sCharState.sCharStateBase.dwConditionFlag = 0;
										//	res.sCharState.sCharStateBase.bFightMode = false;
										state.sCharState.sCharStateBase.dwStateTime = 2;

										sWorld.SendToAll((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
										// SendToPlayerList((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));

									}
									count += 1;
									//	printf("count %d \n", count);
								}
							}
						}
						break;
					}
					case ACTIVE_CANDY:
					{
						sSkil.wResultCode = GAME_SUCCESS;

						skillRes.skillId = skillDataOriginal->tblidx;
						skillRes.wResultCode = GAME_SUCCESS;
						skillRes.byRpBonusType = 0;//Untested
						skillRes.wOpCode = GU_CHAR_ACTION_SKILL;
						skillRes.handle = GetHandle();//My Handle
						skillRes.hAppointedTarget = GetHandle();

						for (int i = 0; i < skillDataOriginal->byApply_Target_Max; i++)
						{
							//	printf("ahApplyTarget %d \n", pCharSkillReq->ahApplyTarget[i]);
							if (ObjectType == OBJTYPE_PC)
							{
								Player* PlayerInfo = static_cast<Player*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (PlayerInfo != NULL  && PlayerInfo->GetIsDead() == false)
								{
									if (PlayerInfo->GetAttributesManager()->IsinPVP == true || PlayerInfo->GetAttributesManager()->PlayerInFreeBatle == true)
									{
										if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
										{
											pBuffData.hHandle = PlayerInfo->GetHandle();
											pBuffData.tblidx = skillDataOriginal->tblidx;
											pBuffData.bySourceType = 0;
											pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs;
											pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs;//Time

											pBuffData.isactive = 1;
											pBuffData.Type = 0;
											pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
											pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs;
											pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];

											int FreePlace = 0;
											for (int i = 0; i <= 32; i++)
											{
												if (PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
												{
													FreePlace = i;
												}

											}
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
											PlayerInfo->SetState(eCHARSTATE::CHARSTATE_STUNNED);
											sGU_UPDATE_CHAR_STATE state;

											state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
											state.wOpCode = GU_UPDATE_CHAR_STATE;

											state.handle = PlayerInfo->GetHandle();
											state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_STUNNED;
											state.sCharState.sCharStateDetail.sCharStateStunned.byStunType = eDBO_STUN_TYPE::DBO_STUN_TYPE_CANDY;

											state.sCharState.sCharStateBase.vCurLoc.x = PlayerInfo->GetVectorPosition().x;
											state.sCharState.sCharStateBase.vCurLoc.y = PlayerInfo->GetVectorPosition().y;
											state.sCharState.sCharStateBase.vCurLoc.z = PlayerInfo->GetVectorPosition().z;
											state.sCharState.sCharStateBase.vCurDir.x = PlayerInfo->GetVectorOriantation().x;
											state.sCharState.sCharStateBase.vCurDir.y = PlayerInfo->GetVectorOriantation().y;
											state.sCharState.sCharStateBase.vCurDir.z = PlayerInfo->GetVectorOriantation().x;
											state.sCharState.sCharStateBase.dwConditionFlag = 0;
											//	res.sCharState.sCharStateBase.bFightMode = false;
											state.sCharState.sCharStateBase.dwStateTime = 2;

											sWorld.SendToAll((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
											//PlayerInfo->SendToPlayerList((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
										}
										count = 1;
										//	printf("count %d \n", count);
									}
								}
							}
							if (ObjectType == OBJTYPE_MOB)
							{
								Mob* MobInfo = static_cast<Mob*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (MobInfo != NULL && MobInfo->GetIsDead() == false)
								{
									if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
									{
										pBuffData.hHandle = MobInfo->GetHandle();

										pBuffData.tblidx = skillDataOriginal->tblidx;
										pBuffData.bySourceType = 0;
										pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs;
										pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs;//Time

										pBuffData.isactive = 1;
										pBuffData.Type = 0;
										pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
										pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs;
										pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];

										int FreePlace = 0;
										for (int i = 0; i <= 32; i++)
										{
											if (GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
											{
												FreePlace = i;
											}

										}
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
										//if (MobInfo->attackers == 0)
										MobInfo->attackers = GetHandle();
										sGU_UPDATE_CHAR_STATE state;

										state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
										state.wOpCode = GU_UPDATE_CHAR_STATE;

										state.handle = MobInfo->GetHandle();
										state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_STUNNED;
										state.sCharState.sCharStateDetail.sCharStateStunned.byStunType = eDBO_STUN_TYPE::DBO_STUN_TYPE_CANDY;

										MobInfo->SetState(eCHARSTATE::CHARSTATE_STUNNED);
										state.sCharState.sCharStateBase.vCurLoc.x = MobInfo->GetVectorPosition().x;
										state.sCharState.sCharStateBase.vCurLoc.y = MobInfo->GetVectorPosition().y;
										state.sCharState.sCharStateBase.vCurLoc.z = MobInfo->GetVectorPosition().z;
										state.sCharState.sCharStateBase.vCurDir.x = MobInfo->GetVectorOriantation().x;
										state.sCharState.sCharStateBase.vCurDir.y = MobInfo->GetVectorOriantation().y;
										state.sCharState.sCharStateBase.vCurDir.z = MobInfo->GetVectorOriantation().x;
										state.sCharState.sCharStateBase.dwConditionFlag = 0;
										//	res.sCharState.sCharStateBase.bFightMode = false;
										state.sCharState.sCharStateBase.dwStateTime = 2;

										sWorld.SendToAll((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
										// SendToPlayerList((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));

									}
									count += 1;
									//	printf("count %d \n", count);
								}
							}
						}
						break;
					}
					case ACTIVE_SLEEP:
					{
						sSkil.wResultCode = GAME_SUCCESS;

						skillRes.skillId = skillDataOriginal->tblidx;
						skillRes.wResultCode = GAME_SUCCESS;
						skillRes.byRpBonusType = 0;//Untested
						skillRes.wOpCode = GU_CHAR_ACTION_SKILL;
						skillRes.handle = GetHandle();//My Handle
						skillRes.hAppointedTarget = GetHandle();

						for (int i = 0; i < skillDataOriginal->byApply_Target_Max; i++)
						{
							//	printf("ahApplyTarget %d \n", pCharSkillReq->ahApplyTarget[i]);
							if (ObjectType == OBJTYPE_PC)
							{
								Player* PlayerInfo = static_cast<Player*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (PlayerInfo != NULL  && PlayerInfo->GetIsDead() == false)
								{
									if (PlayerInfo->GetAttributesManager()->IsinPVP == true || PlayerInfo->GetAttributesManager()->PlayerInFreeBatle == true)
									{
										if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
										{
											pBuffData.hHandle = PlayerInfo->GetHandle();
											pBuffData.tblidx = skillDataOriginal->tblidx;
											pBuffData.bySourceType = 0;
											pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs;
											pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs;//Time

											pBuffData.isactive = 1;
											pBuffData.Type = 0;
											pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
											pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs;
											pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];

											int FreePlace = 0;
											for (int i = 0; i <= 32; i++)
											{
												if (PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
												{
													FreePlace = i;
												}

											}
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
											PlayerInfo->SetState(eCHARSTATE::CHARSTATE_SLEEPING);
											sGU_UPDATE_CHAR_STATE state;

											state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
											state.wOpCode = GU_UPDATE_CHAR_STATE;

											state.handle = PlayerInfo->GetHandle();
											state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_SLEEPING;


											state.sCharState.sCharStateBase.vCurLoc.x = PlayerInfo->GetVectorPosition().x;
											state.sCharState.sCharStateBase.vCurLoc.y = PlayerInfo->GetVectorPosition().y;
											state.sCharState.sCharStateBase.vCurLoc.z = PlayerInfo->GetVectorPosition().z;
											state.sCharState.sCharStateBase.vCurDir.x = PlayerInfo->GetVectorOriantation().x;
											state.sCharState.sCharStateBase.vCurDir.y = PlayerInfo->GetVectorOriantation().y;
											state.sCharState.sCharStateBase.vCurDir.z = PlayerInfo->GetVectorOriantation().x;
											state.sCharState.sCharStateBase.dwConditionFlag = 0;
											//	res.sCharState.sCharStateBase.bFightMode = false;
											state.sCharState.sCharStateBase.dwStateTime = 2;

											sWorld.SendToAll((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
											//PlayerInfo->SendToPlayerList((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
										}
										count = 1;
										//	printf("count %d \n", count);
									}
								}
							}
							if (ObjectType == OBJTYPE_MOB)
							{
								Mob* MobInfo = static_cast<Mob*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (MobInfo != NULL && MobInfo->GetIsDead() == false)
								{
									if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
									{
										pBuffData.hHandle = MobInfo->GetHandle();

										pBuffData.tblidx = skillDataOriginal->tblidx;
										pBuffData.bySourceType = 0;
										pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs;
										pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs;//Time

										pBuffData.isactive = 1;
										pBuffData.Type = 0;
										pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
										pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs;
										pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];

										int FreePlace = 0;
										for (int i = 0; i <= 32; i++)
										{
											if (GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
											{
												FreePlace = i;
											}

										}
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
										//if (MobInfo->attackers == 0)
										MobInfo->attackers = GetHandle();
										sGU_UPDATE_CHAR_STATE state;

										state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
										state.wOpCode = GU_UPDATE_CHAR_STATE;

										state.handle = MobInfo->GetHandle();
										state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_SLEEPING;

										MobInfo->SetState(eCHARSTATE::CHARSTATE_SLEEPING);
										state.sCharState.sCharStateBase.vCurLoc.x = MobInfo->GetVectorPosition().x;
										state.sCharState.sCharStateBase.vCurLoc.y = MobInfo->GetVectorPosition().y;
										state.sCharState.sCharStateBase.vCurLoc.z = MobInfo->GetVectorPosition().z;
										state.sCharState.sCharStateBase.vCurDir.x = MobInfo->GetVectorOriantation().x;
										state.sCharState.sCharStateBase.vCurDir.y = MobInfo->GetVectorOriantation().y;
										state.sCharState.sCharStateBase.vCurDir.z = MobInfo->GetVectorOriantation().x;
										state.sCharState.sCharStateBase.dwConditionFlag = 0;
										//	res.sCharState.sCharStateBase.bFightMode = false;
										state.sCharState.sCharStateBase.dwStateTime = 2;

										sWorld.SendToAll((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
										// SendToPlayerList((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));

									}
									count += 1;
									//	printf("count %d \n", count);
								}
							}
						}
						break;
					}
					case ACTIVE_PARALYZE:
					{
						sSkil.wResultCode = GAME_SUCCESS;

						skillRes.skillId = skillDataOriginal->tblidx;
						skillRes.wResultCode = GAME_SUCCESS;
						skillRes.byRpBonusType = 0;//Untested
						skillRes.wOpCode = GU_CHAR_ACTION_SKILL;
						skillRes.handle = GetHandle();//My Handle
						skillRes.hAppointedTarget = GetHandle();

						for (int i = 0; i < skillDataOriginal->byApply_Target_Max; i++)
						{
							//	printf("ahApplyTarget %d \n", pCharSkillReq->ahApplyTarget[i]);
							if (ObjectType == OBJTYPE_PC)
							{
								Player* PlayerInfo = static_cast<Player*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (PlayerInfo != NULL  && PlayerInfo->GetIsDead() == false)
								{
									if (PlayerInfo->GetAttributesManager()->IsinPVP == true || PlayerInfo->GetAttributesManager()->PlayerInFreeBatle == true)
									{
										if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
										{
											pBuffData.hHandle = PlayerInfo->GetHandle();
											pBuffData.tblidx = skillDataOriginal->tblidx;
											pBuffData.bySourceType = 0;
											pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs;
											pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs;//Time

											pBuffData.isactive = 1;
											pBuffData.Type = 0;
											pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
											pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs;
											pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];

											int FreePlace = 0;
											for (int i = 0; i <= 32; i++)
											{
												if (PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
												{
													FreePlace = i;
												}

											}
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
											PlayerInfo->SetState(eCHARSTATE::CHARSTATE_PARALYZED);
											sGU_UPDATE_CHAR_STATE state;

											state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
											state.wOpCode = GU_UPDATE_CHAR_STATE;

											state.handle = PlayerInfo->GetHandle();
											state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_PARALYZED;


											state.sCharState.sCharStateBase.vCurLoc.x = PlayerInfo->GetVectorPosition().x;
											state.sCharState.sCharStateBase.vCurLoc.y = PlayerInfo->GetVectorPosition().y;
											state.sCharState.sCharStateBase.vCurLoc.z = PlayerInfo->GetVectorPosition().z;
											state.sCharState.sCharStateBase.vCurDir.x = PlayerInfo->GetVectorOriantation().x;
											state.sCharState.sCharStateBase.vCurDir.y = PlayerInfo->GetVectorOriantation().y;
											state.sCharState.sCharStateBase.vCurDir.z = PlayerInfo->GetVectorOriantation().x;
											state.sCharState.sCharStateBase.dwConditionFlag = 0;
											//	res.sCharState.sCharStateBase.bFightMode = false;
											state.sCharState.sCharStateBase.dwStateTime = 2;

											sWorld.SendToAll((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
											//PlayerInfo->SendToPlayerList((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
										}
										count = 1;
										//	printf("count %d \n", count);
									}
								}
							}
							if (ObjectType == OBJTYPE_MOB)
							{
								Mob* MobInfo = static_cast<Mob*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (MobInfo != NULL && MobInfo->GetIsDead() == false)
								{
									if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
									{
										pBuffData.hHandle = MobInfo->GetHandle();

										pBuffData.tblidx = skillDataOriginal->tblidx;
										pBuffData.bySourceType = 0;
										pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs;
										pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs;//Time

										pBuffData.isactive = 1;
										pBuffData.Type = 0;
										pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
										pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs;
										pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];

										int FreePlace = 0;
										for (int i = 0; i <= 32; i++)
										{
											if (GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
											{
												FreePlace = i;
											}

										}
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
										//if (MobInfo->attackers == 0)
										MobInfo->attackers = GetHandle();
										sGU_UPDATE_CHAR_STATE state;

										state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
										state.wOpCode = GU_UPDATE_CHAR_STATE;

										state.handle = MobInfo->GetHandle();
										state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_PARALYZED;

										MobInfo->SetState(eCHARSTATE::CHARSTATE_PARALYZED);
										state.sCharState.sCharStateBase.vCurLoc.x = MobInfo->GetVectorPosition().x;
										state.sCharState.sCharStateBase.vCurLoc.y = MobInfo->GetVectorPosition().y;
										state.sCharState.sCharStateBase.vCurLoc.z = MobInfo->GetVectorPosition().z;
										state.sCharState.sCharStateBase.vCurDir.x = MobInfo->GetVectorOriantation().x;
										state.sCharState.sCharStateBase.vCurDir.y = MobInfo->GetVectorOriantation().y;
										state.sCharState.sCharStateBase.vCurDir.z = MobInfo->GetVectorOriantation().x;
										state.sCharState.sCharStateBase.dwConditionFlag = 0;
										//	res.sCharState.sCharStateBase.bFightMode = false;
										state.sCharState.sCharStateBase.dwStateTime = 2;

										sWorld.SendToAll((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
										// SendToPlayerList((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));

									}
									count += 1;
									//	printf("count %d \n", count);
								}
							}
						}
						break;
					}
					case ACTIVE_WARP_STUN:
					case ACTIVE_STUN:
					{
						sSkil.wResultCode = GAME_SUCCESS;

						skillRes.skillId = skillDataOriginal->tblidx;
						skillRes.wResultCode = GAME_SUCCESS;
						skillRes.byRpBonusType = 0;//Untested
						skillRes.wOpCode = GU_CHAR_ACTION_SKILL;
						skillRes.handle = GetHandle();//My Handle
						skillRes.hAppointedTarget = GetHandle();

						for (int i = 0; i < skillDataOriginal->byApply_Target_Max; i++)
						{
							//	printf("ahApplyTarget %d \n", pCharSkillReq->ahApplyTarget[i]);
							if (ObjectType == OBJTYPE_PC)
							{
								Player* PlayerInfo = static_cast<Player*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (PlayerInfo != NULL  && PlayerInfo->GetIsDead() == false)
								{
									if (PlayerInfo->GetAttributesManager()->IsinPVP == true || PlayerInfo->GetAttributesManager()->PlayerInFreeBatle == true)
									{
										if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
										{
											pBuffData.hHandle = PlayerInfo->GetHandle();

											pBuffData.tblidx = skillDataOriginal->tblidx;
											pBuffData.bySourceType = 0;
											pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs;
											pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs;//Time

											pBuffData.isactive = 1;
											pBuffData.Type = 0;
											pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
											pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs;
											pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];

											int FreePlace = 0;
											for (int i = 0; i <= 32; i++)
											{
												if (PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
												{
													FreePlace = i;
												}

											}
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffSlot = 0;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].StunedType = CHARSTATE_STUNNED;
											PlayerInfo->GetState()->sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_STUNNED;
											PlayerInfo->GetState()->sCharStateBase.dwConditionFlag = 0;
											PlayerInfo->GetState()->sCharStateDetail.sCharStateStunned.byStunType = eDBO_STUN_TYPE::DBO_STUN_TYPE_GENERAL;
											PlayerInfo->SetState(eCHARSTATE::CHARSTATE_STUNNED);
											sGU_UPDATE_CHAR_STATE state;

											state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
											state.wOpCode = GU_UPDATE_CHAR_STATE;

											state.handle = PlayerInfo->GetHandle();
											state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_STUNNED;
											state.sCharState.sCharStateDetail.sCharStateStunned.byStunType = eDBO_STUN_TYPE::DBO_STUN_TYPE_GENERAL;

											state.sCharState.sCharStateBase.vCurLoc.x = PlayerInfo->GetVectorPosition().x;
											state.sCharState.sCharStateBase.vCurLoc.y = PlayerInfo->GetVectorPosition().y;
											state.sCharState.sCharStateBase.vCurLoc.z = PlayerInfo->GetVectorPosition().z;
											state.sCharState.sCharStateBase.vCurDir.x = PlayerInfo->GetVectorOriantation().x;
											state.sCharState.sCharStateBase.vCurDir.y = PlayerInfo->GetVectorOriantation().y;
											state.sCharState.sCharStateBase.vCurDir.z = PlayerInfo->GetVectorOriantation().x;
											state.sCharState.sCharStateBase.dwConditionFlag = 0;
											//	res.sCharState.sCharStateBase.bFightMode = false;
											state.sCharState.sCharStateBase.dwStateTime = 2;

											sWorld.SendToAll((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
											//PlayerInfo->SendToPlayerList((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
										}
										count = 1;
										//printf("count %d \n", count);
									}
								}
							}
							if (ObjectType == OBJTYPE_MOB)
							{
								Mob* MobInfo = static_cast<Mob*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (MobInfo != NULL && MobInfo->GetIsDead() == false)
								{
									if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
									{
										pBuffData.hHandle = MobInfo->GetHandle();

										pBuffData.tblidx = skillDataOriginal->tblidx;
										pBuffData.bySourceType = 0;
										pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs;
										pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs;//Time

										pBuffData.isactive = 1;
										pBuffData.Type = 0;
										pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
										pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs;
										pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];

										int FreePlace = 0;
										for (int i = 0; i <= 32; i++)
										{
											if (GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
											{
												FreePlace = i;
											}

										}
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffSlot = 0;
										//if (MobInfo->attackers == 0)
										MobInfo->attackers = GetHandle();
										MobInfo->SetState(eCHARSTATE::CHARSTATE_STUNNED);
										sGU_UPDATE_CHAR_STATE state;

										state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
										state.wOpCode = GU_UPDATE_CHAR_STATE;

										state.handle = MobInfo->GetHandle();
										state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_STUNNED;


										state.sCharState.sCharStateBase.vCurLoc.x = MobInfo->GetVectorPosition().x;
										state.sCharState.sCharStateBase.vCurLoc.y = MobInfo->GetVectorPosition().y;
										state.sCharState.sCharStateBase.vCurLoc.z = MobInfo->GetVectorPosition().z;
										state.sCharState.sCharStateBase.vCurDir.x = MobInfo->GetVectorOriantation().x;
										state.sCharState.sCharStateBase.vCurDir.y = MobInfo->GetVectorOriantation().y;
										state.sCharState.sCharStateBase.vCurDir.z = MobInfo->GetVectorOriantation().x;
										state.sCharState.sCharStateBase.dwConditionFlag = 0;
										//	res.sCharState.sCharStateBase.bFightMode = false;
										state.sCharState.sCharStateBase.dwStateTime = 2;

										sWorld.SendToAll((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
										// SendToPlayerList((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));

									}
									count += 1;
									//printf("count %d \n", count);
								}
							}
						}
						break;
					}
					case ACTIVE_FREEZE:
					{
						sSkil.wResultCode = GAME_SUCCESS;

						skillRes.skillId = skillDataOriginal->tblidx;
						skillRes.wResultCode = GAME_SUCCESS;
						skillRes.byRpBonusType = 0;//Untested
						skillRes.wOpCode = GU_CHAR_ACTION_SKILL;
						skillRes.handle = GetHandle();//My Handle
						skillRes.hAppointedTarget = GetHandle();

						for (int i = 0; i < skillDataOriginal->byApply_Target_Max; i++)
						{
							//	printf("ahApplyTarget %d \n", pCharSkillReq->ahApplyTarget[i]);
							if (ObjectType == OBJTYPE_PC)
							{
								Player* PlayerInfo = static_cast<Player*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (PlayerInfo != NULL  && PlayerInfo->GetIsDead() == false)
								{
									if (PlayerInfo->GetAttributesManager()->IsinPVP == true || PlayerInfo->GetAttributesManager()->PlayerInFreeBatle == true)
									{
										if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
										{
											pBuffData.hHandle = PlayerInfo->GetHandle();
											pBuffData.tblidx = skillDataOriginal->tblidx;
											pBuffData.bySourceType = 0;
											pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs;
											pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs;//Time

											pBuffData.isactive = 1;
											pBuffData.Type = 0;
											pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
											pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs;
											pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];

											int FreePlace = 0;
											for (int i = 0; i <= 32; i++)
											{
												if (PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
												{
													FreePlace = i;
												}

											}
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
											PlayerInfo->SetState(eCHARSTATE::CHARSTATE_STUNNED);
											sGU_UPDATE_CHAR_STATE state;

											state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
											state.wOpCode = GU_UPDATE_CHAR_STATE;

											state.handle = PlayerInfo->GetHandle();
											state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_STUNNED;
											state.sCharState.sCharStateDetail.sCharStateStunned.byStunType = eDBO_STUN_TYPE::DBO_STUN_TYPE_FROZEN;

											state.sCharState.sCharStateBase.vCurLoc.x = PlayerInfo->GetVectorPosition().x;
											state.sCharState.sCharStateBase.vCurLoc.y = PlayerInfo->GetVectorPosition().y;
											state.sCharState.sCharStateBase.vCurLoc.z = PlayerInfo->GetVectorPosition().z;
											state.sCharState.sCharStateBase.vCurDir.x = PlayerInfo->GetVectorOriantation().x;
											state.sCharState.sCharStateBase.vCurDir.y = PlayerInfo->GetVectorOriantation().y;
											state.sCharState.sCharStateBase.vCurDir.z = PlayerInfo->GetVectorOriantation().x;
											state.sCharState.sCharStateBase.dwConditionFlag = 0;
											//	res.sCharState.sCharStateBase.bFightMode = false;
											state.sCharState.sCharStateBase.dwStateTime = 2;

											sWorld.SendToAll((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
											//PlayerInfo->SendToPlayerList((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
										}
										count = 1;
										//	printf("count %d \n", count);
									}
								}
							}
							if (ObjectType == OBJTYPE_MOB)
							{
								Mob* MobInfo = static_cast<Mob*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (MobInfo != NULL && MobInfo->GetIsDead() == false)
								{
									if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
									{
										pBuffData.hHandle = MobInfo->GetHandle();

										pBuffData.tblidx = skillDataOriginal->tblidx;
										pBuffData.bySourceType = 0;
										pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs;
										pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs;//Time

										pBuffData.isactive = 1;
										pBuffData.Type = 0;
										pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
										pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs;
										pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];

										int FreePlace = 0;
										for (int i = 0; i <= 32; i++)
										{
											if (GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
											{
												FreePlace = i;
											}

										}
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
										//if (MobInfo->attackers == 0)
										MobInfo->attackers = GetHandle();
										sGU_UPDATE_CHAR_STATE state;

										state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
										state.wOpCode = GU_UPDATE_CHAR_STATE;

										state.handle = MobInfo->GetHandle();
										state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_STUNNED;
										state.sCharState.sCharStateDetail.sCharStateStunned.byStunType = eDBO_STUN_TYPE::DBO_STUN_TYPE_FROZEN;

										MobInfo->SetState(eCHARSTATE::CHARSTATE_STUNNED);
										state.sCharState.sCharStateBase.vCurLoc.x = MobInfo->GetVectorPosition().x;
										state.sCharState.sCharStateBase.vCurLoc.y = MobInfo->GetVectorPosition().y;
										state.sCharState.sCharStateBase.vCurLoc.z = MobInfo->GetVectorPosition().z;
										state.sCharState.sCharStateBase.vCurDir.x = MobInfo->GetVectorOriantation().x;
										state.sCharState.sCharStateBase.vCurDir.y = MobInfo->GetVectorOriantation().y;
										state.sCharState.sCharStateBase.vCurDir.z = MobInfo->GetVectorOriantation().x;
										state.sCharState.sCharStateBase.dwConditionFlag = 0;
										//	res.sCharState.sCharStateBase.bFightMode = false;
										state.sCharState.sCharStateBase.dwStateTime = 2;

										sWorld.SendToAll((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
										// SendToPlayerList((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));

									}
									count += 1;
									//	printf("count %d \n", count);
								}
							}
						}
						break;
					}
					case ACTIVE_ENERGY_REFLECTION:
					case ACTIVE_FAKE_DEATH:
					case ACTIVE_BATTLE_INABILITY:
					case ACTIVE_SKILL_INABILITY:
					case ACTIVE_CONFUSE:
					case ACTIVE_TERROR:
						//case ACTIVE_MAX_LP_DOWN:
						//case ACTIVE_MAX_EP_DOWN:
						//case ACTIVE_MAX_RP_DOWN:
						//case ACTIVE_PHYSICAL_OFFENCE_DOWN:
						//case ACTIVE_ENERGY_OFFENCE_DOWN:
						//case ACTIVE_PHYSICAL_DEFENCE_DOWN:
						//case ACTIVE_ENERGY_DEFENCE_DOWN:
						//case ACTIVE_CON_DOWN:
					case ACTIVE_MOVE_SPEED_DOWN:
					{
						sSkil.wResultCode = GAME_SUCCESS;

						skillRes.skillId = skillDataOriginal->tblidx;
						skillRes.wResultCode = GAME_SUCCESS;
						skillRes.byRpBonusType = 0;//Untested
						skillRes.wOpCode = GU_CHAR_ACTION_SKILL;
						skillRes.handle = GetHandle();//My Handle
						skillRes.hAppointedTarget = GetHandle();

						for (int i = 0; i < skillDataOriginal->byApply_Target_Max; i++)
						{
							//	printf("ahApplyTarget %d \n", pCharSkillReq->ahApplyTarget[i]);
							if (ObjectType == OBJTYPE_PC)
							{
								Player* PlayerInfo = static_cast<Player*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (PlayerInfo != NULL  && PlayerInfo->GetIsDead() == false)
								{
									if (PlayerInfo->GetAttributesManager()->IsinPVP == true || PlayerInfo->GetAttributesManager()->PlayerInFreeBatle == true)
									{
										if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
										{
											pBuffData.hHandle = PlayerInfo->GetHandle();

											pBuffData.tblidx = skillDataOriginal->tblidx;
											pBuffData.bySourceType = 0;
											pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs;
											pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs;//Time

											pBuffData.isactive = 1;
											pBuffData.Type = 0;
											pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
											pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs;
											pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];

											int FreePlace = 0;
											for (int i = 0; i <= 32; i++)
											{
												if (PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || PlayerInfo->GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
												{
													FreePlace = i;
												}
											}
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
											PlayerInfo->GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffSlot = 0;
											PlayerInfo->ExecuteEffectPlayerCalculation(pBuffData.tblidx, pBuffData.hHandle, false);

										}
										count = 1;
										//printf("count %d \n", count);
									}
								}
							}
							if (ObjectType == OBJTYPE_MOB)
							{
								Mob* MobInfo = static_cast<Mob*>(GetFromList(pCharSkillReq->ahApplyTarget[i]));
								if (MobInfo != NULL && MobInfo->GetIsDead() == false)
								{
									if (AttackType[count] != eBATTLE_ATTACK_RESULT::BATTLE_ATTACK_RESULT_DODGE)
									{
										pBuffData.hHandle = MobInfo->GetHandle();

										pBuffData.tblidx = skillDataOriginal->tblidx;
										pBuffData.bySourceType = 0;
										pBuffData.dwInitialDuration = skillDataOriginal->dwKeepTimeInMilliSecs;
										pBuffData.dwTimeRemaining = skillDataOriginal->dwKeepTimeInMilliSecs;//Time

										pBuffData.isactive = 1;
										pBuffData.Type = 0;
										pBuffData.BuffInfo[Effect].SystemEffectValue = skillDataOriginal->SkillValue[Effect];
										pBuffData.BuffInfo[Effect].SystemEffectTime = skillDataOriginal->dwKeepTimeInMilliSecs;
										pBuffData.BuffInfo[Effect].dwSystemEffectValue = skillDataOriginal->SkillValue[Effect];

										int FreePlace = 0;
										for (int i = 0; i <= 32; i++)
										{
											if (GetAttributesManager()->sBuffTimeInfo[i].BuffID == 0 || GetAttributesManager()->sBuffTimeInfo[i].BuffID == INVALID_TBLIDX)
											{
												FreePlace = i;
											}

										}
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffID = pBuffData.tblidx;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffTime = GetTickCount();
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffEndTime = pBuffData.dwInitialDuration;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].PlayerHandle = pBuffData.hHandle;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffIsActive = true;
										GetAttributesManager()->sBuffTimeInfo[FreePlace].BuffSlot = 0;
										//if (MobInfo->attackers == 0)
										MobInfo->attackers = GetHandle();
										// moveSpeed = MobInfo->GetMobData().Run_Speed / 100 * skillDataOriginal->SkillValue[Effect];
										// ExecuteEffectPlayerCalculation(pBuffData.tblidx, pBuffData.hHandle, false);

									}
									count += 1;
									//printf("count %d \n", count);
								}
							}
						}
						break;
					}
					case ACTIVE_SUMMON:
					{
						sSkil.wResultCode = GAME_SUCCESS;

						skillRes.skillId = skillDataOriginal->tblidx;
						skillRes.wResultCode = GAME_SUCCESS;
						skillRes.byRpBonusType = 0;//Untested
						skillRes.wOpCode = GU_CHAR_ACTION_SKILL;
						skillRes.handle = GetHandle();//My Handle
						skillRes.hAppointedTarget = GetHandle();
						printf("SkillValue %d \n", skillDataOriginal->SkillValue[Effect]);

						break;
					}
					//NEXT CASE
					}
				}
			}

			SendPacket((char*)&skillRes, sizeof(sGU_CHAR_ACTION_SKILL));
			SendToPlayerList((char*)&skillRes, sizeof(sGU_CHAR_ACTION_SKILL));
			SendPacket((char*)&sSkil, sizeof(sGU_CHAR_SKILL_RES));
			if (pBuffData.hHandle != 0 && bleed == false || pBuffData.hHandle != INVALID_TBLIDX && bleed == false)
			{
				SendPacket((char*)&pBuffData, sizeof(BuffTypeSkill));
				SendToPlayerList((char*)&pBuffData, sizeof(BuffTypeSkill));
			}
		}
		else
		{
			sSkil.wResultCode = ResultCodes::GAME_CHAR_IS_WRONG_STATE;
			SendPacket((char*)&sSkil, sizeof(sGU_CHAR_SKILL_RES));
		}
	}
	else
	{
		sSkil.wResultCode = GAME_SKILL_CANT_CAST_NOW;
		SendPacket((char*)&sSkil, sizeof(sGU_CHAR_SKILL_RES));
	}	
}
void Player::BossEventUpdate()
{
	
		sGU_DYNAMIC_FIELD_COUNT_NFY BossCount;
		BossCount.wOpCode = GU_DYNAMIC_FIELD_COUNT_NFY;
		BossCount.wPacketSize = sizeof(sGU_DYNAMIC_FIELD_COUNT_NFY) - 2;

		BossCount.ValueMax = sWorld.BossEventMajinMaxCount;
		BossCount.ValueMin = sWorld.BossEventMajinCurCount;
		BossCount.ValueMax1 = 0;
		BossCount.ValueMin1 = 0;
		BossCount.unk = 0;		
	
	if (sWorld.BossEventMajinCurCount >= sWorld.BossEventMajinMaxCount)
	{
		BossCount.ValueMax1 = sWorld.BossEventMajinMaxCount;
		BossCount.ValueMin1 = 0;
		BossCount.unk = 1;

		sGU_DYNAMIC_FIELD_SYSTEM_BOSS_POSITION_RES BossSpawn;
		BossSpawn.wOpCode = GU_DYNAMIC_FIELD_SYSTEM_BOSS_POSITION_RES;
		BossSpawn.wPacketSize = sizeof(sGU_DYNAMIC_FIELD_SYSTEM_BOSS_POSITION_RES) - 2;

		BossSpawn.BossCount = 5;
		for (int i = 0; i <= BossSpawn.BossCount; i++)
		{			
			int randomlocx = rand() % 2000;
			int randomlocy = rand() % 400;
			int randomlocz = rand() % 2000;
				if (sWorld.BossIsSpawed == false)
				{
					MobTable * MobTable = sTBM.GetMobTable();
					sMOB_TBLDAT * pMOBTblData = reinterpret_cast<sMOB_TBLDAT*>(MobTable->FindData(15712101));
					if (pMOBTblData != NULL)
					{
						sWorld.BossIsSpawed = true;
						SpawnMOB spawnData;
						memset(&spawnData, 0, sizeof(SpawnMOB));

						spawnData.wOpCode = GU_OBJECT_CREATE;
						spawnData.wPacketSize = sizeof(SpawnMOB) - 2;

						spawnData.curEP = pMOBTblData->wBasic_EP;
						spawnData.curLP = pMOBTblData->wBasic_LP;
						spawnData.Handle = sWorld.AcquireSerialId();
						spawnData.Level = pMOBTblData->byLevel;
						spawnData.maxEP = pMOBTblData->wBasic_EP;
						spawnData.maxLP = pMOBTblData->wBasic_LP;
						spawnData.Size = 10;
						spawnData.Type = OBJTYPE_MOB;
						spawnData.Tblidx = 15712101;

						spawnData.fLastWalkingSpeed = 2;
						spawnData.fLastRunningSpeed = 2;
						spawnData.fLastAirgSpeed = 2;
						spawnData.fLastAirgDashSpeed = 2;
						spawnData.fLastAirgDashAccelSpeed = 2;

						spawnData.State.sCharStateBase.aspectState.sAspectStateBase.byAspectStateId = 255;
						spawnData.State.sCharStateBase.vCurLoc.x = randomlocx;
						spawnData.State.sCharStateBase.vCurLoc.y = randomlocy;
						spawnData.State.sCharStateBase.vCurLoc.z = randomlocz;
						spawnData.State.sCharStateBase.vCurDir.x = m_rotation.x + rand() % 5;
						spawnData.State.sCharStateBase.vCurDir.y = m_rotation.y;
						spawnData.State.sCharStateBase.vCurDir.z = m_rotation.z + rand() % 5;
						spawnData.State.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_SPAWNING;
						
						Mob* created_mob = new Mob;
						if (pMOBTblData)
						{
							if (created_mob->Create(pMOBTblData, spawnData) == true)
							{
								created_mob->GetMapRef().link(this->GetMap(), created_mob);
								//	printf("Mob ID %d inserted into map", 15712101);
							}
							else
								delete created_mob;
						}
						sWorld.Boss_Info[i].Handle = spawnData.Handle;
						sWorld.Boss_Info[i].vBossPos = spawnData.State.sCharStateBase.vCurLoc;
					}

				}				
				BossSpawn.Boss_Info[i].Handle = sWorld.Boss_Info[i].Handle;
				BossSpawn.Boss_Info[i].vBossPos = sWorld.Boss_Info[i].vBossPos;
			}
		SendPacket((char*)&BossSpawn, sizeof(sGU_DYNAMIC_FIELD_SYSTEM_BOSS_POSITION_RES));
	}
	SendPacket((char*)&BossCount, sizeof(sGU_DYNAMIC_FIELD_COUNT_NFY));
}
void Player::SpawnMobByID(TBLIDX MobID)
{
	MobTable * MobTable = sTBM.GetMobTable();
	sMOB_TBLDAT * pMOBTblData = reinterpret_cast<sMOB_TBLDAT*>(MobTable->FindData(MobID));
	if (pMOBTblData != NULL)
	{
		SpawnMOB spawnData;
		memset(&spawnData, 0, sizeof(SpawnMOB));

		spawnData.wOpCode = GU_OBJECT_CREATE;
		spawnData.wPacketSize = sizeof(SpawnMOB) - 2;


		spawnData.curEP = pMOBTblData->wBasic_EP;
		spawnData.curLP = pMOBTblData->wBasic_LP;
		spawnData.Handle = sWorld.AcquireSerialId();
		spawnData.Level = pMOBTblData->byLevel;
		spawnData.maxEP = pMOBTblData->wBasic_EP;
		spawnData.maxLP = pMOBTblData->wBasic_LP;
		spawnData.Size = 10;
		spawnData.Type = OBJTYPE_MOB;
		spawnData.Tblidx = MobID;

		spawnData.fLastWalkingSpeed = 2;
		spawnData.fLastRunningSpeed = 2;
		spawnData.fLastAirgSpeed = 2;
		spawnData.fLastAirgDashSpeed = 2;
		spawnData.fLastAirgDashAccelSpeed = 2;

		spawnData.State.sCharStateBase.aspectState.sAspectStateBase.byAspectStateId = 255;
		spawnData.State.sCharStateBase.vCurLoc = GetVectorPosition();
		spawnData.State.sCharStateBase.vCurDir = GetVectorOriantation();	
		
		spawnData.State.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_SPAWNING;

		//	sWorld.SendToAll((char*)&spawnData, sizeof(SpawnMOB));
		//Need Insert In list
		Mob* created_mob = new Mob;
		if (pMOBTblData)
		{
			if (created_mob->Create(pMOBTblData, spawnData) == true)
			{
				created_mob->GetMapRef().link(this->GetMap(), created_mob);
				//printf("Mob ID %d inserted into map", MobID);
			}
			else
				delete created_mob;

		}
	}
}
void Player::NetPYUpdate()
{
	sGU_UPDATE_CHAR_NETP NetPyUp;
	DWORD netpy = 1;
	
	GetPcProfile()->sLocalize.netp += netpy;
	NetPyUp.wOpCode = GU_UPDATE_CHAR_NETP;
	NetPyUp.wPacketSize = sizeof(sGU_UPDATE_CHAR_NETP) - 2;
	NetPyUp.wResultCode = GAME_SUCCESS;
	NetPyUp.dwBonusNetP = netpy;
	NetPyUp.netP = GetPcProfile()->sLocalize.netp;
	NetPyUp.timeNextGainTime = 60;
	NetPyUp.dwAccumulationNetP = NetPyAcumulate + netpy;
	NetPyAcumulate = NetPyUp.dwAccumulationNetP;
	SendPacket((char*)&NetPyUp, sizeof(sGU_UPDATE_CHAR_NETP));
	
}
void Player::PowerUpUpdate()
{
	if (GetAttributesManager()->IsPowerUp == true)
	{
		
			GetPcProfile()->wCurRP += 2;
			sGU_UPDATE_CHAR_RP RP;

			RP.bHitDelay = false;
			RP.handle = GetHandle();
			RP.wCurRP = GetPcProfile()->wCurRP;
			RP.wMaxRP = GetPcProfile()->avatarAttribute.wBaseMaxRP;
			RP.wOpCode = GU_UPDATE_CHAR_RP;
			RP.wPacketSize = sizeof(sGU_UPDATE_CHAR_RP) - 2;
			SendPacket((char*)&RP, sizeof(sGU_UPDATE_CHAR_RP));
			SendToPlayerList((char*)&RP, sizeof(sGU_UPDATE_CHAR_RP));

		
		if (GetPcProfile()->wCurRP >= GetPcProfile()->avatarAttribute.wBaseMaxRP)
		{
			GetAttributesManager()->SetNumFilledRpBall(1);
			sGU_UPDATE_CHAR_RP_BALL newBall;
			newBall.bDropByTime = false;
			newBall.byCurRPBall = GetAttributesManager()->GetNumFilledRpBall();
			newBall.handle = GetHandle();
			newBall.wOpCode = GU_UPDATE_CHAR_RP_BALL;
			newBall.wPacketSize = sizeof(sGU_UPDATE_CHAR_RP_BALL) - 2;
			SendPacket((char*)&newBall, sizeof(sGU_UPDATE_CHAR_RP_BALL));
			SendToPlayerList((char*)&newBall, sizeof(sGU_UPDATE_CHAR_RP_BALL));
			GetPcProfile()->wCurRP = 0;
		}
	}
}
void Player::CheckPVPArea()
{	
	//printf("Plat Check \n");	
	if (GetVectorPosition().x <= 5792.100 && GetVectorPosition().x >= 5752.100
		&& GetVectorPosition().z >= 748.037 && GetVectorPosition().z <= 787.037 && GetAttributesManager()->IsinPVP == false)
	{
		//	printf("Plat in \n");
		sGU_WORLD_FREE_PVP_ZONE_ENTERED_NFY PVPZone;
		PVPZone.wOpCode = GU_WORLD_FREE_PVP_ZONE_ENTERED_NFY;
		PVPZone.wPacketSize = sizeof(sGU_WORLD_FREE_PVP_ZONE_ENTERED_NFY) - 2;
		PVPZone.Handle = GetHandle();
		SendPacket((char*)&PVPZone, sizeof(sGU_WORLD_FREE_PVP_ZONE_ENTERED_NFY));
		SendToPlayerList((char*)&PVPZone, sizeof(sGU_WORLD_FREE_PVP_ZONE_ENTERED_NFY));
		GetAttributesManager()->IsinPVP = true;
		mutexPlayer.lock();
		for (auto it = objList.begin(); it != objList.end();)
		{
			if (it->second != NULL)
			{
				if (it->second->GetTypeId() == OBJTYPE_PC)
				{
					Player* plr = static_cast<Player*>(it->second);
					if (plr->IsInWorld() == true && plr->GetSession() != NULL)
					{
						if (plr->GetAttributesManager()->IsinPVP == true)
						{
							sGU_WORLD_FREE_PVP_ZONE_ENTERED_NFY PVPZone;
							PVPZone.wOpCode = GU_WORLD_FREE_PVP_ZONE_ENTERED_NFY;
							PVPZone.wPacketSize = sizeof(sGU_WORLD_FREE_PVP_ZONE_ENTERED_NFY) - 2;
							PVPZone.Handle = plr->GetHandle();
							SendPacket((char*)&PVPZone, sizeof(sGU_WORLD_FREE_PVP_ZONE_ENTERED_NFY));
						}
						else
						{
							sGU_WORLD_FREE_PVP_ZONE_LEFT_NFY PVPZone;
							PVPZone.wOpCode = GU_WORLD_FREE_PVP_ZONE_LEFT_NFY;
							PVPZone.wPacketSize = sizeof(sGU_WORLD_FREE_PVP_ZONE_LEFT_NFY) - 2;
							PVPZone.Handle = plr->GetHandle();
							SendPacket((char*)&PVPZone, sizeof(sGU_WORLD_FREE_PVP_ZONE_LEFT_NFY));
						}
					}
				}
			}
			it++;
		}
		mutexPlayer.unlock();
	}
	if (GetVectorPosition().x >= 5793.100 && GetAttributesManager()->IsinPVP == true
		|| GetVectorPosition().x <= 5751.100 && GetAttributesManager()->IsinPVP == true
		|| GetVectorPosition().z <= 746.037 && GetAttributesManager()->IsinPVP == true
		|| GetVectorPosition().z >= 788.037 && GetAttributesManager()->IsinPVP == true)
	{
		//	printf("Plat out \n");
		sGU_WORLD_FREE_PVP_ZONE_LEFT_NFY PVPZone;
		PVPZone.wOpCode = GU_WORLD_FREE_PVP_ZONE_LEFT_NFY;
		PVPZone.wPacketSize = sizeof(sGU_WORLD_FREE_PVP_ZONE_LEFT_NFY) - 2;
		PVPZone.Handle = GetHandle();
		SendPacket((char*)&PVPZone, sizeof(sGU_WORLD_FREE_PVP_ZONE_LEFT_NFY));
		SendToPlayerList((char*)&PVPZone, sizeof(sGU_WORLD_FREE_PVP_ZONE_LEFT_NFY));
		GetAttributesManager()->IsinPVP = false;
		mutexPlayer.lock();
		for (auto it = objList.begin(); it != objList.end();)
		{
			if (it->second != NULL)
			{
				if (it->second->GetTypeId() == OBJTYPE_PC)
				{
					Player* plr = static_cast<Player*>(it->second);
					if (plr->IsInWorld() == true && plr->GetSession() != NULL)
					{
						if (plr->GetAttributesManager()->IsinPVP == true)
						{
							sGU_WORLD_FREE_PVP_ZONE_ENTERED_NFY PVPZone;
							PVPZone.wOpCode = GU_WORLD_FREE_PVP_ZONE_ENTERED_NFY;
							PVPZone.wPacketSize = sizeof(sGU_WORLD_FREE_PVP_ZONE_ENTERED_NFY) - 2;
							PVPZone.Handle = plr->GetHandle();
							SendPacket((char*)&PVPZone, sizeof(sGU_WORLD_FREE_PVP_ZONE_ENTERED_NFY));
						}
						else
						{
							sGU_WORLD_FREE_PVP_ZONE_LEFT_NFY PVPZone;
							PVPZone.wOpCode = GU_WORLD_FREE_PVP_ZONE_LEFT_NFY;
							PVPZone.wPacketSize = sizeof(sGU_WORLD_FREE_PVP_ZONE_LEFT_NFY) - 2;
							PVPZone.Handle = plr->GetHandle();
							SendPacket((char*)&PVPZone, sizeof(sGU_WORLD_FREE_PVP_ZONE_LEFT_NFY));
						}
					}
				}
			}
			it++;
		}
		mutexPlayer.unlock();
	}
}
void Player::TranformationRegen()
{
	DWORD TransformTime = GetTickCount() - TranformationRegTmmer;
	if (TransformTime >= 1200)
	{
		if (GetState()->sCharStateBase.aspectState.sAspectStateBase.byAspectStateId != eASPECTSTATE::ASPECTSTATE_INVALID)
		{
			int curLPP = GetPcProfile()->dwCurLP;
			int curEPP = GetPcProfile()->wCurEP;
			if (GetState()->sCharStateBase.aspectState.sAspectStateBase.byAspectStateId == eASPECTSTATE::ASPECTSTATE_KAIOKEN)
			{
				curLPP -= GetPcProfile()->avatarAttribute.wLastMaxLP / 100 * 2;
				curEPP -= GetPcProfile()->avatarAttribute.wLastMaxEP / 100 * 2;

				GetPcProfile()->dwCurLP = curLPP;
				sGU_UPDATE_CHAR_LP LPs;
				LPs.dwLpEpEventId = 0;
				LPs.handle = GetHandle();
				LPs.wCurLP = GetPcProfile()->dwCurLP;
				LPs.wMaxLP = GetPcProfile()->avatarAttribute.wLastMaxLP;
				LPs.wOpCode = GU_UPDATE_CHAR_LP;
				LPs.wPacketSize = sizeof(sGU_UPDATE_CHAR_LP) - 2;
				SendPacket((char*)&LPs, sizeof(sGU_UPDATE_CHAR_LP));
				SendToPlayerList((char*)&LPs, sizeof(sGU_UPDATE_CHAR_LP));
				if (curEPP <= 0)
					curEPP = 0;
				GetPcProfile()->wCurEP = curEPP;
				sGU_UPDATE_CHAR_EP EP;
				EP.dwLpEpEventId = 0;
				EP.handle = GetHandle();
				EP.wCurEP = GetPcProfile()->wCurEP;
				EP.wMaxEP = GetPcProfile()->avatarAttribute.wLastMaxEP;
				EP.wOpCode = GU_UPDATE_CHAR_EP;
				EP.wPacketSize = sizeof(sGU_UPDATE_CHAR_EP) - 2;
				SendPacket((char*)&EP, sizeof(sGU_UPDATE_CHAR_EP));
				SendToPlayerList((char*)&EP, sizeof(sGU_UPDATE_CHAR_EP));

				//Remove tranformation 
				int myLP = GetPcProfile()->avatarAttribute.wLastMaxLP / 100 * 25;
				if (GetPcProfile()->dwCurLP <= myLP || curLPP <= myLP)
				{
					curLPP = myLP;
					sGU_TRANSFORM_CANCEL_RES TransformCancel;

					TransformCancel.wOpCode = GU_TRANSFORM_CANCEL_RES;
					TransformCancel.wPacketSize = sizeof(sGU_TRANSFORM_CANCEL_RES) - 2;
					TransformCancel.wResultCode = GAME_SUCCESS;
					SendPacket((char*)&TransformCancel, sizeof(sGU_TRANSFORM_CANCEL_RES));

					GetState()->sCharStateBase.aspectState.sAspectStateBase.byAspectStateId = eASPECTSTATE::ASPECTSTATE_INVALID;
					GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.bIsEngineOn = false;
					GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.hVehicleItem = INVALID_TBLIDX;
					GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.idVehicleTblidx = INVALID_TBLIDX;
					UpdateAspectState(eASPECTSTATE::ASPECTSTATE_INVALID);
				}
			}
			//provisory
			if (GetState()->sCharStateBase.aspectState.sAspectStateBase.byAspectStateId != eASPECTSTATE::ASPECTSTATE_INVALID)
			{
				if (GetPcProfile()->dwCurLP <= 0 || curLPP <= 0)
				{
					sGU_TRANSFORM_CANCEL_RES TransformCancel;

					TransformCancel.wOpCode = GU_TRANSFORM_CANCEL_RES;
					TransformCancel.wPacketSize = sizeof(sGU_TRANSFORM_CANCEL_RES) - 2;
					TransformCancel.wResultCode = GAME_SUCCESS;
					SendPacket((char*)&TransformCancel, sizeof(sGU_TRANSFORM_CANCEL_RES));

					GetState()->sCharStateBase.aspectState.sAspectStateBase.byAspectStateId = eASPECTSTATE::ASPECTSTATE_INVALID;
					GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.bIsEngineOn = false;
					GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.hVehicleItem = INVALID_TBLIDX;
					GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.idVehicleTblidx = INVALID_TBLIDX;
					UpdateAspectState(eASPECTSTATE::ASPECTSTATE_INVALID);
				}

			}
			
		}
		TranformationRegTmmer = GetTickCount();
	}
}

//----------------------------------------
//	Trigger by Player Tick()
//	Function use to remove the dropped item each 30 seconds after it has benn dropped
//----------------------------------------
void Player::UpdateDropListTimer()
{
	mutexDropList.lock();
	for (auto it = DropList.begin(); it != DropList.end();)
	{
		if (it->second != NULL)
		{
			DWORD deletionAt = GetTickCount() - it->second->droppedTime;
			if (deletionAt >= 50000)
			{
				sGU_OBJECT_DESTROY destroy;
				destroy.handle = it->first;
				destroy.wOpCode = GU_OBJECT_DESTROY;
				destroy.wPacketSize = sizeof(sGU_OBJECT_DESTROY) - 2;
				SendPacket((char*)&destroy, sizeof(sGU_OBJECT_DESTROY));

				delete it->second;
				it = DropList.erase(it++);
			}
			else
				it++;
		}
	}
	mutexDropList.unlock();
}
//----------------------------------------
//	Trigger When we update our amount of money
//	amount of money to add (positive) or negative to remove
//	type of transaction
//----------------------------------------
void Player::UpdateZennyAmount(DWORD amount, eZENNY_CHANGE_TYPE zennyType)
{
	sGU_UPDATE_CHAR_ZENNY zenny;
	zenny.wOpCode = GU_UPDATE_CHAR_ZENNY;
	zenny.wPacketSize = sizeof(sGU_UPDATE_CHAR_ZENNY) - 2;

	zenny.bIsNew = true;
	zenny.byChangeType = zennyType;
	zenny.handle = GetHandle();
	zenny.dwZenny = GetPcProfile()->dwZenny += amount;

	SendPacket((char*)&zenny, sizeof(sGU_UPDATE_CHAR_ZENNY));
}
void Player::UpdateModusaAmount(DWORD amount)
{
	sGU_UPDATE_CHAR_MUDOSA_POINT Modusa;
	Modusa.wOpCode = GU_UPDATE_CHAR_MUDOSA_POINT;
	Modusa.wPacketSize = sizeof(sGU_UPDATE_CHAR_MUDOSA_POINT) - 2;

	Modusa.dwMudosaPoint = GetPcProfile()->dwMudosaPoint += amount;
	GetPcProfile()->dwMudosaPoint = Modusa.dwMudosaPoint;
	
	SendPacket((char*)&Modusa, sizeof(sGU_UPDATE_CHAR_MUDOSA_POINT));
}
//----------------------------------------
//	Check if we are low life or not
//----------------------------------------
void Player::CalculeLowLife()
{
	bool bEmergency = GetIsEmergency();

	if (isLowLifeSend == false && bEmergency == true)
	{
		isLowLifeSend = true;
		sGU_UPDATE_CHAR_LP_STATUS_NFY res;

		res.bEmergency = true;
		res.handle = GetHandle();
		res.wOpCode = GU_UPDATE_CHAR_LP_STATUS_NFY;
		res.wPacketSize = sizeof(sGU_UPDATE_CHAR_LP_STATUS_NFY) - 2;
		res.wResultCode = GAME_SUCCESS;

		SendPacket((char*)&res, sizeof(sGU_UPDATE_CHAR_LP_STATUS_NFY));
		SendToPlayerList((char*)&res, sizeof(sGU_UPDATE_CHAR_LP_STATUS_NFY));
	}
	else if (isLowLifeSend == true && bEmergency == false)
	{
		isLowLifeSend = false;
		sGU_UPDATE_CHAR_LP_STATUS_NFY res;

		res.bEmergency = false;
		res.handle = GetHandle();
		res.wOpCode = GU_UPDATE_CHAR_LP_STATUS_NFY;
		res.wPacketSize = sizeof(sGU_UPDATE_CHAR_LP_STATUS_NFY) - 2;
		res.wResultCode = GAME_SUCCESS;

		SendPacket((char*)&res, sizeof(sGU_UPDATE_CHAR_LP_STATUS_NFY));
		SendToPlayerList((char*)&res, sizeof(sGU_UPDATE_CHAR_LP_STATUS_NFY));
	}
}
//----------------------------------------
//	Regen Stat each tick
//----------------------------------------
void Player::Regen()
{
	///////////////////////////////
	//Reg LP					//	
	//////////////////////////////
	if (GetPcProfile()->dwCurLP < GetPcProfile()->avatarAttribute.wLastMaxLP && GetIsDead() == false)
	{
		int curLPP = GetPcProfile()->dwCurLP;
		//printf("Reg LP %d", GetPcProfile()->dwCurLP);
		
		if (isfighting == false && GetCharEState() != eCHARSTATE::CHARSTATE_SITTING)
		{
			curLPP += GetPcProfile()->avatarAttribute.wLastLpRegen;
		}			
		else if (GetCharEState() == eCHARSTATE::CHARSTATE_SITTING)
		{
			curLPP += GetPcProfile()->avatarAttribute.wLastLpSitdownRegen;
		}	
		if (GetPcProfile()->dwCurLP >= GetPcProfile()->avatarAttribute.wLastMaxLP || curLPP >= GetPcProfile()->avatarAttribute.wLastMaxLP)
		{
			curLPP = GetPcProfile()->avatarAttribute.wLastMaxLP;
			GetPcProfile()->dwCurLP = curLPP;
		}			
		GetPcProfile()->dwCurLP = curLPP;
		sGU_UPDATE_CHAR_LP LPs;
		LPs.dwLpEpEventId = 0;
		LPs.handle = GetHandle();
		LPs.wCurLP = GetPcProfile()->dwCurLP;
		LPs.wMaxLP = GetPcProfile()->avatarAttribute.wLastMaxLP;
		LPs.wOpCode = GU_UPDATE_CHAR_LP;
		LPs.wPacketSize = sizeof(sGU_UPDATE_CHAR_LP) - 2;
		SendPacket((char*)&LPs, sizeof(sGU_UPDATE_CHAR_LP));
		SendToPlayerList((char*)&LPs, sizeof(sGU_UPDATE_CHAR_LP));
	}
	if (GetPcProfile()->dwCurLP > GetPcProfile()->avatarAttribute.wLastMaxLP)
	{
		GetPcProfile()->dwCurLP = GetPcProfile()->avatarAttribute.wLastMaxLP;		
		sGU_UPDATE_CHAR_LP LPs;
		LPs.dwLpEpEventId = 0;
		LPs.handle = GetHandle();
		LPs.wCurLP = GetPcProfile()->dwCurLP;
		LPs.wMaxLP = GetPcProfile()->avatarAttribute.wLastMaxLP;
		LPs.wOpCode = GU_UPDATE_CHAR_LP;
		LPs.wPacketSize = sizeof(sGU_UPDATE_CHAR_LP) - 2;
		SendPacket((char*)&LPs, sizeof(sGU_UPDATE_CHAR_LP));
		SendToPlayerList((char*)&LPs, sizeof(sGU_UPDATE_CHAR_LP));
	}
	///////////////////////////////
	//Reg EP					//	
	//////////////////////////////
	if (GetPcProfile()->wCurEP < GetPcProfile()->avatarAttribute.wLastMaxEP && GetIsDead() == false)
	{
		int curEPP = GetPcProfile()->wCurEP;
		//	printf("Reg LP %d", GetPcProfile()->dwCurLP);

		if (isfighting == false && GetCharEState() != eCHARSTATE::CHARSTATE_SITTING)
		{
			curEPP += GetPcProfile()->avatarAttribute.wLastEpRegen;
		}
		else if (GetCharEState() == eCHARSTATE::CHARSTATE_SITTING)
		{
			curEPP += GetPcProfile()->avatarAttribute.wLastEpSitdownRegen;
		}
		if (GetPcProfile()->wCurEP >= GetPcProfile()->avatarAttribute.wLastMaxEP || curEPP >= GetPcProfile()->avatarAttribute.wLastMaxEP)
		{
			curEPP = GetPcProfile()->avatarAttribute.wLastMaxEP;
		}
		GetPcProfile()->wCurEP = curEPP;
		sGU_UPDATE_CHAR_EP EP;
		EP.dwLpEpEventId = 0;
		EP.handle = GetHandle();
		EP.wCurEP = GetPcProfile()->wCurEP;
		EP.wMaxEP = GetPcProfile()->avatarAttribute.wLastMaxEP;
		EP.wOpCode = GU_UPDATE_CHAR_EP;
		EP.wPacketSize = sizeof(sGU_UPDATE_CHAR_EP) - 2;
		SendPacket((char*)&EP, sizeof(sGU_UPDATE_CHAR_EP));
		SendToPlayerList((char*)&EP, sizeof(sGU_UPDATE_CHAR_EP));
	}
	if (GetPcProfile()->wCurEP > GetPcProfile()->avatarAttribute.wLastMaxEP)
	{
		GetPcProfile()->wCurEP = GetPcProfile()->avatarAttribute.wLastMaxEP;
		sGU_UPDATE_CHAR_EP EP;
		EP.dwLpEpEventId = 0;
		EP.handle = GetHandle();
		EP.wCurEP = GetPcProfile()->wCurEP;
		EP.wMaxEP = GetPcProfile()->avatarAttribute.wLastMaxEP;
		EP.wOpCode = GU_UPDATE_CHAR_EP;
		EP.wPacketSize = sizeof(sGU_UPDATE_CHAR_EP) - 2;
		SendPacket((char*)&EP, sizeof(sGU_UPDATE_CHAR_EP));
		SendToPlayerList((char*)&EP, sizeof(sGU_UPDATE_CHAR_EP));
	}
	///////////////////////////////
	//Reg AP					//	
	//////////////////////////////
	if (GetPcProfile()->dwCurAp < GetPcProfile()->avatarAttribute.wLastMaxAp && GetIsDead() == false && isFlying == false)
	{
		int curAPP = GetPcProfile()->dwCurAp;
		//	printf("Reg LP %d", GetPcProfile()->dwCurLP);
		curAPP += 5000;
		GetPcProfile()->dwCurAp = curAPP;

		sGU_UPDATE_CHAR_AP AP;
	
		AP.handle = GetHandle();
		AP.dwCurAp = GetPcProfile()->dwCurAp;
		AP.wLastMaxAp = GetPcProfile()->avatarAttribute.wLastMaxAp;
		AP.wBaseMaxAp = GetPcProfile()->avatarAttribute.wBaseMaxAp;
		AP.wOpCode = GU_UPDATE_CHAR_AP;
		AP.wPacketSize = sizeof(sGU_UPDATE_CHAR_AP) - 2;
		SendPacket((char*)&AP, sizeof(sGU_UPDATE_CHAR_AP));
		SendToPlayerList((char*)&AP, sizeof(sGU_UPDATE_CHAR_AP));
	}
	if (GetPcProfile()->dwCurAp > 0 && GetIsDead() == false && isFlying == true)
	{
		int curAPP = GetPcProfile()->dwCurAp;
		//	printf("Reg LP %d", GetPcProfile()->dwCurLP);	
		curAPP -= 5000;
		if (GetPcProfile()->dwCurAp <= 0 || curAPP <= 0)
		{		
			curAPP = 0;
			GetState()->sCharStateDetail.sCharStateFalling.dwTimeStamp = 0x01;
			GetState()->sCharStateDetail.sCharStateFalling.byMoveDirection = 1;
			SetState(eCHARSTATE::CHARSTATE_FALLING);
		}	
		GetPcProfile()->dwCurAp = curAPP;
		sGU_UPDATE_CHAR_AP AP;

		AP.handle = GetHandle();
		AP.dwCurAp = GetPcProfile()->dwCurAp;
		AP.wLastMaxAp = GetPcProfile()->avatarAttribute.wLastMaxAp;
		AP.wBaseMaxAp = GetPcProfile()->avatarAttribute.wBaseMaxAp;
		AP.wOpCode = GU_UPDATE_CHAR_AP;
		AP.wPacketSize = sizeof(sGU_UPDATE_CHAR_AP) - 2;
		SendPacket((char*)&AP, sizeof(sGU_UPDATE_CHAR_AP));
		SendToPlayerList((char*)&AP, sizeof(sGU_UPDATE_CHAR_AP));

	}
	if (GetPcProfile()->dwCurAp > GetPcProfile()->avatarAttribute.wLastMaxAp)
	{
		GetPcProfile()->dwCurAp = GetPcProfile()->avatarAttribute.wLastMaxAp;
		sGU_UPDATE_CHAR_AP AP;

		AP.handle = GetHandle();
		AP.dwCurAp = GetPcProfile()->dwCurAp;
		AP.wLastMaxAp = GetPcProfile()->avatarAttribute.wLastMaxAp;
		AP.wBaseMaxAp = GetPcProfile()->avatarAttribute.wBaseMaxAp;
		AP.wOpCode = GU_UPDATE_CHAR_AP;
		AP.wPacketSize = sizeof(sGU_UPDATE_CHAR_AP) - 2;
		SendPacket((char*)&AP, sizeof(sGU_UPDATE_CHAR_AP));
		SendToPlayerList((char*)&AP, sizeof(sGU_UPDATE_CHAR_AP));
	}	
}
//Need Delet all that Shit and Remake it only handle one buff
void Player::ExecuteLPFood()
{
	for (int i = 0; i <= 16; i++)
	{
		if (GetAttributesManager()->LpFoodIsActive == true && GetAttributesManager()->sFoodInfo[i].FoodItemID != 0 & GetAttributesManager()->sFoodInfo[i].FoodItemID != INVALID_TBLIDX)
		{
			sGU_EFFECT_AFFECTED afect;			
			afect.wOpCode = GU_EFFECT_AFFECTED;
			afect.wPacketSize = sizeof(sGU_EFFECT_AFFECTED) - 2;
			afect.handle = GetHandle();
			afect.hCaster = GetHandle();
			afect.effectTblidx = GetAttributesManager()->sFoodInfo[i].FoodItemID;
			afect.bysource = 1;
			afect.SyestemEffectCode = GetAttributesManager()->sFoodInfo[i].TypeEffect;
			afect.ResultCode = 500;
			afect.ActualArgument[0] = GetAttributesManager()->sFoodInfo[i].FoodHealAumont[0];
			afect.ActualArgument[1] = GetAttributesManager()->sFoodInfo[i].FoodHealAumont[1];

						//ACTIVE_HEAL_OVER_TIME;
			int newlp = GetPcProfile()->dwCurLP;
			if (newlp <= GetPcProfile()->avatarAttribute.wLastMaxLP)
			{
				newlp += GetAttributesManager()->sFoodInfo[i].FoodHealAumont[0];
			}
			if (newlp >= GetPcProfile()->avatarAttribute.wLastMaxLP)
			{
				newlp = GetPcProfile()->avatarAttribute.wLastMaxLP;
			}
			sGU_UPDATE_CHAR_LP LPs;
			LPs.wOpCode = GU_UPDATE_CHAR_LP;
			LPs.wPacketSize = sizeof(sGU_UPDATE_CHAR_LP) - 2;
			GetPcProfile()->dwCurLP = newlp;			
			LPs.dwLpEpEventId = 0;
			LPs.handle = GetHandle();
			LPs.wCurLP = GetPcProfile()->dwCurLP;
			LPs.wMaxLP = GetPcProfile()->avatarAttribute.wLastMaxLP;
			SendPacket((char*)&LPs, sizeof(sGU_UPDATE_CHAR_LP));
			SendToPlayerList((char*)&LPs, sizeof(sGU_UPDATE_CHAR_LP));
			SendPacket((char*)&afect, sizeof(sGU_EFFECT_AFFECTED));
			SendToPlayerList((char*)&afect, sizeof(sGU_EFFECT_AFFECTED));

			DWORD deletionAt1 = GetTickCount() - GetAttributesManager()->sFoodInfo[i].BuffTime;
			if (deletionAt1 >= GetAttributesManager()->sFoodInfo[i].BuffKeepTime || GetCharEState() != eCHARSTATE::CHARSTATE_SITTING)
			{
					sGU_BUFF_DROPPED dropbuff;
					dropbuff.wOpCode = GU_BUFF_DROPPED;
					dropbuff.wPacketSize = sizeof(sGU_BUFF_DROPPED) - 2;
					dropbuff.hHandle = GetHandle();
					dropbuff.bySourceType = eDBO_OBJECT_SOURCE::DBO_OBJECT_SOURCE_ITEM;
					dropbuff.Slot = 0;
					dropbuff.tblidx = GetAttributesManager()->sFoodInfo[i].FoodItemID;
					dropbuff.unk1 = 0;
					SendPacket((char*)&dropbuff, sizeof(sGU_BUFF_DROPPED));
					SendToPlayerList((char*)&dropbuff, sizeof(sGU_BUFF_DROPPED));					
					GetAttributesManager()->LpFoodIsActive = false;
					GetAttributesManager()->sFoodInfo[i].FoodItemID = INVALID_TBLIDX;
								
			}
		}
	}
	
}
void Player::ExecuteEPFood()
{
	for (int i = 0; i <= 16; i++)
	{
		if (GetAttributesManager()->EpFoodIsActive == true && GetAttributesManager()->sFoodInfo[i].FoodItemID != 0 & GetAttributesManager()->sFoodInfo[i].FoodItemID != INVALID_TBLIDX)
		{
			sGU_EFFECT_AFFECTED afect;
			afect.wOpCode = GU_EFFECT_AFFECTED;
			afect.wPacketSize = sizeof(sGU_EFFECT_AFFECTED) - 2;
			afect.handle = GetHandle();
			afect.hCaster = GetHandle();
			afect.effectTblidx = GetAttributesManager()->sFoodInfo[i].FoodItemID;
			afect.bysource = 1;
			afect.SyestemEffectCode = GetAttributesManager()->sFoodInfo[i].TypeEffect;
			afect.ResultCode = 500;
			afect.ActualArgument[0] = GetAttributesManager()->sFoodInfo[i].FoodHealAumont[0];
			afect.ActualArgument[1] = GetAttributesManager()->sFoodInfo[i].FoodHealAumont[1];			
			//ACTIVE_HEAL_OVER_TIME;

			sGU_UPDATE_CHAR_EP EPs;
			EPs.wOpCode = GU_UPDATE_CHAR_EP;
			EPs.wPacketSize = sizeof(sGU_UPDATE_CHAR_EP) - 2;

			int newEp = GetPcProfile()->wCurEP;
			if (newEp <= GetPcProfile()->avatarAttribute.wLastMaxEP)
			{
				newEp += GetAttributesManager()->sFoodInfo[i].FoodHealAumont[0];
			}
			if (newEp >= GetPcProfile()->avatarAttribute.wLastMaxEP)
			{
				newEp = GetPcProfile()->avatarAttribute.wLastMaxEP;
			}
			GetPcProfile()->wCurEP = newEp;
			
			EPs.dwLpEpEventId = 0;
			EPs.handle = GetHandle();
			EPs.wCurEP = GetPcProfile()->wCurEP;
			EPs.wMaxEP = GetPcProfile()->avatarAttribute.wLastMaxEP;

			SendPacket((char*)&EPs, sizeof(sGU_UPDATE_CHAR_EP));
			SendToPlayerList((char*)&EPs, sizeof(sGU_UPDATE_CHAR_EP));
			SendPacket((char*)&afect, sizeof(sGU_EFFECT_AFFECTED));
			SendToPlayerList((char*)&afect, sizeof(sGU_EFFECT_AFFECTED));

			DWORD deletionAt1 = GetTickCount() - GetAttributesManager()->sFoodInfo[i].BuffTime;
			if (deletionAt1 >= GetAttributesManager()->sFoodInfo[i].BuffKeepTime || GetCharEState() != eCHARSTATE::CHARSTATE_SITTING)
			{
				sGU_BUFF_DROPPED dropbuff;
				dropbuff.wOpCode = GU_BUFF_DROPPED;
				dropbuff.wPacketSize = sizeof(sGU_BUFF_DROPPED) - 2;
				dropbuff.hHandle = GetHandle();
				dropbuff.bySourceType = eDBO_OBJECT_SOURCE::DBO_OBJECT_SOURCE_ITEM;
				dropbuff.Slot = 0;
				dropbuff.tblidx = GetAttributesManager()->sFoodInfo[i].FoodItemID;
				dropbuff.unk1 = 0;
				SendPacket((char*)&dropbuff, sizeof(sGU_BUFF_DROPPED));
				SendToPlayerList((char*)&dropbuff, sizeof(sGU_BUFF_DROPPED));
				GetAttributesManager()->EpFoodIsActive = false;
				GetAttributesManager()->sFoodInfo[i].FoodItemID = INVALID_TBLIDX;
				GetAttributesManager()->sFoodInfo[i].BuffKeepTime = INVALID_TBLIDX;

			}
		}
	}

}
void Player::ExecuteEffectCalculation(TBLIDX SkillID, bool isRemove)
{
	SkillTable * skillTable = sTBM.GetSkillTable();
	sSKILL_TBLDAT * skillDataOriginal = NULL;
	skillDataOriginal = reinterpret_cast<sSKILL_TBLDAT*>(skillTable->FindData(SkillID));
	if (skillDataOriginal != NULL)
	{
		for (int Effect = 0; Effect <= 2; Effect++)
		{
			int count = 0;
			sSYSTEM_EFFECT_TBLDAT * SystemEffectData = NULL;
			SystemEffectData = (sSYSTEM_EFFECT_TBLDAT*)sTBM.GetSystemEffectTable()->FindData(skillDataOriginal->skill_Effect[Effect]);
			if (SystemEffectData != NULL)
			{
				//printf("skill_Effect %d \n", skillDataOriginal->skill_Effect[Effect]);
			//	printf("effectCode %d \n", SystemEffectData->effectCode);
			//	printf("Effect %d \n", Effect);
				switch (SystemEffectData->effectCode)
				{
				case ACTIVE_MAX_LP_UP:
				{
					if (isRemove == true)
						GetAttributesManager()->SetLastMaxLP(skillDataOriginal->SkillValue[Effect] * -1);
					else
						GetAttributesManager()->SetLastMaxLP(skillDataOriginal->SkillValue[Effect]);
					break;
				}
				case ACTIVE_MAX_EP_UP:
				{
					if (isRemove == true)
						GetAttributesManager()->SetLastMaxEP(skillDataOriginal->SkillValue[Effect] * -1);
					else
						GetAttributesManager()->SetLastMaxEP(skillDataOriginal->SkillValue[Effect]);
					break;
				}
				case ACTIVE_MAX_RP_UP:
				{

					if (isRemove == true)
						GetAttributesManager()->SetLastMaxRP(skillDataOriginal->SkillValue[Effect] * -1);
					else
						GetAttributesManager()->SetLastMaxRP(skillDataOriginal->SkillValue[Effect]);
					if (GetPcProfile()->avatarAttribute.wLastMaxRP <= 0 || GetPcProfile()->avatarAttribute.wLastMaxRP >= 60000)
						GetAttributesManager()->SetLastMaxRP(0);
					break;
				}
				case ACTIVE_PHYSICAL_OFFENCE_UP:
				{
					int PHyPercent = 0;
					if (skillDataOriginal->bySkill_Effect_Type[Effect] == 1)
						PHyPercent = GetPcProfile()->avatarAttribute.wBasePhysicalOffence / 100 * skillDataOriginal->SkillValue[Effect];
					else
						PHyPercent = skillDataOriginal->SkillValue[Effect];

					if (isRemove == true)
						GetAttributesManager()->SetLastPhysicalOffence(PHyPercent * -1);
					else
						GetAttributesManager()->SetLastPhysicalOffence(PHyPercent);
					if (GetPcProfile()->avatarAttribute.wLastPhysicalOffence <= 0 || GetPcProfile()->avatarAttribute.wLastPhysicalOffence >= 60000)
					{
						GetPcProfile()->avatarAttribute.wLastPhysicalOffence = 0;
						GetAttributesManager()->SetLastPhysicalOffence(0);
					}

					break;
				}
				case ACTIVE_ENERGY_OFFENCE_UP:
				{					
					int EnergyPercent = 0;
					if (skillDataOriginal->bySkill_Effect_Type[Effect] == 1)
						EnergyPercent = GetPcProfile()->avatarAttribute.wBaseEnergyOffence / 100 * skillDataOriginal->SkillValue[Effect];
					else
						EnergyPercent = skillDataOriginal->SkillValue[Effect];
					
					if (isRemove == true)
						GetAttributesManager()->SetLastEnergyOffence(EnergyPercent * -1);
					else
						GetAttributesManager()->SetLastEnergyOffence(EnergyPercent);
					if (GetPcProfile()->avatarAttribute.wLastEnergyOffence <= 0 || GetPcProfile()->avatarAttribute.wLastEnergyOffence >= 60000)
					{
						GetPcProfile()->avatarAttribute.wLastEnergyOffence = 0;
						GetAttributesManager()->SetLastPhysicalOffence(0);
					}
					break;
				}
				case ACTIVE_PHYSICAL_DEFENCE_UP:
				{

					if (isRemove == true)
						GetAttributesManager()->SetLastPhysicalDefence(skillDataOriginal->SkillValue[Effect] * -1);
					else
						GetAttributesManager()->SetLastPhysicalDefence(skillDataOriginal->SkillValue[Effect]);
					if (GetPcProfile()->avatarAttribute.wLastPhysicalDefence <= 0 || GetPcProfile()->avatarAttribute.wLastPhysicalDefence >= 60000)
					{
						GetPcProfile()->avatarAttribute.wLastPhysicalDefence = 0;
						GetAttributesManager()->SetLastPhysicalDefence(0);
					}
					break;
				}
				case ACTIVE_ENERGY_DEFENCE_UP:
				{

					if (isRemove == true)
						GetAttributesManager()->SetLastEnergyDefence(skillDataOriginal->SkillValue[Effect] * -1);
					else
						GetAttributesManager()->SetLastEnergyDefence(skillDataOriginal->SkillValue[Effect]);
					if (GetPcProfile()->avatarAttribute.wLastEnergyDefence <= 0 || GetPcProfile()->avatarAttribute.wLastEnergyDefence >= 60000)
					{
						GetPcProfile()->avatarAttribute.wLastEnergyDefence = 0;
						GetAttributesManager()->SetLastEnergyDefence(0);
					}
					break;
				}
				case ACTIVE_STR_UP:
				{
					if (isRemove == true)
					{
						GetAttributesManager()->SetLastStr(skillDataOriginal->SkillValue[Effect] * -1);

						WORD LevelStr = skillDataOriginal->SkillValue[Effect];
						float StrByPoint = 1.66; // 1Str = 1.66 Physical old tw
						WORD PhysicalOffence = static_cast<WORD>(LevelStr * StrByPoint);

						GetAttributesManager()->SetLastPhysicalOffence(PhysicalOffence * -1);
						if (GetPcProfile()->avatarAttribute.wLastPhysicalOffence <= 0 || GetPcProfile()->avatarAttribute.wLastPhysicalOffence >= 60000)
						{
							GetPcProfile()->avatarAttribute.wLastPhysicalOffence = 0;
							GetAttributesManager()->SetLastPhysicalOffence(0);
						}
					}
					else
					{
						GetAttributesManager()->SetLastStr(skillDataOriginal->SkillValue[Effect]);

						WORD LevelStr = skillDataOriginal->SkillValue[Effect];
						float StrByPoint = 1.66; // 1Str = 1.66 Physical old tw
						WORD PhysicalOffence = static_cast<WORD>(LevelStr * StrByPoint);

						GetAttributesManager()->SetLastPhysicalOffence(PhysicalOffence);
					}
					break;
				}
				case ACTIVE_CON_UP:
				{
					if (isRemove == true)
					{
						GetAttributesManager()->SetLastCon(skillDataOriginal->SkillValue[Effect] * -1);

						float LevelCon = skillDataOriginal->SkillValue[Effect];
						float ConByPoint = 85; // 1con = 85 old tw
						float LP = static_cast<float>(LevelCon * ConByPoint);

						GetAttributesManager()->SetLastMaxLP(LP * -1);
					}
					else
					{
						GetAttributesManager()->SetLastCon(skillDataOriginal->SkillValue[Effect]);

						float LevelCon = skillDataOriginal->SkillValue[Effect];
						float ConByPoint = 85; // 1con = 85 old tw
						float LP = static_cast<float>(LevelCon * ConByPoint);

						GetAttributesManager()->SetLastMaxLP(LP);
					}
					break;
				}
				case ACTIVE_FOC_UP:
				{
					if (isRemove == true)
					{
						GetAttributesManager()->SetLastFoc(skillDataOriginal->SkillValue[Effect] * -1);
						WORD LevelFoc = skillDataOriginal->SkillValue[Effect];
						float EnergyCriticalByPoint = 0.2; // 1Focus = 1 pont critical 
						float EnergyAttackByPoint = 2; // 1Focus = 1 pont critical 
						float HitRateByPoint = 10; // 1 point = 10 hit rate old tw
						WORD EnergyCriticalRate = static_cast<WORD>(LevelFoc * EnergyCriticalByPoint);
						WORD EnergyAttack = static_cast<WORD>(LevelFoc * EnergyAttackByPoint);
						WORD HitRate = static_cast<WORD>(LevelFoc * HitRateByPoint);
						GetAttributesManager()->SetLastEnergyCriticalRate(EnergyCriticalRate * -1);
						GetAttributesManager()->SetLastEnergyOffence(EnergyAttack * -1);
						GetAttributesManager()->SetLastAttackRate(HitRate * -1);
						if (GetPcProfile()->avatarAttribute.wLastEnergyOffence <= 0 || GetPcProfile()->avatarAttribute.wLastEnergyOffence >= 60000)
						{
							GetPcProfile()->avatarAttribute.wLastEnergyOffence = 0;
							GetAttributesManager()->SetLastPhysicalOffence(0);
						}
					}
					else
					{
						GetAttributesManager()->SetLastFoc(skillDataOriginal->SkillValue[Effect]);
						WORD LevelFoc = skillDataOriginal->SkillValue[Effect];
						float EnergyCriticalByPoint = 0.2; // 1Focus = 1 pont critical 
						float EnergyAttackByPoint = 2; // 1Focus = 1 pont critical 
						float HitRateByPoint = 10; // 1 point = 10 hit rate old tw
						WORD EnergyCriticalRate = static_cast<WORD>(LevelFoc * EnergyCriticalByPoint);
						WORD EnergyAttack = static_cast<WORD>(LevelFoc * EnergyAttackByPoint);
						WORD HitRate = static_cast<WORD>(LevelFoc * HitRateByPoint);
						GetAttributesManager()->SetLastEnergyCriticalRate(EnergyCriticalRate);
						GetAttributesManager()->SetLastEnergyOffence(EnergyAttack);
						GetAttributesManager()->SetLastAttackRate(HitRate);
					}
					break;
				}
				case ACTIVE_DEX_UP:
				{
					if (isRemove == true)
					{
						GetAttributesManager()->SetLastDex(skillDataOriginal->SkillValue[Effect] * -1);
						WORD LevelDex = skillDataOriginal->SkillValue[Effect];
						float CriticalAttackByPoint = 0.2; // 1Dex = 1 critical old tw
						float PhyAttackByPoint = 2; // 1Dex = 1 phyattack old tw
						float DoggeByPoint = 5;
						WORD PhysicalCriticalRate = static_cast<WORD>(LevelDex * CriticalAttackByPoint);
						WORD PhysicalAttack = static_cast<WORD>(LevelDex * PhyAttackByPoint);
						WORD DodgeRate = static_cast<WORD>(LevelDex * DoggeByPoint);
						GetAttributesManager()->SetLastPhysicalCriticalRate(PhysicalCriticalRate * -1);
						GetAttributesManager()->SetLastPhysicalOffence(PhysicalAttack * -1);
						GetAttributesManager()->SetLastDodgeRate(DodgeRate * -1);
						if (GetPcProfile()->avatarAttribute.wLastPhysicalOffence <= 0 || GetPcProfile()->avatarAttribute.wLastPhysicalOffence >= 60000)
						{
							GetPcProfile()->avatarAttribute.wLastPhysicalOffence = 0;
							GetAttributesManager()->SetLastPhysicalOffence(0);
						}
						if (GetPcProfile()->avatarAttribute.byLastDex <= 0 || GetPcProfile()->avatarAttribute.byLastDex >= 60000)
						{
							GetPcProfile()->avatarAttribute.byLastDex = 0;
							GetAttributesManager()->SetLastDex(0);
						}
					}
					else
					{
						GetAttributesManager()->SetLastDex(skillDataOriginal->SkillValue[Effect]);
						WORD LevelDex = skillDataOriginal->SkillValue[Effect];
						float CriticalAttackByPoint = 0.2; // 1Dex = 1 critical old tw
						float PhyAttackByPoint = 2; // 1Dex = 1 phyattack old tw
						float DoggeByPoint = 5;
						WORD PhysicalCriticalRate = static_cast<WORD>(LevelDex * CriticalAttackByPoint);
						WORD PhysicalAttack = static_cast<WORD>(LevelDex * PhyAttackByPoint);
						WORD DodgeRate = static_cast<WORD>(LevelDex * DoggeByPoint);
						GetAttributesManager()->SetLastPhysicalCriticalRate(PhysicalCriticalRate);
						GetAttributesManager()->SetLastPhysicalOffence(PhysicalAttack);
						GetAttributesManager()->SetLastDodgeRate(DodgeRate);
					}
					break;
				}
				case ACTIVE_SOL_UP:
				{
					if (isRemove == true)
					{
						GetAttributesManager()->SetLastSol(skillDataOriginal->SkillValue[Effect] * -1);

						WORD LevelSol = skillDataOriginal->SkillValue[Effect];
						float SolByPoint = 1.66; // 1Soul = 1.66 Physical old tw
						WORD EnergyOffence = static_cast<WORD>(LevelSol * SolByPoint);

						GetAttributesManager()->SetLastEnergyOffence(EnergyOffence * -1);
						if (GetPcProfile()->avatarAttribute.wLastEnergyOffence <= 0 || GetPcProfile()->avatarAttribute.wLastEnergyOffence >= 60000)
						{
							GetPcProfile()->avatarAttribute.wLastEnergyOffence = 0;
							GetAttributesManager()->SetLastPhysicalOffence(0);
						}
					}
					else
					{
						GetAttributesManager()->SetLastSol(skillDataOriginal->SkillValue[Effect]);

						WORD LevelSol = skillDataOriginal->SkillValue[Effect];
						float SolByPoint = 1.66; // 1Soul = 1.66 Physical old tw
						WORD EnergyOffence = static_cast<WORD>(LevelSol * SolByPoint);

						GetAttributesManager()->SetLastEnergyOffence(EnergyOffence);
					}
					break;
				}
				case ACTIVE_ENG_UP:
				{
					if (isRemove == true)
					{
						GetAttributesManager()->SetLastEng(skillDataOriginal->SkillValue[Effect] * -1);

						WORD LevelEng = skillDataOriginal->SkillValue[Effect];
						float EngByPoint = 45; // 1Eng = 45 ep old tw
						WORD EP = static_cast<WORD>(LevelEng * EngByPoint);

						GetAttributesManager()->SetLastMaxEP(EP * -1);
					}
					else
					{
						GetAttributesManager()->SetLastEng(skillDataOriginal->SkillValue[Effect]);

						WORD LevelEng = skillDataOriginal->SkillValue[Effect];
						float EngByPoint = 45; // 1Eng = 45 ep old tw
						WORD EP = static_cast<WORD>(LevelEng * EngByPoint);

						GetAttributesManager()->SetLastMaxEP(EP);
					}
					break;
				}
				case ACTIVE_MOVE_SPEED_UP:
				{
					if (isRemove == true)
					{
						int moveSpeed = 10 / 100 * skillDataOriginal->SkillValue[Effect];
						//GetAttributesManager()->SetLastRunSpeed(moveSpeed * -1);
						GetAttributesManager()->SetLastRunSpeed(GetPcProfile()->avatarAttribute.fLastRunSpeed - moveSpeed);
						sGU_UPDATE_CHAR_SPEED speed;
						speed.wOpCode = GU_UPDATE_CHAR_SPEED;
						speed.wPacketSize = sizeof(sGU_UPDATE_CHAR_SPEED) - 2;

						speed.handle = GetHandle();
						speed.fLastWalkingSpeed = GetPcProfile()->avatarAttribute.fLastRunSpeed;
						speed.fLastRunningSpeed = GetPcProfile()->avatarAttribute.fLastRunSpeed;
						speed.fLastFlySpeed = GetPcProfile()->avatarAttribute.fLastAirSpeed;
						speed.fLastFlyBoostSpeed = GetPcProfile()->avatarAttribute.fLastAirDashSpeed;
						speed.fLastFlySpeed2 = GetPcProfile()->avatarAttribute.fLastAirDash2Speed;
						speed.fLastFlyBoostSpeed2 = GetPcProfile()->avatarAttribute.fLastAirDash2Speed;
						
						sWorld.SendToAll((char*)&speed, sizeof(sGU_UPDATE_CHAR_SPEED));
						//SendToPlayerList((char*)&speed, sizeof(sGU_UPDATE_CHAR_SPEED));
					}
					else
					{
						int moveSpeed = 10 / 100 * skillDataOriginal->SkillValue[Effect];
						GetAttributesManager()->SetLastRunSpeed(GetPcProfile()->avatarAttribute.fLastRunSpeed + moveSpeed);
						sGU_UPDATE_CHAR_SPEED speed;
						speed.wOpCode = GU_UPDATE_CHAR_SPEED;
						speed.wPacketSize = sizeof(sGU_UPDATE_CHAR_SPEED) - 2;

						speed.handle = GetHandle();
						speed.fLastWalkingSpeed = GetPcProfile()->avatarAttribute.fLastRunSpeed;
						speed.fLastRunningSpeed = GetPcProfile()->avatarAttribute.fLastRunSpeed;
						speed.fLastFlySpeed = GetPcProfile()->avatarAttribute.fLastAirSpeed;
						speed.fLastFlyBoostSpeed = GetPcProfile()->avatarAttribute.fLastAirDashSpeed;
						speed.fLastFlySpeed2 = GetPcProfile()->avatarAttribute.fLastAirDash2Speed;
						speed.fLastFlyBoostSpeed2 = GetPcProfile()->avatarAttribute.fLastAirDash2Speed;

						sWorld.SendToAll((char*)&speed, sizeof(sGU_UPDATE_CHAR_SPEED));
						//SendToPlayerList((char*)&speed, sizeof(sGU_UPDATE_CHAR_SPEED));
					}
					break;
				}
				case ACTIVE_ATTACK_SPEED_UP:
				{
					if (isRemove == true)
					{						
						sGU_UPDATE_CHAR_ATTACK_SPEEDRATE sAttackSpeed;
						sAttackSpeed.wOpCode = GU_UPDATE_CHAR_ATTACK_SPEEDRATE;
						sAttackSpeed.wPacketSize = sizeof(sGU_UPDATE_CHAR_ATTACK_SPEEDRATE) - 2;
						sAttackSpeed.handle = GetHandle();
						int attackspeed = 1400 / 100 * skillDataOriginal->SkillValue[Effect];
						GetAttributesManager()->SetLastAttackSpeedRate(attackspeed);
						sAttackSpeed.wAttackSpeedRate = GetPcProfile()->avatarAttribute.wLastAttackSpeedRate;						

						if (GetPcProfile()->avatarAttribute.wLastAttackSpeedRate <= 0 || GetPcProfile()->avatarAttribute.wLastAttackSpeedRate >= 1400)
						{
							GetPcProfile()->avatarAttribute.wLastAttackSpeedRate = 1400;
							GetAttributesManager()->SetLastAttackSpeedRate(0);
							sAttackSpeed.wAttackSpeedRate = GetPcProfile()->avatarAttribute.wLastAttackSpeedRate;
						}

						sWorld.SendToAll((char*)&sAttackSpeed, sizeof(sGU_UPDATE_CHAR_ATTACK_SPEEDRATE));
						//SendToPlayerList((char*)&sAttackSpeed, sizeof(sGU_UPDATE_CHAR_ATTACK_SPEEDRATE));
					}
					else
					{
						sGU_UPDATE_CHAR_ATTACK_SPEEDRATE sAttackSpeed;
						sAttackSpeed.wOpCode = GU_UPDATE_CHAR_ATTACK_SPEEDRATE;
						sAttackSpeed.wPacketSize = sizeof(sGU_UPDATE_CHAR_ATTACK_SPEEDRATE) - 2;
						sAttackSpeed.handle = GetHandle();
						int attackspeed = 1400 / 100 * skillDataOriginal->SkillValue[Effect];
						GetAttributesManager()->SetLastAttackSpeedRate(attackspeed * -1);
						sAttackSpeed.wAttackSpeedRate = GetPcProfile()->avatarAttribute.wLastAttackSpeedRate;
						

						if (GetPcProfile()->avatarAttribute.wLastAttackSpeedRate <= 0 || GetPcProfile()->avatarAttribute.wLastAttackSpeedRate >= 1400)
						{
							GetPcProfile()->avatarAttribute.wLastAttackSpeedRate = 1400;
							GetAttributesManager()->SetLastAttackSpeedRate(0);
							sAttackSpeed.wAttackSpeedRate = GetPcProfile()->avatarAttribute.wLastAttackSpeedRate;
						}

						sWorld.SendToAll((char*)&sAttackSpeed, sizeof(sGU_UPDATE_CHAR_ATTACK_SPEEDRATE));
						//SendToPlayerList((char*)&sAttackSpeed, sizeof(sGU_UPDATE_CHAR_ATTACK_SPEEDRATE));
					}
					break;
				}
				case ACTIVE_ATTACK_RATE_UP:
				{
					if (isRemove == true)
					{						
						GetAttributesManager()->SetLastAttackRate(skillDataOriginal->SkillValue[Effect] * -1);
					}
					else
					{						
						GetAttributesManager()->SetLastAttackRate(skillDataOriginal->SkillValue[Effect]);
					}
					break;
				}
				case ACTIVE_DODGE_RATE_UP:
				{
					if (isRemove == true)
					{
						GetAttributesManager()->SetLastDodgeRate(skillDataOriginal->SkillValue[Effect] * -1);
					}
					else
					{
						GetAttributesManager()->SetLastDodgeRate(skillDataOriginal->SkillValue[Effect]);
					}
					break;
				}
				case ACTIVE_BLOCK_RATE_UP:
				{
					if (isRemove == true)
					{
						GetAttributesManager()->SetLastBlockRate(skillDataOriginal->SkillValue[Effect] * -1);
					}
					else
					{
						GetAttributesManager()->SetLastBlockRate(skillDataOriginal->SkillValue[Effect]);
					}
					break;
				}
				//Atributes//
				case ACTIVE_HONEST_DEFENCE_UP:
				{
					if (isRemove == true)
					{
					}
					else
					{
					}
					break;
				}
				case ACTIVE_STRANGE_DEFENCE_UP:
				{
					if (isRemove == true)
					{
					}
					else
					{
					}
					break;
				}
				case ACTIVE_WILD_DEFENCE_UP:
				{
					if (isRemove == true)
					{
					}
					else
					{
					}
					break;
				}
				case ACTIVE_ELEGANCE_DEFENCE_UP:
				{
					if (isRemove == true)
					{
					}
					else
					{
					}
					break;
				}
				case ACTIVE_FUNNY_DEFENCE_UP:
				{
					if (isRemove == true)
					{
					}
					else
					{
					}
					break;
				}
				case ACTIVE_ENERGY_REFLECTION:
				{
					if (isRemove == true)
					{
					}
					else
					{
					}
					break;
				}
				case ACTIVE_LP_REGENERATION:
				{
					if (isRemove == true)
					{
					}
					else
					{
					}
					break;
				}
				case ACTIVE_EP_REGENERATION:
				{
					if (isRemove == true)
					{
					}
					else
					{
					}
					break;
				}
				case ACTIVE_RP_CHARGE_SPEED:
				{
					if (isRemove == true)
					{
					}
					else
					{
					}
					break;
				}
				case ACTIVE_CURSE_SUCCESS:
				{
					if (isRemove == true)
					{
					}
					else
					{
					}
					break;
				}
				case ACTIVE_CURSE_TOLERANCE:
				{
					if (isRemove == true)
					{
					}
					else
					{
					}
					break;
				}
				case ACTIVE_PHYSICAL_CRITICAL:
				{
					if (isRemove == true)
					{
						GetAttributesManager()->SetLastPhysicalCriticalRate(skillDataOriginal->SkillValue[Effect] * -1);
					}
					else
					{
						GetAttributesManager()->SetLastPhysicalCriticalRate(skillDataOriginal->SkillValue[Effect]);
					}
					break;
				}
				case ACTIVE_ENERGY_CRITICAL:
				{
					if (isRemove == true)
					{
						GetAttributesManager()->SetLastEnergyCriticalRate(skillDataOriginal->SkillValue[Effect] * -1);
					}
					else
					{
						GetAttributesManager()->SetLastEnergyCriticalRate(skillDataOriginal->SkillValue[Effect]);
					}
					break;
				}
				case ACTIVE_SKILL_CASTING_TIME_DOWN:
				{
					if (isRemove == true)
					{						
					}
					else
					{
					}
					break;
				}
				case ACTIVE_SKILL_COOL_TIME_DOWN:
				{
					if (isRemove == true)
					{
						//printf("Remove Cool Down  %d \n", skillDataOriginal->SkillValue[Effect]);
						GetAttributesManager()->SetCoolTimeChangePercent(skillDataOriginal->SkillValue[Effect]);
					}
					else
					{
						//printf("Adiciona Cool Down  %d \n", skillDataOriginal->SkillValue[Effect]);
						GetAttributesManager()->SetCoolTimeChangePercent(skillDataOriginal->SkillValue[Effect] * -1);
					}
					break;
				}
				//Next Case
				}
			}
		}
	}
} 
void Player::ExecuteEffectPlayerCalculation(TBLIDX SkillID, HOBJECT Handle, bool isRemove)
{
	SkillTable * skillTable = sTBM.GetSkillTable();
	sSKILL_TBLDAT * skillDataOriginal = NULL;
	skillDataOriginal = reinterpret_cast<sSKILL_TBLDAT*>(skillTable->FindData(SkillID));
	//Player* PlayerInfo = static_cast<Player*>(GetFromList(Handle));
	if (skillDataOriginal != NULL)
	{
		for (int Effect = 0; Effect <= 2; Effect++)
		{
			int count = 0;
			sSYSTEM_EFFECT_TBLDAT * SystemEffectData = NULL;
			SystemEffectData = (sSYSTEM_EFFECT_TBLDAT*)sTBM.GetSystemEffectTable()->FindData(skillDataOriginal->skill_Effect[Effect]);
			if (SystemEffectData != NULL)
			{
				//printf("skill_Effect %d \n", skillDataOriginal->skill_Effect[Effect]);
			//	printf("effectCode %d \n", SystemEffectData->effectCode);
			//	printf("Effect %d \n", Effect);
				switch (SystemEffectData->effectCode)
				{
					case ACTIVE_MAX_LP_DOWN:
					{
						if (isRemove == true)
						{
							float MaxLP = skillDataOriginal->SkillValue[Effect];
							GetAttributesManager()->SetLastMaxLP(MaxLP);
						}
						else
						{
							float MaxLP = skillDataOriginal->SkillValue[Effect];
							if (GetPcProfile()->avatarAttribute.wLastMaxLP - MaxLP <= 0)
							{
								GetPcProfile()->avatarAttribute.wBaseMaxLP = MaxLP;
								GetAttributesManager()->SetLastMaxLP(MaxLP * -1);
							}
							else
							{
								GetAttributesManager()->SetLastMaxLP(MaxLP * -1);
							}
						}
						break;
					}
					case ACTIVE_MAX_EP_DOWN:
					{
						if (isRemove == true)
						{
							float MaxEP = skillDataOriginal->SkillValue[Effect];
							GetAttributesManager()->SetLastMaxEP(MaxEP);
						}
						else
						{
							float MaxEP = skillDataOriginal->SkillValue[Effect];
							if (GetPcProfile()->avatarAttribute.wLastMaxEP - MaxEP <= 0)
							{
								GetPcProfile()->avatarAttribute.wBaseMaxEP = MaxEP;
								GetAttributesManager()->SetLastMaxEP(MaxEP * -1);
							}
							else
							{
								GetAttributesManager()->SetLastMaxEP(MaxEP * -1);
							}
						}
						break;
					}
					case ACTIVE_MAX_RP_DOWN:
					{
						break;
					}
					case ACTIVE_PHYSICAL_OFFENCE_DOWN:
					{
						if (isRemove == true)
						{
							float PhycalOffenseDown = skillDataOriginal->SkillValue[Effect];
							GetAttributesManager()->SetLastPhysicalOffence(PhycalOffenseDown);
						}
						else
						{
							float PhycalOffenseDown = skillDataOriginal->SkillValue[Effect];
							if (GetPcProfile()->avatarAttribute.wLastPhysicalOffence - PhycalOffenseDown <= 0)
							{
								GetPcProfile()->avatarAttribute.wLastPhysicalOffence = PhycalOffenseDown;
								GetAttributesManager()->SetLastPhysicalOffence(PhycalOffenseDown * -1);
							}
							else
							{
								GetAttributesManager()->SetLastPhysicalOffence(PhycalOffenseDown * -1);
							}
						}
						break;
					}
					case ACTIVE_ENERGY_OFFENCE_DOWN:
					{
						if (isRemove == true)
						{
							float EnergyOffenseDown = skillDataOriginal->SkillValue[Effect];
							GetAttributesManager()->SetLastEnergyOffence(EnergyOffenseDown);
						}
						else
						{
							float EnergyOffenseDown = skillDataOriginal->SkillValue[Effect];
							if (GetPcProfile()->avatarAttribute.wLastEnergyOffence - EnergyOffenseDown <= 0)
							{
								GetPcProfile()->avatarAttribute.wLastEnergyOffence = EnergyOffenseDown;
								GetAttributesManager()->SetLastEnergyOffence(EnergyOffenseDown * -1);
							}
							else
							{
								GetAttributesManager()->SetLastEnergyOffence(EnergyOffenseDown * -1);
							}
						}
						break;
					}
					case ACTIVE_PHYSICAL_DEFENCE_DOWN:
					{
						if (isRemove == true)
						{
							float PhycalDefenceDown = skillDataOriginal->SkillValue[Effect];
							GetAttributesManager()->SetLastPhysicalDefence(PhycalDefenceDown);
						}
						else
						{
							float PhycalDefenceDown = skillDataOriginal->SkillValue[Effect];
							if (GetPcProfile()->avatarAttribute.wLastPhysicalDefence - PhycalDefenceDown <= 0)
							{
								GetPcProfile()->avatarAttribute.wLastPhysicalDefence = PhycalDefenceDown;
								GetAttributesManager()->SetLastPhysicalDefence(PhycalDefenceDown * -1);
							}
							else
							{
								GetAttributesManager()->SetLastPhysicalDefence(PhycalDefenceDown * -1);
							}							
						}
						break;
					}
					case ACTIVE_ENERGY_DEFENCE_DOWN:
					{
						if (isRemove == true)
						{
							float EnergyDefenceDown = skillDataOriginal->SkillValue[Effect];
							GetAttributesManager()->SetLastEnergyDefence(EnergyDefenceDown);
						}
						else
						{
							float EnergyDefenceDown = skillDataOriginal->SkillValue[Effect];
							if (GetPcProfile()->avatarAttribute.wLastEnergyDefence - EnergyDefenceDown <= 0)
							{
								GetPcProfile()->avatarAttribute.wLastEnergyDefence = EnergyDefenceDown;
								GetAttributesManager()->SetLastEnergyDefence(EnergyDefenceDown * - 1);
							}
							else
							{
								GetAttributesManager()->SetLastEnergyDefence(EnergyDefenceDown * -1);
							}
						}
						break;
					}
					case ACTIVE_CON_DOWN:
					{
						if (isRemove == true)
						{
							float MaxLP = skillDataOriginal->SkillValue[Effect];
							GetAttributesManager()->SetLastMaxLP(MaxLP);
							sGU_UPDATE_CHAR_LP updLp;

							updLp.handle = Handle;
							updLp.wCurLP = GetPcProfile()->dwCurLP;
							updLp.wMaxLP = GetPcProfile()->avatarAttribute.wLastMaxLP;
							updLp.wOpCode = GU_UPDATE_CHAR_LP;
							updLp.wPacketSize = sizeof(sGU_UPDATE_CHAR_LP) - 2;

							sWorld.SendToAll((char*)&updLp, sizeof(sGU_UPDATE_CHAR_LP));
							//SendToPlayerList((char*)&updLp, sizeof(sGU_UPDATE_CHAR_LP));
						}
						else
						{
							float MaxLP = skillDataOriginal->SkillValue[Effect];
							GetAttributesManager()->SetLastMaxLP(MaxLP * -1);
							sGU_UPDATE_CHAR_LP updLp;

							updLp.handle = Handle;
							updLp.wCurLP = GetPcProfile()->dwCurLP;
							updLp.wMaxLP = GetPcProfile()->avatarAttribute.wLastMaxLP;
							updLp.wOpCode = GU_UPDATE_CHAR_LP;
							updLp.wPacketSize = sizeof(sGU_UPDATE_CHAR_LP) - 2;

							sWorld.SendToAll((char*)&updLp, sizeof(sGU_UPDATE_CHAR_LP));
							//SendToPlayerList((char*)&updLp, sizeof(sGU_UPDATE_CHAR_LP));
						}
						break;
					}
					case ACTIVE_BATTLE_INABILITY:
					{
						if (isRemove == true)
						{
							sGU_UPDATE_CHAR_CONDITION ConfuseTarget;
							ConfuseTarget.wOpCode = GU_UPDATE_CHAR_CONDITION;
							ConfuseTarget.wPacketSize = sizeof(sGU_UPDATE_CHAR_CONDITION) - 2;

							ConfuseTarget.handle = Handle;
							ConfuseTarget.dwConditionFlag = 0;
							GetState()->sCharStateBase.dwConditionFlag = ConfuseTarget.dwConditionFlag;							

							sWorld.SendToAll((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
							//SendToPlayerList((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
						}
						else
						{
							sGU_UPDATE_CHAR_CONDITION ConfuseTarget;
							ConfuseTarget.wOpCode = GU_UPDATE_CHAR_CONDITION;
							ConfuseTarget.wPacketSize = sizeof(sGU_UPDATE_CHAR_CONDITION) - 2;

							ConfuseTarget.handle = Handle;
							ConfuseTarget.dwConditionFlag = eCHARCONDITION_FLAG::CHARCOND_BATTLE_INABILITY_FLAG;
							GetState()->sCharStateBase.dwConditionFlag = ConfuseTarget.dwConditionFlag;
							
							sWorld.SendToAll((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
							//SendToPlayerList((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
						}
						break;
					}
					case ACTIVE_SKILL_INABILITY:
					{
						if (isRemove == true)
						{
							sGU_UPDATE_CHAR_CONDITION ConfuseTarget;
							ConfuseTarget.wOpCode = GU_UPDATE_CHAR_CONDITION;
							ConfuseTarget.wPacketSize = sizeof(sGU_UPDATE_CHAR_CONDITION) - 2;

							ConfuseTarget.handle = Handle;
							ConfuseTarget.dwConditionFlag = 0;
							GetState()->sCharStateBase.dwConditionFlag = ConfuseTarget.dwConditionFlag;

							sWorld.SendToAll((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
							//SendToPlayerList((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
						}
						else
						{
							sGU_UPDATE_CHAR_CONDITION ConfuseTarget;
							ConfuseTarget.wOpCode = GU_UPDATE_CHAR_CONDITION;
							ConfuseTarget.wPacketSize = sizeof(sGU_UPDATE_CHAR_CONDITION) - 2;

							ConfuseTarget.handle = Handle;
							ConfuseTarget.dwConditionFlag = eCHARCONDITION_FLAG::CHARCOND_BATTLE_INABILITY_FLAG;
							GetState()->sCharStateBase.dwConditionFlag = ConfuseTarget.dwConditionFlag;
							
							sWorld.SendToAll((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
							//SendToPlayerList((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
						}
						break;
					}
					case ACTIVE_TERROR:
					{
						if (isRemove == true)
						{
							sGU_UPDATE_CHAR_CONDITION ConfuseTarget;
							ConfuseTarget.wOpCode = GU_UPDATE_CHAR_CONDITION;
							ConfuseTarget.wPacketSize = sizeof(sGU_UPDATE_CHAR_CONDITION) - 2;

							ConfuseTarget.handle = Handle;
							ConfuseTarget.dwConditionFlag = 0;
							GetState()->sCharStateBase.dwConditionFlag = ConfuseTarget.dwConditionFlag;

							sWorld.SendToAll((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
							//SendToPlayerList((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
						}
						else
						{
							sGU_UPDATE_CHAR_CONDITION ConfuseTarget;
							ConfuseTarget.wOpCode = GU_UPDATE_CHAR_CONDITION;
							ConfuseTarget.wPacketSize = sizeof(sGU_UPDATE_CHAR_CONDITION) - 2;

							ConfuseTarget.handle = Handle;
							ConfuseTarget.dwConditionFlag = eCHARCONDITION_FLAG::CHARCOND_TERROR_FLAG;
							GetState()->sCharStateBase.dwConditionFlag = ConfuseTarget.dwConditionFlag;
							
							sWorld.SendToAll((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
							//SendToPlayerList((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
						}
						break;
					}
					case ACTIVE_MOVE_SPEED_DOWN:
					{
						if (isRemove == true)
						{
							int moveSpeed = 10;
							GetAttributesManager()->SetLastRunSpeed(moveSpeed);
							sGU_UPDATE_CHAR_SPEED speed;
							speed.wOpCode = GU_UPDATE_CHAR_SPEED;
							speed.wPacketSize = sizeof(sGU_UPDATE_CHAR_SPEED) - 2;

							speed.handle = Handle;
							speed.fLastWalkingSpeed = GetPcProfile()->avatarAttribute.fLastRunSpeed;
							speed.fLastRunningSpeed = GetPcProfile()->avatarAttribute.fLastRunSpeed;
							speed.fLastFlySpeed = GetPcProfile()->avatarAttribute.fLastAirSpeed;
							speed.fLastFlyBoostSpeed = GetPcProfile()->avatarAttribute.fLastAirDashSpeed;
							speed.fLastFlySpeed2 = GetPcProfile()->avatarAttribute.fLastAirDash2Speed;
							speed.fLastFlyBoostSpeed2 = GetPcProfile()->avatarAttribute.fLastAirDash2Speed;
							
							
							sWorld.SendToAll((char*)&speed, sizeof(sGU_UPDATE_CHAR_SPEED));
							//SendToPlayerList((char*)&speed, sizeof(sGU_UPDATE_CHAR_SPEED));
						}							
						else
						{
							float moveSpeed = GetPcProfile()->avatarAttribute.fLastRunSpeed / 100 * skillDataOriginal->SkillValue[Effect];
							GetAttributesManager()->SetLastRunSpeed(GetPcProfile()->avatarAttribute.fLastRunSpeed - moveSpeed);
							sGU_UPDATE_CHAR_SPEED speed;
							speed.wOpCode = GU_UPDATE_CHAR_SPEED;
							speed.wPacketSize = sizeof(sGU_UPDATE_CHAR_SPEED) - 2;

							speed.handle = Handle;
							speed.fLastWalkingSpeed = GetPcProfile()->avatarAttribute.fLastRunSpeed;
							speed.fLastRunningSpeed = GetPcProfile()->avatarAttribute.fLastRunSpeed;
							speed.fLastFlySpeed = GetPcProfile()->avatarAttribute.fLastAirSpeed;
							speed.fLastFlyBoostSpeed = GetPcProfile()->avatarAttribute.fLastAirDashSpeed;
							speed.fLastFlySpeed2 = GetPcProfile()->avatarAttribute.fLastAirDash2Speed;
							speed.fLastFlyBoostSpeed2 = GetPcProfile()->avatarAttribute.fLastAirDash2Speed;
							
							sWorld.SendToAll((char*)&speed, sizeof(sGU_UPDATE_CHAR_SPEED));
							//SendToPlayerList((char*)&speed, sizeof(sGU_UPDATE_CHAR_SPEED));
						}
						break;
					}

					case ACTIVE_CONFUSE:
					{
						if (isRemove == true)
						{							
							sGU_UPDATE_CHAR_CONDITION ConfuseTarget;
							ConfuseTarget.wOpCode = GU_UPDATE_CHAR_CONDITION;
							ConfuseTarget.wPacketSize = sizeof(sGU_UPDATE_CHAR_CONDITION) - 2;

							ConfuseTarget.handle = Handle;
							ConfuseTarget.dwConditionFlag = 0;
							GetState()->sCharStateBase.dwConditionFlag = ConfuseTarget.dwConditionFlag;
							
							sWorld.SendToAll((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
							//SendToPlayerList((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
						}
						else
						{	
							sGU_UPDATE_CHAR_CONDITION ConfuseTarget;
							ConfuseTarget.wOpCode = GU_UPDATE_CHAR_CONDITION;
							ConfuseTarget.wPacketSize = sizeof(sGU_UPDATE_CHAR_CONDITION) - 2;

							ConfuseTarget.handle = Handle;
							ConfuseTarget.dwConditionFlag = eCHARCONDITION_FLAG::CHARCOND_CONFUSED_FLAG;
							GetState()->sCharStateBase.dwConditionFlag = ConfuseTarget.dwConditionFlag;
							
							sWorld.SendToAll((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
							//SendToPlayerList((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
						}
						break;
					}
					case ACTIVE_FAKE_DEATH:
					{
						if (isRemove == true)
						{
							sGU_UPDATE_CHAR_CONDITION ConfuseTarget;
							ConfuseTarget.wOpCode = GU_UPDATE_CHAR_CONDITION;
							ConfuseTarget.wPacketSize = sizeof(sGU_UPDATE_CHAR_CONDITION) - 2;

							ConfuseTarget.handle = Handle;
							ConfuseTarget.dwConditionFlag = 0;
							GetState()->sCharStateBase.dwConditionFlag = ConfuseTarget.dwConditionFlag;

							sWorld.SendToAll((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
							//SendToPlayerList((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
						}
						else
						{
							sGU_UPDATE_CHAR_CONDITION ConfuseTarget;
							ConfuseTarget.wOpCode = GU_UPDATE_CHAR_CONDITION;
							ConfuseTarget.wPacketSize = sizeof(sGU_UPDATE_CHAR_CONDITION) - 2;

							ConfuseTarget.handle = Handle;
							ConfuseTarget.dwConditionFlag = eCHARCONDITION_FLAG::CHARCOND_FAKE_DEATH_FLAG;
							GetState()->sCharStateBase.dwConditionFlag = ConfuseTarget.dwConditionFlag;

							sWorld.SendToAll((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
							//SendToPlayerList((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
						}
						break;
					}
					case ACTIVE_ENERGY_REFLECTION:
					{
						if (isRemove == true)
						{
							sGU_UPDATE_CHAR_CONDITION ConfuseTarget;
							ConfuseTarget.wOpCode = GU_UPDATE_CHAR_CONDITION;
							ConfuseTarget.wPacketSize = sizeof(sGU_UPDATE_CHAR_CONDITION) - 2;

							ConfuseTarget.handle = Handle;
							ConfuseTarget.dwConditionFlag = 0;
							GetState()->sCharStateBase.dwConditionFlag = ConfuseTarget.dwConditionFlag;

							sWorld.SendToAll((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
							//SendToPlayerList((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
						}
						else
						{
							sGU_UPDATE_CHAR_CONDITION ConfuseTarget;
							ConfuseTarget.wOpCode = GU_UPDATE_CHAR_CONDITION;
							ConfuseTarget.wPacketSize = sizeof(sGU_UPDATE_CHAR_CONDITION) - 2;

							ConfuseTarget.handle = Handle;
							ConfuseTarget.dwConditionFlag = eCHARCONDITION_FLAG::CHARCOND_DAMAGE_REFLECTION_FLAG;
							GetState()->sCharStateBase.dwConditionFlag = ConfuseTarget.dwConditionFlag;

							sWorld.SendToAll((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
							//SendToPlayerList((char*)&ConfuseTarget, sizeof(sGU_UPDATE_CHAR_CONDITION));
						}
						break;
					}
				}
			}
		}
	}
}
//It will Handle all bleed/Posion/Burn Demage
void Player::CharAffect()
{
	for (int i = 0; i <= 32; i++)
	{
		if (GetAttributesManager()->sBuffTimeInfo[i].BuffIsActive == true && GetAttributesManager()->sBuffTimeInfo[i].isMob == false && GetAttributesManager()->sBuffTimeInfo[i].BuffID != 0 & GetAttributesManager()->sBuffTimeInfo[i].BuffID != INVALID_TBLIDX)
		{
			if (GetAttributesManager()->sBuffTimeInfo[i].isAffectPlayer == true)
			{			
					sGU_EFFECT_AFFECTED afect;
					afect.wOpCode = GU_EFFECT_AFFECTED;
					afect.wPacketSize = sizeof(sGU_EFFECT_AFFECTED) - 2;
					afect.handle = GetAttributesManager()->sBuffTimeInfo[i].PlayerHandle;
					afect.hCaster = GetAttributesManager()->sBuffTimeInfo[i].PlayerHandle;
					afect.effectTblidx = GetAttributesManager()->sBuffTimeInfo[i].BuffID;
					afect.bysource = 1;
					afect.SyestemEffectCode = GetAttributesManager()->sBuffTimeInfo[i].EffectType;
					afect.ResultCode = 500;
					afect.ActualArgument[0] = GetAttributesManager()->sBuffTimeInfo[i].EffectValue[0];
					afect.ActualArgument[1] = GetAttributesManager()->sBuffTimeInfo[i].EffectValue[1];

					int newlp = GetPcProfile()->dwCurLP;
					if (newlp <= GetPcProfile()->avatarAttribute.wLastMaxLP)
					{
						newlp -= GetAttributesManager()->sBuffTimeInfo[i].EffectValue[0];
					}
					if (newlp <= 0 || newlp >= 1000000000)
					{
						newlp = 0;
						if (GetIsDead() == false)
						{
							sGU_UPDATE_CHAR_STATE state;

							state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
							state.wOpCode = GU_UPDATE_CHAR_STATE;

							state.handle = GetHandle();
							state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_FAINTING;
							state.sCharState.sCharStateDetail.sCharStateFainting.byReason = eFAINT_REASON::FAINT_REASON_HIT;

							state.sCharState.sCharStateBase.vCurLoc.x = GetVectorPosition().x;
							state.sCharState.sCharStateBase.vCurLoc.y = GetVectorPosition().y;
							state.sCharState.sCharStateBase.vCurLoc.z = GetVectorPosition().z;
							state.sCharState.sCharStateBase.vCurDir.x = GetVectorOriantation().x;
							state.sCharState.sCharStateBase.vCurDir.y = GetVectorOriantation().y;
							state.sCharState.sCharStateBase.vCurDir.z = GetVectorOriantation().x;
							state.sCharState.sCharStateBase.dwConditionFlag = 0;
							//	res.sCharState.sCharStateBase.bFightMode = false;
							state.sCharState.sCharStateBase.dwStateTime = 2;
							SetIsDead(true);
							SendPacket((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
							//SendToPlayerList((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
						}
						
					}
					GetPcProfile()->dwCurLP = newlp;
					sGU_UPDATE_CHAR_LP LPs;
					LPs.wOpCode = GU_UPDATE_CHAR_LP;
					LPs.wPacketSize = sizeof(sGU_UPDATE_CHAR_LP) - 2;
					LPs.dwLpEpEventId = 0;
					LPs.handle = GetAttributesManager()->sBuffTimeInfo[i].PlayerHandle;
					LPs.wCurLP = GetPcProfile()->dwCurLP;
					LPs.wMaxLP = GetPcProfile()->avatarAttribute.wLastMaxLP;
					SendPacket((char*)&LPs, sizeof(sGU_UPDATE_CHAR_LP));
					SendToPlayerList((char*)&LPs, sizeof(sGU_UPDATE_CHAR_LP));

					SendPacket((char*)&afect, sizeof(sGU_EFFECT_AFFECTED));
					SendToPlayerList((char*)&afect, sizeof(sGU_EFFECT_AFFECTED));
					AffectTime = GetTickCount();
			}				
		}
		//mob
		if (GetAttributesManager()->sBuffTimeInfo[i].BuffIsActive == true && GetAttributesManager()->sBuffTimeInfo[i].isMob == true && GetAttributesManager()->sBuffTimeInfo[i].BuffID != 0 & GetAttributesManager()->sBuffTimeInfo[i].BuffID != INVALID_TBLIDX)
		{
			Mob* MobInfo = static_cast<Mob*>(GetFromList(GetAttributesManager()->sBuffTimeInfo[i].PlayerHandle));
			if (MobInfo != NULL && MobInfo->GetIsDead() == false)
			{			
					if (GetAttributesManager()->sBuffTimeInfo[i].isAffectPlayer == true)
					{
						sGU_EFFECT_AFFECTED afect;
						afect.wOpCode = GU_EFFECT_AFFECTED;
						afect.wPacketSize = sizeof(sGU_EFFECT_AFFECTED) - 2;
						afect.handle = GetAttributesManager()->sBuffTimeInfo[i].PlayerHandle;
						afect.hCaster = GetAttributesManager()->sBuffTimeInfo[i].PlayerHandle;
						afect.effectTblidx = GetAttributesManager()->sBuffTimeInfo[i].BuffID;
						afect.bysource = 1;
						afect.SyestemEffectCode = GetAttributesManager()->sBuffTimeInfo[i].EffectType;
						afect.ResultCode = 500;
						afect.ActualArgument[0] = GetAttributesManager()->sBuffTimeInfo[i].EffectValue[0];
						afect.ActualArgument[1] = GetAttributesManager()->sBuffTimeInfo[i].EffectValue[1];
						
						int newlp = MobInfo->GetMobData().CurLP;
						if (newlp <= MobInfo->GetMobData().MaxLP)
						{
							newlp -= GetAttributesManager()->sBuffTimeInfo[i].EffectValue[0];
							MobInfo->TakeDamage(GetAttributesManager()->sBuffTimeInfo[i].EffectValue[0]);
						}
						if (newlp <= 0 || newlp >= 1000000000)
						{
							newlp = 0;
							if (GetIsDead() == false)
							{
								sGU_UPDATE_CHAR_STATE state;

								state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
								state.wOpCode = GU_UPDATE_CHAR_STATE;

								state.handle = GetAttributesManager()->sBuffTimeInfo[i].PlayerHandle;
								state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_FAINTING;
								state.sCharState.sCharStateDetail.sCharStateFainting.byReason = eFAINT_REASON::FAINT_REASON_HIT;

								state.sCharState.sCharStateBase.vCurLoc.x = GetVectorPosition().x;
								state.sCharState.sCharStateBase.vCurLoc.y = GetVectorPosition().y;
								state.sCharState.sCharStateBase.vCurLoc.z = GetVectorPosition().z;
								state.sCharState.sCharStateBase.vCurDir.x = GetVectorOriantation().x;
								state.sCharState.sCharStateBase.vCurDir.y = GetVectorOriantation().y;
								state.sCharState.sCharStateBase.vCurDir.z = GetVectorOriantation().x;
								state.sCharState.sCharStateBase.dwConditionFlag = 0;
								//	res.sCharState.sCharStateBase.bFightMode = false;
								state.sCharState.sCharStateBase.dwStateTime = 2;
															
								SendPacket((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
								GetAttributesManager()->sBuffTimeInfo[i].BuffIsActive = false;
								GetAttributesManager()->sBuffTimeInfo[i].BuffEndTime = INVALID_TBLIDX;
								GetAttributesManager()->sBuffTimeInfo[i].BuffTime = INVALID_TBLIDX;
								GetAttributesManager()->sBuffTimeInfo[i].BuffID = INVALID_TBLIDX;
								GetAttributesManager()->sBuffTimeInfo[i].isAffectPlayer = false;
								GetAttributesManager()->sBuffTimeInfo[i].EffectType = INVALID_TBLIDX;
								GetAttributesManager()->sBuffTimeInfo[i].EffectValue[0] = INVALID_TBLIDX;
								GetAttributesManager()->sBuffTimeInfo[i].isMob = false;
							//	printf("This Mob Have Die \n");
							}

						}

						SendPacket((char*)&afect, sizeof(sGU_EFFECT_AFFECTED));
						SendToPlayerList((char*)&afect, sizeof(sGU_EFFECT_AFFECTED));
						AffectTime = GetTickCount();
					}
				
			}
		}
	}
}
void Player::ExecuteBuffTimmer()
{	
	for (int i = 0; i <= 32; i++)
	{		
		if (GetAttributesManager()->sBuffTimeInfo[i].BuffIsActive == true && GetAttributesManager()->sBuffTimeInfo[i].BuffID != 0 & GetAttributesManager()->sBuffTimeInfo[i].BuffID != INVALID_TBLIDX)
		{				
			DWORD deletionAt1 = GetTickCount() - GetAttributesManager()->sBuffTimeInfo[i].BuffTime;
			if (deletionAt1 >= GetAttributesManager()->sBuffTimeInfo[i].BuffEndTime)
			{				
					sGU_BUFF_DROPPED dropbuff;
					dropbuff.wOpCode = GU_BUFF_DROPPED;
					dropbuff.wPacketSize = sizeof(sGU_BUFF_DROPPED) - 2;

					dropbuff.hHandle = GetAttributesManager()->sBuffTimeInfo[i].PlayerHandle;
					dropbuff.bySourceType = eDBO_OBJECT_SOURCE::DBO_OBJECT_SOURCE_SKILL;
					dropbuff.Slot = 0;
					dropbuff.tblidx = GetAttributesManager()->sBuffTimeInfo[i].BuffID;
					dropbuff.unk1 = 0;
					ExecuteEffectCalculation(dropbuff.tblidx, true);
					ExecuteEffectPlayerCalculation(dropbuff.tblidx, dropbuff.hHandle, true);
					SendPacket((char*)&dropbuff, sizeof(sGU_BUFF_DROPPED));
					SendToPlayerList((char*)&dropbuff, sizeof(sGU_BUFF_DROPPED));

					GetAttributesManager()->sBuffTimeInfo[i].BuffIsActive = false;
					GetAttributesManager()->sBuffTimeInfo[i].BuffEndTime = INVALID_TBLIDX;
					GetAttributesManager()->sBuffTimeInfo[i].BuffTime = INVALID_TBLIDX;
					GetAttributesManager()->sBuffTimeInfo[i].BuffID = INVALID_TBLIDX;
					GetAttributesManager()->sBuffTimeInfo[i].isAffectPlayer = false;
					GetAttributesManager()->sBuffTimeInfo[i].EffectType = INVALID_TBLIDX;
					GetAttributesManager()->sBuffTimeInfo[i].EffectValue[0] = INVALID_TBLIDX;
					GetAttributesManager()->sBuffTimeInfo[i].isMob = false;

					//printf("Time End This Buff Is Removed \n");					
					
					if (GetIsDead() == false)
					{
						sGU_UPDATE_CHAR_STATE state;

						state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
						state.wOpCode = GU_UPDATE_CHAR_STATE;

						state.handle = GetAttributesManager()->sBuffTimeInfo[i].PlayerHandle;
						state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_STANDING;						

						state.sCharState.sCharStateBase.vCurLoc.x = GetVectorPosition().x;
						state.sCharState.sCharStateBase.vCurLoc.y = GetVectorPosition().y;
						state.sCharState.sCharStateBase.vCurLoc.z = GetVectorPosition().z;
						state.sCharState.sCharStateBase.vCurDir.x = GetVectorOriantation().x;
						state.sCharState.sCharStateBase.vCurDir.y = GetVectorOriantation().y;
						state.sCharState.sCharStateBase.vCurDir.z = GetVectorOriantation().x;
						state.sCharState.sCharStateBase.dwConditionFlag = 0;
						//	res.sCharState.sCharStateBase.bFightMode = false;
						state.sCharState.sCharStateBase.dwStateTime = 2;
						state.sCharState.sCharStateBase.isFlying = GetIsFlying();
						SendPacket((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
						SendToPlayerList((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));

					}			
					Mob* MobInfo = static_cast<Mob*>(GetFromList(GetAttributesManager()->sBuffTimeInfo[i].PlayerHandle));
					if (MobInfo != NULL && MobInfo->GetIsDead() == false)
					{
						sGU_UPDATE_CHAR_STATE state;

						state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
						state.wOpCode = GU_UPDATE_CHAR_STATE;

						state.handle = GetAttributesManager()->sBuffTimeInfo[i].PlayerHandle;
						state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_STANDING;

						state.sCharState.sCharStateBase.vCurLoc.x = MobInfo->GetVectorPosition().x;
						state.sCharState.sCharStateBase.vCurLoc.y = MobInfo->GetVectorPosition().y;
						state.sCharState.sCharStateBase.vCurLoc.z = MobInfo->GetVectorPosition().z;
						state.sCharState.sCharStateBase.vCurDir.x = MobInfo->GetVectorOriantation().x;
						state.sCharState.sCharStateBase.vCurDir.y = MobInfo->GetVectorOriantation().y;
						state.sCharState.sCharStateBase.vCurDir.z = MobInfo->GetVectorOriantation().x;
						state.sCharState.sCharStateBase.dwConditionFlag = 0;
					
						state.sCharState.sCharStateBase.isFlying = GetIsFlying();
						state.sCharState.sCharStateBase.dwStateTime = 2;
						MobInfo->SetState(eCHARSTATE::CHARSTATE_STANDING);
						SendPacket((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
						SendToPlayerList((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
					}
			}
		}
	}
}
//Need Delet all that shit and Remake
void Player::HandleFreeBattleRange()
{
	if (GetAttributesManager()->PlayerInFreeBatle == true)
	{
		Player* PlayerInfo = static_cast<Player*>(GetFromList(GetAttributesManager()->FreeBatleChellenger));
		if (PlayerInfo != NULL)
		{
			float dist = NtlGetDistance(GetVectorPosition().x, GetVectorPosition().z, GetAttributesManager()->vRefreeLoc.x, GetAttributesManager()->vRefreeLoc.z);
			float dist1 = NtlGetDistance(PlayerInfo->GetVectorPosition().x, PlayerInfo->GetVectorPosition().z, PlayerInfo->GetAttributesManager()->vRefreeLoc.x, PlayerInfo->GetAttributesManager()->vRefreeLoc.z);
			if (dist >= 50)
			{

				if (GetAttributesManager()->showMensageOut == false && dist >= 50)
				{
					sGU_FREEBATTLE_OUTSIDE_NFY FreeBatleOut;
					FreeBatleOut.wOpCode = GU_FREEBATTLE_OUTSIDE_NFY;
					FreeBatleOut.wPacketSize = sizeof(sGU_FREEBATTLE_OUTSIDE_NFY) - 2;
					GetAttributesManager()->OutBatleTime = GetTickCount();
					GetAttributesManager()->showMensageOut = true;
					SendPacket((char*)&FreeBatleOut, sizeof(sGU_FREEBATTLE_OUTSIDE_NFY));				
				}
				if (GetAttributesManager()->showMensageOut == true && dist >= 50)
				{
					GetAttributesManager()->OutBatleFinalTime = GetTickCount() - GetAttributesManager()->OutBatleTime;
					//printf("Player1 time %d  \n", GetAttributesManager()->OutBatleFinalTime);
					if (GetAttributesManager()->OutBatleFinalTime >= 11000)
					{
						sGU_FREEBATTLE_END_NFY FreebatleEnd;
						FreebatleEnd.wOpCode = GU_FREEBATTLE_END_NFY;
						FreebatleEnd.wPacketSize = sizeof(sGU_FREEBATTLE_END_NFY) - 2;
						FreebatleEnd.byFreeBattleResult = eFREEBATTLE_RESULT::FREEBATTLE_RESULT_LOSE;
						SendPacket((char*)&FreebatleEnd, sizeof(sGU_FREEBATTLE_END_NFY));

						GetAttributesManager()->showMensageOut = false;
						GetAttributesManager()->PlayerInFreeBatle = false;
						GetAttributesManager()->FreeBatleChellenger = INVALID_TBLIDX;

						sGU_FREEBATTLE_END_NFY FreebatleEnd1;
						FreebatleEnd1.wOpCode = GU_FREEBATTLE_END_NFY;
						FreebatleEnd1.wPacketSize = sizeof(sGU_FREEBATTLE_END_NFY) - 2;
						FreebatleEnd1.byFreeBattleResult = eFREEBATTLE_RESULT::FREEBATTLE_RESULT_WIN;
						PlayerInfo->SendPacket((char*)&FreebatleEnd1, sizeof(sGU_FREEBATTLE_END_NFY));

						PlayerInfo->GetAttributesManager()->showMensageOut = false;
						PlayerInfo->GetAttributesManager()->PlayerInFreeBatle = false;
						PlayerInfo->GetAttributesManager()->FreeBatleChellenger = INVALID_TBLIDX;
					}				
				}
			}
			else if(dist < 50 && GetAttributesManager()->showMensageOut == true)
			{
				sGU_FREEBATTLE_INSIDE_NFY FreeBatlein;
				FreeBatlein.wOpCode = GU_FREEBATTLE_INSIDE_NFY;
				FreeBatlein.wPacketSize = sizeof(sGU_FREEBATTLE_INSIDE_NFY) - 2;
				GetAttributesManager()->OutBatleTime = GetTickCount();
				GetAttributesManager()->showMensageOut = false;
				SendPacket((char*)&FreeBatlein, sizeof(sGU_FREEBATTLE_INSIDE_NFY));
			}
			//Player2
			if (dist1 >= 50)
			{

				if (PlayerInfo->GetAttributesManager()->showMensageOut == false && dist1 >= 50)
				{
					sGU_FREEBATTLE_OUTSIDE_NFY FreeBatleOut;
					FreeBatleOut.wOpCode = GU_FREEBATTLE_OUTSIDE_NFY;
					FreeBatleOut.wPacketSize = sizeof(sGU_FREEBATTLE_OUTSIDE_NFY) - 2;
					PlayerInfo->GetAttributesManager()->OutBatleTime = GetTickCount();
					PlayerInfo->GetAttributesManager()->showMensageOut = true;
					PlayerInfo->SendPacket((char*)&FreeBatleOut, sizeof(sGU_FREEBATTLE_OUTSIDE_NFY));
				}
				if (PlayerInfo->GetAttributesManager()->showMensageOut == true && dist1 >= 50)
				{
					PlayerInfo->GetAttributesManager()->OutBatleFinalTime = GetTickCount() - PlayerInfo->GetAttributesManager()->OutBatleTime;
				//	printf("Player2 time %d  \n", PlayerInfo->GetAttributesManager()->OutBatleFinalTime);
					if (PlayerInfo->GetAttributesManager()->OutBatleFinalTime >= 11000)
					{
						sGU_FREEBATTLE_END_NFY FreebatleEnd;
						FreebatleEnd.wOpCode = GU_FREEBATTLE_END_NFY;
						FreebatleEnd.wPacketSize = sizeof(sGU_FREEBATTLE_END_NFY) - 2;
						FreebatleEnd.byFreeBattleResult = eFREEBATTLE_RESULT::FREEBATTLE_RESULT_LOSE;
						PlayerInfo->SendPacket((char*)&FreebatleEnd, sizeof(sGU_FREEBATTLE_END_NFY));

						PlayerInfo->GetAttributesManager()->showMensageOut = false;
						PlayerInfo->GetAttributesManager()->PlayerInFreeBatle = false;
						PlayerInfo->GetAttributesManager()->FreeBatleChellenger = INVALID_TBLIDX;

						sGU_FREEBATTLE_END_NFY FreebatleEnd1;
						FreebatleEnd1.wOpCode = GU_FREEBATTLE_END_NFY;
						FreebatleEnd1.wPacketSize = sizeof(sGU_FREEBATTLE_END_NFY) - 2;
						FreebatleEnd1.byFreeBattleResult = eFREEBATTLE_RESULT::FREEBATTLE_RESULT_WIN;
						SendPacket((char*)&FreebatleEnd1, sizeof(sGU_FREEBATTLE_END_NFY));

						GetAttributesManager()->showMensageOut = false;
						GetAttributesManager()->PlayerInFreeBatle = false;
						GetAttributesManager()->FreeBatleChellenger = INVALID_TBLIDX;
					}
				}
			}
			else if (dist1 < 50 && PlayerInfo->GetAttributesManager()->showMensageOut == true)
			{
				sGU_FREEBATTLE_INSIDE_NFY FreeBatlein;
				FreeBatlein.wOpCode = GU_FREEBATTLE_INSIDE_NFY;
				FreeBatlein.wPacketSize = sizeof(sGU_FREEBATTLE_INSIDE_NFY) - 2;
				PlayerInfo->GetAttributesManager()->OutBatleTime = GetTickCount();
				PlayerInfo->GetAttributesManager()->showMensageOut = false;
				PlayerInfo->SendPacket((char*)&FreeBatlein, sizeof(sGU_FREEBATTLE_INSIDE_NFY));
			}
		}
	}
}
void Player::HandleFreeBattleWinners()
{

}
void Player::UpdateAspectState(BYTE State)
{
	sGU_UPDATE_CHAR_ASPECT_STATE CharAspect;
	CharAspect.wOpCode = GU_UPDATE_CHAR_ASPECT_STATE;
	CharAspect.wPacketSize = sizeof(sGU_UPDATE_CHAR_ASPECT_STATE) - 2;
	CharAspect.handle = GetHandle();
	CharAspect.aspectState.sAspectStateBase.byAspectStateId = GetState()->sCharStateBase.aspectState.sAspectStateBase.byAspectStateId = State;
	CharAspect.aspectState.sAspectStateDetail.sVehicle.bIsEngineOn = GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.bIsEngineOn;
	CharAspect.aspectState.sAspectStateDetail.sVehicle.hVehicleItem = GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.hVehicleItem;
	CharAspect.aspectState.sAspectStateDetail.sVehicle.idVehicleTblidx = GetState()->sCharStateBase.aspectState.sAspectStateDetail.sVehicle.idVehicleTblidx;
	
	SendToPlayerList((char*)&CharAspect, sizeof(sGU_UPDATE_CHAR_ASPECT_STATE));
	SendPacket((char*)&CharAspect, sizeof(sGU_UPDATE_CHAR_ASPECT_STATE));
	
}
//----------------------------------------
//	Generate RP when fight or decrease when not fight
//----------------------------------------
void Player::CalculeRpBall()
{
	bool sendRpBall = false;

	if (isfighting == false)
	{
		if (GetPcProfile()->wCurRP > 0 || GetAttributesManager()->GetNumFilledRpBall() > 0)
		{
			if (rpBallTimer >= CHAR_RP_BALL_UPDATE_INTERVAL)
			{
				int rpCur = GetPcProfile()->wCurRP;
				rpCur -= 5;
				if (rpCur <= 0 && GetAttributesManager()->GetNumFilledRpBall() > 0)
				{
					GetAttributesManager()->SetNumFilledRpBall(-1);
					GetPcProfile()->wCurRP = GetPcProfile()->avatarAttribute.wBaseMaxRP;
					sGU_UPDATE_CHAR_RP_BALL newBall;
					newBall.bDropByTime = true;
					newBall.byCurRPBall = GetAttributesManager()->GetNumFilledRpBall();
					newBall.handle = GetHandle();
					newBall.wOpCode = GU_UPDATE_CHAR_RP_BALL;
					newBall.wPacketSize = sizeof(sGU_UPDATE_CHAR_RP_BALL) - 2;
					SendPacket((char*)&newBall, sizeof(sGU_UPDATE_CHAR_RP_BALL));
					SendToPlayerList((char*)&newBall, sizeof(sGU_UPDATE_CHAR_RP_BALL));
				}
				else
					GetPcProfile()->wCurRP = rpCur;
				sendRpBall = true;
			}
		}
	}
	else
	{
		if (GetPcProfile()->wCurRP < GetPcProfile()->avatarAttribute.wBaseMaxRP
			&& GetAttributesManager()->GetNumFilledRpBall() < GetAttributesManager()->GetNumRpBall())
		{
			GetPcProfile()->wCurRP += 2;
			if (GetPcProfile()->wCurRP >= GetPcProfile()->avatarAttribute.wBaseMaxRP)
			{
				GetAttributesManager()->SetNumFilledRpBall(1);
				GetPcProfile()->wCurRP = 0;
				sGU_UPDATE_CHAR_RP_BALL newBall;
				newBall.bDropByTime = true;
				newBall.byCurRPBall = GetAttributesManager()->GetNumFilledRpBall();
				newBall.handle = GetHandle();
				newBall.wOpCode = GU_UPDATE_CHAR_RP_BALL;
				newBall.wPacketSize = sizeof(sGU_UPDATE_CHAR_RP_BALL) - 2;
				SendPacket((char*)&newBall, sizeof(sGU_UPDATE_CHAR_RP_BALL));
				SendToPlayerList((char*)&newBall, sizeof(sGU_UPDATE_CHAR_RP_BALL));
			}
			sendRpBall = true;
			rpBallTimer = 0;
		}
	}
	if (sendRpBall == true)
	{
		sGU_UPDATE_CHAR_RP RP;

		RP.bHitDelay = false;
		RP.handle = GetHandle();
		RP.wCurRP = GetPcProfile()->wCurRP;
		RP.wMaxRP = GetPcProfile()->avatarAttribute.wBaseMaxRP;
		RP.wOpCode = GU_UPDATE_CHAR_RP;
		RP.wPacketSize = sizeof(sGU_UPDATE_CHAR_RP) - 2;
		SendPacket((char*)&RP, sizeof(sGU_UPDATE_CHAR_RP));
		SendToPlayerList((char*)&RP, sizeof(sGU_UPDATE_CHAR_RP));
	}
}
//----------------------------------------
//	Refresh our current position
//----------------------------------------
void Player::CalculePosition(uint32 _update_diff)
{
	//--------------------------
	//	Do calculation to determine the current player position
	//--------------------------
	BYTE moveDir = GetMoveDirection();
	if (moveDir > AKCORE_STOP && moveDir < AKCORE_COUNT && GetCharEState() == eCHARSTATE::CHARSTATE_MOVING)
	{
		CNtlVector newHeading;
		CNtlVector newPosition;
		// class those function ?
		NtlGetDestination_Keyboard(m_rotation_tmp.x, m_rotation_tmp.z, 8, m_position_tmp.x, m_position_tmp.y, m_position_tmp.z, moveDir, _update_diff, 8, newHeading, newPosition);
		if (newPosition.x != 0 && newPosition.z != 0)
		{
			Relocate(newPosition.x, newPosition.y, newPosition.z, newHeading.x, newHeading.y, newHeading.z);
		}
	}
	else if (GetCharEState() == eCHARSTATE::CHARSTATE_DESTMOVE)
	{
		//.outError("HEY ! DON't YOU FORGOT SOMETHING HERE !?");
		//NtlGetDestination_Mouse();
		CNtlVector newHeading;
		CNtlVector newPosition;
		// class those function ?
		NtlGetDestination_Keyboard(m_rotation_tmp.x, m_rotation_tmp.z, 8, m_position_tmp.x, m_position_tmp.y, m_position_tmp.z, moveDir, _update_diff, 8, newHeading, newPosition);
		if (newPosition.x != 0 && newPosition.z != 0)
		{
			Relocate(newPosition.x, newPosition.y, newPosition.z, newHeading.x, newHeading.y, newHeading.z);
		}
	}	
	else if (GetCharEState() == eCHARSTATE::CHARSTATE_FOLLOWING)
	{
		CNtlVector newHeading;
		CNtlVector newPosition;

		float fDistance = NtlGetDistance(m_position_tmp.x, m_position_tmp.z, GetMoveDestinationVector().x, GetMoveDestinationVector().z);

		NtlGetDestination_Follow(m_rotation_tmp.x, m_rotation_tmp.z, 8, m_position_tmp.x, m_position_tmp.y, m_position_tmp.z, GetMoveDestinationVector().x, GetMoveDestinationVector().y, GetMoveDestinationVector().z, _update_diff, 2, newHeading, newPosition);
		if (newPosition.x != 0 && newPosition.z != 0)
		{
			Relocate(newPosition.x, newPosition.y, newPosition.z, newHeading.x, newHeading.y, newHeading.z);
		}
	}
	else if (GetCharEState() == eCHARSTATE::CHARSTATE_DASH_PASSIVE)
	{
		CNtlVector newHeading;
		CNtlVector newPosition;

		float fDistance = NtlGetDistance(m_position_tmp.x, m_position_tmp.z, GetMoveDestinationVector().x, GetMoveDestinationVector().z);

		NtlGetDestination_Follow(m_rotation_tmp.x, m_rotation_tmp.z, 8, m_position_tmp.x, m_position_tmp.y, m_position_tmp.z, GetMoveDestinationVector().x, GetMoveDestinationVector().y, GetMoveDestinationVector().z, _update_diff, 2, newHeading, newPosition);
		if (newPosition.x != 0 && newPosition.z != 0)
		{
			Relocate(newPosition.x, newPosition.y, newPosition.z, newHeading.x, newHeading.y, newHeading.z);
		}
	}
	else if (GetCharEState() == eCHARSTATE::CHARSTATE_AIR_JUMP)
	{
		CNtlVector newHeading;
		CNtlVector newPosition;

		float fDistance = NtlGetDistance(m_position_tmp.x, m_position_tmp.z, GetMoveDestinationVector().x, GetMoveDestinationVector().z);

		NtlGetDestination_Follow(m_rotation_tmp.x, m_rotation_tmp.z, 8, m_position_tmp.x, m_position_tmp.y, m_position_tmp.z, GetMoveDestinationVector().x, GetMoveDestinationVector().y, GetMoveDestinationVector().z, _update_diff, 2, newHeading, newPosition);
		if (newPosition.x != 0 && newPosition.z != 0)
		{
			Relocate(newPosition.x, newPosition.y, newPosition.z, newHeading.x, newHeading.y, newHeading.z);
		}
	}
	else if (GetCharEState() == eCHARSTATE::CHARSTATE_AIR_DASH_ACCEL)
	{
		CNtlVector newHeading;
		CNtlVector newPosition;

		float fDistance = NtlGetDistance(m_position_tmp.x, m_position_tmp.z, GetMoveDestinationVector().x, GetMoveDestinationVector().z);

		NtlGetDestination_Follow(m_rotation_tmp.x, m_rotation_tmp.z, 8, m_position_tmp.x, m_position_tmp.y, m_position_tmp.z, GetMoveDestinationVector().x, GetMoveDestinationVector().y, GetMoveDestinationVector().z, _update_diff, 2, newHeading, newPosition);
		if (newPosition.x != 0 && newPosition.z != 0)
		{
			Relocate(newPosition.x, newPosition.y, newPosition.z, newHeading.x, newHeading.y, newHeading.z);
		}
	}
}
//----------------------------------------
//	Add a new player to our storage list
//----------------------------------------
void Player::FillList(Object& object)
{
	mutexPlayer.lock();
	objList.insert(std::make_pair(object.GetHandle(), &object));
	mutexPlayer.unlock();
}
//----------------------------------------
//	Remove an Object from our list
//----------------------------------------
void Player::RemoveFromList(Object& player)
{
	mutexPlayer.lock();
	for (auto it = objList.begin(); it != objList.end();)
	{
		if (it->second != NULL)
		{
			if (it->first == player.GetHandle())
			{
				it = objList.erase(it++);
				break;
			}
			else
				it++;
		}
	}
	mutexPlayer.unlock();
}
//----------------------------------------
//	Get a new player from our storage list
//----------------------------------------
Object *Player::GetFromList(uint32 id)
{
	mutexPlayer.lock();
	for (auto it = objList.begin(); it != objList.end();)
	{
		if (it->second != NULL)
		{
			if (it->first == id)
			{
				mutexPlayer.unlock();
				return it->second;
			}
			else
				it++;
		}
	}
	mutexPlayer.unlock();
	return NULL;
}
//----------------------------------------
//	Check if Player X is in list
//----------------------------------------
bool Player::isInList(uint32 id)
{
	mutexPlayer.lock();
	auto it = objList.find(id);
	if (it == objList.end())
	{
		mutexPlayer.unlock();
		return false;
	}
	if (it->second != NULL)
	{
		if (it->second->IsInWorld() == true)
		{
			if (it->second->GetHandle() == id)
			{
				mutexPlayer.unlock();
				return true;
			}
		}
	}
	mutexPlayer.unlock();
	return false;
}
//----------------------------------------
//	Send packet to all player in the list
//----------------------------------------
void Player::SendToPlayerList(char* data, size_t size)
{
	mutexPlayer.lock();
	
	for (auto it = objList.begin(); it != objList.end();)
	{
		if (it->second != NULL)
		{
			if (it->second->GetTypeId() == OBJTYPE_PC)
			{
				Player* plr = static_cast<Player*>(it->second);
				if (plr->IsInWorld() == true && plr->GetSession() != NULL)
				{					
					plr->SendPacket(data, size);						
				}
			}
		}
		it++;
	}
	mutexPlayer.unlock();
}
//----------------------------------------
//	Add a dropped item to our list
//----------------------------------------
void Player::AddDropToList(DroppedObject& drop, HOBJECT dropHandle)
{
	mutexDropList.lock();
	DropList.insert(std::make_pair(dropHandle, &drop));
	mutexDropList.unlock();
}
//----------------------------------------
//	Remove a dropped object from our list
//----------------------------------------
void Player::RemoveDropFromList(HOBJECT handle)
{
	mutexDropList.lock();
	for (auto it = DropList.begin(); it != DropList.end();)
	{
		if (it->second != NULL)
		{
			if (it->first == handle)
			{
				sGU_OBJECT_DESTROY destroy;
				destroy.handle = it->first;
				destroy.wOpCode = GU_OBJECT_DESTROY;
				destroy.wPacketSize = sizeof(sGU_OBJECT_DESTROY) - 2;
				SendPacket((char*)&destroy, sizeof(sGU_OBJECT_DESTROY));

				delete it->second;
				it = DropList.erase(it++);
				break;
			}
			else
				it++;
		}
	}
	mutexDropList.unlock();
}
//----------------------------------------
//	Get the data from object dropped
//	Requiert the dropped handle
//----------------------------------------
DroppedObject *Player::GetPickupData(HOBJECT handle)
{
	mutexDropList.lock();
	for (auto it = DropList.begin(); it != DropList.end(); ++it)
	{
		if (it->second != NULL)
		{
			if (it->first == handle)
			{
				mutexDropList.unlock();
				return it->second;
			}
		}
	}
	mutexDropList.unlock();
	return NULL;
}
//----------------------------------------
//	Perform an auto attack
//----------------------------------------
void Player::Attack()
{
	if (cur_obj_tagert == NULL || cur_obj_tagert->GetTypeId() == OBJTYPE_NPC)
	{
		SetIsFighting(false);
		return;
	}
	if (fightManager.HandleDamage(*this, *cur_obj_tagert) == false)
	{
		SetIsFighting(false);
		return;
	}
	//target->TakeDamage(attackValue);

	/*
		Physical attack	=	thing		Energy Attack	=	thing		Hit Rate	=	thing
		Physical def	=	thing		Energy Def		=	thing		Dodge		=	thing
		Crit physical	=	thing		Crit Energy		=	thing

		Abdominal		=	thing

	*/
}
//----------------------------------------
//Need Change That Shit and Remake
//	Take some damage (need to inheritance that)
//----------------------------------------
void	Player::TakeDamage(uint32 amount)
{
	int32_t newLife = GetPcProfile()->dwCurLP;
	
	newLife -= amount;
	if (newLife <= 0 || newLife > 1000000000)
	{
		if (GetAttributesManager()->PlayerInFreeBatle == true)
		{
			GetAttributesManager()->LoseBatle = true;
			newLife = 1;
		}	
		else
		{
			newLife = 0;
		}
		SetIsFighting(false);		
	}
	GetPcProfile()->dwCurLP = newLife;

	sGU_UPDATE_CHAR_LP updLp;
	updLp.handle = handle;
	updLp.wCurLP = GetPcProfile()->dwCurLP = newLife;
	updLp.wMaxLP = GetPcProfile()->avatarAttribute.wLastMaxLP;
	updLp.wOpCode = GU_UPDATE_CHAR_LP;
	updLp.wPacketSize = sizeof(sGU_UPDATE_CHAR_LP) - 2;

	
	SendPacket((char*)&updLp, sizeof(sGU_UPDATE_CHAR_LP));
	SendToPlayerList((char*)&updLp, sizeof(sGU_UPDATE_CHAR_LP));

	if (GetPcProfile()->dwCurLP == 0)
	{
		sGU_UPDATE_CHAR_STATE state;

		state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
		state.wOpCode = GU_UPDATE_CHAR_STATE;

		state.handle = GetHandle();
		state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_FAINTING;
		state.sCharState.sCharStateDetail.sCharStateFainting.byReason = eFAINT_REASON::FAINT_REASON_HIT;

		state.sCharState.sCharStateBase.vCurLoc.x = GetVectorPosition().x;
		state.sCharState.sCharStateBase.vCurLoc.y = GetVectorPosition().y;
		state.sCharState.sCharStateBase.vCurLoc.z = GetVectorPosition().z;
		state.sCharState.sCharStateBase.vCurDir.x = GetVectorOriantation().x;
		state.sCharState.sCharStateBase.vCurDir.y = GetVectorOriantation().y;
		state.sCharState.sCharStateBase.vCurDir.z = GetVectorOriantation().x;
		state.sCharState.sCharStateBase.dwConditionFlag = 0;
		//	res.sCharState.sCharStateBase.bFightMode = false;
		state.sCharState.sCharStateBase.dwStateTime = 2;
		SetIsDead(true);
		SendPacket((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
		SendToPlayerList((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
	}

	Player* PlayerInfo = static_cast<Player*>(GetFromList(GetAttributesManager()->FreeBatleChellenger));
	if (PlayerInfo != NULL)
	{
		if (GetAttributesManager()->LoseBatle == true)
		{
			sGU_FREEBATTLE_END_NFY FreebatleEnd;
			FreebatleEnd.wOpCode = GU_FREEBATTLE_END_NFY;
			FreebatleEnd.wPacketSize = sizeof(sGU_FREEBATTLE_END_NFY) - 2;
			FreebatleEnd.byFreeBattleResult = eFREEBATTLE_RESULT::FREEBATTLE_RESULT_LOSE;
			SendPacket((char*)&FreebatleEnd, sizeof(sGU_FREEBATTLE_END_NFY));
			GetAttributesManager()->showMensageOut = false;
			GetAttributesManager()->PlayerInFreeBatle = false;
			GetAttributesManager()->FreeBatleChellenger = INVALID_TBLIDX;

			sGU_FREEBATTLE_END_NFY FreebatleEnd1;
			FreebatleEnd1.wOpCode = GU_FREEBATTLE_END_NFY;
			FreebatleEnd1.wPacketSize = sizeof(sGU_FREEBATTLE_END_NFY) - 2;
			FreebatleEnd1.byFreeBattleResult = eFREEBATTLE_RESULT::FREEBATTLE_RESULT_WIN;
			PlayerInfo->SendPacket((char*)&FreebatleEnd1, sizeof(sGU_FREEBATTLE_END_NFY));
			PlayerInfo->GetAttributesManager()->showMensageOut = false;
			PlayerInfo->GetAttributesManager()->PlayerInFreeBatle = false;
			PlayerInfo->GetAttributesManager()->FreeBatleChellenger = INVALID_TBLIDX;
			GetAttributesManager()->LoseBatle = false;
			PlayerInfo->GetAttributesManager()->LoseBatle = false;
		}
		if (PlayerInfo->GetAttributesManager()->LoseBatle == true)
		{
			sGU_FREEBATTLE_END_NFY FreebatleEnd;
			FreebatleEnd.wOpCode = GU_FREEBATTLE_END_NFY;
			FreebatleEnd.wPacketSize = sizeof(sGU_FREEBATTLE_END_NFY) - 2;
			FreebatleEnd.byFreeBattleResult = eFREEBATTLE_RESULT::FREEBATTLE_RESULT_WIN;
			SendPacket((char*)&FreebatleEnd, sizeof(sGU_FREEBATTLE_END_NFY));
			GetAttributesManager()->showMensageOut = false;
			GetAttributesManager()->PlayerInFreeBatle = false;
			GetAttributesManager()->FreeBatleChellenger = INVALID_TBLIDX;

			sGU_FREEBATTLE_END_NFY FreebatleEnd1;
			FreebatleEnd1.wOpCode = GU_FREEBATTLE_END_NFY;
			FreebatleEnd1.wPacketSize = sizeof(sGU_FREEBATTLE_END_NFY) - 2;
			FreebatleEnd1.byFreeBattleResult = eFREEBATTLE_RESULT::FREEBATTLE_RESULT_LOSE;
			PlayerInfo->SendPacket((char*)&FreebatleEnd1, sizeof(sGU_FREEBATTLE_END_NFY));
			PlayerInfo->GetAttributesManager()->showMensageOut = false;
			PlayerInfo->GetAttributesManager()->PlayerInFreeBatle = false;
			PlayerInfo->GetAttributesManager()->FreeBatleChellenger = INVALID_TBLIDX;
			PlayerInfo->GetAttributesManager()->LoseBatle = false;
			GetAttributesManager()->LoseBatle = false;
		}

	}
}
void Player::TakeMobDemage(uint32 amount)
{	
		int32_t newLife = GetPcProfile()->dwCurLP;		
		newLife -= amount;
		if (newLife <= 0 || newLife > 1000000000)
		{					
			SetIsFighting(false);
			newLife = 0;
		}
		GetPcProfile()->dwCurLP = newLife;

		sGU_UPDATE_CHAR_LP updLp;
		updLp.handle = GetHandle();
		updLp.wCurLP = GetPcProfile()->dwCurLP = newLife;
		updLp.wMaxLP = GetPcProfile()->avatarAttribute.wLastMaxLP;
		updLp.wOpCode = GU_UPDATE_CHAR_LP;
		updLp.wPacketSize = sizeof(sGU_UPDATE_CHAR_LP) - 2;

		
		SendPacket((char*)&updLp, sizeof(sGU_UPDATE_CHAR_LP));
		SendToPlayerList((char*)&updLp, sizeof(sGU_UPDATE_CHAR_LP));

		if (GetPcProfile()->dwCurLP == 0)
		{
			sGU_UPDATE_CHAR_STATE state;

			state.wPacketSize = sizeof(sGU_UPDATE_CHAR_STATE) - 2;
			state.wOpCode = GU_UPDATE_CHAR_STATE;

			state.handle = GetHandle();
			state.sCharState.sCharStateBase.byStateID = eCHARSTATE::CHARSTATE_FAINTING;
			state.sCharState.sCharStateDetail.sCharStateFainting.byReason = eFAINT_REASON::FAINT_REASON_HIT;
			
			state.sCharState.sCharStateBase.vCurLoc.x = GetVectorPosition().x;
			state.sCharState.sCharStateBase.vCurLoc.y = GetVectorPosition().y;
			state.sCharState.sCharStateBase.vCurLoc.z = GetVectorPosition().z;
			state.sCharState.sCharStateBase.vCurDir.x = GetVectorOriantation().x;
			state.sCharState.sCharStateBase.vCurDir.y = GetVectorOriantation().y;
			state.sCharState.sCharStateBase.vCurDir.z = GetVectorOriantation().x;
			state.sCharState.sCharStateBase.dwConditionFlag = 0;
			//	res.sCharState.sCharStateBase.bFightMode = false;
			state.sCharState.sCharStateBase.dwStateTime = 2;			
			SetIsDead(true);
			SendPacket((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
			SendToPlayerList((char*)&state, sizeof(sGU_UPDATE_CHAR_STATE));
		}
	
}
//----------------------------------------
//	Get zeny that is in the bank
//  Currently returning a static value
//  Need to get from the Database 
//----------------------------------------

DWORD Player::GetBankMoney()
{

	return 10000U;
}

//----------------------------------------
//	Get some XP from a mob killed
//	Mob reference
//----------------------------------------
void	Player::RewardExpFromMob(MonsterData& data)
{
	if (GetPcProfile()->byLevel < 70)
	{
		DWORD exp = data.Exp;
	/*	int levelDiff = GetPcProfile()->byLevel - data.Level;
		(levelDiff > 0) ? levelDiff *= 1 : levelDiff *= -1;*/
		DWORD bonus = 0;
		sGU_UPDATE_CHAR_EXP expPacket;
		// hard fix
		//exp /= 10;
		if (sWorld.BonusActive == true)
			bonus = exp / 100 * sWorld.BonuxEXP;
		else
			bonus = 0;
		
		expPacket.dwIncreasedExp = exp + bonus;
		expPacket.dwAcquisitionExp = exp;
		expPacket.dwBonusExp = bonus;
		expPacket.wOpCode = GU_UPDATE_CHAR_EXP;
		expPacket.wPacketSize = sizeof(sGU_UPDATE_CHAR_EXP) - 2;
		expPacket.handle = GetHandle();
		GetPcProfile()->dwCurExp += (exp + bonus);
		expPacket.dwCurExp = GetPcProfile()->dwCurExp;

		if (GetPcProfile()->dwCurExp >= GetPcProfile()->dwMaxExpInThisLevel)
		{
			expPacket.dwCurExp = GetPcProfile()->dwCurExp -= GetPcProfile()->dwMaxExpInThisLevel;
			LevelUp();			
		}
		SendPacket((char*)&expPacket, sizeof(sGU_UPDATE_CHAR_EXP));
	}
}
void Player::RewardDropFromMob(MonsterData& data)
{
	for (int i = 0; i <= 30; i++)
	{
		if (GetAttributesManager()->QuestDat[i].count < GetAttributesManager()->QuestDat[i].Maxcount)
		{
			if (GetAttributesManager()->QuestDat[i].MobID == data.MonsterID || GetAttributesManager()->QuestDat[i].MobID +1 == data.MonsterID)
			{
				sLog.outDebug("Count Quest Mob");
				GetAttributesManager()->KillerCount += 1;
				GetAttributesManager()->QuestDat[i].count += 1;
				sGU_QUEST_SVREVT_UPDATE_NFY start;
				start.wOpCode = GU_QUEST_SVREVT_UPDATE_NFY;
				start.wPacketSize = sizeof(sGU_QUEST_SVREVT_UPDATE_NFY) - 2;
				start.tId = GetAttributesManager()->QuestDat[i].QuestID;
				start.tcId = 2;
				start.taId = 3;
				start.bySvrEvtType = 0;
				start.bySlot = 0;
				start.uEvtData.sMobKillCnt.nCurMobCnt = GetAttributesManager()->QuestDat[i].count;
				SendPacket((char*)&start, sizeof(sGU_QUEST_SVREVT_UPDATE_NFY));

			}
		}	
	}	
	DroppedObject *dropped;
	sITEM_TBLDAT *itemSrc = NULL;
	sITEM_PROFILE *Item = NULL;
	//Dragon Ball Event Drop
	int DragonBallDropRate = rand() % 100;
	const int DragonBalls[7] = { 200001,200002,200003,200004,200005,200006,200007 };
	int GetBall = rand() % 7;
	int levelDiff = GetPcProfile()->byLevel - data.Level;
	(levelDiff > 0) ? levelDiff *= 1 : levelDiff *= -1;
	if (sWorld.DragonBallEventa == true && data.ByDagonBall == 1 && levelDiff >= 0 && levelDiff <= 5)
	{
		if (DragonBallDropRate >= 0 && DragonBallDropRate <= 30)
		{
			Item = GetInventoryManager()->GetItemByTblidx(DragonBalls[GetBall]);
			if (Item == NULL)
			{
				dropped = new DroppedObject;
				dropped->droppedTime = GetTickCount();
				dropped->objType = OBJTYPE_DROPITEM;
				dropped->owner = GetHandle();

				dropped->item.wPacketSize = sizeof(Drop) - 2;
				dropped->item.wOpCode = GU_OBJECT_CREATE;

				dropped->item.Handle = sWorld.AcquireItemSerialId();
				dropped->item.Tblidx = DragonBalls[GetBall];

				if ((itemSrc = (sITEM_TBLDAT*)sTBM.GetItemTable()->FindData(dropped->item.Tblidx)) == NULL)
				{
					itemSrc = (sCASHITEM_TBLDAT*)sTBM.GetCashItemTable()->FindData(dropped->item.Tblidx);
				}
				if (itemSrc != NULL)
				{
					dropped->item.Type = OBJTYPE_DROPITEM;
					dropped->item.Grade = 0;
					dropped->item.Rank = itemSrc->eRank;
					dropped->item.BattleAttribute = 0;
					dropped->item.Loc.x = data.curPos.x + rand() % 6;
					dropped->item.Loc.y = data.curPos.y;
					dropped->item.Loc.z = data.curPos.z + rand() % 6;
					dropped->item.IsNew = true;
					dropped->item.NeedToIdentify = false;

					/* Add Drop to list	*/
					SendPacket((char*)&dropped->item, sizeof(Drop));
					AddDropToList(*dropped, dropped->item.Handle);
				}
			}
			if (Item != NULL)
			{
				if (Item->tblidx != DragonBalls[GetBall])
				{
					dropped = new DroppedObject;
					dropped->droppedTime = GetTickCount();
					dropped->objType = OBJTYPE_DROPITEM;
					dropped->owner = GetHandle();

					dropped->item.wPacketSize = sizeof(Drop) - 2;
					dropped->item.wOpCode = GU_OBJECT_CREATE;

					dropped->item.Handle = sWorld.AcquireItemSerialId();
					dropped->item.Tblidx = DragonBalls[GetBall];

					if ((itemSrc = (sITEM_TBLDAT*)sTBM.GetItemTable()->FindData(dropped->item.Tblidx)) == NULL)
					{
						itemSrc = (sCASHITEM_TBLDAT*)sTBM.GetCashItemTable()->FindData(dropped->item.Tblidx);
					}
					if (itemSrc != NULL)
					{
						dropped->item.Type = OBJTYPE_DROPITEM;
						dropped->item.Grade = 0;
						dropped->item.Rank = itemSrc->eRank;
						dropped->item.BattleAttribute = 0;
						dropped->item.Loc.x = data.curPos.x + rand() % 6;
						dropped->item.Loc.y = data.curPos.y;
						dropped->item.Loc.z = data.curPos.z + rand() % 6;
						dropped->item.IsNew = true;
						dropped->item.NeedToIdentify = false;

						/* Add Drop to list	*/
						SendPacket((char*)&dropped->item, sizeof(Drop));
						AddDropToList(*dropped, dropped->item.Handle);
					}
				}
			}
		}
	}
	

	//////
	/* /!\ NEED FUNCTION TO CREATE CORRECT DROP	/!\	*/
	/* ITEM	*/
	int DropAmount = rand() % 10;
	for (int i = 0; i <= DropAmount; i++)
	{
		dropped = new DroppedObject;
		dropped->droppedTime = GetTickCount();
		dropped->objType = OBJTYPE_DROPITEM;
		dropped->owner = GetHandle();

		dropped->item.wPacketSize = sizeof(Drop) - 2;
		dropped->item.wOpCode = GU_OBJECT_CREATE;

		dropped->item.Handle = sWorld.AcquireItemSerialId();
		dropped->item.Tblidx = 11170019 + rand() % 261 + 1;

		if ((itemSrc = (sITEM_TBLDAT*)sTBM.GetItemTable()->FindData(dropped->item.Tblidx)) == NULL)
		{
			itemSrc = (sCASHITEM_TBLDAT*)sTBM.GetCashItemTable()->FindData(dropped->item.Tblidx);
		}
		if (itemSrc != NULL)
		{
			dropped->item.Type = OBJTYPE_DROPITEM;
			dropped->item.Grade = 0;
			dropped->item.Rank = itemSrc->eRank;
			dropped->item.BattleAttribute = 0;
			dropped->item.Loc.x = data.curPos.x + rand() % 6;
			dropped->item.Loc.y = data.curPos.y;
			dropped->item.Loc.z = data.curPos.z + rand() % 6;
			dropped->item.IsNew = true;
			dropped->item.NeedToIdentify = false;

			/* Add Drop to list	*/
			SendPacket((char*)&dropped->item, sizeof(Drop));
			AddDropToList(*dropped, dropped->item.Handle);
		}
	}
	/*	ZENNY	*/
	dropped = new DroppedObject;
	dropped->zenny.dwZenny = data.Drop_Zenny;
	if (dropped->zenny.dwZenny == 0)
	{
		dropped->zenny.dwZenny = 15;
	}

	dropped->objType = OBJTYPE_DROPMONEY;
	dropped->droppedTime = GetTickCount();
	dropped->owner = GetHandle();

	dropped->zenny.wPacketSize = sizeof(Dropzenny) - 2;
	dropped->zenny.wOpCode = GU_OBJECT_CREATE;

	dropped->zenny.Loc.x = data.curPos.x + rand() % 6;
	dropped->zenny.Loc.y = data.curPos.y;
	dropped->zenny.Loc.z = data.curPos.z + rand() % 6;
	dropped->zenny.Dir.x = data.Spawn_Dir.x;
	dropped->zenny.Dir.y = data.Spawn_Dir.y;
	dropped->zenny.Dir.z = data.Spawn_Dir.z;

	dropped->zenny.Handle = sWorld.AcquireItemSerialId();
	dropped->zenny.IsNew = false;
	dropped->zenny.Type = OBJTYPE_DROPMONEY;
	SendPacket((char*)&dropped->zenny, sizeof(Dropzenny));
	AddDropToList(*dropped, dropped->zenny.Handle);

}
//----------------------------------------
//	Levelup our player
//----------------------------------------
void	Player::LevelUp()
{
	if (GetPcProfile()->byLevel < 70)
	{
		sGU_UPDATE_CHAR_LEVEL levelPacket;
		sEXP_TBLDAT *pTblExp = (sEXP_TBLDAT*)sTBM.GetExpTable()->FindData(GetPcProfile()->byLevel + 1);

		// fill new level
		GetPcProfile()->byLevel += 1;
		levelPacket.byCurLevel = GetPcProfile()->byLevel;
		levelPacket.byPrevLevel = GetPcProfile()->byLevel - 1;
		levelPacket.dwMaxExpInThisLevel = GetPcProfile()->dwMaxExpInThisLevel = pTblExp->dwNeed_Exp;
		levelPacket.handle = GetHandle();
		levelPacket.wOpCode = GU_UPDATE_CHAR_LEVEL;
		levelPacket.wPacketSize = sizeof(sGU_UPDATE_CHAR_LEVEL) - 2;
		SendPacket((char*)&levelPacket, sizeof(sGU_UPDATE_CHAR_LEVEL));
		SendToPlayerList((char*)&levelPacket, sizeof(sGU_UPDATE_CHAR_LEVEL));
		// calculate our new state
	//	characterManager.UpdateLevelUpAtributes();
	//	characterManager.SendRpBallInformation();
		// fill lp ep to max and calculate RP -
		GetPcProfile()->dwCurLP = GetPcProfile()->avatarAttribute.wBaseMaxLP;
		GetPcProfile()->wCurEP = GetPcProfile()->avatarAttribute.wBaseMaxEP;

		// give 1 sp pouint
		sGU_UPDATE_CHAR_SP spPoint;
		GetPcProfile()->dwSpPoint += 1;
		sDB.UpdateSPPoint(GetPcProfile()->dwSpPoint, GetCharacterID());
		spPoint.dwSpPoint = GetPcProfile()->dwSpPoint;
		spPoint.wOpCode = GU_UPDATE_CHAR_SP;
		spPoint.wPacketSize = sizeof(sGU_UPDATE_CHAR_SP) - 2;
		SendPacket((char*)&spPoint, sizeof(sGU_UPDATE_CHAR_SP));

		// save new level to db
		sDB.UpdatePlayerLevel(GetPcProfile()->byLevel, GetCharacterID(), GetPcProfile()->dwCurExp);
	}
}
void Player::HandleEvent()
{
	//Dragon Balls Event
	sGU_DRAGONBALL_SCHEDULE_INFO res;

	res.bIsAlive = true;
	res.byEventType = 1;
	res.byTermType = 1;
	res.dwMainTerm = 1;
	res.dwSubTerm = 1;
	res.nEndTime = 100;
	res.nStartTime = 1;
	res.wOpCode = GU_DRAGONBALL_SCHEDULE_INFO;
	res.wPacketSize = sizeof(sGU_DRAGONBALL_SCHEDULE_INFO) - 2;

	sWorld.SendToAll((char*)&res, sizeof(sGU_DRAGONBALL_SCHEDULE_INFO));
	
	sGU_SYSTEM_DISPLAY_TEXT sNotice;
	sNotice.wOpCode = GU_SYSTEM_DISPLAY_TEXT;
	sNotice.wPacketSize = sizeof(sGU_SYSTEM_DISPLAY_TEXT) - 2;
	sNotice.byDisplayType = 3;
	wcscpy_s(sNotice.awchMessage, BUDOKAI_MAX_NOTICE_LENGTH, L" Dragon Ball Hunt Start");
	wcscpy_s(sNotice.awGMChar, MAX_SIZE_CHAR_NAME_UNICODE, (L"System"));
	sNotice.wMessageLengthInUnicode = 50;
	sWorld.SendToAll((char*)&sNotice, sizeof(sGU_SYSTEM_DISPLAY_TEXT));
	sWorld.DragonBallEventa = true;
	

}
void	Player::LevelUpByComand(int Level)
{

	sGU_UPDATE_CHAR_LEVEL levelPacket;
	sEXP_TBLDAT *pTblExp = (sEXP_TBLDAT*)sTBM.GetExpTable()->FindData(Level);

	// fill new level
	GetPcProfile()->byLevel = Level;
	levelPacket.byCurLevel = Level;
	levelPacket.byPrevLevel = Level - 1;
	levelPacket.dwMaxExpInThisLevel = GetPcProfile()->dwMaxExpInThisLevel = pTblExp->dwNeed_Exp;
	levelPacket.handle = GetHandle();
	levelPacket.wOpCode = GU_UPDATE_CHAR_LEVEL;
	levelPacket.wPacketSize = sizeof(sGU_UPDATE_CHAR_LEVEL) - 2;
	SendPacket((char*)&levelPacket, sizeof(sGU_UPDATE_CHAR_LEVEL));
	SendToPlayerList((char*)&levelPacket, sizeof(sGU_UPDATE_CHAR_LEVEL));
	// calculate our new state
	//characterManager.UpdateLevelUpAtributes();
	//characterManager.SendRpBallInformation();
	// fill lp ep to max and calculate RP -
	GetPcProfile()->dwCurLP = GetPcProfile()->avatarAttribute.wBaseMaxLP;
	GetPcProfile()->wCurEP = GetPcProfile()->avatarAttribute.wBaseMaxEP;

	// give 1 sp pouint
	sGU_UPDATE_CHAR_SP spPoint;


	spPoint.dwSpPoint = Level - 1;
	GetPcProfile()->dwSpPoint = spPoint.dwSpPoint;
	spPoint.wOpCode = GU_UPDATE_CHAR_SP;
	spPoint.wPacketSize = sizeof(sGU_UPDATE_CHAR_SP) - 2;
	SendPacket((char*)&spPoint, sizeof(sGU_UPDATE_CHAR_SP));

	// save new level to db
	sDB.UpdatePlayerLevel(GetPcProfile()->byLevel, GetCharacterID(), GetPcProfile()->dwCurExp);
	sDB.UpdateSPPoint(GetPcProfile()->dwSpPoint, GetCharacterID());

}
void Player::SetSpeed(int sppeed)
{
	sGU_UPDATE_CHAR_SPEED speed;
	speed.wOpCode = GU_UPDATE_CHAR_SPEED;
	speed.wPacketSize = sizeof(sGU_UPDATE_CHAR_SPEED) - 2;

	speed.handle = GetHandle();
	speed.fLastWalkingSpeed = sppeed* .50;
	speed.fLastRunningSpeed = sppeed;
	speed.fLastFlySpeed = sppeed* .50;
	speed.fLastFlyBoostSpeed = sppeed;
	speed.fLastFlySpeed2 = sppeed* .50;
	speed.fLastFlyBoostSpeed2 = sppeed;

	SendPacket((char*)&speed, sizeof(sGU_UPDATE_CHAR_SPEED));
	SendToPlayerList((char*)&speed, sizeof(sGU_UPDATE_CHAR_SPEED));
}
//Comand
void Player::ConvertClass(int Class, HOBJECT Target)
{	
	if (Target == NULL || Target == GetHandle())
	{
		sGU_CHAR_CONVERT_CLASS clase;
		clase.wOpCode = GU_CHAR_CONVERT_CLASS;
		clase.wPacketSize = sizeof(sGU_CHAR_CONVERT_CLASS) - 2;

		clase.hTarget = GetHandle();
		clase.byClass = Class;
		if (Class == 0)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_HUMAN_FIGHTER);
		}
		if (Class == 1)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_HUMAN_MYSTIC);
		}
		if (Class == 2)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_HUMAN_ENGINEER);
		}
		if (Class == 3)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_NAMEK_FIGHTER);
		}
		if (Class == 4)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_NAMEK_MYSTIC);
		}
		if (Class == 5)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_MIGHTY_MAJIN);
		}
		if (Class == 6)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_WONDER_MAJIN);
		}
		if (Class == 7)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_STREET_FIGHTER);			
				m_session->LearnMestrySkill(729991);
		}
		if (Class == 8)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_SWORD_MASTER);			
				m_session->LearnMestrySkill(829991);
		}
		if (Class == 9)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_CRANE_ROSHI);			
				m_session->LearnMestrySkill(929991);
		}
		if (Class == 10)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_TURTLE_ROSHI);			
				m_session->LearnMestrySkill(1029991);
		}
		if (Class == 11)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_GUN_MANIA);
		}
		if (Class == 12)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_MECH_MANIA);
		}
		if (Class == 13)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_DARK_WARRIOR);			
				m_session->LearnMestrySkill(1329991);
		}
		if (Class == 14)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_SHADOW_KNIGHT);			
				m_session->LearnMestrySkill(1429991);
		}
		if (Class == 15)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_DENDEN_HEALER);			
				m_session->LearnMestrySkill(1529991);
		}
		if (Class == 16)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_POCO_SUMMONER);			
				m_session->LearnMestrySkill(1629991);
		}
		if (Class == 17)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_ULTI_MA);			
				m_session->LearnMestrySkill(1729991);
		}
		if (Class == 18)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_GRAND_MA);			
				m_session->LearnMestrySkill(1829991);
		}
		if (Class == 19)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_PLAS_MA);			
				m_session->LearnMestrySkill(1929991);
		}
		if (Class == 20)
		{
			SetMyClass(ePC_CLASS::PC_CLASS_KAR_MA);		
				m_session->LearnMestrySkill(2029991);
		}

		SendPacket((char*)&clase, sizeof(sGU_CHAR_CONVERT_CLASS));
		SendToPlayerList((char*)&clase, sizeof(sGU_CHAR_CONVERT_CLASS));
	}	
	else
	{
		Player* PlayerInfo = static_cast<Player*>(GetFromList(Target));
		if (PlayerInfo != NULL)
		{
			sGU_CHAR_CONVERT_CLASS clase;
			clase.wOpCode = GU_CHAR_CONVERT_CLASS;
			clase.wPacketSize = sizeof(sGU_CHAR_CONVERT_CLASS) - 2;

			clase.hTarget = PlayerInfo->GetHandle();
			clase.byClass = Class;
			if (Class == 0)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_HUMAN_FIGHTER);
			}
			if (Class == 1)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_HUMAN_MYSTIC);
			}
			if (Class == 2)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_HUMAN_ENGINEER);
			}
			if (Class == 3)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_NAMEK_FIGHTER);
			}
			if (Class == 4)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_NAMEK_MYSTIC);
			}
			if (Class == 5)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_MIGHTY_MAJIN);
			}
			if (Class == 6)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_WONDER_MAJIN);
			}
			if (Class == 7)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_STREET_FIGHTER);
				PlayerInfo->m_session->LearnMestrySkill(729991);
			}
			if (Class == 8)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_SWORD_MASTER);
				PlayerInfo->m_session->LearnMestrySkill(829991);
			}
			if (Class == 9)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_CRANE_ROSHI);
				PlayerInfo->m_session->LearnMestrySkill(929991);
			}
			if (Class == 10)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_TURTLE_ROSHI);
				PlayerInfo->m_session->LearnMestrySkill(1029991);
			}
			if (Class == 11)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_GUN_MANIA);
			}
			if (Class == 12)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_MECH_MANIA);
			}
			if (Class == 13)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_DARK_WARRIOR);
				PlayerInfo->m_session->LearnMestrySkill(1329991);
			}
			if (Class == 14)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_SHADOW_KNIGHT);
				PlayerInfo->m_session->LearnMestrySkill(1429991);
			}
			if (Class == 15)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_DENDEN_HEALER);
				PlayerInfo->m_session->LearnMestrySkill(1529991);
			}
			if (Class == 16)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_POCO_SUMMONER);
				PlayerInfo->m_session->LearnMestrySkill(1629991);
			}
			if (Class == 17)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_ULTI_MA);
				PlayerInfo->m_session->LearnMestrySkill(1729991);
			}
			if (Class == 18)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_GRAND_MA);
				PlayerInfo->m_session->LearnMestrySkill(1829991);
			}
			if (Class == 19)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_PLAS_MA);
				PlayerInfo->m_session->LearnMestrySkill(1929991);
			}
			if (Class == 20)
			{
				PlayerInfo->SetMyClass(ePC_CLASS::PC_CLASS_KAR_MA);
				PlayerInfo->m_session->LearnMestrySkill(2029991);
			}

			PlayerInfo->SendPacket((char*)&clase, sizeof(sGU_CHAR_CONVERT_CLASS));
			PlayerInfo->SendToPlayerList((char*)&clase, sizeof(sGU_CHAR_CONVERT_CLASS));
		}
	}

}
//Comand
void Player::ConvertAdult(int Adult)
{
	sGU_CHILD_ADULT_SWITCHED_NFY IsAdult;
	IsAdult.wOpCode = GU_CHILD_ADULT_SWITCHED_NFY;
	IsAdult.wPacketSize = sizeof(sGU_CHILD_ADULT_SWITCHED_NFY) - 2;

	IsAdult.hSubject = GetHandle();
	IsAdult.bIsAdult = Adult;
	IsAdult.wResultCode = GAME_SUCCESS;	
	SendPacket((char*)&IsAdult, sizeof(sGU_CHILD_ADULT_SWITCHED_NFY));
	SendToPlayerList((char*)&IsAdult, sizeof(sGU_CHILD_ADULT_SWITCHED_NFY));
	GetPcProfile()->bIsAdult = Adult;

}
//Comand
void Player::SendUpdateSize(int Size)
{
	sGU_CHANGED_CHAR_SIZE_NFY CharSize;
	CharSize.wOpCode = GU_CHANGED_CHAR_SIZE_NFY;
	CharSize.wPacketSize = sizeof(sGU_CHANGED_CHAR_SIZE_NFY) - 2;

	CharSize.handle = GetHandle();
	CharSize.size = Size;


	SendPacket((char*)&CharSize, sizeof(sGU_CHANGED_CHAR_SIZE_NFY));
	SendToPlayerList((char*)&CharSize, sizeof(sGU_CHANGED_CHAR_SIZE_NFY));

}
//Comand
void Player::TeleportByCommand(TBLIDX WorldID)
{
	sGU_CHAR_TELEPORT_RES teleport;

	sWORLD_TBLDAT *world = (sWORLD_TBLDAT*)sTBM.GetWorldTable()->FindData(WorldID);
	teleport.wResultCode = GAME_SUCCESS;
	teleport.wOpCode = GU_CHAR_TELEPORT_RES;
	teleport.wPacketSize = sizeof(sGU_CHAR_TELEPORT_RES) - 2;

	if (world != NULL)
	{
		if (WorldID != GetWorldID())
		{
			teleport.bIsToMoveAnotherServer = true;
			teleport.sWorldInfo.worldID = world->tblidx;
			teleport.sWorldInfo.tblidx = world->tblidx;
			teleport.sWorldInfo.sRuleInfo.byRuleType = world->byWorldRuleType;
			GetState()->sCharStateDetail.sCharStateTeleporting.byTeleportType = eTELEPORT_TYPE::TELEPORT_TYPE_WORLD_MOVE;
		}
		else
		{
			teleport.bIsToMoveAnotherServer = false;

		}
		teleport.vNewDir.x = world->vStart1Dir.x;
		teleport.vNewDir.y = world->vStart1Dir.y;
		teleport.vNewDir.z = world->vStart1Dir.z;
		if (WorldID == 211000) 
		{
			teleport.vNewLoc.x = 635;
			teleport.vNewLoc.y = 100;
			teleport.vNewLoc.z = 1670;

		}
		else
		{
			teleport.vNewLoc.x = world->vStart1Loc.x;
			teleport.vNewLoc.y = world->vStart1Loc.y;
			teleport.vNewLoc.z = world->vStart1Loc.z;
		}
		

		SetState(eCHARSTATE::CHARSTATE_TELEPORTING);


		SetWorldID(world->tblidx);
		SetWorldTableID(world->tblidx);
		Relocate(teleport.vNewLoc.x, teleport.vNewLoc.y, teleport.vNewLoc.z, teleport.vNewDir.x, teleport.vNewDir.y, teleport.vNewDir.z);


		SendPacket((char*)&teleport, sizeof(sGU_CHAR_TELEPORT_RES));
	}
	//mutexPlayer.lock();
	for (auto it = objList.begin(); it != objList.end();)
	{
		if (it->second != NULL)
		{
			if (it->second->GetTypeId() == OBJTYPE_PC)
			{
				Player* plr = static_cast<Player*>(it->second);
				if (plr->IsInWorld() == true && plr->GetSession() != NULL)
				{				
						sLog.outDetail("Invited Now");
						sGU_CHAR_TELEPORT_RES teleport;

						sWORLD_TBLDAT *world = (sWORLD_TBLDAT*)sTBM.GetWorldTable()->FindData(WorldID);
						teleport.wResultCode = GAME_SUCCESS;
						teleport.wOpCode = GU_CHAR_TELEPORT_RES;
						teleport.wPacketSize = sizeof(sGU_CHAR_TELEPORT_RES) - 2;

						if (world != NULL)
						{
							if (WorldID != plr->GetWorldID())
							{
								teleport.bIsToMoveAnotherServer = true;
								teleport.sWorldInfo.worldID = world->tblidx;
								teleport.sWorldInfo.tblidx = world->tblidx;
								teleport.sWorldInfo.sRuleInfo.byRuleType = world->byWorldRuleType;
								plr->GetState()->sCharStateDetail.sCharStateTeleporting.byTeleportType = eTELEPORT_TYPE::TELEPORT_TYPE_WORLD_MOVE;
							}
							else
							{
								teleport.bIsToMoveAnotherServer = false;

							}
							teleport.vNewDir.x = world->vStart1Dir.x;
							teleport.vNewDir.y = world->vStart1Dir.y;
							teleport.vNewDir.z = world->vStart1Dir.z;
							if (WorldID == 211000)
							{
								teleport.vNewLoc.x = 635;
								teleport.vNewLoc.y = 100;
								teleport.vNewLoc.z = 1670;

							}
							else
							{
								teleport.vNewLoc.x = world->vStart1Loc.x;
								teleport.vNewLoc.y = world->vStart1Loc.y;
								teleport.vNewLoc.z = world->vStart1Loc.z;
							}


							plr->SetState(eCHARSTATE::CHARSTATE_TELEPORTING);


							plr->SetWorldID(world->tblidx);
							plr->SetWorldTableID(world->tblidx);
							plr->Relocate(teleport.vNewLoc.x, teleport.vNewLoc.y, teleport.vNewLoc.z, teleport.vNewDir.x, teleport.vNewDir.y, teleport.vNewDir.z);


							plr->SendPacket((char*)&teleport, sizeof(sGU_CHAR_TELEPORT_RES));

						}
				}
			}
		}
		it++;
	}
	//mutexPlayer.unlock();
}
void Player::SendAddTitle(int TitleID)
{
	sGU_CHARTITLE_ADD title;

	title.wOpCode = GU_CHARTITLE_ADD;
	title.wPacketSize = sizeof(sGU_CHARTITLE_ADD) - 2;
	title.ResultCode = GAME_SUCCESS;
	title.TitleID = TitleID;
	sDB.AddTitle(charid, title.TitleID);
	SendPacket((char*)&title, sizeof(sGU_CHARTITLE_ADD));
}
DWORD Player::GetClassFlag(int Class, int ItemType) const
{

	if (Class == ePC_CLASS::PC_CLASS_HUMAN_FIGHTER)
	{
		if(ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return  PC_CLASS_FLAG_HUMAN_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_GLOVE)
			return  PC_CLASS_FLAG_MIX;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3)
			return  PC_CLASS_FLAG_FULLMIX;
		else
			return  PC_CLASS_FLAG_HUMAN_FIGHTER;
	}
	else if (Class == ePC_CLASS::PC_CLASS_HUMAN_MYSTIC)
	{
		if (ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return  PC_CLASS_FLAG_NAMEK_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_GLOVE)
			return  PC_CLASS_FLAG_MIX;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3)
			return  PC_CLASS_FLAG_FULLMIX;
		else
			return  PC_CLASS_FLAG_HUMAN_MYSTIC;
	}	
	else if (Class == ePC_CLASS::PC_CLASS_NAMEK_FIGHTER)
	{
		if (ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return  PC_CLASS_FLAG_NAMEK_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_GLOVE)
			return  PC_CLASS_FLAG_MIX;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3)
			return  PC_CLASS_FLAG_FULLMIX;
		else
			return  PC_CLASS_FLAG_NAMEK_FIGHTER;
	}
	else if (Class == ePC_CLASS::PC_CLASS_NAMEK_MYSTIC)
	{
		if (ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return  PC_CLASS_FLAG_NAMEK_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_STAFF)
			return  PC_CLASS_FLAG_NAMEK_MYSTIC;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3)
			return  PC_CLASS_FLAG_FULLMIX;
		else
			return  PC_CLASS_FLAG_NAMEK_MYSTIC;
	}
	else if (Class == ePC_CLASS::PC_CLASS_MIGHTY_MAJIN)
	{
		if (ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return PC_CLASS_FLAG_MAJIN_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_GLOVE)
			return PC_CLASS_FLAG_MIX;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3)
			return  PC_CLASS_FLAG_FULLMIX;
		else
			return   PC_CLASS_FLAG_MIGHTY_MAJIN;
	}
	else if (Class == ePC_CLASS::PC_CLASS_WONDER_MAJIN)
	{
		if (ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return PC_CLASS_FLAG_MAJIN_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_GLOVE)
			return  PC_CLASS_FLAG_MIX;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3)
			return  PC_CLASS_FLAG_FULLMIX;
		else
			return   PC_CLASS_FLAG_WONDER_MAJIN;
	}
	else if (Class == ePC_CLASS::PC_CLASS_STREET_FIGHTER)
	{
		if (ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return  PC_CLASS_FLAG_HUMAN_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_GLOVE)
			return  PC_CLASS_FLAG_MIX;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3)
			return  PC_CLASS_FLAG_FULLMIX;
		else
			return  PC_CLASS_FLAG_STREET_FIGHTER;
	}
	else if (Class == ePC_CLASS::PC_CLASS_SWORD_MASTER)
	{
		if (ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return PC_CLASS_FLAG_HUMAN_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_GLOVE)
			return  PC_CLASS_FLAG_MIX;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3)
			return  PC_CLASS_FLAG_FULLMIX;
		else
			return PC_CLASS_FLAG_SWORD_MASTER;
	}
	else if (Class == ePC_CLASS::PC_CLASS_CRANE_ROSHI)
	{
		if (ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return  PC_CLASS_FLAG_HUMAN_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_GLOVE)
			return  PC_CLASS_FLAG_MIX;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3)
			return  PC_CLASS_FLAG_FULLMIX;
		else
			return PC_CLASS_FLAG_CRANE_ROSHI;
	}
	else if (Class == ePC_CLASS::PC_CLASS_TURTLE_ROSHI)
	{
		if (ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return PC_CLASS_FLAG_HUMAN_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_GLOVE)
			return  PC_CLASS_FLAG_MIX;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3)
			return  PC_CLASS_FLAG_FULLMIX;
		else
			return  PC_CLASS_FLAG_TURTLE_ROSHI;
	}	
	else if (Class == ePC_CLASS::PC_CLASS_DARK_WARRIOR)
	{
		if (ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return  PC_CLASS_FLAG_NAMEK_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_GLOVE)
			return  PC_CLASS_FLAG_MIX;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3)
			return  PC_CLASS_FLAG_FULLMIX;
		else
			return PC_CLASS_FLAG_DARK_WARRIOR;
	}
	else if (Class == ePC_CLASS::PC_CLASS_SHADOW_KNIGHT)
	{
		if (ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return  PC_CLASS_FLAG_NAMEK_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_GLOVE)
			return  PC_CLASS_FLAG_MIX;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3 && ItemType <= eITEM_TYPE::ITEM_TYPE_RING)
			return  PC_CLASS_FLAG_FULLMIX;
		else
			return PC_CLASS_FLAG_SHADOW_KNIGHT;
	}
	else if (Class == ePC_CLASS::PC_CLASS_DENDEN_HEALER)
	{
		if (ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return  PC_CLASS_FLAG_NAMEK_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_STAFF)
			return  PC_CLASS_FLAG_MIX;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3)
			return  PC_CLASS_FLAG_FULLMIX;
		else 
			return  PC_CLASS_FLAG_DENDEN_HEALER;
	}
	else if (Class == ePC_CLASS::PC_CLASS_POCO_SUMMONER)
	{
		if (ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return PC_CLASS_FLAG_NAMEK_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_STAFF)
			return PC_CLASS_FLAG_MIX;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3)
			return  PC_CLASS_FLAG_FULLMIX;
		else 
			return  PC_CLASS_FLAG_POCO_SUMMONER;
	}
	else if (Class == ePC_CLASS::PC_CLASS_GRAND_MA)
	{
		if (ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return  PC_CLASS_FLAG_NAMEK_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_GLOVE)
			return PC_CLASS_FLAG_MIX;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3)
			return  PC_CLASS_FLAG_FULLMIX;
		else 
			return  PC_CLASS_FLAG_GRAND_MA;
	}
	else if (Class == ePC_CLASS::PC_CLASS_ULTI_MA)
	{
		if (ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return  PC_CLASS_FLAG_NAMEK_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_GLOVE)
			return  PC_CLASS_FLAG_MIX;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3)
			return  PC_CLASS_FLAG_FULLMIX;
		else 
			return  PC_CLASS_FLAG_ULTI_MA;
	}	
	else if (Class == ePC_CLASS::PC_CLASS_PLAS_MA)
	{
		if (ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return PC_CLASS_FLAG_NAMEK_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_GLOVE)
			return  PC_CLASS_FLAG_MIX;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3)
			return  PC_CLASS_FLAG_FULLMIX;
		else 
			return  PC_CLASS_FLAG_PLAS_MA;
	}
	else if (Class == ePC_CLASS::PC_CLASS_KAR_MA)
	{
		if (ItemType == eITEM_TYPE::ITEM_TYPE_JACKET || ItemType == eITEM_TYPE::ITEM_TYPE_PANTS || ItemType == eITEM_TYPE::ITEM_TYPE_BOOTS)
			return  PC_CLASS_FLAG_NAMEK_MIX;
		else if (ItemType == eITEM_TYPE::ITEM_TYPE_GLOVE)
			return  PC_CLASS_FLAG_MIX;
		else if (ItemType >= eITEM_TYPE::ITEM_TYPE_COSTUME && ItemType <= eITEM_TYPE::ITEM_TYPE_ACCESSORY3)
			return  PC_CLASS_FLAG_FULLMIX;
		else
			return  PC_CLASS_FLAG_KAR_MA;
	}		
}
void Player::SetGSHandle()
{
	sql::ResultSet* result = sDB.executes("UPDATE Characters SET GSHandle = %d where characterID = %d", GetHandle(), charid);
}