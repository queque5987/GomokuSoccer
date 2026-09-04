#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interface/IGoSoccerGameInstance.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "Containers/Ticker.h"
#include "GoSoccerGameInstance.generated.h"

DECLARE_DELEGATE(FOnSessionCreateDataSet);

// ===== Steam Invite =====
/** 초대 수락 -> 조인 시도 시작 시점에 브로드캐스트 (UI 로딩 표시용) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSteamInviteAccepted, bool, bWasSuccessful, const FString&, HostName);

class UMaterialParameterCollection;

UCLASS()
class GOSOCCER_API UGoSoccerGameInstance : public UGameInstance, public IIGoSoccerGameInstance
{
	GENERATED_BODY()

	UGoSoccerGameInstance();
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setting, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundMix> SM_DefaultSoundMix;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setting, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundClass> SC_MasterVolume;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setting, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundClass> SC_FXVolume;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setting, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundClass> SC_BGMVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setting, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialParameterCollection> MPC_MaterialSettings;
		
	UPROPERTY()
	EMainUIState MainUIState;

	// FGameSettings 는 UGameUserSettings / UGoSoccerGameUserSettings 로 이전되어 더 이상 쓰지 않는다.
	//UPROPERTY()
	//FGameSettings GameSettings;

	UPROPERTY()
	EPlayGameMode PlayGameMode;

	UPROPERTY()
	FSessionCreateData HostingSessionCreateData;

	IOnlineSessionPtr OnlineSessionInterface;
	IOnlineIdentityPtr OnlineIdentity;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	//FOnCreateSessionCompleteDelegate Delegate_OnCreateSessionComplete;
	//FOnFindSessionsCompleteDelegate Delegate_OnFindSessionsComplete;
	//FOnJoinSessionCompleteDelegate Delegate_OnJoinSessionComplete;

	FOnUpdateOnlineSessionList Delegate_OnUpdateOnlineSessionList;
	FOnClearOnlineSessionList Delegate_OnClearOnlineSessionList;
	FOnSessionCreateDataSet Delegate_OnSessionCreateDataSet;

	FDelegateHandle DelegateHandle_OnCreateSessionComplete;
	FDelegateHandle DelegateHandle_OnFindSessionsCompleteDelegate;
	FDelegateHandle DelegateHandle_OnJoinSessionCompleteDelegate;
	FDelegateHandle DelegateHandle_OnUpdateOnlineSessionList;
	FDelegateHandle DelegateHandle_OnClearOnlineSessionList;
	FDelegateHandle DelegateHandle_OnDestroySessionComplete;
	FDelegateHandle DelegateHandle_OnSessionFailure;
	FDelegateHandle DelegateHandle_OnUpdateOnlineSessionComplete;

	bool Init_OSS();

	UPROPERTY()
	bool bIsHost;
	UPROPERTY()
	EMainUIState ClientTravel_Succeeded_CallbakUIState;
	UPROPERTY()
	EMainUIState ClientTravel_Failed_CallbakUIState;
	UPROPERTY()
	TArray<TObjectPtr<UObject>> StackedChattingMessage;

	/**
	 * 현재 진행 중인 세션 생성/조인 단계.
	 * PlayerController 는 트래블 때 파괴되지만 GameInstance 는 유지되므로 여기서 보관한다.
	 */
	UPROPERTY()
	ESessionLoadingPhase SessionLoadingPhase = ESessionLoadingPhase::ESLP_None;

	/**
	 * 세션 로딩 UI 토글 요청.
	 * 상태를 기록하고, 로컬 PlayerController 가 있으면 즉시 UI 에도 반영한다.
	 * (초대 부팅 등으로 PC 가 아직 없으면 기록만 하고, 새 PC 가 Load_Widget 에서 복원한다)
	 */
	void NotifySessionLoadingScreen(bool bEnable, ESessionLoadingPhase InLoadingPhase);

