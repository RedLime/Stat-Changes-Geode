#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/utils/web.hpp>
#include <cmath>

using namespace geode::prelude;
using namespace std::chrono;

int totalClicks = 0;
matjson::Value updatedStars = NULL;
matjson::Value updatedDemons = NULL;
matjson::Value updatedCoins = NULL;
matjson::Value updatedDiamonds = NULL;
matjson::Value updatedCp = NULL;

auto lastRefreshed = system_clock::now() - minutes(60);
void refreshStats() {
	if (updatedStars != NULL || system_clock::now() - lastRefreshed < minutes(5)) return;

	lastRefreshed = system_clock::now();
	web::AsyncWebRequest()
		.fetch("https://gmdkoreaforum.com/api/statrank/stars")
		.json()
		.then([](matjson::Value const& json) {
			if (json["status"].as_string() == "success") {
				updatedStars = json["data"];
			}
		})
		.expect([](std::string const& error) {
			log::debug("KFSTATS Error: {}", error);
		});
	web::AsyncWebRequest()
		.fetch("https://gmdkoreaforum.com/api/statrank/demons")
		.json()
		.then([](matjson::Value const& json) {
			if (json["status"].as_string() == "success") {
				updatedDemons = json["data"];
			}
		})
		.expect([](std::string const& error) {
			log::debug("KFSTATS Error: {}", error);
		});
	web::AsyncWebRequest()
		.fetch("https://gmdkoreaforum.com/api/statrank/coins")
		.json()
		.then([](matjson::Value const& json) {
			if (json["status"].as_string() == "success") {
				updatedCoins = json["data"];
			}
		})
		.expect([](std::string const& error) {
			log::debug("KFSTATS Error: {}", error);
		});
	web::AsyncWebRequest()
		.fetch("https://gmdkoreaforum.com/api/statrank/cp")
		.json()
		.then([](matjson::Value const& json) {
			if (json["status"].as_string() == "success") {
				updatedCp = json["data"];
			}
		})
		.expect([](std::string const& error) {
			log::debug("KFSTATS Error: {}", error);
		});
	// web::AsyncWebRequest()
	// 	.fetch("https://gmdkoreaforum.com/api/statrank/diamonds")
	// 	.json()
	// 	.then([](matjson::Value const& json) {
	// 		if (json["status"].as_string() == "success") {
	// 			updatedDiamonds = json["data"];
	// 		}
	// 	})
	// 	.expect([](std::string const& error) {
	// 		log::debug("KFSTATS Error: {}", error);
	// 	});
	return;
}

void updateLabel(int accountID, int stats, matjson::Value json, cocos2d::CCLabelBMFont* label) {
	if (json == NULL || label == nullptr) return;
	auto array = json.as_array();
	for (int i = 0; i < array.size(); i++) {
		const int accountId = array.at(i)["account_id"].as_int();
		if (accountId == accountID) {
			const int changes = stats - array.at(i)["prev_stat_value"].as_int();
			if (changes != 0) {
				std::string originalText = label->getString();
				std::string appendText = "";
				appendText = appendText + "(" + (changes >= 0 ? "+" : "") + std::to_string(changes) + ")";
				std::string changeText = "";
				changeText = changeText + label->getString() + "\n" + appendText;
				label->setAlignment(kCCTextAlignmentCenter);
				label->setString(changeText.c_str(), true);

				int lengthDiff = appendText.length() - 1 - originalText.length();
				if (lengthDiff > 0) {
					label->setScale(label->getScale() * std::pow(0.9, lengthDiff + 1));
				}
			}
			break;
		}
	}
}

class $modify(ProfilePage) {
	bool init(int p0, bool p1) {
		if (!ProfilePage::init(p0, p1)) return false;
		refreshStats();
		return true;
	}

	virtual void loadPageFromUserInfo(GJUserScore* p0) {
		ProfilePage::loadPageFromUserInfo(p0);

		const auto profileId = p0->m_accountID;
		const auto rankLabel = static_cast<cocos2d::CCLabelBMFont*>(this->m_mainLayer->getChildByID("global-rank-label"));
		const auto starsLabel = static_cast<cocos2d::CCLabelBMFont*>(this->m_mainLayer->getChildByIDRecursive("stars-label"));

		if (updatedStars != NULL) {
			auto array = updatedStars.as_array();
			for (int i = 0; i < array.size(); i++) {
				const int accountId = array.at(i)["account_id"].as_int();
				if (accountId == profileId) {
					if (rankLabel != nullptr && rankLabel->isVisible()) {
						std::string resultText = "";
						resultText = resultText + rankLabel->getString() + " / KR #" + std::to_string(array.at(i)["rank"].as_int());
						rankLabel->setString(resultText.c_str());
					}

					const int starAmount = p0->m_stars - array.at(i)["prev_stat_value"].as_int();
					if (starAmount != 0 && starsLabel != nullptr) {
						std::string starChanges = "";
						starChanges = starChanges + starsLabel->getString() + "\n(" + (starAmount >= 0 ? "+" : "") + std::to_string(starAmount) + ")";
						starsLabel->setString(starChanges.c_str());
						starsLabel->setAlignment(kCCTextAlignmentCenter);
					}
					break;
				}
			}
		}
		updateLabel(p0->m_accountID, p0->m_userCoins, updatedCoins, static_cast<cocos2d::CCLabelBMFont*>(this->m_mainLayer->getChildByIDRecursive("user-coins-label")));
		updateLabel(p0->m_accountID, p0->m_demons, updatedDemons, static_cast<cocos2d::CCLabelBMFont*>(this->m_mainLayer->getChildByIDRecursive("demons-label")));
		updateLabel(p0->m_accountID, p0->m_creatorPoints, updatedCp, static_cast<cocos2d::CCLabelBMFont*>(this->m_mainLayer->getChildByIDRecursive("creator-points-label")));
	}
};
