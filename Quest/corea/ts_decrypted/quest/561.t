CNtlTSTrigger
{
	sm = 1;
	sq = 1;
	rq = 0;
	tid = 561;
	title = 56102;

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
		CDboTSContProposal
		{
			cancellnk = 253;
			cid = 1;
			cont = 56108;
			gtype = 3;
			oklnk = 2;
			area = 56101;
			goal = 56104;
			sort = 56105;
			prelnk = "0;";
			ds = 1;
			grade = 132203;
			rwd = 100;
			title = 56102;
		}
		CDboTSContReward
		{
			canclnk = 255;
			cid = 100;
			desc = 56114;
			nextlnk = 101;
			rwdtbl = 56101;
			rwdtype = 0;
			ltime = -1;
			prelnk = "3;";
			usetbl = 1;

			CDboTSClickNPC
			{
				npcidx = 4141107;
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
			stdiag = 56107;
			nolnk = 253;
			rm = 0;
			yeslnk = 1;

			CDboTSCheckPCRace
			{
				raceflg = 4;
			}
			CDboTSClickNPC
			{
				npcidx = 4141107;
			}
			CDboTSCheckLvl
			{
				maxlvl = 28;
				minlvl = 20;
			}
		}
		CDboTSContGAct
		{
			cid = 2;
			elnk = 252;
			nextlnk = 3;
			prelnk = "1;";

			CDboTSActNPCConv
			{
				conv = 56109;
				ctype = 1;
				idx = 4141107;
				taid = 2;
			}
			CDboTSActRegQInfo
			{
				cont = 56108;
				m1fx = "6754.000000";
				area = 56101;
				goal = 56104;
				m0fz = "98.000000";
				m0widx = 7;
				m1fy = "0.000000";
				m1ttip = 56116;
				sort = 56105;
				stype = 1;
				m0fy = "0.000000";
				m1fz = "-4270.000000";
				grade = 132203;
				m0fx = "294.000000";
				m0ttip = 56115;
				m1widx = 1;
				rwd = 100;
				taid = 1;
				title = 56102;
				gtype = 3;
			}
			CDboTSActSToCEvt
			{
				apptype = 0;
				cnt1 = 0;
				eitype = 0;
				idx0 = 2133102;
				cnt0 = 20;
				cnt2 = 0;
				ectype = -1;
				etype = 0;
				idx1 = -1;
				taid = 3;
				esctype = 0;
				idx2 = -1;
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