// ===== Steam Invite Start =====
	/** 초대 수락으로 받은 세션 검색 결과 (조인 대기용) */
	FOnlineSessionSearchResult PendingInviteSearchResult;
	/** 초대를 수락한 로컬 플레이어의 ControllerId */
	int32 PendingInviteControllerId = 0;
	/** 조인 대기 중인 초대가 있는지 */
	bool bHasPendingInviteJoin = false;
	/** 조인 대기 누적 시간 */
	float PendingInviteElapsedSeconds = 0.f;

	FDelegateHandle DelegateHandle_OnSessionUserInviteAccepted;
	/** 초대 수락 시 기존 세션 정리용. LeaveSession 의 핸들과 섞이면 TravelToStandAlone 이 불려서 따로 둔다. */
	FDelegateHandle DelegateHandle_OnDestroySessionComplete_Invite;
	FTSTicker::FDelegateHandle TickerHandle_PendingInviteJoin;

	/** OnSessionUserInviteAccepted 델리게이트 바인딩. Init() 에서 호출된다. */
	bool Init_SteamInvite();

	/** 오버레이/부팅 인자(+connect_lobby) 양쪽 모두 이 콜백으로 들어온다. */
	void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);
	void OnDestroySessionComplete_Invite(FName SessionName, bool bWasSuccessful);

	/** 로컬 PlayerController 가 준비될 때까지 기다렸다가 조인하도록 티커를 건다. */
	void StartPendingInviteJoin();
	bool Tick_PendingInviteJoin(float DeltaTime);
	bool ExecutePendingInviteJoin();

	/** 현재 NAME_GameSession 의 SessionId 를 CSteamID 원시값으로 반환. 없으면 0. */
	uint64 GetCurrentSessionSteamId();
// ===== Steam Invite End =====

public:
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld) override;


// Session Callback Start
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionComplete(bool bWasSuccessful);
	void OnJoinSessionComplate(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplate(FName SessionName, bool bWasSuccessful);
	void OnSessionFailure(const FUniqueNetId& UniqueNetId, ESessionFailure::Type SessionFailure);
	void OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful);
// Session Callback End

	//virtual bool HostGame_Implementation(const FUniqueNetIdRepl& UniqueNetIdRepl, const FName& CreateSessionName) override;
	virtual bool HostGame_Implementation(const FUniqueNetIdRepl& UniqueNetIdRepl, const FSessionCreateData& SessionCreateData) override;

	virtual bool FindSession_Implementation(const FUniqueNetIdRepl& UniqueNetIdRepl) override;
	virtual bool JoinSelectedSession_Implementation(UObject* SelectedOnlineSessionData, const FUniqueNetIdRepl& UniqueNetIdRepl) override;
	virtual bool LeaveSession_Implementation() override;

	virtual void SetCurrentMainUIState_Implementation(const EMainUIState& NewMainUIState) override { Client_SetCurrentMainUIState(NewMainUIState); };
	virtual bool GetCurrentMainUIState_Implementation(EMainUIState& OutMainUIState) override;
	virtual bool SetMaxFPS_Implementation(float NewMaxFPS) override;
	virtual bool SetbUseLowTexture_Implementation(bool NewbUseLowTexture) override;
	virtual bool SetfShadowLevel_Implementation(float NewfShadowLevel) override;
	virtual bool SetAAType_Implementation(int32 NewAAType) override;
	virtual bool SetSettingFloat_Implementation(const ESettingConfigParamType SettingConfigParamType, float fNewLevel) override;
	virtual bool SetScreenResolution_Implementation(FIntPoint NewResolution) override;
	virtual bool SetWindowMode_Implementation(EWindowMode::Type NewWindowMode) override;
	virtual bool SetCultureSetting_Implementation(const FString& NewCulture) override;


	virtual bool ApplySavedSettings_Implementation() override;
	virtual bool GetPlayGameMode_Implementation(EPlayGameMode& OutPlayGameMode) override;
	virtual void SetPlayGameMode_Implementation(EPlayGameMode InPlayGameMode) override;

	//virtual TObjectPtr<UTexture2D> GetSteamAvatar_Implementation(int32 ImageRGBAHandle) override;

	virtual bool BindDelegate_Widget_Implementation(UObject* WidgetDelegate) override;

	virtual bool GetIdentityPlayerNickName_Implementation(FString& OutPlayerNickName) override;
	virtual bool SetSessionCreateData_Implementation(const FSessionCreateData& InSessionCreateData) override;
	virtual bool GetHostingSessionCreateData_Implementation(FSessionCreateData& OutHostingSessionCreateData) override
	{
		OutHostingSessionCreateData = HostingSessionCreateData;
		return HostingSessionCreateData.IsValidSessionConfig();
	};

	virtual int64 GetCSteamID_Implementation() override;
	virtual bool GetSteamOSSOnline_Implementation() override;
	virtual bool GetSessionCreateData_FromSessionInterface_Implementation(FSessionCreateData& OutSessionCreateData) override;

	virtual bool SetPlayerSessionHost_Implementation(bool e) override;
	virtual bool GetPlayerSessionHost_Implementation(bool& OutResult) override { OutResult = bIsHost; return true; };

	virtual bool MarkUIState_ClientTravelCallback_Implementation(EMainUIState InSucceededUIState, EMainUIState InFailededUIState) override;
	virtual bool GetClientTravelCallbackUIState_Implementation(EMainUIState& OutSucceededUIState, EMainUIState& OutFailededUIState) override;

	virtual bool MarkSessionLoadingPhase_Implementation(ESessionLoadingPhase InLoadingPhase) override { SessionLoadingPhase = InLoadingPhase; return true; };
	virtual bool GetSessionLoadingPhase_Implementation(ESessionLoadingPhase& OutLoadingPhase) override { OutLoadingPhase = SessionLoadingPhase; return true; };

	virtual bool StackChattingMessage_Implementation(UObject* ChattingBoxObject) override;


	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	UFUNCTION(Client, Reliable)
	void Client_SetCurrentMainUIState(const EMainUIState& NewMainUIState);
	UFUNCTION(Client, Reliable)
	void Client_SetMaxFPS(float NewMaxFPS, bool bForce);
	UFUNCTION(Client, Reliable)
	void Client_SetbUseLowTexture(bool NewbUseLowTexture, bool bForce);
	UFUNCTION(Client, Reliable)
	void Client_SetfShadowLevel(int32 NewShadowResolution, bool bForce);
	UFUNCTION(Client, Reliable)
	void Client_SetAAType(int32 NewAAType, bool bForce);
	UFUNCTION(Client, Reliable)
	void Client_SetSettingFloat(const ESettingConfigParamType SettingConfigParamType, float fNewLevel, bool bForce);
	UFUNCTION(Client, Reliable)
	void Client_SetScreenResolution(FIntPoint NewResolution);
	UFUNCTION(Client, Reliable)
	void Client_SetWindowMode(EWindowMode::Type NewWindowMode);
	UFUNCTION(Client, Reliable)
	void Client_LoadSavedSettings();
	UFUNCTION(Client, Reliable)
	void Client_ApplySavedSettings();
	UFUNCTION(Client, Reliable)
	void Client_SetPlayGameMode(EPlayGameMode InPlayGameMode);
	UFUNCTION(Server, Reliable)
	void Server_OnSessionFailure();

