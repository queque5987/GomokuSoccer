#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "Interface/IGoSoccerPlayerController.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "GoPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UStageHUD;
class UFlickWidget;
class UMainWidget;
class UIngameCursor;

UCLASS()
class GOSOCCER_API AGoPlayerController : public APlayerController, public IIGoSoccerPlayerController
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LMBAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RMBAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* TabAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ESCAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ScrollAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EnterAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BGM, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> SB_BGM;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BGM, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> SB_BGM_GameOver;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BGM, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> SB_BGM_InGame;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BGM, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> SB_SFX_Flicker;

	UPROPERTY()
	TObjectPtr<UAudioComponent> AudioComponent_BGM;

	UPROPERTY()
	TObjectPtr<UAudioComponent> AudioComponent_SFX_Flicker;

	AGoPlayerController();

	/*Require Client Response Only*/
	FOnClientLoadingCompleted	Delegate_OnClientLoadingCompleted;
	FOnSwitchFingerMode			Delegate_OnSwitchFingerMode;
	FOnAimFlicker				Delegate_OnAimFlicker;
	FOnGameStartResponse		Delgeate_OnGameStartResponse;
	UPROPERTY(BlueprintAssignable)
	FOnGameStarted				Delegate_OnGameStarted;

	FDelegateHandle DelegateHandle_OnClientLoadingCompleted;
	FDelegateHandle DelegateHandle_OnSwitchFingerMode;
	FDelegateHandle DelegateHandle_OnAimFlicker;
	FDelegateHandle DelegateHandle_OnAimFlicker_Controller;
	FDelegateHandle DelegateHandle_OnGameStartResponse;
	//FDelegateHandle DelegateHandle_OnDestroySession;



	//FOnCreateSessionCompleteDelegate Delegate_OnCreateSessionComplete;
	//FOnFindSessionsCompleteDelegate Delegate_OnFindSessionsComplete;
	//FOnJoinSessionCompleteDelegate Delegate_OnJoinSessionComplete;


	//FDelegateHandle DelegateHandle_OnCreateSessionComplete;
	//FDelegateHandle DelegateHandle_OnFindSessionsCompleteDelegate;
	//FDelegateHandle DelegateHandle_OnJoinSessionCompleteDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Widget, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget>  WidgetClass_Loading;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Widget, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget>  WidgetClass_Main;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Widget, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget>  WidgetClass_HUD;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Widget, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget>  WidgetClass_FlickWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Widget, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget>  WidgetClass_IngameCursor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Widget, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UUserWidget> Widget_Loading;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Widget, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMainWidget> Widget_Main;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Widget, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UIngameCursor> Widget_IngameCursor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Widget, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStageHUD> Widget_HUD;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Widget, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFlickWidget> Widget_FlickWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Widget, meta = (AllowPrivateAccess = "true"))
	FPlayerCard PlayerCard;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCameraShakeBase> CameraShake;

// Sessions Start

	IOnlineSessionPtr OnlineSessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	//FName CreateSessionName;
public:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionComplete(bool bWasSuccessful);
	void OnJoinSessionComplate(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
// Sessions End
	UPROPERTY(ReplicatedUsing = OnRep_bItsMyTurn)
	bool bItsMyTurn = false;
// Server Only
	FTimerHandle TurnTimerHandle;
	UPROPERTY(ReplicatedUsing = OnRep_PlaceCoolDown)
	float TakeTurn_PlaceCoolDown;
// Client Only
	bool bEnablePlay = false;
	bool bESCSettingOn = false;
	//UPROPERTY()
	//EMainUIState CurrentMainUIState = EMainUIState::EMUIS_Default;
	UPROPERTY(Replicated)
	bool bTakeTurn = false;
	UPROPERTY()
	bool bTakeTurn_SwipingScreen = false;
	const float TakeTurn_PlaceCoolDownThreshold = 5.f;
	UPROPERTY()
	float TakeTurn_FlickCoolDown;
	const float TakeTurn_FlickCoolDownThreshold = 1.f;
	const float TakeTurn_PlaceInsideCoolDownThreshold = 0.25f;
	uint8 HotSeatDollColor;
	ECameraPosition CurrentCameraPosition;
	UPROPERTY()
	FVector2D PrevMouseScreenPos;
	UPROPERTY()
	double StartTimeStamp;
	UPROPERTY()
	double TimeLimit;
	UPROPERTY()
	FCylinderConfig CylinderConfig;
	UPROPERTY()
	float Cylinder_LastPlaceedTime;
	UPROPERTY()
	FSessionCreateData CurrentPlayingSessionData;

	UPROPERTY()
	ESessionLoadingPhase CurrentSessionLoadingPhase = ESessionLoadingPhase::ESLP_None;


	virtual bool SetDollColor_Implementation(uint8 InDollColor) override;
	virtual bool HostGame_Implementation(FSessionCreateData& SessionCreateData) override;
	virtual bool RefreshOnlineSessions_Implementation() override;
	virtual bool StartGame_Implementation() override;
	virtual bool LeaveGame_Implementation() override;
	//virtual bool Steam_CreateSession_Implementation() override;
	virtual bool WinGame_HotSeat_Implementation(uint8 WinDollColor) override;
	virtual bool JoinSelectedSession_Implementation(UObject* SelectedOnlineSessionData) override;
	virtual bool HostHotSeatGame_Implementation(const FSessionCreateData& SessionCreateData) override;
	virtual bool HostSingleAIGame_Implementation(const FSessionCreateData& SessionCreateData) override;
	virtual bool OpenLocalHost_Implementation() override;
	virtual bool JoinLocalHost_Implementation() override;
	virtual bool WinGame_Implementation() override;
	virtual bool LoseGame_Implementation() override;
	virtual bool DrawGame_Implementation() override;
	virtual bool GiveUpGame_Implementation() override;
	virtual bool ReadyGame_Implementation() override;
	virtual bool RefreshPlayerCardListView_Implementation() override;
	virtual bool EndProcess_Implementation() override;
	//virtual bool ChangePlayerCardListView_Implementation(const FPlayerCard& NewPlayerCard) override;

	virtual bool GameStarted_Implementation(const FSessionCreateData& InSessionCreateData) override;
	virtual bool TurnStart_Implementation() override;
	virtual bool TurnEnd_Implementation() override;
	virtual bool LoadPlayerCardFromGameInstance_Implementation() override;
	virtual bool SetCameraPosition_Implementation(ECameraPosition CameraPosition) override;

	virtual bool ChangeCreateSessionName_Implementation(const FName& NewSessionName) override;
	virtual bool ChangePlayerName_Implementation(const FName& NewPlayerName) override;
	virtual bool ChangePlayerDollColor_Implementation(uint8 NewDollColor) override;
	UFUNCTION(exec, Category = "Debug")
	virtual bool SetMainUIState_Implementation(const EMainUIState& NewMainUIState) override;

	virtual bool PlayerJoinedOrCreatedSessionComplete_Implementation(int64 RawSteamID) override;
	virtual bool PlayerJoinedSessionComplete_Implementation() override;
	virtual bool PlayerCreateSessionComplete_Implementation() override;
	virtual bool OnlineSubsystemLoaded_Implementation() override;

	virtual bool NotifyPlayerNameOfTurnPlayer_Implementation(const FName& TurnPlayerName) override;
	virtual bool NotifyFallenDollScore_Implementation(int32 InFallenDollScore) override;

	virtual bool ThrowTemporalMessage_Implementation(const FString& ThrowMessage, ETemporalMessageType TemporalMessageType) override;
	virtual bool ThrowContinueMatchWidget_Implementation(bool bRespond, const FMatchScoreLampStateConfig& InMatchScoreLampStateConfig) override;
	virtual bool CloseInGameSettingsWidget_Implementation() override;

	virtual bool SetLoadingScreen_Implementation(bool bEnable) override;
	virtual bool SetSessionLoadingScreen_Implementation(bool bEnable, ESessionLoadingPhase LoadingPhase) override;

	virtual UObject* GetPlayerStateAsObject_Implementation() override;
	virtual bool AI_ConsiderTurnPassed_Implementation() override;

	virtual bool ClientTravel_UILoaded_Implementation(EMainUIState SucceededUIState, const FString& URL, ETravelType TravelType, bool bSeamless) override;

	virtual bool StartCameraShake_Implementation(float InScale) override;
	virtual bool AddRealtimeCoolDownBonus_Implementation(float Portion_MaxCooldown, bool bFlick) override;
	virtual bool SetOminousHUDPercent_Implementation(float CurrentTimeSec, float MaxTimeSec) override;

	virtual bool UpdateTooltip_SessionConfig_Implementation(const FSessionCreateData& HostSessionCreateData) override;
	virtual bool Request_UpdateTooltip_SessionConfig_ToAll_Implementation(const FSessionCreateData& InHostSessionCreateData) override;
	virtual bool SendChattingMessage_Implementation(const FText& TextMessage) override;
	virtual bool ReceiveChattingMessage_Implementation(const FName& SendPlayerName, const FText& TextMessage) override;

	virtual bool StartGameTimer_Implementation(double InStartTimeStamp, double InTimeLimit) override;
	virtual bool ThrowScoringBoard_Implementation(const TArray<FScoreBoard>& ScoreArr) override;
	virtual bool UpdateScoringBoard_Implementation(uint8 InDollColor, EScoreType ScoreType, int32 UpdatedScore) override;

	virtual bool SetCurrentPlayingSessionData_Implementation(const FSessionCreateData& InSessionData) override;
	virtual bool SetTurnMode_CylinderConfig_Implementation(const TArray<EDollType>& ItemDollArr) override;
	virtual bool Flicked_InvertCylinderCharge_SetInvertParam_Implementation(int32 InFlickCylinderInvert) override;
	virtual bool Flicked_Locking_SetBoolParam_Implementation(bool bLocked) override;
	virtual bool NextDollType_Received_SetDollType_Implementation(const EDollType InNextDollType) override;

	virtual bool SetMatchScoreLampState_Implementation(int32 Index, const EMatchScoreLampState NewState, uint8 FillDollColor) override;
	virtual bool ResetMatchScoreLamp_Implementation() override;
	virtual bool SetMatchScoreLampCount_Implementation(int32 MaxCount) override;
	virtual bool WindupMatchWidget_Implementation(const FMatchScoreLampStateConfig& InMatchScoreLampStateConfig) override;

	//virtual bool GetMainWidgetAsObject_Implementation(UObject*& OutMainWidget) override;


	UFUNCTION(Client, Reliable)
	void Client_RequestHostGame(const FUniqueNetIdRepl& UniqueNetIdRepl, const FSessionCreateData& SessionCreateData);
	//void Client_RequestHostGame(const FUniqueNetIdRepl& UniqueNetIdRepl, const FName& ClientCreateSessionName);
	UFUNCTION(Client, Reliable)
	void Client_RequestRefreshOnlineSessions(const FUniqueNetIdRepl& UniqueNetIdRepl);
	UFUNCTION(Server, Reliable)
	void Server_Callback_ClientLoadingCompleted();
	UFUNCTION(Client, Reliable)
	void Client_Callback_ClientLoadingCompleted();
	UFUNCTION(Server, Reliable)
	void Server_SpawnDoll(FVector DollSpawnLocation, const EDollType& SpawnDollType);
	UFUNCTION(Client, Reliable)
	void Client_RefreshPlayerCardListView();
	UFUNCTION(Server, Reliable)
	void Server_GameStarted(const FSessionCreateData& InSessionCreateData);
	UFUNCTION(Server, Reliable)
	void Server_StartGame(const FSessionCreateData& SessionCreateData);
	UFUNCTION(Server, Reliable)
	void Server_GiveUpGame();
	UFUNCTION(Server, Reliable)
	void Server_ReadyGame();
	UFUNCTION(Server, Reliable)
	void Server_TurnEnd();

	UFUNCTION(Server, Reliable)
	void Server_PlayerJoinedOrCreatedSessionComplete(int64 RawSteamID);
	UFUNCTION(Server, Reliable)
	void Server_PlayerJoinedSessionComplete();
	UFUNCTION(Server, Reliable)
	void Server_DebugPlayerNickname();
	/**
	 * PlayerState 로 직접 Server RPC 를 보내면 Owner(=PlayerController) 가 아직 복제되지 않은
	 * 시점에는 소유 커넥션이 없어 조용히 버려진다. PlayerController 는 항상 커넥션을 가지므로
	 * 이름 변경 요청은 여기로 우회한다.
	 */
	UFUNCTION(Server, Reliable)
	void Server_ChangePlayerName(const FName& NewPlayerName);
	UFUNCTION(Server, Reliable)
	void Server_PlayerCreateSessionComplete();
	UFUNCTION(Server, Reliable)
	void Server_Request_UpdateTooltip_SessionConfig();
	/** 호스트가 로비에서 바꾼 세션 설정을 GameMode 로 올린다. (GameMode 가 모든 플레이어에게 뿌린다) */
	UFUNCTION(Server, Reliable)
	void Server_Notify_UpdateTooltip_SessionConfig(const FSessionCreateData& InHostSessionCreateData);

	UFUNCTION(Server, Reliable)
	void Server_SendChattingMessage(const FText& TextMessage);
	UFUNCTION(Server, Reliable)
	void Server_AddRealtimeCooldownBonus(float Portion_MaxCooldown);

	UFUNCTION(Client, Reliable)
	void Client_DebugPlayerNickname();
	UFUNCTION(Client, Reliable)
	void Client_PlayerCreateSessionComplete();
	UFUNCTION(Client, Reliable)
	void Client_PlayerJoinedSessionComplete();

	UFUNCTION(Client, Reliable)
	void Client_SetCameraPosition(ECameraPosition CameraPosition);

	UFUNCTION(Client, Reliable)
	void Client_GameStarted(const FSessionCreateData& InSessionCreateData);
	UFUNCTION(Client, Reliable)
	void Client_WinGame_HotSeat(uint8 WinDollColor);
	UFUNCTION(Client, Reliable)
	void Client_WinGame();
	UFUNCTION(Client, Reliable)
	void Client_LoseGame();
	UFUNCTION(Client, Reliable)
	void Client_DrawGame();
	UFUNCTION(Client, Reliable)
	void Client_ChangeCreateSessionName(const FName& NewSessionName);
	UFUNCTION(Client, Reliable)
	void Client_SetbEnablePlay(bool e);
	UFUNCTION(Client, Reliable)
	void Client_NotifyPlayerNameOfTurnPlayer(const FName& TurnPlayerName);
	UFUNCTION(Client, Reliable)
	void Client_NotifyFallenDollScore(int32 InFallenDollScore);
	UFUNCTION(Client, Reliable)
	void Client_NotifyGameStartResponse(bool bWasSuccessful);
	UFUNCTION(Client, Reliable)
	void Client_ThrowTemporalMessage(const FString& ThrowMessage, ETemporalMessageType TemporalMessageType);
	UFUNCTION(Client, Reliable)
	void Client_ThrowContinueMatchWidget();
	UFUNCTION(Client, Reliable)
	void Client_SwitchInGameSettingsWidget(bool bEnable);
	UFUNCTION(Client, Reliable)
	void Client_SetLoadingScreen(bool bEnable);
	UFUNCTION(Client, Reliable)
	void Client_SetSessionLoadingScreen(bool bEnable, ESessionLoadingPhase LoadingPhase);
	UFUNCTION(Client, Reliable)
	void Client_ClientTravel_UILoaded(EMainUIState SucceededUIState, const FString& URL, ETravelType TravelType, bool bSeamless);
	UFUNCTION(Client, Reliable)
	void Client_StartCameraShake(float InScale);
	UFUNCTION(Client, Unreliable)
	void Client_StartGame(const FSessionCreateData& InSessionCreateData);
	UFUNCTION(Client, Reliable)
	void Client_SetOminousHUDPercent(float CurrentTimeSec, float MaxTimeSec);
	UFUNCTION(Client, Reliable)
	void Client_AddRealtimeCooldownBonus(float Portion_MaxCooldown);
	UFUNCTION(Client, Reliable)
	void Client_AddRealtimeCooldownBonus_Flick(float Portion_MaxCooldown);
	UFUNCTION(Client, Reliable)
	void Client_UpdateTooltip_SessionConfig(const FSessionCreateData& HostSessionCreateData);
	UFUNCTION(Client, Reliable)
	void Client_ReceiveChattingMessage(const FName& SendPlayerName, const FText& TextMessage);
	UFUNCTION(Client, Reliable)
	void Client_StartGameTimer(double InStartTimeStamp, double InTimeLimit);
	UFUNCTION(Client, Reliable)
	void Client_ThrowScoringBoard(bool bEnable, const TArray<FScoreBoard>& ScoreArr);
	UFUNCTION(Client, Reliable)
	void Client_UpdateScoringBoard(uint8 InDollColor, EScoreType ScoreType, int32 UpdatedScore);
	UFUNCTION(Client, Reliable)
	void Client_NextDollType_Received(const EDollType InDollType);
	UFUNCTION(Client, Reliable)
	void Client_SetCurrentPlayingSessionData(const FSessionCreateData& InSessionData);
	UFUNCTION(Client, Reliable)
	void Client_SetTurnMode_CylinderConfig(const TArray<EDollType>& ItemDollArr);

	UFUNCTION(Client, UnReliable)
	void Client_Flicked_InvertCylinderCharge(int32 InFlickCylinderInvert);
	UFUNCTION(Client, UnReliable)
	void Client_Flicked_bFlickLock(bool bLocked);
	UFUNCTION(Client, UnReliable)
	void Client_SetMatchScoreLampState(int32 Index, const EMatchScoreLampState NewState, int32 FillDollColor);
	UFUNCTION(Client, UnReliable)
	void Client_ResetMatchScoreLamp();
	UFUNCTION(Client, UnReliable)
	void Client_SetMatchScoreLampCount(int32 MaxCount);
	UFUNCTION(Client, UnReliable)
	void Client_WindupMatchWidget(const FMatchScoreLampStateConfig& InMatchScoreLampStateConfig);
	UFUNCTION(Client, Reliable)
	void Client_SetMainUIState(const EMainUIState& NewMainUIState);
	//UFUNCTION(NetMulticast, UnReliable)
	//void NetMulticast_NextDollType_Received(const EDollType InDollType);
	//UFUNCTION(Client, Reliable)
	//void Client_LoadSettingForSettingWidget();
	UFUNCTION(Server, Reliable)
	void Server_SetCurrentPlayingSessionData(const FSessionCreateData& InSessionData);
	UFUNCTION(Server, UnReliable)
	void Server_Flicked_InvertCylinderCharge(int32 InFlickCylinderInvert);

	/* 
	* Widget Interaction
	*/
private:
	TWeakObjectPtr<UUserWidget> HoveredWidget;
public:
	virtual void OnHoveredHoverableWidget_Implementation (UUserWidget* InHoveredWidget) override;

// Delegate Callbacks
public:
	UFUNCTION()
	void OnSetOminousHUDPercent(float CurrentTimeSec, float MaxTimeSec) { Client_SetOminousHUDPercent(CurrentTimeSec, MaxTimeSec); };
	UFUNCTION()
	void OnAddRealtimeCooldownBonus(float Portion_MaxCooldown);
	UFUNCTION()
	void OnAddRealtimeCooldownBonus_Flick(float Portion_MaxCooldown);
	UFUNCTION()
	void Callback_AimFlicker(FVector InAimDirection);
// Delegate Callbacks End


//Debug
	//virtual void Debug_DrawDebugLine_Implementation(FVector StartLocation, FVector EndLocation, FColor DrawColor) override;
	//UFUNCTION(Client, Reliable)
	//void Client_DrawDebugLine(FVector StartLocation, FVector EndLocation, FColor DrawColor);
protected:
	UFUNCTION()
	void OnRep_bItsMyTurn();
	UFUNCTION()
	void OnRep_PlaceCoolDown();
	UFUNCTION()
	void OnRep_FlickLocking();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;
	virtual void AcknowledgePossession(APawn* P) override;
	virtual bool InputKey(const FInputKeyParams& Params) override;

	void Load();
	bool Load_Widget();
	/* Load Player Card Info From Client GameInstance */
	bool Load_GameInstance();
	bool Load_SessionInterface();
// Loading Boolean Check Start
	bool bLoad_CompleteBind;
	bool bLoad_HUD;
	bool bLoad_FlickWidget;
	bool bLoad_MainWidget;
	bool bLoad_IngameCursor;
// Loading Boolean Check End

	bool bLoad_Widget = false;
	bool bLoad_GameInstance = false;
	bool bLoad_SessionInterface = false;
	/** PlayerJoinedSessionComplete 를 두 번 통지하지 않기 위한 플래그 */
	bool bNotified_PlayerJoinedSession = false;
	/** 아바타용 SteamID 를 GameState 에 올렸는지 */
	bool bPosted_SteamID = false;
	/** 스팀 닉네임을 서버에 한 번 보냈는지 (매 틱 재전송 방지) */
	bool bSent_PlayerNickname = false;
	/** SteamID 등록 재시도 횟수. NetId 복제를 기다리다 Load() 를 영구히 막지 않도록 상한을 둔다. */
	int32 PostSteamID_RetryCount = 0;
	bool bClicking = false;
	UPROPERTY(ReplicatedUsing = OnRep_FlickLocking)
	bool bFlickLocking = false;
	float FlickCylinderSpinWeight = 0.f;
	int32 FlickCylinderInvert = -1;

	TWeakObjectPtr<UObject> MouseOverObject;
	FVector TracingLocation;
	UPROPERTY(Replicated)
	uint8 DollColor;
	uint8 FingerMode = 0;
	FVector2D CameraMoveCriteriaLocation;
	FVector CameraToMoveDirection;
	TObjectPtr<AActor> CurrentPossessingCamera;
	FTimerHandle CameraXYTimerHandle;
	FTimerHandle CameraZTimerHandle;
	FTimerHandle ContinueMatchTimerHandle;
	FTimerHandle ContinueMatchScoreTimerHandle;
	// Use Getter
	TWeakObjectPtr<UObject> GoBoardObjectPtr;
	FVector GoBoardLocation;
	//TMap<EPlayerTask, EPlayerTaskStatus> LoadingScreenTasks;

	// All Mouse Button
	void LMBStarted(const FInputActionValue& Value);
	void LMBTriggered(const FInputActionValue& Value);
	void LMBCompleted(const FInputActionValue& Value);
	// Right Button Call Func
	void RMBStarted();
	void RMBTriggered();
	void RMBCompleted();
	void ScrollStarted();
	void ScrollTriggered(const FInputActionValue& Value);
	void ScrollCompleted();

	void TabTriggered();
	void TabCompleted();
	void ESCCompleted();
	void EnterCompleted();

	void MoveTriggered(const FInputActionValue& Value);

	void OnClickCanceled(uint8 InFingerMode);
	TWeakObjectPtr<UObject> GetGoBoardObjectPtr();
	UFUNCTION(Client, Unreliable)
	void Client_SwitchBGM(USoundBase* ToPlay, float fFadeOut = 1.f, float fFadeIn = 1.f, float fFadeOutLevel = 0.f, float fVolume = 1.f);
	UFUNCTION(Client, Unreliable)
	void Client_SetBGMVolume(float InVolume);
	UFUNCTION()
	void PlayLobbyBGM();
private:
	TWeakObjectPtr<AGameStateBase> CachedGameState;
	AGameStateBase* GetCachedGameState();

	bool IsPlacable_CoolDown();
	void SetbFlickLocking(bool e);
};
