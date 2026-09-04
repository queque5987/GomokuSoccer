#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "GoSoccer_PCH.h"
#include "Interface/IGoSoccerGameUserSettings.h"
#include "GoSoccerGameUserSettings.generated.h"

// UGameUserSettings 와 같은 GameUserSettings.ini 에 저장되도록 config 카테고리를 맞춘다.
// (SaveSettings()/LoadSettings() 는 어차피 GGameUserSettingsIni 를 명시해서 Save/LoadConfig 하므로
//  최종 저장 파일은 이 지정 없이도 같지만, UPROPERTY(config) 를 쓰려면 소유 클래스에 config 카테고리가 있어야 한다)
UCLASS(config = GameUserSettings)
class GOSOCCER_API UGoSoccerGameUserSettings : public UGameUserSettings, public IIGoSoccerGameUserSettings
{
	GENERATED_BODY()

private:
	UPROPERTY(config)
	float Volume_Master = 1.f;
	UPROPERTY(config)
	float Volume_BGM = 1.f;
	UPROPERTY(config)
	float Volume_SFX = 1.f;
	UPROPERTY(config)
	ELanguageSetting LanguageSetting = ELanguageSetting::ELS_English;

public:
	virtual bool GetVolume_Master_Implementation(float& OutVolume_Master) override { OutVolume_Master = Volume_Master; return true; };
	virtual bool SetVolume_Master_Implementation(float InVolume_Master) override { Volume_Master = InVolume_Master; return true; };

	virtual bool GetVolume_BGM_Implementation(float& OutVolume_BGM) override { OutVolume_BGM = Volume_BGM; return true; };
	virtual bool SetVolume_BGM_Implementation(float InVolume_BGM) override { Volume_BGM = InVolume_BGM; return true; };

	virtual bool GetVolume_SFX_Implementation(float& OutVolume_SFX) override { OutVolume_SFX = Volume_SFX; return true; };
	virtual bool SetVolume_SFX_Implementation(float InVolume_SFX) override { Volume_SFX = InVolume_SFX; return true; };

	virtual bool GetLanguageSetting_Implementation(ELanguageSetting& OutLanguageSetting) override { OutLanguageSetting = LanguageSetting; return true; };
	virtual bool SetLanguageSetting_Implementation(ELanguageSetting InLanguageSetting) override { LanguageSetting = InLanguageSetting; return true; };
};
