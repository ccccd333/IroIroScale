#pragma once

#include "thread_pool.h"

namespace KMCCT {
    class IroIroScale {
    public:
        static IroIroScale& GetSingleton() {
            static IroIroScale instance;
            return instance;
        }

        ~IroIroScale() {
            _running = false;
         
        }

        static void Init();

        void MonitorLoop();

        // ゲームロード時に触るけど、ロード時はセルにアタッチされないのでatomicいらんかった
        bool is_already_init = false;

    private:
        // どうせSkyrimはGPUがやばくてCPUはスレッドかコアが駄々余りなのでスレッドで
        IroIroScale() : _pool(1) {}


        void StartMonitoring() {
            _pool.submit([this] { MonitorLoop(); });
        }

        void UpdateCriticalDamageMultiplier(float new_ratio);
        void UpdateMagicDamageMultiplier(RE::BGSPerk* a_perk, float new_ratio);

        ThreadPoolExecutor _pool;
        std::atomic<bool> _running{ true };
    };

}