#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IGoSoccerPlayMode.generated.h"

class APlayerController;

UINTERFACE(MinimalAPI)
class UIGoSoccerPlayMode : public UInterface
{
	GENERATED_BODY()
};

class AActor;

class GOSOCCER_API IIGoSoccerPlayMode
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool StartLocalHostGame(const FSessionCreateData& SessionCreateData);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool HostHotSeatGame(const FSessionCreateData& SessionCreateData);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool HostSingleAIGame(const FSessionCreateData& SessionCreateData);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool StartGame_IfAllReady(const FSessionCreateData& SessionCreateData);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool TurnStart(APlayerController* Player);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool TurnEnd_FlickDoll(UObject* PlayerObject);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool WinCheck(APlayerController* LatestPlayer);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SpawnDoll(UObject* PlayerObject, FVector SpawnLocation, uint8 DollColor, const EDollType& SpawnDollType);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool NotifyPlayerCardChange(const FPlayerCard& NewPlayerCard);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool Notify_PlayerJoinedSessionComplete(UObject* JoinedPlayerObject);
	/** 클라이언트가 세션에서 나갔을 때(정상 이탈 / 타임아웃 / 강제 종료) 서버에서 호출된다. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool Notify_PlayerLeftSession(UObject* LeftPlayerObject);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool DebugOnlyFunction();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ClientLoadingCompleted(UObject* PlayerObject);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool RefreshPlayerCardArrWidgets();
	/** ExceptPlayerControllerObject 를 제외한 모든 PlayerController 의 PlayerCard 목록을 다시 만든다. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool RefreshPlayerCardArrWidgets_Except(UObject* ExceptPlayerControllerObject);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ReplicatedReady_PlayerCardArr();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetPlacedDollCounter();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool MarkDollAsMoved(UObject* DollObject);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetGoBoardActor(UObject*& OutGoBoardObject);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetCameraActor(AActor*& OutCameraActor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool WalkOverPlayer(UObject* GaveUpPlayerObject);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool AIPlay_SpawnDollAtLocation(UObject* NextPlayerObject, FVector PlaceLocation);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool AIPlay_FlickDoll(UObject* NextPlayerObject, UObject* FlickDollObject, FVector FlickDirection);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool AddFallenScore(UObject* FallenDollObject);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool DollBrokenAdvantage(UObject* BrokenDollObject);
	/**
	 * 호스트의 세션 설정을 GameMode 에 보관한다.
	 * 호스트의 PlayerState 는 리슨 서버 트래블 때 새로 만들어지면서 설정을 잃으므로,
	 * 세션이 살아있는 동안 유지되는 GameMode 를 공유 지점으로 쓴다.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetHostingSessionCreateData(const FSessionCreateData& InHostingSessionCreateData);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetHostingSessionCreateData(FSessionCreateData& OutHostingSessionCreateData);
	/** 보관 중인 호스트 세션 설정을 갱신하고 접속 중인 모든 플레이어의 툴팁을 갱신한다. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool Notify_UpdateTooltip_SessionConfig(const FSessionCreateData& InHostingSessionCreateData);
	/** 특정 플레이어 한 명에게만 현재 호스트 세션 설정을 내려준다. (조인 직후 / 로비 진입 시) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool Request_UpdateTooltip_SessionConfig(UObject* PlayerControllerObject);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool AddChattingMessage(const FText& InTextMessage, const FName& InPlayerName);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetNextDollType(EDollType& OutNextDollType);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetPlacedDollCount(int32& OutDollCount);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ContinueMatch();
};
