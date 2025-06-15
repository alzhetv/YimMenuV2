#include "core/commands/LoopedCommand.hpp"
#include "game/backend/Self.hpp"
#include "game/gta/Natives.hpp"
#include "game/gta/ScriptLocal.hpp"
#include "game/gta/Stats.hpp"
#include "core/commands/ListCommand.hpp"
#include "game/backend/Tunables.hpp"

#include <set>


namespace YimMenu::Features
{
	class BypassCasinoBans : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;
		virtual void OnTick() override
		{
			static Tunable addSpins("VC_LUCKY_WHEEL_ADDITIONAL_SPINS_ENABLE"_J);
			static Tunable spinsPerDay("VC_LUCKY_WHEEL_NUM_SPINS_PER_DAY"_J);

			// Bypass Casino Slots and Tables Ban/Cooldown
			Stats::SetInt("MPPLY_CASINO_CHIPS_WON_GD", 0);
			Stats::SetInt("MPPLY_CASINO_CHIPS_WONTIM", 0);
			Stats::SetInt("MPPLY_CASINO_GMBLNG_GD", 0);
			Stats::SetInt("MPPLY_CASINO_BAN_TIME", 0);
			Stats::SetInt("MPPLY_CASINO_CHIPS_PURTIM", 0);
			Stats::SetInt("MPPLY_CASINO_CHIPS_PUR_GD", 0);
			Stats::SetInt("MPPLY_CASINO_CHIPS_SOLD", 0);
			Stats::SetInt("MPPLY_CASINO_CHIPS_SELTIM", 0);

			if (addSpins.IsReady() && spinsPerDay.IsReady())
			{
				addSpins.Set(1);
				spinsPerDay.Set(99);
			}
		}
		virtual void OnDisable() override
		{
			// This may need some values, but will work as is for now.
		}
	};
	class CasinoManipulateRigSlotMachines : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

		int slots_random_results_table = 1348;
		std::set<int> slots_blacklist = {9, 21, 22, 87, 152};

		virtual void OnTick() override
		{
			if (Scripts::SafeToModifyFreemodeBroadcastGlobals() && SCRIPT::GET_NUMBER_OF_THREADS_RUNNING_THE_SCRIPT_WITH_THIS_HASH("casino_slots"_J))
			{
				Player casinoSlotsScriptHostPlayer = NETWORK::NETWORK_GET_HOST_OF_SCRIPT("casino_slots", -1, 0);
				auto casinoSlotsScriptHostPlayerId = casinoSlotsScriptHostPlayer.GetId();
				auto selfPlayerId = Self::GetPlayer().GetId();
				if (casinoSlotsScriptHostPlayerId != selfPlayerId)
				{
					Scripts::ForceScriptHost(Scripts::FindScriptThread("casino_slots"_J));
				}

				bool needs_run = false;
				for (int slots_iter = 3; slots_iter <= 196; ++slots_iter)
				{
					if (!slots_blacklist.contains(slots_iter))
					{
						if (*ScriptLocal("casino_slots"_J, slots_random_results_table + slots_iter).As<int*>() != 6)
						{
							needs_run = true;
						}
					}
				}
				if (needs_run)
				{
					for (int slots_iter = 3; slots_iter <= 196; ++slots_iter)
					{
						if (!slots_blacklist.contains(slots_iter))
						{
							int slot_result = 6;
							*ScriptLocal("casino_slots"_J, slots_random_results_table + slots_iter).As<int*>() = slot_result;
						}
					}
				}
			}
		}

		virtual void OnDisable() override
		{
			if (Scripts::SafeToModifyFreemodeBroadcastGlobals() && SCRIPT::GET_NUMBER_OF_THREADS_RUNNING_THE_SCRIPT_WITH_THIS_HASH("casino_slots"_J))
			{
				Player casinoSlotsScriptHostPlayer = NETWORK::NETWORK_GET_HOST_OF_SCRIPT("casino_slots", -1, 0);
				auto casinoSlotsScriptHostPlayerId = casinoSlotsScriptHostPlayer.GetId();
				auto selfPlayerId = Self::GetPlayer().GetId();
				if (casinoSlotsScriptHostPlayerId != selfPlayerId)
				{
					Scripts::ForceScriptHost(Scripts::FindScriptThread("casino_slots"_J));
				}

				for (int slots_iter = 3; slots_iter <= 196; ++slots_iter)
				{
					if (!slots_blacklist.contains(slots_iter))
					{
						int slot_result = 6;
						std::srand(static_cast<unsigned int>(std::time(0)) + slots_iter);
						slot_result = std::rand() % 7; // Generates a pseudo random number between 0 and 7
						*ScriptLocal("casino_slots"_J, slots_random_results_table + slots_iter).As<int*>() = slot_result;
					}
				}
			}
		}
	};

	// To come in next feature update
	//static CasinoAutoSpin _CasinoAutoSpin{"casinoautospin", "Auto Spin Slots", "Automatically spins slot machine when seated."};
	static BypassCasinoBans _BypassCasinoBans{"bypasscasinobans", "Bypass Casino Ban", "SEVERELY RISKY! Bypasses the Casino Ban and cooldowns for everything (wheel/slots/tables/cashier)"};
	static CasinoManipulateRigSlotMachines _CasinoManipulateRigSlotMachines{"casinomanipulaterigslotmachines", "Manipulate Rig Slot Machines", "Lets you win the Rig Slot Machines every time"};
}