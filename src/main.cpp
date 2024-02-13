#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

matjson::Value cachedJson = nullptr;
int lastAccountID = 0;


void updateLabel(int liveStat, int oldStat, cocos2d::CCLabelBMFont* label) {
	const int changes = liveStat - oldStat;
	if (changes != 0 && oldStat != 0 && label != nullptr && label->isVisible()) {
		std::ostringstream ss;
		ss.imbue(std::locale("en_US.UTF-8"));
		ss << abs(changes);
		std::string changeText = ss.str();

		auto newLabel = CCLabelBMFont::create("", "bigFont.fnt");
		float scale = Mod::get()->getSettingValue<double>("textScale");
		newLabel->setAlignment(kCCTextAlignmentCenter);
		newLabel->setString(changeText.c_str());
		newLabel->setScale(label->getScale() * scale);
		newLabel->setID(label->getID() + "-changes");

		auto spr = changes > 0 ? CCSprite::create("up_arrow.png"_spr) : CCSprite::create("down_arrow.png"_spr);
		spr->setColor(Mod::get()->getSettingValue<cocos2d::ccColor3B>(changes > 0 ? "upArrowColor" : "downArrowColor"));
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
	auto profilePage = static_cast<ProfilePage*>(CCScene::get()->getChildByID("ProfilePage"));
	if (profilePage == nullptr || profilePage->m_accountID != lastAccountID) return;
	
	auto starsLabel = profilePage->m_mainLayer->getChildByIDRecursive("stars-label");
	if (cachedJson == nullptr || starsLabel == nullptr || !starsLabel->isVisible()) {
		Loader::get()->queueInMainThread(&tryUpdateLabels);
		return;
	}
	if (cachedJson.is_null() || starsLabel->getParent()->getChildByID("stars-label-changes") != nullptr) return;

	updateLabel(profilePage->m_score->m_stars, cachedJson["stars"].as_int(), static_cast<cocos2d::CCLabelBMFont*>(starsLabel));
	updateLabel(profilePage->m_score->m_moons, cachedJson["moons"].as_int(), static_cast<cocos2d::CCLabelBMFont*>(profilePage->m_mainLayer->getChildByIDRecursive("moons-label")));
	updateLabel(profilePage->m_score->m_userCoins, cachedJson["ucoins"].as_int(), static_cast<cocos2d::CCLabelBMFont*>(profilePage->m_mainLayer->getChildByIDRecursive("user-coins-label")));
	updateLabel(profilePage->m_score->m_demons, cachedJson["demons"].as_int(), static_cast<cocos2d::CCLabelBMFont*>(profilePage->m_mainLayer->getChildByIDRecursive("demons-label")));
	updateLabel(profilePage->m_score->m_creatorPoints, cachedJson["cp"].as_int(), static_cast<cocos2d::CCLabelBMFont*>(profilePage->m_mainLayer->getChildByIDRecursive("creator-points-label")));
	log::debug("updated");
}

class $modify(ProfilePage) {
	bool init(int p0, bool p1) {
		if (!ProfilePage::init(p0, p1)) return false;

		if (p0 != lastAccountID) {
			const auto weeks = Mod::get()->getSettingValue<int64_t>("weeks");
			const std::string targetUrl = "https://me.redlimerl.com/gdstats/" + std::to_string(p0) + "/" + std::to_string(weeks);
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
		tryUpdateLabels();
		return true;
	}

	virtual TodoReturn loadPageFromUserInfo(GJUserScore* p0) {
		ProfilePage::loadPageFromUserInfo(p0);
		log::debug("tried");
		tryUpdateLabels();
	}
};
