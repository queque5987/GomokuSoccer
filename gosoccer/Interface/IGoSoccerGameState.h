#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "IGoSoccerGameState.generated.h"

UINTERFACE(MinimalAPI)
class UIGoSoccerGameState : public UInterface
{
	GENERATED_BODY()
};

class GOSOCCER_API IIGoSoccerGameState
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool PostSteamID(UObject* PlayerControllerObject, int64 RawSteamID);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetSteamAvatar(FUniqueNetIdRepl& FindNetIdRef, UObject*& OutAvatarTexture2DObject);
	/** SteamIDArr 에 추가하고 아바타를 요청한다. 로드가 끝나면 OnAvatarLoaded 가 UTexture2D 를 UObject 로 넘겨준다. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool RequestSteamAvatar(int64 RawSteamID, const FOnSteamAvatarLoaded& OnAvatarLoaded);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetPlayGameMode(EPlayGameMode InPlayGameMode);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GetPlayGameMode(EPlayGameMode& OutPlayGameMode);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetGlobalTimeDilation(float InDilationScale);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool Flicked_InvertCylinderCharge(int32 InFlickCylinderInvert);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool Flicked_Locking(bool bLocked, UObject* CalledBy);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool Placed_NextDollType_Received(const EDollType InNextDollType);
};
