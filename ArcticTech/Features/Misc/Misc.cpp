#include "Misc.h"
#include "../../Utils/Utils.h"
#include "../../SDK/Interfaces.h"
#include "../../SDK/Config.h"
#include "../../SDK/Globals.h"

void Miscelleaneus::Clantag()
{
	static auto removed = false;

	if (config.misc.miscellaneous.clantag->get())
	{
		auto nci = EngineClient->GetNetChannelInfo();

		if (!nci)
			return;

		static auto time = -1;

		auto ticks = TIME_TO_TICKS(nci->GetAvgLatency(FLOW_OUTGOING)) + (float)GlobalVars->tickcount; //-V807
		auto intervals = 0.2f / GlobalVars->interval_per_tick;

		auto main_time = (int)(ticks / intervals) % 28;

		if (main_time != time && !ClientState->m_nChokedCommands)
		{
			auto tag = ("");

			switch (main_time)
			{
			case 0: tag = (""); break;
			case 1: tag = ("a"); break;
			case 2: tag = ("ar"); break;
			case 3: tag = ("arc");break;
			case 4: tag = ("arct");break;
			case 5: tag = ("arcti");break;
			case 6: tag = ("arctic");break;
			case 7: tag = ("arctict");break;
			case 8: tag = ("arcticte");break;
			case 9: tag = ("arctictec");break;
			case 10: tag = ("arctictech");break;
			case 11: tag = ("arctictech");break;
			case 12: tag = ("arctictech");break;
			case 13: tag = ("arctictech");break;
			case 14: tag = ("arctictec");break;
			case 15: tag = ("arcticte");break;
			case 16: tag = ("arctict");break;
			case 17: tag = ("arctic");break;
			case 18: tag = ("arcti");break;
			case 19: tag = ("arct");break;
			case 20: tag = ("arc");break;
			case 21: tag = ("ar");break;
			case 22: tag = ("a");break;
			case 23: tag = ("");break;
			}

			Utils::SetClantag(tag);
			time = main_time;
		}

		removed = false;
	}
}