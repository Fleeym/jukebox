#pragma once

#include "../../include/index.hpp"
#include "../../include/index_serialize.hpp"
#include "indexes_popup.hpp"
#include <Geode/c++stl/gdstdlib.hpp>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/loader/Setting.hpp>
#include <Geode/loader/SettingEvent.hpp>
#include <Geode/loader/SettingNode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <matjson.hpp>

using namespace geode::prelude;
using namespace jukebox;

namespace jukebox {

struct IndexesSettingStruct {
  std::vector<IndexSource> m_indexes;
};

class IndexesSettingValue;

class IndexesSettingValue : public SettingValue {
protected:
  std::vector<IndexSource> m_indexes;

public:
  IndexesSettingValue(std::string const &key, std::string const &modID,
                      std::vector<IndexSource> indexes)
      : SettingValue(key, modID), m_indexes(indexes) {}

  bool load(matjson::Value const &json) override {
    m_indexes.clear();
    auto array = json.as_array();
    for (auto const &elem : array) {
      m_indexes.push_back(matjson::Serialize<IndexSource>::from_json(elem));
    }
    return true;
  }

  bool save(matjson::Value &json) const override {
    auto array = matjson::Array();
    for (auto const &index : m_indexes) {
      array.push_back(matjson::Serialize<IndexSource>::to_json(index));
    }
    json = array;
    return true;
  }

  SettingNode *createNode(float width) override;

  void setIndexes(std::vector<IndexSource> indexes) {
    this->m_indexes = indexes;
    this->valueChanged();
  }

  std::vector<IndexSource> getIndexes() const { return this->m_indexes; }
};

class IndexesSettingNode : public SettingNode {
protected:
  IndexesSettingValue *m_value;
  CCMenuItemSpriteExtra *m_resetBtn;
  CCLabelBMFont *m_label;
  std::vector<IndexSource> m_localValue;
  std::string m_name;
  std::string m_description;
  std::vector<IndexSource> m_defaultValue;

  bool init(IndexesSettingValue *value, float width);

public:
  void updateVisuals();
  void onView(CCObject *);
  void onReset(CCObject *);
  void onDesc(CCObject *);
  void commit() override;
  bool hasUncommittedChanges() override;
  bool hasNonDefaultValue() override;
  void resetToDefault() override;
  static IndexesSettingNode *create(IndexesSettingValue *value, float width);
};

}

template <> struct SettingValueSetter<IndexesSettingStruct> {
  static IndexesSettingStruct get(SettingValue *setting) {
    return IndexesSettingStruct{
        static_cast<IndexesSettingValue *>(setting)->getIndexes()};
  };
  static void set(IndexesSettingValue *setting,
                  IndexesSettingStruct const &value) {
    setting->setIndexes(value.m_indexes);
  };
};
