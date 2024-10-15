#include "Geode/cocos/base_nodes/CCNode.h"

#include "Geode/cocos/cocoa/CCGeometry.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "index.hpp"

using namespace geode::prelude;
using namespace jukebox::index;

namespace jukebox {

class IndexSongCell : public CCNode {
protected:
    IndexSongMetadata* m_song = nullptr;
    
    CCNode* m_songInfoNode = nullptr;
    CCLabelBMFont* m_songNameLabel = nullptr;
    CCLabelBMFont* m_artistLabel = nullptr;
    CCLabelBMFont* m_indexNameLabel = nullptr;

    bool init(IndexSongMetadata* song, const CCSize& size);

public:
    static IndexSongCell* create(IndexSongMetadata* song, const CCSize& size);
};

}  // namespace jukebox
