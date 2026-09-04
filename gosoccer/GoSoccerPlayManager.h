#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "GoSoccerPlayManager.generated.h"

UCLASS()
class GOSOCCER_API UGoSoccerPlayManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	

public:
// Object Type Check
	static bool IsGoBoard(UObject* CheckObject);
	static bool IsGoDoll(UObject* CheckObject);
	/* Also Check Is Doll Fallen */
	static bool IsDollFallen(UObject* BoardObject, UObject* DollObject);
	static bool GetDollFallFromBoard(UObject* DollObject, bool& bIsFallen);
	static bool GetDollColor(UObject* DollObject, uint8& OutDollColor);
	static bool GetGoBoardBorderLine(UObject* BoardObject, float& U, float& D, float& L, float& R);
	static bool IsDollConsideredStationary(UObject* DollObject);

// Game Play Controll
	static bool GameStarted(UObject* PlayerObject, const FSessionCreateData& SessionCreateData);
	static FVector GetPutLocation(UObject* BoardObject, FVector CursorLocation);
	static bool PutDoll(UObject* GameModeObject, UObject* PlayerControllerObject, FVector Location, uint8 DollColor, const EDollType& SpawnDollType);
	static bool SetDollPlacedIndex(UObject* DollObject, int32 InPlacedIndex);
	static bool GetDollPlacedIndex(UObject* DollObject, int32& OutPlacedIndex);
	static bool MouseOver(UObject* MouseOverObject, EDollStencilValue DollStencilValue);
	static bool MouseOverEnd(UObject* MouseOverEndObject);
	static bool FlickDoll(UObject* DollObject, FVector FlickDirection);
	static bool FlickDoll_Completed(UObject* GameModeObject, UObject* DollOwnerControllerObject);
	static bool TurnStart(UObject* StartPlayerObject);
	static bool TurnEnd(UObject* EndPlayerObject);
	static bool MarkDollAsMoved(UObject* GameModeObject, UObject* DollObject);
	static bool SetDollStreakSpline(UObject* BoardObject, const TArray<FVector> DollPosArr);
	static bool SetWinDollCircularEffect(UObject* DollObject, bool bOnEffect);
	/// <summary>
	/// Set MID For Circular Effect
	/// </summary>
	/// <param name="DollObject">To Change Doll Object</param>
	/// <param name="fPercent">0 - 1, Off Effect If fPercent Less Than 0</param>
	/// <returns></returns>
	static bool SetDollCircularEffect_Progressive(UObject* DollObject, float CurrentTimeSec, float MaxTimeSec);
	static bool UpdateFallenDollScore(UObject* PlayerControllerObject, int32 FallenDollScore);
	static bool SetTranclucentDollLocation(UObject* GoBoardObject, FVector TDollLocation);
	static bool GetTranclucentDoll(UObject* GoBoardObject, UObject*& OutTranclucentDollObject);
	static bool GetOverlapingDoll_TrancluentDoll(UObject* TranclucentDollObject);
	UFUNCTION(BlueprintCallable)
	static bool SetDollFallenFromBoard(UObject* DollObject, bool bFallen);
	/*Promote Player To Host Set Nullptr To Demote*/
	static bool SetHostingSessionCreateData(UObject* PlayerControllerObject, const FSessionCreateData& InHostSessionCreateData);
	static bool GetHostingSessionCreateData(UObject* PlayerStateObject, FSessionCreateData& OutHostSessionCreateData);
	static bool ClearHostingSessionCreateData(UObject* PlayerControllerObject);
	static bool GetPossibleCameraLocation(UObject* GoBoardObject, FVector& ToMoveLocation);
	static bool AlertDollBoardStationary(UObject* DollObejct);

	static bool AIPlay_SpawnDollAtLocation(UObject* GameModeObject, UObject* NextPlayerObject, FVector PlaceLocation);
	static bool AIPlay_FlickDoll(UObject* GameModeObject, UObject* NextPlayerObject, UObject* FlickDollObject, FVector FlickDirection);
	static bool AIPlay_ThrowLoadingScreen(UObject* PlayerObject, bool bEnable);
	static bool AIPlay_ConsiderTurnPassed(UObject* PlayerObject);

	static bool AddFallenScore(UObject* GameModeObject, UObject* FallenDollObject);
	static bool AddRealtimeCooldownBonus(UObject* PlayerControllerObject, float CooldownBonusPortion, bool bFlick = false);
	static bool SetOminousHUDPercent(UObject* PlayerControllerObject, float CurrentTimeSec, float MaxTimeSec);

	static bool StartGameTimer(UObject* PlayerControllerObject, double InStartTimeStamp, double InTimeLimit);
	static bool ThrowScoringBoard(UObject* PlayerControllerObject, const TArray<FScoreBoard>& ScoreArr);
	static bool UpdateScoringBoard(UObject* PlayerControllerObject, uint8 DollColor, EScoreType ScoreType, int32 UpdatedScore);
	static bool GetDollHP(UObject* DollObject, float& OutHP);
	static bool SetDollStencil_Scoring(UObject* DollObject);

	static bool GetNextDollType(UObject* GameModeObject, EDollType& OutNextDollType);
	static bool SendCurrentSessionData(UObject* PlayerControllerObject, const FSessionCreateData& InSessionCreateData);

	static bool GetPlacedDollCount(UObject* GameModeObject, int32& OutDollCount);

	static bool SetTurnMode_CylinderConfig(UObject* PlayerControllerObject, const TArray<EDollType>& InItemDollArr);

	static bool SetMatchScoreLampState(UObject* PlayerControllerObject, int32 Index, EMatchScoreLampState InMatchScoreLampState, uint8 FillDollColor);
	static bool ResetMatchScoreLampState(UObject* PlayerControllerObject);
	static bool SetMatchScoreLampCount(UObject* PlayerControllerObject, int32 MaxCount);

