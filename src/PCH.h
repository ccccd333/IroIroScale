#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
using namespace std::literals;

//we are not making nasa sw, so stfu
#pragma warning( disable : 4100 ) 
#pragma warning( disable : 4244 )

using namespace std::literals;

#define PAPYRUSFUNCHANDLE RE::StaticFunctionTag*
#define PAPYRUSUNHOOKFPSALL 0

#define ROUND(x) std::lround(x)

#define USETASK 1

//1=ON,0=OFF
#define LOGGING 1


#define SINGLETONHEADER(cname)                          \
        public:                                         \
            cname(cname &) = delete;                    \
            void operator=(const cname &) = delete;     \
            static cname* GetSingleton();               \
        protected:                                      \
            cname(){}                                   \
            static cname* _this;

#define SINGLETONBODY(cname)                            \
        cname * cname::_this = new cname;               \
        cname * cname::GetSingleton(){return _this;}
