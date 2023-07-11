#include "NetMessages.h"

#include "../Utils/Utils.h"
#include "../Utils/Console.h"
#include "Memory.h"

#include "Globals.h"

CNetMessages* NetMessages = new CNetMessages;


//#define DEBUG_PRINT_VOICEDATA


CCLCMsg_VoiceData_t::CCLCMsg_VoiceData_t() {
	static auto init_fn = reinterpret_cast<CCLCMsg_VoiceData_t* (__thiscall*)(CCLCMsg_VoiceData_t*)>(Memory->ToAbsolute((uintptr_t)Utils::PatternScan("engine.dll", "E8 ? ? ? ? 56 8D 84 24 ? ? ? ? 50 8D 4C 24 28", 0x1)));

	init_fn(this);
}

void CCLCMsg_VoiceData_t::set_data(void* data, int length) {
	static auto set_data_fn = reinterpret_cast<void(__thiscall*)(void*, void*, size_t)>(Memory->ToAbsolute((uintptr_t)Utils::PatternScan("engine.dll", "E8 ? ? ? ? 83 4C 24 ? ? 83 7C 24", 0x1)));

	set_data_fn(reinterpret_cast<void*>((uintptr_t)this + 0x8), data, length);
}

void CNetMessages::SendNetMessage(void* data, int length) {
	CCLCMsg_VoiceData_t msg;

	const int new_size = length + 4;

	void* formatted_data = new char[new_size];
	*(int*)formatted_data = NET_ARCTIC_CODE;
	std::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(formatted_data) + 0x4), data, length);

	msg.has_bits() = 0xFFFFFFFF;
	msg.format() = 0;

	msg.set_data(formatted_data, new_size);

	//player_info_t pinfo;
	//if (EngineClient->GetPlayerInfo(EngineClient->GetLocalPlayer(), &pinfo)) {
	//	msg.xuid() = pinfo.steamID64;
	//	msg.has_bits() |= VoiceData_Has::Xuid;
	//}

	INetChannel* netChan = ClientState->m_NetChannel;

	if (netChan) {
		netChan->SendNetMsg(&msg, false, false);
	}

	delete[] formatted_data;
}

bool CNetMessages::OnVoiceDataRecieved(const CSVCMsg_VoiceData& msg) {
	if (msg.client + 1 == EngineClient->GetLocalPlayer())
		return true;

	for (auto handler : m_voiceDataCallbacks)
		handler(msg);

#ifdef DEBUG_PRINT_VOICEDATA
	if (msg.voice_data->size() == 0) {
		CBasePlayer* player = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(msg.client + 1));

		if (player)
			Console->Log(std::format("recieved msg from {} [format: {}] [xuid_low: {}] [xuid_high: {}] [seq: {}] [sect: {}] [uso: {}]", player->GetName(), msg.format, msg.xuid_low, msg.xuid_high, msg.sequence_bytes, msg.section_number, msg.uncompressed_sample_offset));
	}
#endif

	if (msg.voice_data->size() < 4) {
		return false;
	}

	void* raw_data = msg.voice_data->data();

	if (*(int*)raw_data != NET_ARCTIC_CODE)
		return false;

	int data_size = msg.voice_data->size() - 4;
	void* data = new char[data_size];

	memcpy(data, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(raw_data) + 4), data_size);

	for (auto handler : m_arcticDataCallbacks)
		handler(data, data_size);

	delete[] data;
}