// ===== Steam Invite Start =====
	/** 초대를 수락해 조인 절차에 들어갔을 때 브로드캐스트. UI 에서 로딩 표시 용도로 바인딩하면 된다. */
	UPROPERTY(BlueprintAssignable, Category = "GoSoccer|SteamInvite")
	FOnSteamInviteAccepted Delegate_OnSteamInviteAccepted;

	/**
	 * 초대 수락 후 실제 JoinSession 까지 추가로 대기할 시간(초).
	 * 게임이 꺼진 상태에서 초대로 부팅된 경우 PlayerController/위젯이 아직 준비되지 않았을 수 있어서 둔다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "GoSoccer|SteamInvite")
	float InviteJoinDelaySeconds = 1.0f;

	/** 지금 초대를 보낼 수 있는 상태인지 (Steam OSS 온라인 + 세션 보유) */
	UFUNCTION(BlueprintCallable, Category = "GoSoccer|SteamInvite")
	bool Steam_IsInviteAvailable();

	/** Steam 오버레이의 친구 초대 다이얼로그를 띄운다. (호스트/클라이언트 모두 호출 가능) */
	UFUNCTION(Exec, BlueprintCallable, Category = "GoSoccer|SteamInvite")
	bool Steam_ShowInviteOverlay();

	/** 오버레이 없이 특정 친구에게 바로 초대를 보낸다. */
	virtual bool Steam_InviteFriend_Implementation(int64 FriendSteamID) override;

	/** 여러 명에게 한 번에 초대를 보낸다. */
	UFUNCTION(BlueprintCallable, Category = "GoSoccer|SteamInvite")
	bool Steam_InviteFriends(const TArray<int64>& FriendSteamIDs);

	/** 자체 초대 UI 용 온라인 친구 목록. 이 게임을 플레이 중인 친구가 앞에 온다. */
	UFUNCTION(Exec, Category = "Debug")
	void Debug_Steam_GetInvitableFriends();
	//UFUNCTION(BlueprintCallable, Category = "GoSoccer|SteamInvite")
	virtual bool Steam_GetInvitableFriends_Implementation(TArray<FSteamFriendData>& OutFriends) override;

	/** 디버그용. 현재 세션의 Steam 로비 ID 문자열. 로비 세션이 아니면 빈 문자열. */
	UFUNCTION(BlueprintCallable, Category = "GoSoccer|SteamInvite")
	FString Steam_GetCurrentLobbyIdString();

	/** 대기 중인 초대 조인을 취소한다. */
	UFUNCTION(BlueprintCallable, Category = "GoSoccer|SteamInvite")
	void Steam_CancelPendingInviteJoin();
// ===== Steam Invite End =====

	UFUNCTION(Exec, Category = "Debug")
	void Debug_CurrentMainUIState();

private:

	bool TravelToStandAlone();

	void ApplySessionSettings(FOnlineSessionSettings* OutSessionSettings, const FSessionCreateData& InSessionCreateData);
};
