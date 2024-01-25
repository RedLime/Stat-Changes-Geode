#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

ProfilePage* profilePage = nullptr;
matjson::Value cachedJson = nullptr;
int lastAccountID = 0;


void updateLabel(int liveStat, int oldStat, cocos2d::CCLabelBMFont* label) {
	const int changes = liveStat - oldStat;
	if (changes != 0 && oldStat != 0 && label != nullptr && label->isVisible()) {
		std::string changeText = std::to_string(abs(changes));

		auto newLabel = CCLabelBMFont::create("", "bigFont.fnt");
		float scale = Mod::get()->getSettingValue<double>("textScale");
		newLabel->setAlignment(kCCTextAlignmentCenter);
		newLabel->setString(changeText.c_str());
		newLabel->setScale(label->getScale() * scale);

		auto spr = changes > 0 ? CCSprite::create("up_arrow.png"_spr) : CCSprite::create("down_arrow.png"_spr);
		spr->setScale(newLabel->getScale() * 0.7f);
		spr->setPosition({ (label->getScaledContentSize().width / 2) - (newLabel->getScaledContentSize().width / 2) - (spr->getScaledContentSize().width / 5), spr->getPositionY() - 1 });
		newLabel->setPositionX((label->getScaledContentSize().width / 2) + (spr->getScaledContentSize().width / 2));
		label->getParent()->addChild(spr);
		
		label->setAnchorPoint({ label->getAnchorPoint().x, label->getAnchorPoint().y - 0.3f });
		label->getParent()->addChild(newLabel);

		label->getParent()->setPositionY(label->getParent()->getPositionY() + ((newLabel->getScaledContentSize().height) / 5));
		label->getParent()->updateLayout();
	}
}

void tryUpdateLabels() {
	if (profilePage == nullptr || profilePage->m_accountID != lastAccountID) return;
	
	auto starsLebel = profilePage->m_mainLayer->getChildByIDRecursive("stars-label");
	if (cachedJson == nullptr || starsLebel == nullptr || !starsLebel->isVisible()) {
		Loader::get()->queueInMainThread(&tryUpdateLabels);
		return;
	}
	if (cachedJson.is_null()) return;

	updateLabel(profilePage->m_score->m_stars, cachedJson["stars"].as_int(), static_cast<cocos2d::CCLabelBMFont*>(starsLebel));
	updateLabel(profilePage->m_score->m_moons, cachedJson["moons"].as_int(), static_cast<cocos2d::CCLabelBMFont*>(profilePage->m_mainLayer->getChildByIDRecursive("moons-label")));
	updateLabel(profilePage->m_score->m_userCoins, cachedJson["ucoins"].as_int(), static_cast<cocos2d::CCLabelBMFont*>(profilePage->m_mainLayer->getChildByIDRecursive("user-coins-label")));
	updateLabel(profilePage->m_score->m_demons, cachedJson["demons"].as_int(), static_cast<cocos2d::CCLabelBMFont*>(profilePage->m_mainLayer->getChildByIDRecursive("demons-label")));
	updateLabel(profilePage->m_score->m_creatorPoints, cachedJson["cp"].as_int(), static_cast<cocos2d::CCLabelBMFont*>(profilePage->m_mainLayer->getChildByIDRecursive("creator-points-label")));
}

class $modify(ProfilePage) {
	bool init(int p0, bool p1) {
		if (!ProfilePage::init(p0, p1)) return false;
		
		auto weeks = Mod::get()->getSettingValue<int64_t>("weeks");
		std::string targetUrl = "https://me.redlimerl.com/gdstats/" + std::to_string(p0) + "/" + std::to_string(weeks);

		if (p0 != lastAccountID) {
			cachedJson = nullptr;
			web::AsyncWebRequest()
				.get(targetUrl)
				.json()
				.then([p0](matjson::Value const& json) {
					cachedJson = json;
				})
				.expect([](std::string const& error) {
					cachedJson = matjson::parse("null"); 
				});
		}
			
		lastAccountID = p0;
		profilePage = this;
		tryUpdateLabels();
		return true;
	}

	void onClose(CCObject* sender) {
		ProfilePage::onClose(sender);
		profilePage = nullptr;
	}
};
