#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

std::unordered_map<int, matjson::Value> cache;

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

void tryUpdateLabels(ProfilePage* profilePage) {
	if (profilePage == nullptr || !cache.contains(profilePage->m_accountID)) return;
	matjson::Value cachedJson = cache[profilePage->m_accountID];

	auto starsLabel = profilePage->m_mainLayer->getChildByIDRecursive("stars-label");
	if (cachedJson == nullptr || starsLabel == nullptr || !starsLabel->isVisible()) {
		log::warn("can't update, missing data or label");
		return;
	}
	if (cachedJson.isNull() || starsLabel->getParent()->getChildByID("stars-label-changes") != nullptr) return;

	updateLabel(profilePage->m_score->m_stars, cachedJson["stars"].asInt().unwrapOr(profilePage->m_score->m_stars), static_cast<CCLabelBMFont*>(starsLabel));
	updateLabel(profilePage->m_score->m_moons, cachedJson["moons"].asInt().unwrapOr(profilePage->m_score->m_moons), static_cast<CCLabelBMFont*>(profilePage->m_mainLayer->getChildByIDRecursive("moons-label")));
	updateLabel(profilePage->m_score->m_userCoins, cachedJson["ucoins"].asInt().unwrapOr(profilePage->m_score->m_userCoins), static_cast<CCLabelBMFont*>(profilePage->m_mainLayer->getChildByIDRecursive("user-coins-label")));
	updateLabel(profilePage->m_score->m_demons, cachedJson["demons"].asInt().unwrapOr(profilePage->m_score->m_demons), static_cast<CCLabelBMFont*>(profilePage->m_mainLayer->getChildByIDRecursive("demons-label")));
	updateLabel(profilePage->m_score->m_creatorPoints, cachedJson["cp"].asInt().unwrapOr(profilePage->m_score->m_creatorPoints), static_cast<CCLabelBMFont*>(profilePage->m_mainLayer->getChildByIDRecursive("creator-points-label")));
	log::debug("updated");
}

class $modify(ProfilePage) {
	struct Fields {
        TaskHolder<web::WebResponse> m_listener;
    };

	bool init(const int accountID, const bool ownProfile) {
		if (!ProfilePage::init(accountID, ownProfile)) return false;

		if (!cache.contains(accountID)) {
			const auto weeks = Mod::get()->getSettingValue<int64_t>("weeks");

			m_fields->m_listener.spawn(
				web::WebRequest().get(fmt::format("https://me.redlimerl.com/gdstats/{}/{}", accountID, weeks)),
				[this, accountID] (const web::WebResponse& res) {
					if (res.ok()) {
						cache[accountID] = res.json().unwrapOr(nullptr);
						log::debug("cached data for account {}", accountID);
						tryUpdateLabels(this);
					} else {
						cache[accountID] = matjson::parse("null").unwrapOr(nullptr);
					}
				}
			);
			log::debug("start caching");
		}
			
		return true;
	}

	void loadPageFromUserInfo(GJUserScore* score){
		ProfilePage::loadPageFromUserInfo(score);
		log::debug("tried");
		tryUpdateLabels(this);
	}
};