#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "IGoSoccerGameUserSettings.generated.h"

UINTERFACE(MinimalAPI)
class UIGoSoccerGameUserSettings : public UInterface
{
	GENERATED_BODY()
};

class GOSOCCER_API IIGoSoccerGameUserSettings
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool GetVolume_Master(float& OutVolume_Master);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool SetVolume_Master(float InVolume_Master);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool GetVolume_BGM(float& OutVolume_BGM);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool SetVolume_BGM(float InVolume_BGM);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool GetVolume_SFX(float& OutVolume_SFX);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool SetVolume_SFX(float InVolume_SFX);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool GetLanguageSetting(ELanguageSetting& OutLanguageSetting);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool SetLanguageSetting(ELanguageSetting InLanguageSetting);
};