// Server Function;
	static bool HostGame(UObject* HostPlayerObject, const FUniqueNetIdRepl& UniqueNetIdRepl, FSessionCreateData& SessionCreateData);
	static bool RequestHostGame(UObject* GameInstanceObject, const FUniqueNetIdRepl& UniqueNetIdRepl, const FSessionCreateData& SessionCreateData);
	static bool RefreshOnlineSessions(UObject* JoinPlayerControllerObject);
	static bool FindSession(UObject* GameInstanceObject, const FUniqueNetIdRepl& UniqueNetIdRepl);
	static bool JoinSelectedSession(UObject* PlayerObject, UObject* SelectedOnlineSessionData, const FUniqueNetIdRepl& UniqueNetIdRepl);
	static bool StartGame(UObject* PlayerObject);
	static bool StartGameIfAllReady(UObject* GameModeObejct, const FSessionCreateData& SessionCreateData);
	static bool ReadyGame(UObject* PlayerObject);
	static bool LeaveGame(UObject* PlayerObject);
	static bool Steam_CreateSession(UObject* PlayerObject);
	static bool SetReadyState(UObject* PlayerStateObject, bool bReady);

	static bool GetPlayerCard(UObject* PlayerStateObject, FPlayerCard& OutPlayerCard);
	static bool SetPlayerCard(UObject* PlayerStateObject, FPlayerCard& InPlayerCard);
	//static bool StartHotSeatGame(UObject* PlayerObject, EPlayGameRule PlayGameRule);
	static bool StartHotSeatGame(UObject* PlayerObject, const FSessionCreateData& SessionCreateData);
	//static bool StartSingleAIGame(UObject* PlayerObject, EPlayGameRule PlayGameRule, int32 PlayAsDollColor);
	static bool StartSingleAIGame(UObject* PlayerObject, const FSessionCreateData& SessionCreateData);
	static bool SetDollColor(UObject* PlayerObject, uint8 DollColor);

	static bool ClientLoadingCompleted(UObject* PlayerControllerObject, UObject* GameModeObject);
	static bool RefreshPlayerCardListView(UObject* PlayerControllerObject);
	static bool RefreshAllPlayerCardListView(UObject* GameModeObject);
	/*Refresh Every PlayerCard ListView Except The Given PlayerController*/
	static bool RefreshAllPlayerCardListView_Except(UObject* GameModeObject, UObject* ExceptPlayerControllerObject);
	static bool PlayerWin_HotSeat(UObject* PlayerObject, uint8 InDollColor);
	static bool PlayerWin(UObject* PlayerObject);
	static bool PlayerLose(UObject* PlayerObject);
	static bool PlayerDraw(UObject* PlayerObject);
	static bool PlayerGiveUp(UObject* PlayerObject);
	static bool PlayerWalkOver(UObject* GameModeObject, UObject* GaveUpPlayerObject);
	static bool NotifyPlayerTurn(UObject* PlayerControllerObject, const FName& TurnPlayerName);
	static bool GetCameraObject(UObject* GameModeObject, UObject*& CameraObject);

	static bool SetGlobalTimeDilation(UObject* GameStateObject, float DilationScale);
	static bool ThrowContinueMatchWidget(UObject* PlayerControllerObject, bool bRespond, const FMatchScoreLampStateConfig& InMatchScoreLampStateConfig);
	static bool WindupMatchWidget(UObject* PlayerControllerObject, const FMatchScoreLampStateConfig& InMatchScoreLampStateConfig);
	static bool InstantMatchScoreInMatchWidget(UObject* PlayerControllerObject, const FMatchScoreLampStateConfig& InMatchScoreLampStateConfig);
	static bool ContinueMatch(UObject* GameModeObject);
	UFUNCTION(BlueprintCallable)
	static bool SendChattingMessage(UObject* PlayerControllerObject, const FText& TextMessage);
	static bool AddChattingMessage(UObject* GameModeObject, const FText& InTextMessage, const FName& InPlayerName);
	static bool ReceiveChattingMessage(UObject* PlayerControllerObject, const FName& SendPlayerName, const FText& TextMessage);
	static bool StackChattingMessage(UObject* GameInstanceObject, UObject* ChattingBoxObject);

	// Steam
	static bool GetSteamID(UObject* GameInstanceObject, int64& OutSteamID);
	static bool PostSteamID(UObject* GameStateObject, UObject* PlayerControllerObject, int64 RawSteamID);
	static bool GetSteamAvatar(UObject* GameStateObject, FUniqueNetIdRepl& FindNetID, UObject*& OutAvatarTexture2DObject);
	static bool RequestSteamAvatar(UObject* GameStateObject, int64 RawSteamID, const FOnSteamAvatarLoaded& OnAvatarLoaded);
	static bool GetSteamOSSOnline(UObject* GameInstanceObject);

	static bool Steam_InviteFriend(UObject* GameInstanceObject, int64 FriendSteamID);
	static bool Steam_GetInvitableFriends(UObject* GameInstanceObject, TArray<FSteamFriendData>& InInvitableArr);
