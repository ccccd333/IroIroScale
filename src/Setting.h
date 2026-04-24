#pragma once
namespace KMCCT {
    // İ’è‚ğ•Û‚·‚é\‘¢‘Ì
    struct ModConfig {
        bool enable_critical_damage_scaling = true;
        bool enable_magic_power_scaling = true;
        bool unarmed_perk_mult = true;
        float illusion_magnitude_multiplier = 1.0f;
        float destruction_magnitude_multiplier = 1.0f;
        float restoration_magnitude_multiplier = 1.0f;
        float double_enchant_multiplier = 1.0f;
        float triple_enchant_multiplier = 1.0f;
    };

    inline ModConfig g_config;

    void LoadConfig();
}