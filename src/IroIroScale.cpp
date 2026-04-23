#include "PCH.h"
#include "IroIroScale.h"
#include "Setting.h"

namespace KMCCT {
	const float MAX_RATIO = 100.0f;
	const float MIN_RATIO = 1.0f;
	RE::BGSPerk* critical_damage_re = nullptr;
	RE::BGSPerk* iro_iro_scale_destruction = nullptr;
	RE::BGSPerk* iro_iro_scale_restoration = nullptr;
	RE::BGSPerk* iro_iro_scale_illusion = nullptr;

	std::vector<RE::Effect*> GetEquippedWeaponEffects(RE::InventoryEntryData* a_entry, RE::TESObjectWEAP* a_weapon) {
		RE::EnchantmentItem* enchantment = nullptr;

		// プレイヤー付呪を優先
		if (a_entry->extraLists) {
			for (auto* x_list : *a_entry->extraLists) {
				auto* x_ench = x_list->GetByType<RE::ExtraEnchantment>();
				if (x_ench && x_ench->enchantment) {
					enchantment = x_ench->enchantment;
					break;
				}
			}
		}

		// なければ武器自体の付呪
		if (!enchantment) {
			enchantment = a_weapon->formEnchanting;
		}

		std::vector<RE::Effect*> effects;
		if (enchantment) {
			for (auto* effect : enchantment->effects) {
				if (effect) effects.push_back(effect);
			}
		}
		return effects;
	}

	std::vector<RE::ActorValue> GetPriorityMagicSchools(const std::vector<RE::Effect*>& a_effects) {
		std::vector<RE::ActorValue> foundSchools;
		bool has_destruction = false;
		bool has_restoration = false;
		bool has_illusion = false;

		for (auto* effect : a_effects) {
			if (!effect || !effect->baseEffect) continue;
			RE::ActorValue skill = effect->baseEffect->GetMagickSkill();

			if (skill == RE::ActorValue::kIllusion) has_illusion = true;
			else if (skill == RE::ActorValue::kDestruction) has_destruction = true;
			else if (skill == RE::ActorValue::kRestoration) has_restoration = true;
		}

		if (has_illusion) foundSchools.push_back(RE::ActorValue::kIllusion);
		if (has_destruction) foundSchools.push_back(RE::ActorValue::kDestruction);
		if (has_restoration) foundSchools.push_back(RE::ActorValue::kRestoration);

		return foundSchools;
	}

	void IroIroScale::Init() {
		auto* handler = RE::TESDataHandler::GetSingleton();
		if (!handler) return;

		bool use_critical = KMCCT::g_config.enable_critical_damage_scaling;
		bool use_magic = KMCCT::g_config.enable_magic_power_scaling;

		if (!use_critical && !use_magic) {
			SKSE::log::info("All scaling features disabled in config.");
			return;
		}

		if (use_critical) {
			critical_damage_re = (RE::BGSPerk*)handler->LookupForm(0x800, "IroIroScale.esp");
			if (!critical_damage_re) {
				SKSE::log::error("Critical damage perk not found.");
				use_critical = false;
			}
		}

		if (use_magic) {
			iro_iro_scale_destruction = (RE::BGSPerk*)handler->LookupForm(0x802, "IroIroScale.esp");
			iro_iro_scale_restoration = (RE::BGSPerk*)handler->LookupForm(0x803, "IroIroScale.esp"); // FormID 0x803? 変性なら0x803、回復なら...? 
			iro_iro_scale_illusion = (RE::BGSPerk*)handler->LookupForm(0x804, "IroIroScale.esp");

			if (!iro_iro_scale_destruction || !iro_iro_scale_restoration || !iro_iro_scale_illusion) {
				SKSE::log::error("One or more magic perks not found.");
				use_magic = false;
			}
		}

		if (use_critical || use_magic) {
			SKSE::log::info("Starting monitor loop...");
			GetSingleton().StartMonitoring();
		}
	}

	bool HasSchool(const std::vector<RE::ActorValue>& a_schools, RE::ActorValue a_target) {
		return std::find(a_schools.begin(), a_schools.end(), a_target) != a_schools.end();
	}

