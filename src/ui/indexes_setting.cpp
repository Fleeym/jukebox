#include "indexes_setting.hpp"
#include "indexes_popup.hpp"
#include <algorithm>
#include <vector>

using namespace geode::prelude;

SettingNode *IndexesSettingValue::createNode(float width) {
  return IndexesSettingNode::create(this, width);
}

bool IndexesSettingNode::init(IndexesSettingValue *value, float width) {
  if (!SettingNode::init(value))
    return false;
  this->m_value = value;
  for (Index &index : value->getIndexes()) {
    m_localValue.push_back(index);
  }

  float height = 40.f;
  this->setContentSize({width, height});

  auto menu = CCMenu::create();
  menu->setPosition(0, 0);
  menu->setID("inputs-menu");

  // No way to get the JSON without hardcoding the setting ID...
  auto settingJson =
      Mod::get()->getSettingDefinition("indexes")->get<CustomSetting>()->json;
  m_defaultValue = settingJson->get<std::vector<Index>>("default");
  m_name = settingJson->get<std::string>("name");
  m_description = settingJson->get<std::string>("description");

  m_label = CCLabelBMFont::create(m_name.c_str(), "bigFont.fnt");
  m_label->setAnchorPoint({0.f, 0.5f});
  m_label->setPosition(20.f, height / 2);
  m_label->setScale(0.5f);
  menu->addChild(m_label);

  auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
  infoSpr->setScale(.6f);
  auto infoBtn = CCMenuItemSpriteExtra::create(
      infoSpr, this, menu_selector(IndexesSettingNode::onDesc));
  infoBtn->setPosition(m_label->getScaledContentSize().width + 40.f,
                       height / 2);
  menu->addChild(infoBtn);

  auto resetSpr =
      CCSprite::createWithSpriteFrameName("geode.loader/reset-gold.png");
  resetSpr->setScale(.5f);
  m_resetBtn = CCMenuItemSpriteExtra::create(
      resetSpr, this, menu_selector(IndexesSettingNode::onReset));
  m_resetBtn->setPosition(m_label->getScaledContentSize().width + 40.f + 20.f,
                          height / 2);
  menu->addChild(m_resetBtn);

  auto viewSpr = ButtonSprite::create("View");
  viewSpr->setScale(0.72f);
  auto viewBtn = CCMenuItemSpriteExtra::create(
      viewSpr, this, menu_selector(IndexesSettingNode::onView));
  viewBtn->setPosition(width - 40.f, height - 20.f);
  menu->addChild(viewBtn);

  this->addChild(menu);
  handleTouchPriority(this);

  updateVisuals();

  return true;
}

void IndexesSettingNode::onView(CCObject *) {
  auto popup =
      IndexesPopup::create(m_localValue, [this](std::vector<Index> newIndexes) {
        m_localValue = newIndexes;
        updateVisuals();
      });
  popup->m_noElasticity = true;
  popup->show();
}

void IndexesSettingNode::onReset(CCObject *) { resetToDefault(); }

void IndexesSettingNode::onDesc(CCObject *) {
  FLAlertLayer::create(m_name.c_str(), m_description.c_str(), "OK")->show();
}

void IndexesSettingNode::updateVisuals() {
  m_resetBtn->setVisible(hasNonDefaultValue());
  m_label->setColor(hasUncommittedChanges() ? cc3x(0x1d0) : cc3x(0xfff));
  this->dispatchChanged();
}

void IndexesSettingNode::commit() {
  this->m_value->setIndexes(m_localValue);
  updateVisuals();
  this->dispatchCommitted();
}

bool IndexesSettingNode::hasUncommittedChanges() {
  if (m_localValue.size() != m_value->getIndexes().size())
    return true;
  for (int i = 0; i < m_localValue.size(); i++) {
    if (m_localValue[i] != m_value->getIndexes()[i])
      return true;
  }
  return false;
}

bool IndexesSettingNode::hasNonDefaultValue() {
  if (m_localValue.size() != m_defaultValue.size())
    return true;
  for (int i = 0; i < m_localValue.size(); i++) {
    if (m_localValue[i] != m_defaultValue[i])
      return true;
  }
  return false;
}

void IndexesSettingNode::resetToDefault() {
  m_localValue.clear();
  for (Index &index : m_defaultValue) {
    m_localValue.push_back(index);
  }
  updateVisuals();
}

IndexesSettingNode *IndexesSettingNode::create(IndexesSettingValue *value,
                                               float width) {
  auto ret = new IndexesSettingNode();
  if (ret && ret->init(value, width)) {
    ret->autorelease();
    return ret;
  }
  CC_SAFE_DELETE(ret);
  return nullptr;
}
