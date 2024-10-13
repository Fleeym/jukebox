#include "Geode/binding/FMODAudioEngine.hpp"
#include "Geode/modify/FMODAudioEngine.hpp"  // IWYU pragma: keep

#include "../managers/nong_manager.hpp"

using namespace jukebox;

class $modify(FMODAudioEngine) {
    void queueStartMusic(gd::string audioFilename, float p1, float p2, float p3,
                         bool p4, int ms, int p6, int p7, int p8, int p9,
                         bool p10, int p11, bool p12) {
        if (NongManager::get()->m_currentlyPreparingNong) {
            int additionalOffset = NongManager::get()
                                       ->m_currentlyPreparingNong.value()
                                       ->active()
                                       ->metadata()
                                       ->startOffset;
            FMODAudioEngine::queueStartMusic(audioFilename, p1, p2, p3, p4,
                                             ms + additionalOffset, p6, p7, p8,
                                             p9, p10, p11, p12);
        } else {
            FMODAudioEngine::queueStartMusic(audioFilename, p1, p2, p3, p4, ms,
                                             p6, p7, p8, p9, p10, p11, p12);
        }
    }

    void setMusicTimeMS(unsigned int ms, bool p1, int channel) {
        if (NongManager::get()->m_currentlyPreparingNong) {
            int additionalOffset = NongManager::get()
                                       ->m_currentlyPreparingNong.value()
                                       ->active()
                                       ->metadata()
                                       ->startOffset;
            FMODAudioEngine::setMusicTimeMS(ms + additionalOffset, p1, channel);
        } else {
            FMODAudioEngine::setMusicTimeMS(ms, p1, channel);
        }
    }
};