// Server State Changed
	static bool PlayerJoinedOrCreatedSessionComplete(UObject* PlayerObject, uint64 RawSteamID);
	static bool PlayerJoinedSessionComplete(UObject* PlayerObject);
	static bool PlayerCreateSessionComplete(UObject* PlayerObject);
	static bool OnlineSubsystemLoaded(UObject* PlayerObject);
	/*Alert All Player via Gamemode*/
	static bool Notify_PlayerJoinedSessionComplete(UObject* GameModeObject, UObject* JoinedPlayerObject);
	/*Alert Remaining Players via Gamemode When A Client Left The Session*/
	static bool Notify_PlayerLeftSession(UObject* GameModeObject, UObject* LeftPlayerObject);
	static bool GetSessionCreateData_FromSessionInterface(UObject* GameInstanceObject, FSessionCreateData& OutSessionCreateData);
	static bool SetSessionCreateData_ToGameInstance(UObject* GameInstanceObject, const FSessionCreateData& InSessionCreateData);

// Client Function
	UFUNCTION(BlueprintCallable)
	static bool SwitchDollColor(UObject* PlayerControllerObject);
	static bool ChangePlayerName(UObject* PlayerObject, const FName& NewPlayerName);
	static bool ChangePlayerDollColor(UObject* PlayerObject, uint8 DollColor);
	static bool SetMainUIState(UObject* GameInstanceObject, UObject* PlayerControllerObject, const EMainUIState& NewMainUIState);
	static bool GetMainUIState(UObject* GameInstanceObject, EMainUIState& OutMainUIState);
	static bool ChangeSessionName(UObject* PlayerControllerObject, const FName& NewSessionName);
	static bool StartCameraShake(UObject* PlayerControllerObject, float Scale);

	static bool Flicked_InvertCylinderCharge(UObject* GameStateObject, int32 InFlickCylinderInvert);
	static bool Flicked_InvertCylinderCharge_SetInvertParam(UObject* PlayerControllerObject, int32 InFlickCylinderInvert);

	static bool Flicked_Locking(UObject* PlayerControllerObject, UObject* GameStateObject, bool bLocked);
	static bool Flicked_Locking_SetBoolParam(UObject* PlayerControllerObject, bool bLocked);

	static bool Placed_NextDollType_Received(UObject* GameStateObject, const EDollType InNextDollType);
	static bool Placed_NextDollType_Received_SetDollType(UObject* PlayerControllerObject, const EDollType InNextDollType);
