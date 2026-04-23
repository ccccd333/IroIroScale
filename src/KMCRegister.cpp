#include "IroIroScale.h"
#include "Setting.h"

namespace KMCCT {

    void OnMessageReceived(SKSE::MessagingInterface::Message* a_msg) {
        switch (a_msg->type) {
            case SKSE::MessagingInterface::kPostLoad: 

                // 事前
                LoadConfig();
                break;
            case SKSE::MessagingInterface::kDataLoaded:

                // form idを含む解決
                IroIroScale::Init();

                break;
            case SKSE::MessagingInterface::kPreLoadGame:  // set reload flag, so we can prevent in papyrus calls of
                                                          // native function untill view get reset by invoking _reset
                // ゲームリロード
                IroIroScale::GetSingleton().is_already_init = false;
                break;
            case SKSE::MessagingInterface::kPostLoadGame:  // for loading existing game
            case SKSE::MessagingInterface::kSaveGame:
                
                                
                break;
        }
    }

}