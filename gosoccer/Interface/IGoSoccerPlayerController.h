#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "IGoSoccerPlayerController.generated.h"

UINTERFACE(MinimalAPI)
class UIGoSoccerPlayerController : public UInterface
{
	GENERATED_BODY()
};

class GOSOCCER_API IIGoSoccerPlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetDollColor(uint8 DollColor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool HostGame(FSessionCreateData& SessionCreateData);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool RefreshOnlineSessions();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool StartGame();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ReadyGame();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool LeaveGame();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool EndProcess();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool JoinSelectedSession(UObject* SelectedOnlineSessionData);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool HostHotSeatGame(const FSessionCreateData& SessionCreateData);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool HostSingleAIGame(const FSessionCreateData& SessionCreateData);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool OpenLocalHost();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool JoinLocalHost();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool WinGame_HotSeat(uint8 WinDollColor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool WinGame();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool LoseGame();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool DrawGame();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GiveUpGame();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GameStarted(const FSessionCreateData& InSessionCreateData);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool RefreshPlayerCardListView();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool TurnStart();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool TurnEnd();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool LoadPlayerCardFromGameInstance();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetCameraPosition(ECameraPosition CameraPosition);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool PlayerJoinedOrCreatedSessionComplete(int64 RawSteamID);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool PlayerJoinedSessionComplete();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool PlayerCreateSessionComplete();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool OnlineSubsystemLoaded();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ChangeCreateSessionName(const FName& NewSessionName);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ChangePlayerDollColor(uint8 NewDollColor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ChangePlayerName(const FName& NewPlayerName);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetMainUIState(const EMainUIState& NewMainUIState);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool NotifyPlayerNameOfTurnPlayer(const FName& TurnPlayerName);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool NotifyFallenDollScore(int32 FallenDollScore);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CloseInGameSettingsWidget();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ThrowTemporalMessage(const FString& ThrowMessage, ETemporalMessageType TemporalMessageType);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetLoadingScreen(bool bEnable);

	/**
	 * 세션 생성/조인 진행용 로딩 UI 토글.
	 * bEnable == false 면 LoadingPhase 는 무시하고 무조건 내린다.
	 * 요청은 GameInstance(HostGame / JoinSelectedSession / Steam Invite) 에서,
	 * 해제는 트래블 이후 새 PlayerController 의 Load() 완료 시점에서 호출된다.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetSessionLoadingScreen(bool bEnable, ESessionLoadingPhase LoadingPhase);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool AI_ConsiderTurnPassed();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UObject* GetPlayerStateAsObject();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool StartCameraShake(float InSacle);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnHoveredHoverableWidget(UUserWidget* HoveredWidget);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ClientTravel_UILoaded(EMainUIState SucceededUIState, const FString& URL, ETravelType TravelType, bool bSeamless);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool AddRealtimeCoolDownBonus(float Portion_MaxCooldown, bool bFlick);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetOminousHUDPercent(float CurrentTimeSec, float MaxTimeSec);

	/** 이 클라이언트의 툴팁만 갱신한다. (서버가 각 플레이어에게 내려줄 때 사용) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool UpdateTooltip_SessionConfig(const FSessionCreateData& HostSessionCreateData);

	/**
	 * 바뀐 세션 설정을 서버(GameMode)로 올려 모든 플레이어의 툴팁을 갱신하도록 요청한다.
	 * 세션이 아닌 상태(메인 메뉴 / 핫시트 / 싱글)에서는 서버가 없으므로 로컬 툴팁만 갱신한다.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool Request_UpdateTooltip_SessionConfig_ToAll(const FSessionCreateData& InHostSessionCreateData);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SendChattingMessage(const FText& TextMessage);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ReceiveChattingMessage(const FName& SendPlayerName, const FText& TextMessage);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool StartGameTimer(double InStartTimeStamp, double InTimeLimit);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ThrowScoringBoard(const TArray<FScoreBoard>& ScoreArr);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ThrowContinueMatchWidget(bool bRespond, const FMatchScoreLampStateConfig& InMatchScoreLampStateConfig);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool UpdateScoringBoard(uint8 DollColor, EScoreType ScoreType, int32 UpdatedScore);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetCurrentPlayingSessionData(const FSessionCreateData& InSessionData);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetTurnMode_CylinderConfig(const TArray<EDollType>& ItemDollArr);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool Flicked_InvertCylinderCharge_SetInvertParam(int32 InFlickCylinderInvert);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool Flicked_Locking_SetBoolParam(bool bLocked);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool NextDollType_Received_SetDollType(const EDollType InNextDollType);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetMatchScoreLampState(int32 Index, const EMatchScoreLampState NewState, uint8 FillDollColor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ResetMatchScoreLamp();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetMatchScoreLampCount(int32 MaxCount);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool WindupMatchWidget(const FMatchScoreLampStateConfig& InMatchScoreLampStateConfig);

	//UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	//bool GetMainWidgetAsObject(UObject*& OutMainWidget);
};
