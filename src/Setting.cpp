#include "Setting.h"
#include <iostream>
#include <yaml-cpp/yaml.h>

namespace KMCCT {
    void LoadConfig() {
        try {
            // ファイルパスの指定（通常は SKSE/Plugins 配下）
            const std::string path = "Data/SKSE/Plugins/IroIroScale.yaml";
            YAML::Node config = YAML::LoadFile(path);

            if (config["Settings"]) {
                auto settings = config["Settings"];

                if (settings["Weapons"]) {
                    auto weapons = settings["Weapons"];
                    g_config.enable_critical_damage_scaling = weapons["EnableCriticalDamageScaling"].as<bool>(true);
                    g_config.enable_magic_power_scaling = weapons["EnableMagicPowerScaling"].as<bool>(true);

                    if (weapons["Enchant"]) {
                        auto enchant = weapons["Enchant"];
                        g_config.destruction_magnitude_multiplier = enchant["DestructionMagnitudeMultiplier"].as<float>(1.0f);
                        g_config.illusion_magnitude_multiplier = enchant["IllusionMagnitudeMultiplier"].as<float>(1.0f);
                        g_config.restoration_magnitude_multiplier = enchant["RestorationMagnitudeMultiplier"].as<float>(1.0f);

                        g_config.double_enchant_multiplier = enchant["DoubleEnchantMultiplier"].as<float>(1.0f);
                        g_config.triple_enchant_multiplier = enchant["TripleEnchantMultiplier"].as<float>(1.0f);

                    }
                }

                if (settings["Unarmed"]) {
                    g_config.unarmed_perk_mult = settings["Unarmed"]["EnablePerkMultiplier"].as<bool>(true);
                }
            }
            SKSE::log::info("Config loaded successfully.");
        }
        catch (const std::exception& e) {
            SKSE::log::error("Failed to load config: {}", e.what());
        }
    }
}