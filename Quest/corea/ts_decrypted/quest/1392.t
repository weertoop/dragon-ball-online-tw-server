CNtlTSTrigger
{
	sm = 1;
	sq = 1;
	rq = 0;
	tid = 1392;
	title = 139202;

	CNtlTSGroup
	{
		gid = 0;

		CDboTSContGAct
		{
			cid = 101;
			elnk = 255;
			nextlnk = 254;
			prelnk = "100;";

			CDboTSActSToCEvt
			{
				apptype = 0;
				cnt1 = 0;
				eitype = 0;
				idx0 = -1;
				cnt0 = 0;
				cnt2 = 0;
				ectype = -1;
				etype = 1;
				idx1 = -1;
				taid = 1;
				esctype = 0;
				idx2 = -1;
			}
		}
		CDboTSContGAct
		{
			cid = 252;
			elnk = 255;
			nextlnk = 253;
			prelnk = "2;";
		}
		CDboTSContEnd
		{
			cid = 254;
			prelnk = "101;";
			type = 1;
		}
		CDboTSContGAct
		{
			cid = 2;
			elnk = 252;
			nextlnk = 3;
			prelnk = "1;";

			CDboTSActNPCConv
			{
				conv = 139209;
				ctype = 1;
				idx = 8755301;
				taid = 2;
			}
			CDboTSActRegQInfo
			{
				cont = 139208;
				gtype = 2;
				area = 139201;
				goal = 139204;
				m0fz = "3773.000000";
				m0widx = 1;
				sort = 139205;
				stype = 2;
				m0fy = "0.000000";
				grade = 132203;
				m0fx = "3525.000000";
				m0ttip = 139215;
				rwd = 100;
				taid = 1;
				title = 139202;
			}
			CDboTSActSToCEvt
			{
				apptype = 0;
				cnt1 = 0;
				eitype = 0;
				idx0 = 4812101;
				cnt0 = 32;
				cnt2 = 0;
				ectype = -1;
				etype = 0;
				idx1 = -1;
				taid = 3;
				esctype = 0;
				idx2 = -1;
			}
		}
		CDboTSContProposal
		{
			cancellnk = 253;
			cid = 1;
			cont = 139208;
			gtype = 2;
			oklnk = 2;
			area = 139201;
			goal = 139204;
			sort = 139205;
			prelnk = "0;";
			ds = 1;
			grade = 132203;
			rwd = 100;
			title = 139202;
		}
		CDboTSContReward
		{
			canclnk = 255;
			cid = 100;
			desc = 139214;
			nextlnk = 101;
			rwdtbl = 139201;
			rwdtype = 0;
			ltime = -1;
			prelnk = "3;";
			usetbl = 1;

			CDboTSClickNPC
			{
				npcidx = 8755301;
			}
		}
		CDboTSContEnd
		{
			cid = 253;
			prelnk = "0;252;1;";
			type = 0;
		}
		CDboTSContStart
		{
			cid = 0;
			stdiag = 139207;
			nolnk = 253;
			rm = 0;
			yeslnk = 1;

			CDboTSClickNPC
			{
				npcidx = 8755301;
			}
			CDboTSCheckLvl
			{
				maxlvl = 54;
				minlvl = 46;
			}
			CDboTSCheckClrQst
			{
				and = "1391;";
			}
		}
		CDboTSContGCond
		{
			cid = 3;
			prelnk = "2;";
			nolnk = 255;
			rm = 0;
			yeslnk = 100;

			CDboTSCheckSToCEvt
			{
				itype = 0;
			}
		}
	}
}
