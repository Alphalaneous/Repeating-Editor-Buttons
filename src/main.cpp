#include <Geode/Geode.hpp>
#include <Geode/modify/CCMenuItemSpriteExtra.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/SetGroupIDLayer.hpp>
#include <Geode/modify/EditButtonBar.hpp>

using namespace geode::prelude;

class $modify(MyCCMenuItemSpriteExtra, CCMenuItemSpriteExtra) {
	
	struct Fields {
		SEL_SCHEDULE m_holdSchedule;
		bool m_isHolding;
		bool m_repeatable;
		int m_repeatRate;
		int m_repeatDelay;
	};

    bool init(cocos2d::CCNode* sprite, cocos2d::CCNode* disabledSprite, cocos2d::CCObject* target, cocos2d::SEL_MenuHandler callback) {
		if (!CCMenuItemSpriteExtra::init(sprite, disabledSprite, target, callback)) return false;
		auto fields = m_fields.self();
		auto mod = Mod::get();
		fields->m_holdSchedule = schedule_selector(MyCCMenuItemSpriteExtra::checkHold);
		fields->m_repeatDelay = mod->getSettingValue<int>("repeat-delay");
		fields->m_repeatRate = mod->getSettingValue<int>("repeat-rate");
		return true;
	}

	void setRepeatable(bool repeatable) {
		auto fields = m_fields.self();
		fields->m_repeatable = repeatable;
	}

	void checkHold(float dt) {
		auto fields = m_fields.self();
		fields->m_isHolding = true;
		(m_pListener->*m_pfnSelector)(this);
	}

    void activate() {
		auto fields = m_fields.self();
		if (!fields->m_isHolding) {
			CCMenuItemSpriteExtra::activate();
		}
	}

    void selected() {
		CCMenuItemSpriteExtra::selected();
		auto fields = m_fields.self();
		if (fields->m_repeatable) {
			schedule(fields->m_holdSchedule, fields->m_repeatRate/1000.f, kCCRepeatForever, fields->m_repeatDelay/1000.f);
		}
	}

    void unselected() {
		CCMenuItemSpriteExtra::unselected();
		auto fields = m_fields.self();
		if (fields->m_repeatable) {
			unschedule(fields->m_holdSchedule);
			queueInMainThread([fields] {
				fields->m_isHolding = false;
			});
		}
	}
};

class $modify(MyEditButtonBar, EditButtonBar) {

	void loadFromItems(cocos2d::CCArray* p0, int p1, int p2, bool p3) {
		EditButtonBar::loadFromItems(p0, p1, p2, p3);
		queueInMainThread([this] {
			for (CCNode* node : CCArrayExt<CCNode*>(getChildren())) {
			if (!typeinfo_cast<CCMenu*>(node)) continue;
				for (CCNode* btn : CCArrayExt<CCNode*>(node->getChildren())) {
					if (!typeinfo_cast<CCMenuItemSpriteExtra*>(btn)) continue;
					static_cast<MyCCMenuItemSpriteExtra*>(btn)->setRepeatable(true);
				}
			}
		});
	}
};

class $modify(MySetGroupIDLayer, SetGroupIDLayer) {

    bool init(GameObject* obj, cocos2d::CCArray* objs) {
		if (!SetGroupIDLayer::init(obj, objs)) return false;

		applyRepeatIfExist("add-group-id-next-button");
		applyRepeatIfExist("add-group-id-prev-button");
		applyRepeatIfExist("editor-layer-next-button");
		applyRepeatIfExist("editor-layer-prev-button");
		applyRepeatIfExist("editor-layer-2-next-button");
		applyRepeatIfExist("editor-layer-2-prev-button");
		applyRepeatIfExist("z-order-next-button");
		applyRepeatIfExist("z-order-prev-button");
		applyRepeatIfExist("channel-order-next-button");
		applyRepeatIfExist("channel-order-prev-button");
		applyRepeatIfExist("channel-next-button");
		applyRepeatIfExist("channel-prev-button");

		return true;
	}

	void applyRepeatIfExist(std::string id) {
		if (CCNode* node = getChildByIDRecursive(id)) {
			if (typeinfo_cast<CCMenuItemSpriteExtra*>(node)) {
				static_cast<MyCCMenuItemSpriteExtra*>(node)->setRepeatable(true);
			}
		}
	}
};

class $modify(MyEditorUI, EditorUI) {

    bool init(LevelEditorLayer* editorLayer) {
		if (!EditorUI::init(editorLayer)) return false;

		for (auto btn : CCArrayExt<MyCCMenuItemSpriteExtra*>(m_editButtonBar->m_buttonArray)) {
			btn->setRepeatable(true);
		}

		static_cast<MyCCMenuItemSpriteExtra*>(m_undoBtn)->setRepeatable(true);
		static_cast<MyCCMenuItemSpriteExtra*>(m_redoBtn)->setRepeatable(true);
		static_cast<MyCCMenuItemSpriteExtra*>(m_layerNextBtn)->setRepeatable(true);
		static_cast<MyCCMenuItemSpriteExtra*>(m_layerPrevBtn)->setRepeatable(true);

		if (CCNode* zoomMenu = getChildByID("zoom-menu")) {
			for (auto btn : CCArrayExt<MyCCMenuItemSpriteExtra*>(zoomMenu->getChildren())) {
				btn->setRepeatable(true);
			}
		}

		if (CCNode* customEditMenu = getChildByID("hjfod.betteredit/custom-move-menu")) {
			recursivelySetRepeat(customEditMenu);
		}

		return true;
	}

	void recursivelySetRepeat(CCNode* node) {
		for (CCNode* node : CCArrayExt<CCNode*>(node->getChildren())) {
			if (CCMenuItemSpriteExtra* btn = typeinfo_cast<CCMenuItemSpriteExtra*>(node)) {
				static_cast<MyCCMenuItemSpriteExtra*>(btn)->setRepeatable(true);
				continue;
			}
			recursivelySetRepeat(node);
		}
	}
};