	void IroIroScale::MonitorLoop() {
		SKSE::log::info("Monitoring thread started.");

		while (_running) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

			auto player = RE::PlayerCharacter::GetSingleton();
			if (!player || !player->IsHandleValid() || !player->Is3DLoaded()) continue;

			auto pcell = player->GetParentCell();

			if (pcell == nullptr) {
				// そもそもプレイヤーがセルにアタッチされてないならそれは
				// ゲームタイトルかセーブデータが読み込まれてない状態
				continue;
			}

			// SkyPatcherで付与するようにする
			if (!is_already_init) {
				if (g_config.enable_critical_damage_scaling) {
					if (!player->HasPerk(critical_damage_re)) {
						SKSE::log::error("[CritDamage]SkyPatcher is not installed. Failed to assign the mod-derived perk to the player. Terminating the loop.");
						break;
					}
				}

				if (g_config.enable_magic_power_scaling) {
					if (!player->HasPerk(iro_iro_scale_destruction)) {
						SKSE::log::error("[Destruction]SkyPatcher is not installed. Failed to assign the mod-derived perk to the player. Terminating the loop.");
						break;
					}

					if (!player->HasPerk(iro_iro_scale_restoration)) {
						SKSE::log::error("[Alteration]SkyPatcher is not installed. Failed to assign the mod-derived perk to the player. Terminating the loop.");
						break;
					}

					if (!player->HasPerk(iro_iro_scale_illusion)) {
						SKSE::log::error("[Illusion]SkyPatcher is not installed. Failed to assign the mod-derived perk to the player. Terminating the loop.");
						break;
					}
				}
				is_already_init = true;
				SKSE::log::info("Player perk confirmed. Monitor active.");
			}

			auto entry = player->GetEquippedEntryData(false);

			if (!entry) {
				entry = player->GetEquippedEntryData(true);
			}

			if (entry && entry->object) {

				auto weapon = entry->object->As<RE::TESObjectWEAP>();

				if (weapon) {
					float base_damage = weapon->GetAttackDamage();
					float current_damage = player->GetDamage(entry);
					float ratio = (base_damage > 0.0f) ? (current_damage / base_damage) : 1.0f;

					// どれだけ上昇しても10000倍以上にはしないようにする
					// こっち側で指数になると微妙なので
					float safe_ratio = std::clamp(ratio, MIN_RATIO, MAX_RATIO);
					if (g_config.enable_critical_damage_scaling) {
						UpdateCriticalDamageMultiplier(safe_ratio);
					}

					if (g_config.enable_magic_power_scaling) {
						auto effects = GetEquippedWeaponEffects(entry, weapon);
						auto schools = GetPriorityMagicSchools(effects);

						auto GetSafeDeflatedRatio = [&](float a_ratio, float a_deflate) {
							float deflated = a_ratio * a_deflate;
							if (deflated < 1.0f) {
								return 1.0f;
							}
							return deflated;
						};


						UpdateMagicDamageMultiplier(iro_iro_scale_destruction,
							HasSchool(schools, RE::ActorValue::kDestruction) ? GetSafeDeflatedRatio(safe_ratio, g_config.destruction_magnitude_multiplier) : 1.0f);

						UpdateMagicDamageMultiplier(iro_iro_scale_illusion,
							HasSchool(schools, RE::ActorValue::kIllusion) ? GetSafeDeflatedRatio(safe_ratio, g_config.illusion_magnitude_multiplier) : 1.0f);

						UpdateMagicDamageMultiplier(iro_iro_scale_restoration,
							HasSchool(schools, RE::ActorValue::kRestoration) ? GetSafeDeflatedRatio(safe_ratio, g_config.restoration_magnitude_multiplier) : 1.0f);
					}
				}
			}
			else {
				// 素手
				if(!g_config.unarmed_perk_mult){
					UpdateCriticalDamageMultiplier(1.0);
				}
			}

		}
	}

	void IroIroScale::UpdateCriticalDamageMultiplier(float new_ratio) {
		for (auto entry : critical_damage_re->perkEntries) {
			if (entry && entry->GetType() == RE::PERK_ENTRY_TYPE::kEntryPoint) {

				auto entry_point = static_cast<RE::BGSEntryPointPerkEntry*>(entry);
				if (entry_point->entryData.entryPoint == RE::BGSEntryPoint::ENTRY_POINT::kCalculateMyCriticalHitDamage) {

					auto one_value_data = static_cast<RE::BGSEntryPointFunctionDataOneValue*>(entry_point->functionData);

					if (one_value_data) {
						auto before_value = one_value_data->data;
						one_value_data->data = new_ratio;

						//SKSE::log::info("Critical damage now: {} to: {}", before_value, safe_ratio);
					}
				}
			}
		}
	}

	void IroIroScale::UpdateMagicDamageMultiplier(RE::BGSPerk* a_perk, float new_ratio) {
		if (!a_perk) return;

		for (auto entry : a_perk->perkEntries) {
			if (entry && entry->GetType() == RE::PERK_ENTRY_TYPE::kEntryPoint) {
				auto entry_point = static_cast<RE::BGSEntryPointPerkEntry*>(entry);
				if (entry_point->entryData.entryPoint == RE::BGSEntryPoint::ENTRY_POINT::kModSpellMagnitude) {
					auto one_value_data = static_cast<RE::BGSEntryPointFunctionDataOneValue*>(entry_point->functionData);
					if (one_value_data) {
						one_value_data->data = new_ratio;
					}
				}
			}
		}
	}

}