#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "PlayerCardData.generated.h"

UCLASS(BlueprintType)
class GOSOCCER_API UPlayerCardData : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	FPlayerCard PlayerCard;

	UPROPERTY()
	FSteamFriendData SteamFriendData;

	FOnWidgetSelected Delegate_OnWidgetSelected;
public:
	UFUNCTION(BlueprintCallable)
	void GetPlayerCard(FPlayerCard& OutPlayerCard) const { OutPlayerCard = PlayerCard; };
	void SetPlayerCard(const FPlayerCard& InPlayerCard) { PlayerCard = InPlayerCard; };

	UFUNCTION(BlueprintCallable)
	void GetSteamFriendData(FSteamFriendData& OutSteamFriendData) const { OutSteamFriendData = SteamFriendData; };
	void SetSteamFriendData(const FSteamFriendData& InSteamFriendData) { SteamFriendData = InSteamFriendData; };

	FOnWidgetSelected* GetOnWidgetSelected() { return &Delegate_OnWidgetSelected; };
};
