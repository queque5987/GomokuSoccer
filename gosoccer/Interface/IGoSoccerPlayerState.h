#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "IGoSoccerPlayerState.generated.h"

UINTERFACE(MinimalAPI)
class UIGoSoccerPlayerState : public UInterface
{
	GENERATED_BODY()
};

class GOSOCCER_API IIGoSoccerPlayerState
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetPlayerCard(FPlayerCard& OutPlayerCard);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetPlayerCard(FPlayerCard& InPlayerCard);
	//UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	//bool RefreshPlayerCardArr(TArray<FPlayerCard>& NewPlayerCardArr);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetDollColor(uint8& OutDollColor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetDollColor(uint8 DollColor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool TurnStart();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool TurnEnd();
	/* Switch Ready State */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ReadyGame();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetReadyState(bool bReady);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ClientLoadingCompleted();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetPlayerName(const FName& NewPlayerName, bool bUpdate);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetPlayerDollColor(uint8 NewDollColor, bool bUpdate);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetPlayerName(FName& OutPlayerName);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetHostingSessionCreateData(const FSessionCreateData& HostingSessionCreateData);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ClearHostingSessionCreateData();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetHostingSessionCreateData(FSessionCreateData& OutHostingSessionCreateDataPtr);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetPlayerSessionHost(bool& OutbIsHost);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetPlayerSessionHost(bool bIsHost);
	//UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	//bool StartGame();
};
