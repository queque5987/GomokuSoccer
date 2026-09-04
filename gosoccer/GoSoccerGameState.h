#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Interface/IGoSoccerGameState.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "ThirdParty/Steamworks/Steamv153/sdk/public/steam/steam_api.h"
#include "GoSoccerGameState.generated.h"

class UTexture2D;

UCLASS()
class GOSOCCER_API AGoSoccerGameState : public AGameStateBase, public IIGoSoccerGameState
{
	GENERATED_BODY()

	STEAM_CALLBACK(AGoSoccerGameState, OnAvatarImageLoaded, AvatarImageLoaded_t);

public:
	UPROPERTY(ReplicatedUsing = OnRep_SteamIdArr)
	TArray<uint64> SteamIDArr;

	UPROPERTY()
	TMap<uint64, UTexture2D*> Map_SteamAvatar;

	UPROPERTY()
	EPlayGameMode CurrentPlayGameMode;

	//UPROPERTY(ReplicatedUsing = OnRep_CurrentDilationScale)
	float CurrentDilationScale = 1.f;
	float DilationTimeLapse;
	FTimerHandle TimerHandle_GlobalDilation;
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool PostSteamID_Implementation(UObject* PlayerControllerObject, int64 RawSteamID) override;
	virtual bool GetSteamAvatar_Implementation(FUniqueNetIdRepl& FindNetIdRef, UObject*& OutAvatarTexture2DObject) override;
	virtual bool RequestSteamAvatar_Implementation(int64 RawSteamID, const FOnSteamAvatarLoaded& OnAvatarLoaded) override;
	virtual void SetPlayGameMode_Implementation(EPlayGameMode InPlayGameMode) override { CurrentPlayGameMode = InPlayGameMode; };
	virtual void GetPlayGameMode_Implementation(EPlayGameMode& OutPlayGameMode) override { OutPlayGameMode = CurrentPlayGameMode; };

	virtual bool SetGlobalTimeDilation_Implementation(float InDilationScale) override;

	virtual bool Flicked_InvertCylinderCharge_Implementation(int32 InFlickCylinderInvert) override;
	virtual bool Flicked_Locking_Implementation(bool bLocked, UObject* CalledBy) override;
	virtual bool Placed_NextDollType_Received_Implementation(const EDollType InNextDollType) override;


	UFUNCTION()
	void OnRep_SteamIdArr();
	UFUNCTION(Client, Reliable)
	void Client_OnRep_SteamIDArr();
	//UFUNCTION()
	//void OnRep_CurrentDilationScale();
	//void OnAvatarImageLoaded(AvatarImageLoaded_t* CallbackPtr);
private:
	UTexture2D* GetSteamAvatarAsTexture2D(int ImageHandle);

	/** 아직 아바타를 못 받은 요청들. RawSteamID -> 대기 중인 콜백들 */
	TMap<uint64, TArray<FOnSteamAvatarLoaded>> Map_PendingAvatarRequest;
	/** 해당 SteamID 로 대기 중인 요청들을 모두 실행하고 대기열에서 제거한다. */
	void FlushPendingAvatarRequest(uint64 RawSteamID, UTexture2D* AvatarTexture);
};