// MainWidget_Settings
	static bool ApplySavedSettings(UObject* GameInstanceObject);
	static bool ChangeSettings_MaxFPS(UObject* GameInstanceObject, float NewMaxFPS);
	static bool ChangeSettings_bUseLowTexture(UObject* GameInstanceObject, bool NewbUseLowTexture);
	static bool ChangeSettings_fShadowLevel(UObject* GameInstanceObject, float NewfShadowLevel);
	static bool ChangeSettings_AA(UObject* GameInstanceObject, int32 NewAAType);
	static bool ChangeSettings_Float(UObject* GameInstanceObject, ESettingConfigParamType SettingConfigParamType, float fNewLevel);
	static bool ChangeSettings_ScreenResolution(UObject* GameInstanceObject, FIntPoint NewResolution);
	static bool ChangeSettings_WindowMode(UObject* GameInstanceObject, EWindowMode::Type NewWindowMode);
	static bool ChangeSettings_Language(UObject* GameInstanceObject, const FString& NewCulture);
	/*Ask The GameMode To Send Its Hosting Session Config To The Given Player Only*/
	static bool Request_UpdateTooltip_SessionConfig(UObject* GameModeObject, UObject* PlayerControllerObject);
	/*Ask The Server(GameMode) To Update Every Player's Tooltip, Standalone Falls Back To Local Only*/
	static bool Request_UpdateTooltip_SessionConfig_ToAll(UObject* PlayerControllerObject, const FSessionCreateData& InHostSessionCreateData);
	/*Store The Given Session Config Into The GameMode And Push It To Every Player*/
	static bool Notify_UpdateTooltip_SessionConfig(UObject* GameModeObject, const FSessionCreateData& InHostSessionCreateData);
	static bool UpdateTooltip_SessionConfig(UObject* PlayerControllerObject, const FSessionCreateData& HostSessionCreateData);
	static bool SetHostingSessionCreateData_ToGameMode(UObject* GameModeObject, const FSessionCreateData& InHostSessionCreateData);
	static bool GetHostingSessionCreateData_FromGameMode(UObject* GameModeObject, FSessionCreateData& OutHostSessionCreateData);
	static bool GetHostingSessionCreateData_FromGameInstance(UObject* GameInstanceObject, FSessionCreateData& OutHostSessionCreateData);
// Widget Thingy
	//static bool GetMainWidgetAsObject(UObject* PlayerControllerObject, UObject*& OutMainWidget);
	static bool ChangeWidgetSelected(UObject* WidgetObjecct, bool bSelected);
// Client Function
	static bool GetPlayGameMode(UObject* PlayerObject, EPlayGameMode& OutPlayGameMode);
	static bool SetPlayGameMode(UObject* PlayerObject, EPlayGameMode InPlayGameMode);
	static bool EndProcess(UObject* PlayerControllerObject);

	static bool ThrowTemporalMessage(UObject* PlayerControllerObject, const FString& ThrowMessage, ETemporalMessageType TemporalMessageType);
	static bool SetCameraPosition(UObject* PlayerControllerObject, ECameraPosition CameraPosition);
	static bool CloseInGameSettingsWidget(UObject* PlayerControllerObject);

	static bool ClientTravel_UILoaded(UObject* PlayerControllerObject, EMainUIState SucceededUIState, const FString& URL, enum ETravelType TravelType, bool bSeamless = false);
	static bool MarkUIState_ClientTravelCallback(UObject* GameInstanceObject, EMainUIState SucceededUIState, EMainUIState FailededUIState);
	static bool GetClientTravelCallbackUIState(UObject* GameInstanceObject, EMainUIState& OutSucceededUIState, EMainUIState& OutFailededUIState);

// Session Loading UI
	static bool SetSessionLoadingScreen(UObject* PlayerControllerObject, bool bEnable, ESessionLoadingPhase LoadingPhase);
	static bool MarkSessionLoadingPhase(UObject* GameInstanceObject, ESessionLoadingPhase LoadingPhase);
	static bool GetSessionLoadingPhase(UObject* GameInstanceObject, ESessionLoadingPhase& OutLoadingPhase);

	static bool SetPlayerSessionHost(UObject* GameInstanceObject, bool e);
	static bool GetPlayerSessionHost(UObject* GameInstanceObject, bool& OutIsHost);
	//static bool Steam_GetSteamAvatar(UObject* GameInstance, UObject*& SteamAvatarObjectPtr);
// Delegate Function
	static bool BindDelegate_Widget(UObject* BindObject, UObject* WidgetObject);

// Debug Function
	static bool OpenLocalHost(UObject* PlayerControllerObject);
	static bool JoinLocalHost(UObject* PlayerControllerObject);
	static bool StartLocalHostGame(UObject* GameModeObject, const FSessionCreateData& SessionCreateData);
	static bool GetPlayerName(UObject* PlayerStateObject, FName& OutPlayerName);
	static bool GetOnlineIdentityPlayerNickName(UObject* GameInstanceObject, FString& OutPlayerName);

// Tool
	static EDollType ConvertIntToDollType(int32 In);
private:
// Debug Function
	static bool NullCheck(UObject* CheckObject, FName ObjectName = NAME_None, FName FuncName = NAME_None);
	static ECameraPosition UIStateToCamPosition(EMainUIState MainUIState);
};
