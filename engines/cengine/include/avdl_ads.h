#ifndef AVDL_ADS_H
#define AVDL_ADS_H

int avdl_ads_enabled();

void avdl_ads_loadFullScreenAd();
void avdl_ads_showFullScreenAd();

void avdl_ads_loadRewardedAd();
void avdl_ads_showRewardedAd();

void avdl_ads_onRewardedAd(int amount, const char *type);
int avdl_ads_hasReward();
int avdl_ads_getRewardAmount();
int avdl_ads_isRewardTypeEqualTo(const char *to);
void avdl_ads_consumeReward();

#endif
