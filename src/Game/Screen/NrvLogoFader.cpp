#include "Game/Screen/LogoFader.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"

namespace NrvLogoFader {
    LogoFaderNrvBlank::LogoFaderNrvBlank() {}
    LogoFaderNrvDisplay::LogoFaderNrvDisplay() {}
    LogoFaderNrvFadeIn::LogoFaderNrvFadeIn() {}
    LogoFaderNrvFadeOut::LogoFaderNrvFadeOut() {}
    void LogoFaderNrvFadeOut::execute(Spine* spine) const {
        LogoFader* host = static_cast<LogoFader*>(spine->mExecutor);
        host->exeFadeOut();
    }
    void LogoFaderNrvFadeIn::execute(Spine* spine) const {
        LogoFader* host = static_cast<LogoFader*>(spine->mExecutor);
        host->exeFadeIn();
    }
    void LogoFaderNrvDisplay::execute(Spine* spine) const {
        LogoFader* host = static_cast<LogoFader*>(spine->mExecutor);
        if (MR::isFirstStep(host)) host->mRate = 0.0f;
    }
    void LogoFaderNrvBlank::execute(Spine* spine) const {
        LogoFader* host = static_cast<LogoFader*>(spine->mExecutor);
        if (MR::isFirstStep(host)) host->mRate = 1.0f;
    }
}
