#include "GoPlayerController.h"
#include "Actor/GoBoard.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "GoSoccerPlayManager.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/StageHUD.h"
#include "Widget/FlickWidget.h"
#include "Widget/MainWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"
#include "DrawDebugHelpers.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Widget/Data/GameSessionData.h"
#include "Widget/IngameCursor.h"
#include "ThirdParty/Steamworks/Steamv153/sdk/public/steam/steam_api.h"
#include "GameFramework/PlayerState.h"
#include "Interface/IGoWidget.h"
#include "Components/AudioComponent.h"
#include "GameFramework/GameUserSettings.h"

constexpr float EXTRA_COOLDOWN_SAVE_PERCENT = 3.f;
constexpr float MATCHCONTINUE_WIDGET_TIME = 6.5f;

AGoPlayerController::AGoPlayerController()
{
	AudioComponent_BGM = CreateDefaultSubobject<UAudioComponent>(TEXT("BGM_AudioComponent"));
	AudioComponent_SFX_Flicker = CreateDefaultSubobject<UAudioComponent>(TEXT("SFX_AudioComponent"));
	if (AudioComponent_BGM)
	{
		AudioComponent_BGM->SetupAttachment(GetRootComponent());
	}
	if (AudioComponent_SFX_Flicker) AudioComponent_SFX_Flicker->SetupAttachment(GetRootComponent());
}

void AGoPlayerController::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("Session Create : %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
		GEngine->AddOnScreenDebugMessage(-1, 3.f, bWasSuccessful ? FColor::Green : FColor::Red, LogString);
	}
#endif

	if (bWasSuccessful && GetWorld())
	{
		GetWorld()->ServerTravel("/Game/Asset/GoStage?listen");
	}
}

void AGoPlayerController::OnFindSessionComplete(bool bWasSuccessful)
{
	if (!OnlineSessionInterface.IsValid() || !bWasSuccessful || SessionSearch == nullptr)
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("Session Not Found"));
			GEngine->AddOnScreenDebugMessage(-1, 3.f, bWasSuccessful ? FColor::Red : FColor::Red, LogString);
		}
#endif
		return;
	}

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("Session Found %d"), SessionSearch->SearchResults.Num());
		GEngine->AddOnScreenDebugMessage(-1, 3.f, bWasSuccessful ? FColor::Green : FColor::Red, LogString);
	}
#endif
	for (auto Result : SessionSearch->SearchResults)
	{
		FString ID = Result.GetSessionIdStr();
		FString User = Result.Session.OwningUserName;
		
		FString SessionName;
		Result.Session.SessionSettings.Get(FName("SessionName"), SessionName);

#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(
				TEXT("Session ID : %s, Owner : %s, SessionName : %s"),
				*ID,
				*User,
				*SessionName);
			GEngine->AddOnScreenDebugMessage(-1, 3.f, Result.IsValid() ? FColor::Green : FColor::Red, LogString);
		}
#endif
		if (Widget_Main)
		{
			Widget_Main->AddOnlineSessionSearchResult(Result);
			//Widget_Main->Debug_AddSessionName(SessionName);
		}
	}

}

void AGoPlayerController::OnJoinSessionComplate(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!OnlineSessionInterface.IsValid())
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("Join Session Failed : OnlineSessionInterface nullptr"));
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, LogString);
		}
#endif
		return;
	}
	if (Result != EOnJoinSessionCompleteResult::Type::Success)
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("Join Session Failed : %d"), Result);
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, LogString);
		}
#endif
	}
	FString Address;
	if (OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("Join Session IP Address: %s"), *Address);
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
			UE_LOG(LogTemp, Log, TEXT("Join Session IP Address: %s"), *Address);
		}
#endif
		//Execute_ClientTravel_UILoaded(this, EMainUIState::EMUIS_InLobby, Address, ETravelType::TRAVEL_Absolute, true);
		ClientTravel(Address, ETravelType::TRAVEL_Absolute, true);
	}
}

bool AGoPlayerController::SetDollColor_Implementation(uint8 InDollColor)
{
	DollColor = InDollColor;
	return true;
}

bool AGoPlayerController::HostGame_Implementation(FSessionCreateData& SessionCreateData)
{
	if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		//Client_RequestHostGame(LocalPlayer->GetPreferredUniqueNetId(), CreateSessionName);
		//FSessionCreateData SessionCreateData;
		//SessionCreateData.UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
		//SessionCreateData.SessionName = CreateSessionName.ToString();
		//if (Widget_Main) SessionCreateData.PlayGameRule = Widget_Main->GetSelectedMode_CreateSession();
		Client_RequestHostGame(LocalPlayer->GetPreferredUniqueNetId(), SessionCreateData);
		return true;
	}
	return false;
}

bool AGoPlayerController::RefreshOnlineSessions_Implementation()
{
	if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		Client_RequestRefreshOnlineSessions(LocalPlayer->GetPreferredUniqueNetId());
		return true;
	}
	return false;
//#if !UE_BUILD_SHIPPING
//	if (GEngine)
//	{
//		FString LogString = FString::Printf(TEXT("JoinGame Pressed"));
//		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
//	}
//#endif
//	if (!OnlineSessionInterface.IsValid())
//	{
//#if !UE_BUILD_SHIPPING
//		if (GEngine)
//		{
//			FString LogString = FString::Printf(TEXT("OnlineSessionInterface Invalied"));
//			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, LogString);
//		}
//#endif
//		return false;
//	}
//
//	SessionSearch = MakeShareable(new FOnlineSessionSearch());
//	SessionSearch->MaxSearchResults = 100;
//	SessionSearch->bIsLanQuery = true;
//	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
//
//
//	if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
//	{
//#if !UE_BUILD_SHIPPING
//		if (GEngine)
//		{
//			FString LogString = FString::Printf(TEXT("FindSessions"));
//			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
//		}
//#endif
//		return OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
//	}
//	return false;
}

bool AGoPlayerController::StartGame_Implementation()
{

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(
			TEXT("AGoPlayerController : StartGame")
		);
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
	}
#endif
	HotSeatDollColor = 0;
	FSessionCreateData CurrentSessionCreateData;
	if (UGoSoccerPlayManager::GetSessionCreateData_FromSessionInterface(GetGameInstance(), CurrentSessionCreateData))
	{
		Server_StartGame(CurrentSessionCreateData);
		Client_StartGame(CurrentSessionCreateData);
	}
	return true;
}

bool AGoPlayerController::LeaveGame_Implementation()
{
	if (OnlineSessionInterface.IsValid())
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("LeaveGame - Destroy Session"));
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
		}
#endif
		//OnlineSessionInterface->DestroySession(NAME_GameSession);
		UGameplayStatics::OpenLevel(this, TEXT("GoStage"), true);
		return true;
	}
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("OnlineSessionInterface Not Valid"));
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
	}
#endif
	return false;
}

bool AGoPlayerController::WinGame_HotSeat_Implementation(uint8 WinDollColor)
{
	Client_SetbEnablePlay(false);
	Client_WinGame_HotSeat(WinDollColor);
	return true;
}

bool AGoPlayerController::JoinSelectedSession_Implementation(UObject* SelectedOnlineSessionData)
{
	if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		return UGoSoccerPlayManager::JoinSelectedSession(GetGameInstance(), SelectedOnlineSessionData, LocalPlayer->GetPreferredUniqueNetId());
	}
	return false;
}

//bool AGoPlayerController::HostHotSeatGame_Implementation(EPlayGameRule PlayGameRule)
bool AGoPlayerController::HostHotSeatGame_Implementation(const FSessionCreateData& SessionCreateData)
{
	return UGoSoccerPlayManager::StartHotSeatGame(GetWorld()->GetAuthGameMode(), SessionCreateData);
}

//bool AGoPlayerController::HostSingleAIGame_Implementation(EPlayGameRule PlayGameRule, int32 PlayAsDollColor)
bool AGoPlayerController::HostSingleAIGame_Implementation(const FSessionCreateData& SessionCreateData)
{
	return UGoSoccerPlayManager::StartSingleAIGame(GetWorld()->GetAuthGameMode(), SessionCreateData);
}

bool AGoPlayerController::OpenLocalHost_Implementation()
{
	UGameplayStatics::OpenLevel(this, TEXT("GoStage"), true, "Listen");
	return true;
}

bool AGoPlayerController::JoinLocalHost_Implementation()
{
	ClientTravel("127.0.0.1", ETravelType::TRAVEL_Absolute, true);
	return true;
}
bool AGoPlayerController::WinGame_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("Calling Client_WinGame %s"), *GetFName().ToString());
	Client_SetbEnablePlay(false);
	Client_WinGame();
	return true;
}

bool AGoPlayerController::LoseGame_Implementation()
{
	Client_SetbEnablePlay(false);
	Client_LoseGame();
	return true;
}

bool AGoPlayerController::DrawGame_Implementation()
{
	Client_SetbEnablePlay(false);
	Client_DrawGame();
	UE_LOG(LogTemp, Log, TEXT("Calling Client_DrawGame %s"), *GetFName().ToString());
	return true;
}

bool AGoPlayerController::GiveUpGame_Implementation()
{
	//Execute_LoseGame(this); Do On GameMode
	Server_GiveUpGame();
	return true;
}

bool AGoPlayerController::ReadyGame_Implementation()
{
	Server_ReadyGame();
	return true;
}

bool AGoPlayerController::RefreshPlayerCardListView_Implementation()
{
	Client_RefreshPlayerCardListView();
	return true;
}

bool AGoPlayerController::EndProcess_Implementation()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), this, EQuitPreference::Quit, false);
	return true;
}

bool AGoPlayerController::GameStarted_Implementation(const FSessionCreateData& InSessionCreateData)
{
	Client_GameStarted(InSessionCreateData);
	Server_GameStarted(InSessionCreateData);
	UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), this, EMainUIState::EMUIS_GameStarted);
	UGoSoccerPlayManager::ReadyGame(this);
	Server_AddRealtimeCooldownBonus(-(EXTRA_COOLDOWN_SAVE_PERCENT + 1.f));
	Delegate_OnGameStarted.Broadcast();
	return true;
}

bool AGoPlayerController::TurnStart_Implementation()
{
	bItsMyTurn = true;
	if (HasAuthority())
	{
		OnRep_bItsMyTurn();
	}

	GetWorld()->GetTimerManager().SetTimer(
		TurnTimerHandle, [this]() {
			Client_SetbEnablePlay(true); 
		}, 1.75f, false
	);
	EPlayGameMode PlayGameMode;
	UGoSoccerPlayManager::GetPlayGameMode(GetGameInstance(), PlayGameMode);
	if (PlayGameMode == EPlayGameMode::EPGM_HotSeatGame)
	{
		UGoSoccerPlayManager::ChangePlayerDollColor(GetPlayerState<APlayerState>(), HotSeatDollColor);
	}
#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Log, TEXT("TurnStart_Implementation : bItsMyTurn = true %s"), *this->GetFName().ToString());
#endif
	return UGoSoccerPlayManager::TurnStart(GetPlayerState<APlayerState>());
}

bool AGoPlayerController::TurnEnd_Implementation()
{
	bItsMyTurn = false;
	if (HasAuthority())
	{
		OnRep_bItsMyTurn();
	}

	GetWorld()->GetTimerManager().ClearTimer(TurnTimerHandle);
	Client_SetbEnablePlay(false);
	if (CurrentPlayingSessionData.PlayGameMode == EPlayGameMode::EPGM_HotSeatGame)
	{
		HotSeatDollColor++;
		HotSeatDollColor %= 2;
		Client_Flicked_InvertCylinderCharge(1 - HotSeatDollColor);
	}
#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Log, TEXT("TurnEnd_Implementation : bItsMyTurn = false %s"), *this->GetFName().ToString());
#endif
	return UGoSoccerPlayManager::TurnEnd(GetPlayerState<APlayerState>());
}

bool AGoPlayerController::LoadPlayerCardFromGameInstance_Implementation()
{
	return Load_GameInstance();
}

bool AGoPlayerController::SetCameraPosition_Implementation(ECameraPosition CameraPosition)
{
	Client_SetCameraPosition(CameraPosition);

	//FName Tag;
	//if (CameraPosition == ECameraPosition::ECP_InPlay)
	//{
	//	Tag = TEXT("InPlayCamera");
	//}
	//else if (CameraPosition == ECameraPosition::ECP_Lobby)
	//{
	//	Tag = TEXT("DefaultCamera");
	//}
	//TArray<AActor*> FoundActors;
	//UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ACameraActor::StaticClass(), Tag, FoundActors);
	//if (FoundActors.IsValidIndex(0) && FoundActors[0] != nullptr)
	//{
	//	SetViewTarget(FoundActors[0]);
	//	return true;
	//}
	return true;
}

bool AGoPlayerController::ChangeCreateSessionName_Implementation(const FName& NewSessionName)
{
	//Client_ChangeCreateSessionName(NewSessionName);
	return true;
}

bool AGoPlayerController::ChangePlayerName_Implementation(const FName& NewPlayerName)
{
	APlayerState* tempPlayerState = GetPlayerState<APlayerState>();
	if (tempPlayerState == nullptr) return false;
#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Log, TEXT("ChangePlayerName : %s"), *NewPlayerName.ToString());
#endif
	if (HasAuthority())
	{
		return UGoSoccerPlayManager::ChangePlayerName(tempPlayerState, NewPlayerName);
	}
	// UGoSoccerPlayManager::ChangePlayerName -> AGoSoccerPlayerState::Server_SetPlayerName 은
	// PlayerState 의 Server RPC 다. 조인 직후에는 PlayerState 의 Owner 가 아직 복제되지 않아
	// 소유 커넥션이 없고, 그러면 RPC 가 조용히 버려져서 호스트 쪽 PlayerCard 이름이 빈 채로 남는다.
	// PlayerController 는 항상 커넥션을 가지므로 여기로 우회해서 보낸다.
	Server_ChangePlayerName(NewPlayerName);
	return true;
}

bool AGoPlayerController::ChangePlayerDollColor_Implementation(uint8 NewDollColor)
{
	APlayerState* tempPlayerState = GetPlayerState<APlayerState>();
	if (tempPlayerState == nullptr) return false;
	//EPlayGameMode PlayGameMode;
	//UGoSoccerPlayManager::GetPlayGameMode(GetGameInstance(), PlayGameMode);
	//if (PlayGameMode == EPlayGameMode::EPGM_HotSeatGame)
	//{

	//}
	return UGoSoccerPlayManager::ChangePlayerDollColor(tempPlayerState, NewDollColor);
}

bool AGoPlayerController::SetMainUIState_Implementation(const EMainUIState& NewMainUIState)
{
	Client_SetMainUIState(NewMainUIState);
	//if (Widget_Main == nullptr || Widget_HUD == nullptr) return false;
	//Widget_Main->OnUIStateChanged(NewMainUIState);
	//Widget_HUD->OnUIStateChanged(NewMainUIState);

	if (NewMainUIState == EMainUIState::EMUIS_InLobby)
	{
		Server_Request_UpdateTooltip_SessionConfig();
	}
	return true;
}

bool AGoPlayerController::PlayerJoinedOrCreatedSessionComplete_Implementation(int64 RawSteamID)
{
	if (GetPlayerState<APlayerState>() == nullptr) return false;
	FUniqueNetIdRepl tempNetId = GetPlayerState<APlayerState>()->GetUniqueId();
	if (tempNetId.IsValid())
	{
		uint64 NetId_StamId = *((uint64*)tempNetId->GetBytes());
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("RawSteamId : %d : RawNetId : %d"), RawSteamID, NetId_StamId);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
		}
#endif
	}
	else
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("NetId InValid"));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, LogString);
		}
#endif
		return false;
	}

	Server_PlayerJoinedOrCreatedSessionComplete(RawSteamID);
	return true;
}

bool AGoPlayerController::PlayerJoinedSessionComplete_Implementation()
{
	Client_PlayerJoinedSessionComplete();
	Server_PlayerJoinedSessionComplete();
	Server_Request_UpdateTooltip_SessionConfig();
	return true;
}

bool AGoPlayerController::PlayerCreateSessionComplete_Implementation()
{
	Client_PlayerCreateSessionComplete();
	Server_PlayerCreateSessionComplete();
	return true;
}

bool AGoPlayerController::OnlineSubsystemLoaded_Implementation()
{
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("PlayerController : OnlineSubsystemLoaded"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
	}
#endif

	//Client_DebugPlayerNickname();
	//Server_DebugPlayerNickname();
	return true;
}

bool AGoPlayerController::NotifyPlayerNameOfTurnPlayer_Implementation(const FName& TurnPlayerName)
{
	Client_NotifyPlayerNameOfTurnPlayer(TurnPlayerName);
	return true;
}

bool AGoPlayerController::NotifyFallenDollScore_Implementation(int32 InFallenDollScore)
{
	Client_NotifyFallenDollScore(InFallenDollScore);
	return true;
}

bool AGoPlayerController::ThrowTemporalMessage_Implementation(const FString& ThrowMessage, ETemporalMessageType TemporalMessageType)
{
	Client_ThrowTemporalMessage(ThrowMessage, TemporalMessageType);
	return true;
}

bool AGoPlayerController::ThrowContinueMatchWidget_Implementation(bool bRespond, const FMatchScoreLampStateConfig& InMatchScoreLampStateConfig)
{
	Client_ThrowContinueMatchWidget();

	GetWorld()->GetTimerManager().ClearTimer(ContinueMatchScoreTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		ContinueMatchScoreTimerHandle,
		FTimerDelegate::CreateLambda([&, InMatchScoreLampStateConfig]()
			{
				IIGoSoccerPlayerController::Execute_SetMatchScoreLampState(
					this, InMatchScoreLampStateConfig.MatchScoreLampIndex,
					InMatchScoreLampStateConfig.TargetLampState,
					InMatchScoreLampStateConfig.MatchScoreFillColor
				);
				//UGoSoccerPlayManager::SetMatchScoreLampState(
				//	this,
				//	InMatchScoreLampStateConfig.MatchScoreLampIndex, 
				//	InMatchScoreLampStateConfig.TargetLampState, 
				//	InMatchScoreLampStateConfig.MatchScoreFillColor
				//);
			}
		), MATCHCONTINUE_WIDGET_TIME * 0.125f, false
	);
	
	GetWorld()->GetTimerManager().ClearTimer(ContinueMatchTimerHandle);
	if (bRespond)
	{
		GetWorld()->GetTimerManager().SetTimer(
			ContinueMatchTimerHandle,
			FTimerDelegate::CreateLambda([&]() 
				{
					UGoSoccerPlayManager::ContinueMatch(GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr);
				}
			), MATCHCONTINUE_WIDGET_TIME * 1.1f, false
		);
	}
	return true;
}

bool AGoPlayerController::CloseInGameSettingsWidget_Implementation()
{
	if (!bESCSettingOn) return false;
	Client_SwitchInGameSettingsWidget(false);
	return true;
}

bool AGoPlayerController::SetLoadingScreen_Implementation(bool bEnable)
{
	Client_SetLoadingScreen(bEnable);
	return true;
}

bool AGoPlayerController::SetSessionLoadingScreen_Implementation(bool bEnable, ESessionLoadingPhase LoadingPhase)
{
	Client_SetSessionLoadingScreen(bEnable, LoadingPhase);
	return true;
}

UObject* AGoPlayerController::GetPlayerStateAsObject_Implementation()
{
	return PlayerState;
}

bool AGoPlayerController::AI_ConsiderTurnPassed_Implementation()
{
	HotSeatDollColor++;
	HotSeatDollColor %= 2;
	return true;
}

bool AGoPlayerController::ClientTravel_UILoaded_Implementation(EMainUIState SucceededUIState, const FString& URL, ETravelType TravelType, bool bSeamless)
{
	Client_ClientTravel_UILoaded(SucceededUIState, URL, TravelType, bSeamless);
	return true;
}

bool AGoPlayerController::StartCameraShake_Implementation(float InScale)
{
	Client_StartCameraShake(InScale);
	return true;
}

bool AGoPlayerController::AddRealtimeCoolDownBonus_Implementation(float Portion_MaxCooldown, bool bFlick)
{
	if (bFlick)
	{
		Client_AddRealtimeCooldownBonus_Flick(Portion_MaxCooldown);
	}
	else
	{
		Server_AddRealtimeCooldownBonus(Portion_MaxCooldown);
		//Client_AddRealtimeCooldownBonus(Portion_MaxCooldown);
	}
	return true;
}

bool AGoPlayerController::SetOminousHUDPercent_Implementation(float CurrentTimeSec, float MaxTimeSec)
{
	Client_SetOminousHUDPercent(CurrentTimeSec, MaxTimeSec);
	return true;
}

bool AGoPlayerController::UpdateTooltip_SessionConfig_Implementation(const FSessionCreateData& HostSessionCreateData)
{
	Client_UpdateTooltip_SessionConfig(HostSessionCreateData);
	return true;
}

bool AGoPlayerController::Request_UpdateTooltip_SessionConfig_ToAll_Implementation(const FSessionCreateData& InHostSessionCreateData)
{
	// 메인 메뉴 / 핫시트 / 싱글 플레이는 서버가 없으므로 로컬 툴팁만 갱신한다.
	if (GetNetMode() == ENetMode::NM_Standalone)
	{
		return Execute_UpdateTooltip_SessionConfig(this, InHostSessionCreateData);
	}
	Server_Notify_UpdateTooltip_SessionConfig(InHostSessionCreateData);
	return true;
}

bool AGoPlayerController::SendChattingMessage_Implementation(const FText& TextMessage)
{
	Server_SendChattingMessage(TextMessage);
	return true;
}

bool AGoPlayerController::ReceiveChattingMessage_Implementation(const FName& SendPlayerName, const FText& TextMessage)
{
	Client_ReceiveChattingMessage(SendPlayerName, TextMessage);
	return true;
}

bool AGoPlayerController::StartGameTimer_Implementation(double InStartTimeStamp, double InTimeLimit)
{
	Client_StartGameTimer(InStartTimeStamp, InTimeLimit);
	return true;
}

bool AGoPlayerController::ThrowScoringBoard_Implementation(const TArray<FScoreBoard>& ScoreArr)
{
	Client_ThrowScoringBoard(true, ScoreArr);
	return true;
}

bool AGoPlayerController::UpdateScoringBoard_Implementation(uint8 InDollColor, EScoreType ScoreType, int32 UpdatedScore)
{
	Client_UpdateScoringBoard(InDollColor, ScoreType, UpdatedScore);
	return true;
}

bool AGoPlayerController::SetCurrentPlayingSessionData_Implementation(const FSessionCreateData& InSessionData)
{
	Server_SetCurrentPlayingSessionData(InSessionData);
	Client_SetCurrentPlayingSessionData(InSessionData);
	return true;
}

bool AGoPlayerController::SetTurnMode_CylinderConfig_Implementation(const TArray<EDollType>& ItemDollArr)
{
	Client_SetTurnMode_CylinderConfig(ItemDollArr);
	//CylinderConfig.ShadowTextureIndex.X;
	//CylinderConfig.ShadowTextureIndex.Y;
	//CylinderConfig.ShadowTextureIndex.Z;
	//CylinderConfig.ShadowTextureIndex.W;
	return true;
}

bool AGoPlayerController::Flicked_InvertCylinderCharge_SetInvertParam_Implementation(int32 InFlickCylinderInvert)
{
	Client_Flicked_InvertCylinderCharge(InFlickCylinderInvert);
	return true;
}

bool AGoPlayerController::Flicked_Locking_SetBoolParam_Implementation(bool bLocked)
{
	Client_Flicked_bFlickLock(bLocked);
	return true;
}

bool AGoPlayerController::NextDollType_Received_SetDollType_Implementation(const EDollType InNextDollType)
{
	Client_NextDollType_Received(InNextDollType);
	return true;
}

bool AGoPlayerController::SetMatchScoreLampState_Implementation(int32 Index, const EMatchScoreLampState NewState, uint8 FillDollColor)
{
	Client_SetMatchScoreLampState(Index, NewState, static_cast<int32>(FillDollColor));
	return true;
}

void AGoPlayerController::Client_SetMatchScoreLampState_Implementation(int32 Index, const EMatchScoreLampState NewState, int32 FillDollColor)
{
	if (Widget_HUD) Widget_HUD->SetMatchScoreLampState(Index, NewState, FillDollColor);
}

bool AGoPlayerController::ResetMatchScoreLamp_Implementation()
{
	Client_ResetMatchScoreLamp();
	return true;
}

void AGoPlayerController::Client_ResetMatchScoreLamp_Implementation()
{
	if (Widget_HUD) Widget_HUD->ResetMatchScoreLampState();
}

bool AGoPlayerController::SetMatchScoreLampCount_Implementation(int32 MaxCount)
{
	Client_SetMatchScoreLampCount(MaxCount);
	return true;
}

bool AGoPlayerController::WindupMatchWidget_Implementation(const FMatchScoreLampStateConfig& InMatchScoreLampStateConfig)
{
	Client_WindupMatchWidget(InMatchScoreLampStateConfig);
	return true;
}

//bool AGoPlayerController::GetMainWidgetAsObject_Implementation(UObject*& OutMainWidget)
//{
//	OutMainWidget = Widget_Main;
//	return OutMainWidget != nullptr;
//}

void AGoPlayerController::Client_SetMatchScoreLampCount_Implementation(int32 MaxCount)
{
	if (Widget_HUD) Widget_HUD->SetMatchScoreLampCount(MaxCount);
}

void AGoPlayerController::Client_RequestHostGame_Implementation(const FUniqueNetIdRepl& UniqueNetIdRepl, const FSessionCreateData& SessionCreateData)
{
	UGoSoccerPlayManager::RequestHostGame(GetGameInstance(), UniqueNetIdRepl, SessionCreateData);
}

void AGoPlayerController::Client_RequestRefreshOnlineSessions_Implementation(const FUniqueNetIdRepl& UniqueNetIdRepl)
{
	UGoSoccerPlayManager::FindSession(GetGameInstance(), UniqueNetIdRepl);
}

void AGoPlayerController::Client_RefreshPlayerCardListView_Implementation()
{
	EPlayGameMode CurrentPlayGameMode;
	UGoSoccerPlayManager::GetPlayGameMode(GetGameInstance(), CurrentPlayGameMode);
	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (GameState == nullptr || 
		Widget_HUD == nullptr || 
		GameState->PlayerArray.IsEmpty() ||
		!Widget_HUD->ResetCursorPlayerCards())
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AGoPlayerController::Client_RefreshPlayerCardListView);
		return;
	}
	bool bFlag = false;
	if (GameState == nullptr) return;
	TArray<TObjectPtr<APlayerState>> PlayerStateArr = GameState->PlayerArray;
	PlayerStateArr.Sort([](const APlayerState& A, const APlayerState& B) {
		return A.GetPlayerId() < B.GetPlayerId();
		}
	);
	for (APlayerState* PlayerStateIter : PlayerStateArr)
	{
		if (PlayerStateIter != nullptr)
		{
			FPlayerCard tempPlayerCard;
			if (!UGoSoccerPlayManager::GetPlayerCard(PlayerStateIter, tempPlayerCard))
			{
				bFlag = true;
				break;
			}
			FUniqueNetIdRepl NetID = PlayerStateIter->GetUniqueId();
			if (NetID.IsValid())
			{
#if !UE_BUILD_SHIPPING
				if (GEngine)
				{
					FString LogString = FString::Printf(
						TEXT("Updating Texture User: %s"),
						*NetID.ToString()
					);
					//GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, LogString);
				}
#endif
				UObject* tempAvatar = nullptr;
				UGoSoccerPlayManager::GetSteamAvatar(GameState, NetID, tempAvatar);
				UTexture2D* tempAvatarTexture = Cast<UTexture2D>(tempAvatar);
#if !UE_BUILD_SHIPPING
				if (GEngine)
				{
					FString LogString = FString::Printf(
						TEXT("Texture Loaded %s Cast %s"),
						tempAvatar != nullptr ? TEXT("Succeded") : TEXT("Failed"),
						tempAvatarTexture != nullptr ? TEXT("Succeded") : TEXT("Failed")
					);
					//GEngine->AddOnScreenDebugMessage(-1, 6.f, tempAvatar != nullptr ? FColor::Green : FColor::Red, LogString);
				}
#endif
				if (tempAvatarTexture != nullptr)
				{
					tempPlayerCard.PlayerAvatar = tempAvatarTexture;
				}
			}
			Widget_HUD->AddPlayerCard(tempPlayerCard);
		}
	}
	if (CurrentPlayGameMode == EPlayGameMode::EPGM_SingleAIPlay)
	{
		//FPlayerCard AIPlayerCard;
		//AIPlayerCard.PlayerName = FName("AI");
		//Widget_HUD->AddPlayerCard(AIPlayerCard);
	}
	if (bFlag)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AGoPlayerController::Client_RefreshPlayerCardListView);
	}
}

void AGoPlayerController::Server_Callback_ClientLoadingCompleted_Implementation()
{
	UGoSoccerPlayManager::ClientLoadingCompleted(this, GetWorld()->GetAuthGameMode());

	//TArray<AActor*> FoundActors;
	//UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ACameraActor::StaticClass(), TEXT("DefaultCamera"), FoundActors);
	//if (FoundActors.IsValidIndex(0) && FoundActors[0] != nullptr)
	//{
	//	SetViewTarget(FoundActors[0]);
	//}
}

void AGoPlayerController::Client_Callback_ClientLoadingCompleted_Implementation()
{
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(
			TEXT("Client Loading Completed : %s"),
			GetWorld()->GetNetMode() == ENetMode::NM_Standalone ? TEXT("StandAlone") : TEXT("Not StandAlone")
		);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, GetWorld()->GetNetMode() == ENetMode::NM_Standalone ? FColor::Red : FColor::Green, LogString);
	}
#endif
	Server_Callback_ClientLoadingCompleted();

	if (Widget_HUD == nullptr || Widget_Main == nullptr) return;
	if (GetWorld()->GetNetMode() == ENetMode::NM_Standalone)
	{
		Widget_Main->OnLoadingComplete_Lobby();
		Widget_HUD->OnLoadingComplete_Lobby();
	}
	else
	{
		Widget_Main->OnLoadingComplete_Main();
		Widget_HUD->OnLoadingComplete_Main();
	}
	UGoSoccerPlayManager::ApplySavedSettings(GetGameInstance());
	if (UGameUserSettings* UserSettingsObj = UGameUserSettings::GetGameUserSettings())
	{
		Widget_Main->ApplyGameSettings(UserSettingsObj);
	}
	EMainUIState CurrState;
	if (UGoSoccerPlayManager::GetMainUIState(GetGameInstance(), CurrState))
	{
		Widget_HUD->OnUIStateChanged(CurrState);
		Widget_Main->OnUIStateChanged(CurrState);
	}
	EMainUIState CurrentMainUIState;
	UGoSoccerPlayManager::GetMainUIState(GetGameInstance(), CurrentMainUIState);
	if (CurrentMainUIState == EMainUIState::EMUIS_InLobby)
	{
		Client_SetCameraPosition(ECameraPosition::ECP_Lobby);
	}
	else Client_SetCameraPosition(ECameraPosition::ECP_VeryFirst);

	if (AudioComponent_BGM != nullptr)
	{
		AudioComponent_BGM->OnAudioFinished.RemoveDynamic(this, &AGoPlayerController::PlayLobbyBGM);
		AudioComponent_BGM->OnAudioFinished.AddDynamic(this, &AGoPlayerController::PlayLobbyBGM);
	}

}

void AGoPlayerController::Server_SpawnDoll_Implementation(FVector DollSpawnLocation, const EDollType& SpawnDollType)
{
	FPlayerCard tempPlayerCard;
	UGoSoccerPlayManager::GetPlayerCard(GetPlayerState<APlayerState>(), tempPlayerCard);
	EDollType NextDollType = EDollType::EDT_Default;
	bool bSucceedPutDoll = UGoSoccerPlayManager::PutDoll(GetWorld()->GetAuthGameMode(), this, DollSpawnLocation, tempPlayerCard.PlayerDollColor, SpawnDollType);
	if (bSucceedPutDoll)
	{
		if (CurrentPlayingSessionData.bEnableItem)
		{
			UGoSoccerPlayManager::GetNextDollType(GetWorld()->GetAuthGameMode(), NextDollType);
			//if (CurrentPlayingSessionData.PlayTurnRule == EPlayTurnRule::EPTR_RealTime)
			//{
			Client_NextDollType_Received(NextDollType);
			//}
		}
#if UE_EDITOR
		UEnum* EnumPtr = StaticEnum<EDollType>();
		if (EnumPtr)
		{
			FString ENumString0 = EnumPtr->GetValueAsString(SpawnDollType);
			FString ENumString1 = EnumPtr->GetValueAsString(NextDollType);
			UE_LOG(LogTemp, Log, TEXT("SpawnDollType : %s / NextDollType : %s"), *ENumString0,  *ENumString1);
		}
#endif
		//if (CurrentPlayingSessionData.PlayTurnRule == EPlayTurnRule::EPTR_TakeTurn)
		//{
		//	//Cylinder_LastPlaceedTime = GetWorld()->GetTimeSeconds();
		//	UGoSoccerPlayManager::Placed_NextDollType_Received(GetWorld()->GetGameState(), NextDollType);
		//	//NetMulticast_NextDollType_Received(NextDollType);
		//}
	}
}

//void AGoPlayerController::Client_ChangePlayerCardListView_Implementation(const FPlayerCard& NewPlayerCard)
//{
//	if (Widget_HUD != nullptr)
//	{
//		Widget_HUD->RefreshPlayerCardListView(NewPlayerCard);
//	}
//}

void AGoPlayerController::Server_GameStarted_Implementation(const FSessionCreateData& InSessionCreateData)
{
	bTakeTurn = InSessionCreateData.PlayTurnRule == EPlayTurnRule::EPTR_TakeTurn;
	//UObject* CameraObject;	
	//if (GetWorld() && UGoSoccerPlayManager::GetCameraObject(GetWorld()->GetAuthGameMode(), CameraObject))
	//{
	//	ACameraActor* SwitchCamera = Cast<ACameraActor>(CameraObject);
	//	FViewTargetTransitionParams ViewTargetTransitionParams;
	//	ViewTargetTransitionParams.BlendTime = 1.5f;
	//	ViewTargetTransitionParams.BlendFunction = EViewTargetBlendFunction::VTBlend_EaseIn;
	//	SetViewTarget(SwitchCamera, ViewTargetTransitionParams);
	//}
}

void AGoPlayerController::Server_StartGame_Implementation(const FSessionCreateData& SessionCreateData)
{
	// Tsudo
	// Throw Client Loading UI
	//Client_SetLoadingScreen(true, EPlayerTask::EPT_GameReady);
	// Gamemode -> Check All Player Ready
	bool bResult = UGoSoccerPlayManager::StartGameIfAllReady(GetWorld()->GetAuthGameMode(), SessionCreateData);
	bTakeTurn = SessionCreateData.PlayTurnRule == EPlayTurnRule::EPTR_TakeTurn;
	Client_NotifyGameStartResponse(bResult);
	// Then BroadCash Delegate
	// Callback - StartGame (Delegate Bind Required)
	// Callback Loading UI Clear
}

void AGoPlayerController::Server_ReadyGame_Implementation()
{
	UGoSoccerPlayManager::ReadyGame(GetPlayerState<APlayerState>());
}

//void AGoPlayerController::Client_DeclareLoadingTask_Implementation(bool bSet, EPlayerTask TodoTask)
//{
//	if (Widget_Loading == nullptr) return;
//	LoadingScreenTasks.FindOrAdd(TodoTask)
//	Widget_Loading->SetVisibility(bSet ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
//}

void AGoPlayerController::Server_TurnEnd_Implementation()
{
	Execute_TurnEnd(this); // UI, Controller boolean Set
	//UGoSoccerPlayManager::TurnEnd(this, GetWorld()->GetAuthGameMode());
}

void AGoPlayerController::Client_GameStarted_Implementation(const FSessionCreateData& InSessionCreateData)
{
	if (Widget_HUD == nullptr || Widget_Main == nullptr) return;

#if !UE_BUILD_SHIPPING
	FName PlayerName;
	UGoSoccerPlayManager::GetPlayerName(GetPlayerState<APlayerState>(), PlayerName);
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("Client_GameStarted : %s"), *PlayerName.ToString());
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
	}
#endif
	Widget_HUD->OnGameStarted(InSessionCreateData);
	UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), this, EMainUIState::EMUIS_GameStarted);
	Client_SetCameraPosition(ECameraPosition::ECP_InPlay);

	CylinderConfig = FCylinderConfig();
	if (InSessionCreateData.PlayTurnRule == EPlayTurnRule::EPTR_RealTime)
	{
		CylinderConfig.OverlayColor_0 = FLinearColor{ 0.400830f, 1.f, 0.234375f, 1.f };
		CylinderConfig.OverlayColor_1 = FLinearColor{ 0.968750f, 0.539004f, 0.080729f, 1.f };
		CylinderConfig.OverlayColor_2 = FLinearColor{ 0.9375f, 0.067866f, 0.095848f, 1.f };
		//CylinderConfig.OverlayColor_3 = FLinearColor{ 0.12f, 0.12f, 0.1f, 1.f };
	}
	else
	{
		FPlayerCard tempPlayerCard;
		//uint8 tempColor = 0;
		if (UGoSoccerPlayManager::GetPlayerCard(PlayerState, tempPlayerCard))
		{
			DollColor = tempPlayerCard.PlayerDollColor;
		}
		

		CylinderConfig.OverlayColor_0 = FLinearColor{ 0.12f, 0.12f, 0.1f, 1.f };
		CylinderConfig.OverlayColor_1 = FLinearColor{ 0.12f, 0.12f, 0.1f, 1.f };
		CylinderConfig.OverlayColor_2 = FLinearColor{ 0.12f, 0.12f, 0.1f, 1.f };
		//CylinderConfig.OverlayColor_3 = FLinearColor{ 0.12f, 0.12f, 0.1f, 1.f };

		Client_NextDollType_Received(EDollType::EDT_Default);
		Client_NextDollType_Received(EDollType::EDT_Default);
		Client_NextDollType_Received(EDollType::EDT_Default);

		//CylinderConfig.ChargePercent.X = 1.f - DollColor;
		//CylinderConfig.ChargePercent.Y = 1.f - DollColor;
		//CylinderConfig.ChargePercent.Z = 1.f - DollColor;

		CylinderConfig.Clock = 1.f;
		
		Client_Flicked_InvertCylinderCharge(1 - DollColor);
	}
	//bTakeTurn = InSessionCreateData.PlayTurnRule == EPlayTurnRule::EPTR_TakeTurn;
	//Widget_Main->OnUIStateChanged(EMainUIState::EMUIS_GameStarted);



	Client_SwitchBGM(SB_BGM_InGame);
}

void AGoPlayerController::Client_WinGame_HotSeat_Implementation(uint8 WinDollColor)
{
	if (Widget_HUD != nullptr)
	{
		Widget_HUD->OnWinGame_HotSeat(WinDollColor);
	}
	if (Widget_Main != nullptr)
	{
		Widget_Main->OnWinGame_HotSeat(WinDollColor);
	}
	Client_SwitchBGM(SB_BGM_GameOver);
}

void AGoPlayerController::Client_WinGame_Implementation()
{
	if (Widget_HUD != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Client_WinGame %s"), *GetFName().ToString());
		Widget_HUD->OnWinGame();
	}
	if (Widget_Main != nullptr)
	{
		Widget_Main->OnWinGame();
	}
	Client_SwitchBGM(SB_BGM_GameOver);
}

void AGoPlayerController::Client_LoseGame_Implementation()
{
	if (Widget_HUD != nullptr)
	{
		Widget_HUD->OnLoseGame();
	}
	if (Widget_Main != nullptr)
	{
		Widget_Main->OnLoseGame();
	}
	Client_SwitchBGM(SB_BGM_GameOver);
}

void AGoPlayerController::Client_DrawGame_Implementation()
{
	if (Widget_HUD != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Client_DrawGame %s"), *GetFName().ToString());
		Widget_HUD->OnDrawGame();
	}
	if (Widget_Main != nullptr)
	{
		Widget_Main->OnDrawGame();
	}
	Client_SwitchBGM(SB_BGM_GameOver);
}

void AGoPlayerController::Client_ChangeCreateSessionName_Implementation(const FName& NewSessionName)
{
//	CreateSessionName = NewSessionName;
//#if !UE_BUILD_SHIPPING
//	if (GEngine)
//	{
//		FString LogString = FString::Printf(
//			TEXT("Client_ChangeCreateSessionName NewSessionName : %s"), *CreateSessionName.ToString());
//		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
//	}
//#endif
}

void AGoPlayerController::Client_SetbEnablePlay_Implementation(bool e)
{
	bEnablePlay = e;
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(
			TEXT("Client_SetbEnablePlay(%s) : %s"), e ? TEXT("True") : TEXT("False"), *GetFName().ToString()
		);
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
	}
#endif

	if (!e && Widget_FlickWidget != nullptr)
	{
		Widget_FlickWidget->SetVisibility(ESlateVisibility::Collapsed);
		TracingLocation = FVector(0.f, 0.f, -999.f);
	}
}

void AGoPlayerController::Client_NotifyPlayerNameOfTurnPlayer_Implementation(const FName& TurnPlayerName)
{
	if (Widget_HUD != nullptr)
	{
		EPlayGameMode PlayGameMode;
		UGoSoccerPlayManager::GetPlayGameMode(GetGameInstance(), PlayGameMode);
		if (PlayGameMode == EPlayGameMode::EPGM_HotSeatGame)
		{
			FString TurnName = FString::Printf(TEXT("%s"), HotSeatDollColor == 0 ? TEXT("흑돌") : TEXT("백돌"));
			Widget_HUD->OnNotifyTurn(FName(TurnName));
		}
		else if (PlayGameMode == EPlayGameMode::EPGM_SingleAIPlay)
		{
			//FString TurnName = FString::Printf(TEXT("%s"), HotSeatDollColor == 0 ? TEXT("흑돌") : TEXT("백돌"));
			Widget_HUD->OnNotifyTurn(TurnPlayerName);
		}
		else
		{
			Widget_HUD->OnNotifyTurn(TurnPlayerName);
		}
	}
}

void AGoPlayerController::Client_NotifyFallenDollScore_Implementation(int32 InFallenDollScore)
{
	if (Widget_HUD)
	{
		Widget_HUD->OnNotifyFallenDollScore(InFallenDollScore);
	}
}

void AGoPlayerController::Client_NotifyGameStartResponse_Implementation(bool bWasSuccessful)
{
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(
			TEXT("Client_NotifyGameStartResponse : %s"),
			Delgeate_OnGameStartResponse.IsBound() ? TEXT("Bound") : TEXT("UnBound")
		);
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
	}
#endif
	Delgeate_OnGameStartResponse.Broadcast(bWasSuccessful);
}

void AGoPlayerController::Client_ThrowTemporalMessage_Implementation(const FString& ThrowMessage, ETemporalMessageType TemporalMessageType)
{
	//TODO HUD->(*Param)->MakeObject->AddToTileView
	if (Widget_HUD == nullptr) return;
	Widget_HUD->AddTemporalMessage(ThrowMessage, TemporalMessageType, 3.f);
}

void AGoPlayerController::Client_ThrowContinueMatchWidget_Implementation()
{
	if (Widget_HUD == nullptr) return;

	Widget_HUD->ThrowContinueMatch(MATCHCONTINUE_WIDGET_TIME);
}

void AGoPlayerController::Client_SwitchInGameSettingsWidget_Implementation(bool bEnable)
{
	if (bEnable == bESCSettingOn) return;
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(
			TEXT("Client_SwitchInGameSettingsWidget : %s"),
			bEnable ? TEXT("Enable") : TEXT("Disable")
		);
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
	}
#endif
	EMainUIState CurrMainUIState;
	UGoSoccerPlayManager::GetMainUIState(GetGameInstance(), CurrMainUIState);
	Widget_Main->SetSettingWidget_InGameManuPanelVisibility(true); // Enable InGame Settings Widget Mode(Add Give Up Button)
	Execute_SetMainUIState(this, bEnable ? EMainUIState::EMUIS_Settings : CurrMainUIState);
	bESCSettingOn = !bESCSettingOn;
	Client_SetbEnablePlay(!bESCSettingOn);
}

void AGoPlayerController::Client_SetLoadingScreen_Implementation(bool bEnable)
{
	UE_LOG(LogTemp, Log, TEXT("Client_SetLoadingScreen : %s"), bEnable ? TEXT("Enable") : TEXT("Disable"));
	if (Widget_HUD == nullptr) return;
	Widget_HUD->OnSetLoadingScreen(bEnable);
}

void AGoPlayerController::Client_SetSessionLoadingScreen_Implementation(bool bEnable, ESessionLoadingPhase LoadingPhase)
{
	CurrentSessionLoadingPhase = bEnable ? LoadingPhase : ESessionLoadingPhase::ESLP_None;

	UE_LOG(LogTemp, Log, TEXT("Client_SetSessionLoadingScreen : %s (%s)"),
		bEnable ? TEXT("Enable") : TEXT("Disable"),
		*UEnum::GetValueAsString(CurrentSessionLoadingPhase)
	);

	// 이 PC 는 OpenLevel / ClientTravel 로 파괴되므로, 다음 레벨의 새 PC 가 로딩 UI 를 이어서
	// 세울 수 있도록 진행 상태를 GameInstance 에 남긴다. (복원은 Load_Widget 에서)
	UGoSoccerPlayManager::MarkSessionLoadingPhase(GetGameInstance(), CurrentSessionLoadingPhase);

	if (Widget_HUD == nullptr) return;
	Widget_HUD->OnSetSessionLoadingScreen(bEnable, CurrentSessionLoadingPhase);
}

void AGoPlayerController::Client_ClientTravel_UILoaded_Implementation(EMainUIState SucceededUIState, const FString& URL, ETravelType TravelType, bool bSeamless)
{
	EMainUIState CurrUIState = EMainUIState::EMUIS_VeryFirst;
	UGoSoccerPlayManager::GetMainUIState(GetGameInstance(), CurrUIState);
	if (UGoSoccerPlayManager::MarkUIState_ClientTravelCallback(GetGameInstance(), SucceededUIState, CurrUIState))
	{
		ClientTravel(URL, TravelType, bSeamless);
	}
}

void AGoPlayerController::Client_StartCameraShake_Implementation(float InScale)
{
	if (CameraShake != nullptr) ClientStartCameraShake(CameraShake, InScale);
}

void AGoPlayerController::Client_StartGame_Implementation(const FSessionCreateData& InSessionCreateData)
{
	//bTakeTurn = InSessionCreateData.PlayTurnRule == EPlayTurnRule::EPTR_TakeTurn;
}

void AGoPlayerController::Client_SetOminousHUDPercent_Implementation(float CurrentTimeSec, float MaxTimeSec)
{
	if (Widget_HUD)
	{
		Widget_HUD->OnSetOminousHUD(CurrentTimeSec, MaxTimeSec);
	}
}

void AGoPlayerController::Client_AddRealtimeCooldownBonus_Implementation(float Portion_MaxCooldown)
{
	//TakeTurn_PlaceCoolDown += Portion_MaxCooldown * TakeTurn_PlaceCoolDownThreshold; // *1.5f;
	//TakeTurn_PlaceCoolDown = FMath::Clamp(TakeTurn_PlaceCoolDown, 0.f, TakeTurn_PlaceCoolDownThreshold * EXTRA_COOLDOWN_SAVE_PERCENT);
	//if (Widget_IngameCursor)
	//{
		//Widget_IngameCursor->SetCoolDown(TakeTurn_PlaceCoolDown / TakeTurn_PlaceCoolDownThreshold);
		//UE_LOG(LogTemp, Log, TEXT("%s - Client_AddRealtimeCooldownBonus : %f"), *GetFName().ToString(), TakeTurn_PlaceCoolDown);
	//}
}

void AGoPlayerController::Client_UpdateTooltip_SessionConfig_Implementation(const FSessionCreateData& HostSessionCreateData)
{
	if (Widget_Main)
	{
		Widget_Main->OnTooltipSet(HostSessionCreateData);
	}
}

void AGoPlayerController::Client_ReceiveChattingMessage_Implementation(const FName& SendPlayerName, const FText& TextMessage)
{
	if (Widget_HUD)
	{
		UObject* OutChatObject = Widget_HUD->CreateChattingBoxDataAsObject(SendPlayerName, TextMessage);
		UGoSoccerPlayManager::StackChattingMessage(GetGameInstance(), OutChatObject);
	}
}

void AGoPlayerController::Client_StartGameTimer_Implementation(double InStartTimeStamp, double InTimeLimit)
{
	StartTimeStamp = InStartTimeStamp;
	TimeLimit = InTimeLimit;
}

void AGoPlayerController::Client_ThrowScoringBoard_Implementation(bool bEnable, const TArray<FScoreBoard>& ScoreArr)
{
	if (Widget_HUD)
	{
		Widget_HUD->OnSetScoringBoard(bEnable, ScoreArr);
		Client_SetBGMVolume(0.5f);
		//Client_SwitchBGM(nullptr, 5.f, 1.f, 0.5f);
	}
}

void AGoPlayerController::Client_UpdateScoringBoard_Implementation(uint8 InDollColor, EScoreType ScoreType, int32 UpdatedScore)
{
	if (Widget_HUD)
	{
		Widget_HUD->OnUpdateScoringBoard(InDollColor, ScoreType, UpdatedScore);
		if (ScoreType == EScoreType::EST_OnBoardScore)		Client_SetBGMVolume(0.35f);
		else if (ScoreType == EScoreType::EST_FallScore)	Client_SetBGMVolume(0.2f);
		else if (ScoreType == EScoreType::EST_BreakScore)	Client_SetBGMVolume(0.f);
	}
}

void AGoPlayerController::Client_NextDollType_Received_Implementation(const EDollType InDollType)
{
	CylinderConfig.ShadowTextureIndex.X = CylinderConfig.ShadowTextureIndex.Y;
	CylinderConfig.ShadowTextureIndex.Y = CylinderConfig.ShadowTextureIndex.Z;
	CylinderConfig.ShadowTextureIndex.Z = StaticCast<uint8>(InDollType);

	if (CurrentPlayingSessionData.PlayTurnRule == EPlayTurnRule::EPTR_TakeTurn)
	{
		//CylinderConfig.ChargePercent.X = CylinderConfig.ChargePercent.Y;
		//CylinderConfig.ChargePercent.Y = CylinderConfig.ChargePercent.Z;
		//CylinderConfig.ChargePercent.Z = CylinderConfig.ChargePercent.Z > 0.f ? 0.f : 1.f;
		Cylinder_LastPlaceedTime = GetWorld()->GetTimeSeconds();
	}
}

void AGoPlayerController::Server_Flicked_InvertCylinderCharge_Implementation(int32 InFlickCylinderInvert)
{
	UGoSoccerPlayManager::Flicked_InvertCylinderCharge(GetWorld()->GetGameState(), InFlickCylinderInvert);
	//NetMulticast_Flicked_InvertCylinderCharge(InFlickCylinderInvert);
}

void AGoPlayerController::OnHoveredHoverableWidget_Implementation(UUserWidget* InHoveredWidget)
{
	if (HoveredWidget.IsValid() && HoveredWidget.Get())
	{
		IIGoWidget::Execute_OnHoveredHoverableWidget(HoveredWidget.Get(), InHoveredWidget);
	}
	HoveredWidget = InHoveredWidget;
}

void AGoPlayerController::OnAddRealtimeCooldownBonus(float Portion_MaxCooldown)
{
	//UE_LOG(LogTemp, Log, TEXT("%s - OnAddRealtimeCooldownBonus"), *GetFName().ToString());
	Server_AddRealtimeCooldownBonus(Portion_MaxCooldown);

	//Client_AddRealtimeCooldownBonus(Portion_MaxCooldown);
	//Execute_AddRealtimeCoolDownBonus(this, Portion_MaxCooldown);
}

void AGoPlayerController::OnAddRealtimeCooldownBonus_Flick(float Portion_MaxCooldown)
{
	Client_AddRealtimeCooldownBonus_Flick(Portion_MaxCooldown);
}

void AGoPlayerController::Callback_AimFlicker(FVector InAimDirection)
{
	float DirSize = FMath::Clamp(InAimDirection.Size2D(), 0.f, 500.f);
	float Volume = FMath::Pow(DirSize / 500.f, 2.f);
	if (AudioComponent_SFX_Flicker != nullptr)
	{
		AudioComponent_SFX_Flicker->SetVolumeMultiplier(Volume * 2.f);
		UE_LOG(LogTemp, Log, TEXT("Callback_AimFlicker : %f, %f"), InAimDirection.Size2D(), Volume);
	}
}

void AGoPlayerController::OnRep_bItsMyTurn()
{
	if (!IsLocalController()) return;

	if (bTakeTurn)
	{
		if (bItsMyTurn)
		{
			Widget_HUD->OnItsMyTurn();
		}
		else
		{
			Widget_HUD->OnItWasMyTurn();
		}
	}
}

void AGoPlayerController::OnRep_PlaceCoolDown()
{
	if (!IsLocalController()) return;
#if UE_EDITOR
	UE_LOG(LogTemp, Log, TEXT("OnRep_PlaceCoolDown : %f"), TakeTurn_PlaceCoolDown);
#endif
	if (Widget_IngameCursor)
	{
		Widget_IngameCursor->SetCoolDown(TakeTurn_PlaceCoolDown / TakeTurn_PlaceCoolDownThreshold);
	}
	if (Widget_HUD && CurrentPlayingSessionData.PlayTurnRule == EPlayTurnRule::EPTR_RealTime)
	{
		CylinderConfig.Clock = FMath::Clamp(GetWorld()->GetTimeSeconds() - Cylinder_LastPlaceedTime, 0.f, 1.f);// + FlickCylinderSpinWeight;
		CylinderConfig.ChargePercent.X = FMath::Clamp(TakeTurn_PlaceCoolDown / TakeTurn_PlaceCoolDownThreshold, 0.f, 1.f);
		CylinderConfig.ChargePercent.Y = FMath::Clamp((TakeTurn_PlaceCoolDown - TakeTurn_PlaceCoolDownThreshold			) / TakeTurn_PlaceCoolDownThreshold, 0.f, 1.f);
		CylinderConfig.ChargePercent.Z = FMath::Clamp((TakeTurn_PlaceCoolDown - TakeTurn_PlaceCoolDownThreshold * 2.f	) / TakeTurn_PlaceCoolDownThreshold, 0.f, 1.f);
		CylinderConfig.ChargePercent.W = FMath::Clamp((TakeTurn_PlaceCoolDown - TakeTurn_PlaceCoolDownThreshold * 3.f	) / TakeTurn_PlaceCoolDownThreshold, 0.f, 1.f);
		Widget_HUD->UpdateCylinder(CylinderConfig);
	}
}

void AGoPlayerController::OnRep_FlickLocking()
{
	//UGoSoccerPlayManager::Flicked_Locking(this, GetWorld()->GetGameState(), bFlickLocking);
}

void AGoPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGoPlayerController, bItsMyTurn);
	DOREPLIFETIME(AGoPlayerController, DollColor);
	DOREPLIFETIME(AGoPlayerController, TakeTurn_PlaceCoolDown);
	DOREPLIFETIME(AGoPlayerController, bTakeTurn);
	DOREPLIFETIME(AGoPlayerController, bFlickLocking);
}

void AGoPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FVector2D CurrMousePos;
	if (!bTakeTurn) GetMousePosition(CurrMousePos.X, CurrMousePos.Y);

	if (IsLocalController() && Widget_HUD)
	{
		bool bIsTurnBased = CurrentPlayingSessionData.PlayTurnRule == EPlayTurnRule::EPTR_TakeTurn;
		float CurrTimeSec = GetWorld()->GetTimeSeconds();
		if (bFlickLocking)
		{
			float tempf = FlickCylinderSpinWeight + DeltaSeconds * 1.25f;
			FlickCylinderSpinWeight = tempf > 1.f ? tempf - 1.f : tempf;
		}
		else if (FlickCylinderSpinWeight > 0.f)
		{
			FlickCylinderSpinWeight = FMath::Lerp(FlickCylinderSpinWeight, FlickCylinderSpinWeight < 0.5f ? 0.f : 1.f, bIsTurnBased ? 0.125f : 0.45f);
			if (FMath::IsNearlyZero(FlickCylinderSpinWeight) || FMath::IsNearlyEqual(FlickCylinderSpinWeight, 1.f))
			{
				FlickCylinderSpinWeight = 0.f;
				CylinderConfig.bUseClockCurve = true;
			}
		}
		else if (!CylinderConfig.bUseClockCurve)
		{
			CylinderConfig.bUseClockCurve = true;
		}

		if (bIsTurnBased && FlickCylinderInvert != CylinderConfig.ChargePercent.X)
		{
			CylinderConfig.ChargePercent.X = FMath::Lerp(CylinderConfig.ChargePercent.X, FlickCylinderInvert, 0.0625f);
			CylinderConfig.ChargePercent.Y = FMath::Lerp(CylinderConfig.ChargePercent.Y, FlickCylinderInvert, 0.0625f);
			CylinderConfig.ChargePercent.Z = FMath::Lerp(CylinderConfig.ChargePercent.Z, FlickCylinderInvert, 0.0625f);

			bool bX = FMath::IsNearlyEqual(CylinderConfig.ChargePercent.X, FlickCylinderInvert, 0.1f);
			bool bY = FMath::IsNearlyEqual(CylinderConfig.ChargePercent.Y, FlickCylinderInvert, 0.1f);
			bool bZ = FMath::IsNearlyEqual(CylinderConfig.ChargePercent.Z, FlickCylinderInvert, 0.1f);

			if (bX || bY || bZ)
			{
				CylinderConfig.ChargePercent.X = FlickCylinderInvert;
				CylinderConfig.ChargePercent.Y = FlickCylinderInvert;
				CylinderConfig.ChargePercent.Z = FlickCylinderInvert;
				//FlickCylinderInvert = -1;
			}
		}

		if (bIsTurnBased)
		{
			CylinderConfig.Clock = FMath::Clamp(CurrTimeSec - Cylinder_LastPlaceedTime, 0.f, 1.f) + FlickCylinderSpinWeight;

			Widget_HUD->UpdateCylinder(CylinderConfig);
		}
	}

	if (IsLocalController() &&
		//(
		!FMath::IsNearlyZero(CameraToMoveDirection.Size()) &&
		//	|| (!bTakeTurn && FVector2D::Distance(PrevMouseScreenPos, CurrMousePos) > 10.f)) &&
		CurrentPossessingCamera != nullptr)
	{
		if (!GoBoardObjectPtr.IsValid())
		{
			GoBoardObjectPtr = GetGoBoardObjectPtr();
		}
		if (GoBoardObjectPtr.IsValid())
		{
			FVector CurrentCamPos = CurrentPossessingCamera->GetActorLocation();
			FVector PostCamPos = FMath::VInterpTo(
				CurrentCamPos, CurrentCamPos + CameraToMoveDirection, DeltaSeconds, 3.f
			);
			UGoSoccerPlayManager::GetPossibleCameraLocation(GoBoardObjectPtr.Get(), PostCamPos);
			CurrentPossessingCamera->SetActorLocation(PostCamPos);
			//CameraToMoveDirection -= PostCamPos - CurrentCamPos;
			//UE_LOG(LogTemp, Log, TEXT("CamPos : %s Direction : %s"), *CurrentCamPos.ToString(), *CameraToMoveDirection.ToString());
			//else if (!bClicking)
			//{
			//	int32 ViewX;
			//	int32 ViewY;
			//	GetViewportSize(ViewX, ViewY);
			//	FVector2D MouseDirection = CurrMousePos - FVector2D(ViewX / 2.f, ViewY / 2.f);
			//	MouseDirection /= 10.f;
			//	FVector PostCamPos = FVector(GoBoardLocation.X - MouseDirection.Y, GoBoardLocation.Y + MouseDirection.X, CurrentCamPos.Z);
			//	PostCamPos = FMath::VInterpTo(
			//		CurrentCamPos, PostCamPos, DeltaSeconds, 3.f
			//	);
			//	UGoSoccerPlayManager::GetPossibleCameraLocation(GoBoardObjectPtr.Get(), PostCamPos);
			//	CurrentPossessingCamera->SetActorLocation(PostCamPos);
			//	PrevMouseScreenPos = CurrMousePos;
			//}
		}
	}

	if (HasAuthority() && CurrentPlayingSessionData.PlayTurnRule == EPlayTurnRule::EPTR_RealTime
		&& (TakeTurn_PlaceCoolDown < TakeTurn_PlaceCoolDownThreshold * EXTRA_COOLDOWN_SAVE_PERCENT)
		)
	{
		//Client_AddRealtimeCooldownBonus(DeltaSeconds / TakeTurn_PlaceCoolDownThreshold);
		//Server_AddRealtimeCooldownBonus(DeltaSeconds / TakeTurn_PlaceCoolDownThreshold);
		float tempAdded = TakeTurn_PlaceCoolDown + DeltaSeconds;
		TakeTurn_PlaceCoolDown = FMath::Clamp(tempAdded, -TakeTurn_PlaceCoolDownThreshold, TakeTurn_PlaceCoolDownThreshold * EXTRA_COOLDOWN_SAVE_PERCENT);
		if (IsLocalController()) OnRep_PlaceCoolDown();
		//TakeTurn_PlaceCoolDown = FMath::Clamp(TakeTurn_PlaceCoolDown + DeltaSeconds, 0.f, TakeTurn_PlaceCoolDownThreshold * 1.5f);
		//if (Widget_IngameCursor) Widget_IngameCursor->SetCoolDown(TakeTurn_PlaceCoolDown / TakeTurn_PlaceCoolDownThreshold);
	}
	if (IsLocalController() && CurrentPlayingSessionData.PlayTurnRule == EPlayTurnRule::EPTR_RealTime
		&& (TakeTurn_FlickCoolDown < TakeTurn_FlickCoolDownThreshold * 1.1f)
		)
	{
		Client_AddRealtimeCooldownBonus_Flick(DeltaSeconds / TakeTurn_FlickCoolDownThreshold);
	}
	auto* GS = GetCachedGameState();
	if (IsLocalController() && Widget_HUD && GS && StartTimeStamp > 0.f)
	{
		Widget_HUD->OnSetGameTimer(TimeLimit - (GS->GetServerWorldTimeSeconds() - StartTimeStamp));
	}

	if (!bItsMyTurn) return;


	if (FingerMode == 0 && bTakeTurn)
	{
#if !UE_BUILD_SHIPPING
		//DrawDebugSphere(GetWorld(), TracingLocation, 1.f, 8, FColor::Green);
#endif
	}

	if (!bClicking)
	{
		FHitResult HitResult;
		bool bHit = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, HitResult);
		EPlayGameMode CurrentPlayGameMode;
		UGoSoccerPlayManager::GetPlayGameMode(GetGameInstance(), CurrentPlayGameMode);
		if (bHit && UGoSoccerPlayManager::IsGoDoll(HitResult.GetActor()))
		{
			if (MouseOverObject != nullptr && MouseOverObject.Get() != HitResult.GetActor())
			{
				UGoSoccerPlayManager::MouseOverEnd(MouseOverObject.Get());
			}
			bool bIsFlickable = false;
			if (CurrentPlayGameMode != EPlayGameMode::EPGM_HotSeatGame && CurrentPlayGameMode != EPlayGameMode::EPGM_SingleAIPlay)
			{
				bIsFlickable = HitResult.GetActor()->Owner == this;
			}
			else
			{
				uint8 OverDollColor;
				UGoSoccerPlayManager::GetDollColor(HitResult.GetActor(), OverDollColor);
				bIsFlickable = (CurrentPlayGameMode == EPlayGameMode::EPGM_SingleAIPlay ? 0 : HotSeatDollColor) == OverDollColor;
			}
			if (bIsFlickable)
			{
				bool IsFallen = true;
				UGoSoccerPlayManager::GetDollFallFromBoard(HitResult.GetActor(), IsFallen);
				bIsFlickable = !IsFallen;
			}
			UGoSoccerPlayManager::MouseOver(
				HitResult.GetActor(),
				bIsFlickable ? EDollStencilValue::EDSV_MouseOver : EDollStencilValue::EDSV_MouseOver_NotMine
			);
			MouseOverObject = HitResult.GetActor();
		}
		else
		{
			UGoSoccerPlayManager::MouseOverEnd(MouseOverObject.Get());
			MouseOverObject = nullptr;
		}
	}
	else
	{
		if (IsLocalController() && MouseOverObject.IsValid() && UGoSoccerPlayManager::IsGoBoard(MouseOverObject.Get()))
		{
			UGoSoccerPlayManager::SetTranclucentDollLocation(MouseOverObject.Get(), TracingLocation);
		}
	}

//	if (FingerMode == 0 && bTakeTurn)
//	{
//#if !UE_BUILD_SHIPPING
//		//DrawDebugSphere(GetWorld(), TracingLocation, 1.f, 8, FColor::Green);
//#endif
//		if (IsLocalController() && MouseOverObject.IsValid() && UGoSoccerPlayManager::IsGoBoard(MouseOverObject.Get()))
//		{
//			UGoSoccerPlayManager::SetTranclucentDollLocation(MouseOverObject.Get(), TracingLocation);
//		}
//	}
//	else
//	{
//		if (!bClicking)
//		{
//			FHitResult HitResult;
//			bool bHit = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, HitResult);
//			EPlayGameMode CurrentPlayGameMode;
//			UGoSoccerPlayManager::GetPlayGameMode(GetGameInstance(), CurrentPlayGameMode);
//			if (bHit && UGoSoccerPlayManager::IsGoDoll(HitResult.GetActor()))
//			{
//				if (MouseOverObject != nullptr && MouseOverObject.Get() != HitResult.GetActor())
//				{
//					UGoSoccerPlayManager::MouseOverEnd(MouseOverObject.Get());
//				}
//				bool bIsFlickable = false;
//				if (CurrentPlayGameMode != EPlayGameMode::EPGM_HotSeatGame && CurrentPlayGameMode != EPlayGameMode::EPGM_SingleAIPlay)
//				{
//					bIsFlickable = HitResult.GetActor()->Owner == this;
//				}
//				else
//				{
//					uint8 OverDollColor;
//					UGoSoccerPlayManager::GetDollColor(HitResult.GetActor(), OverDollColor);
//					bIsFlickable = (CurrentPlayGameMode == EPlayGameMode::EPGM_SingleAIPlay ? 0 : HotSeatDollColor) == OverDollColor;
//				}
//				if (bIsFlickable)
//				{
//					bool IsFallen = true;
//					UGoSoccerPlayManager::GetDollFallFromBoard(HitResult.GetActor(), IsFallen);
//					bIsFlickable = !IsFallen;
//				}
//				UGoSoccerPlayManager::MouseOver(
//					HitResult.GetActor(),
//					bIsFlickable ? EDollStencilValue::EDSV_MouseOver : EDollStencilValue::EDSV_MouseOver_NotMine
//				);
//				MouseOverObject = HitResult.GetActor();
//			}
//			else
//			{
//				UGoSoccerPlayManager::MouseOverEnd(MouseOverObject.Get());
//				MouseOverObject = nullptr;
//			}
//		}
//	}

	//FVector2D CurrMousePos;
	//GetMousePosition(CurrMousePos.X, CurrMousePos.Y);
	//if (CurrentPossessingCamera != nullptr && FVector2D::Distance(PrevMouseScreenPos, CurrMousePos) > 10.f)
	//{
	//	PrevMouseScreenPos = CurrMousePos;
	//	int32 ViewX;
	//	int32 ViewY;
	//	GetViewportSize(ViewX, ViewY);
	//	FVector2D MouseRotDirection = CurrMousePos - FVector2D(ViewX / 2.f, ViewY / 2.f);
	//	UE_LOG(LogTemp, Log, TEXT("MouseRotDirection : %s"), *MouseRotDirection.ToString());
	//	FRotator CurrRot = CurrentPossessingCamera->GetActorRotation();
	//	//CurrRot.Pitch = MouseRotDirection.X / 10.f;
	//	CurrRot.Roll = MouseRotDirection.Y / 100.f;
	//	CurrentPossessingCamera->SetActorRotation(CurrRot);
	//	CurrentPossessingCamera->GetActorLocation();
	//}
}

void AGoPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(LMBAction,	ETriggerEvent::Started, this, &AGoPlayerController::LMBStarted);
		EnhancedInputComponent->BindAction(LMBAction,	ETriggerEvent::Triggered, this, &AGoPlayerController::LMBTriggered);
		EnhancedInputComponent->BindAction(LMBAction,	ETriggerEvent::Completed, this, &AGoPlayerController::LMBCompleted);

		EnhancedInputComponent->BindAction(RMBAction,	ETriggerEvent::Started, this, &AGoPlayerController::RMBStarted);
		EnhancedInputComponent->BindAction(RMBAction,	ETriggerEvent::Triggered, this, &AGoPlayerController::RMBTriggered);
		EnhancedInputComponent->BindAction(RMBAction,	ETriggerEvent::Completed, this, &AGoPlayerController::RMBCompleted);

		//EnhancedInputComponent->BindAction(TabAction,	ETriggerEvent::Triggered, this, &AGoPlayerController::TabTriggered);
		//EnhancedInputComponent->BindAction(TabAction,	ETriggerEvent::Completed, this, &AGoPlayerController::TabCompleted);
		EnhancedInputComponent->BindAction(ESCAction,	ETriggerEvent::Completed, this, &AGoPlayerController::ESCCompleted);

		EnhancedInputComponent->BindAction(ScrollAction,ETriggerEvent::Started, this, &AGoPlayerController::ScrollStarted);
		EnhancedInputComponent->BindAction(ScrollAction,ETriggerEvent::Triggered, this, &AGoPlayerController::ScrollTriggered);
		EnhancedInputComponent->BindAction(ScrollAction,ETriggerEvent::Completed, this, &AGoPlayerController::ScrollCompleted);

		EnhancedInputComponent->BindAction(MoveAction,	ETriggerEvent::Triggered, this, &AGoPlayerController::MoveTriggered);

		EnhancedInputComponent->BindAction(EnterAction,	ETriggerEvent::Triggered, this, &AGoPlayerController::EnterCompleted);
	}
}

void AGoPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		Load();
	}
}

void AGoPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);
}

bool AGoPlayerController::InputKey(const FInputKeyParams& Params)
{
	bool rtn = Super::InputKey(Params);
	return rtn;
}

void AGoPlayerController::Load()
{
	if (!bLoad_Widget)
	{
		bLoad_Widget = Load_Widget();
	}
	if (!bLoad_GameInstance)
	{
		bLoad_GameInstance = Load_GameInstance();
	}
	if (!bLoad_SessionInterface)
	{
		bLoad_SessionInterface = Load_SessionInterface();
	}
	if (bLoad_Widget && bLoad_GameInstance && bLoad_SessionInterface)
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(
				TEXT("%s - Loading Complete : Delegate : %s"),
				GetWorld()->GetNetMode() == ENetMode::NM_Client ? TEXT("Client") : TEXT("Server"),
				Delegate_OnClientLoadingCompleted.IsBound() ? TEXT("Bound") : TEXT("Not Bound")
			);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
		}
#endif
		// Create / Join 요청 시 켜둔 세션 로딩 UI 의 해제 지점.
		// 요청을 낸 PC 는 OpenLevel / ClientTravel 로 이미 파괴됐으므로,
		// 트래블 후 새 PC 의 로드(위젯 + GameInstance + 세션 인터페이스)가 전부 끝난 여기가 "완료" 시점이다.
		Execute_SetSessionLoadingScreen(this, false, ESessionLoadingPhase::ESLP_None);

		Delegate_OnClientLoadingCompleted.Broadcast();
		return;
	}
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AGoPlayerController::Load);
	}
}

bool AGoPlayerController::Load_Widget()
{
	if (!IsLocalController()) return true; // Client Only

	if (!bLoad_CompleteBind)
	{
		if (Delegate_OnClientLoadingCompleted.IsBound() && DelegateHandle_OnClientLoadingCompleted.IsValid())
		{
			Delegate_OnClientLoadingCompleted.Remove(DelegateHandle_OnClientLoadingCompleted);
			DelegateHandle_OnClientLoadingCompleted.Reset();
		}
		DelegateHandle_OnClientLoadingCompleted = Delegate_OnClientLoadingCompleted.AddUFunction(this, TEXT("Client_Callback_ClientLoadingCompleted"));
		if (!Delegate_OnClientLoadingCompleted.IsBound() || !DelegateHandle_OnClientLoadingCompleted.IsValid()) return false;
	}
	if (!bLoad_HUD)
	{
		if (WidgetClass_HUD == nullptr) return false;
		Widget_HUD = CreateWidget<UStageHUD>(this, WidgetClass_HUD);
		if (Widget_HUD == nullptr) return false;
		Widget_HUD->AddToViewport(255);
		Widget_HUD->SetVisibility(ESlateVisibility::HitTestInvisible);
		bLoad_HUD = true;
	}
	if (Delegate_OnSwitchFingerMode.IsBound() && DelegateHandle_OnSwitchFingerMode.IsValid()) // If Bound -> Reset
	{
		Delegate_OnSwitchFingerMode.Remove(DelegateHandle_OnSwitchFingerMode);
		DelegateHandle_OnSwitchFingerMode.Reset();
	}
	DelegateHandle_OnSwitchFingerMode = Delegate_OnSwitchFingerMode.AddUFunction(
		Widget_HUD, TEXT("CallBack_SwitchFingerMode")
	);

	if (!bLoad_FlickWidget)
	{
		if (WidgetClass_FlickWidget == nullptr) return false;
		Widget_FlickWidget = CreateWidget<UFlickWidget>(this, WidgetClass_FlickWidget);
		if (Widget_FlickWidget == nullptr) return false;
		Widget_FlickWidget->AddToViewport(254);
		Widget_FlickWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
		Widget_FlickWidget->SetVisibility(ESlateVisibility::Collapsed);

		if (Delegate_OnAimFlicker.IsBound() && DelegateHandle_OnAimFlicker.IsValid()) // If Bound -> Reset
		{
			Delegate_OnAimFlicker.Remove(DelegateHandle_OnAimFlicker);
			DelegateHandle_OnAimFlicker.Reset();
		}
		if (Delegate_OnAimFlicker.IsBound() && DelegateHandle_OnAimFlicker_Controller.IsValid()) // If Bound -> Reset
		{
			Delegate_OnAimFlicker.Remove(DelegateHandle_OnAimFlicker_Controller);
			DelegateHandle_OnAimFlicker_Controller.Reset();
		}
		DelegateHandle_OnAimFlicker = Delegate_OnAimFlicker.AddUFunction(
			Widget_FlickWidget, TEXT("Callback_AimFlicker")
		);
		DelegateHandle_OnAimFlicker_Controller = Delegate_OnAimFlicker.AddUFunction(
			this, TEXT("Callback_AimFlicker")
		);
		bLoad_FlickWidget = true;
		if (!DelegateHandle_OnAimFlicker.IsValid() || !Delegate_OnAimFlicker.IsBound()) return false;
	}


	if (!bLoad_MainWidget)
	{
		EMainUIState CurrentMainUIState;
		if (!UGoSoccerPlayManager::GetMainUIState(GetGameInstance(), CurrentMainUIState)) return false;
		if (WidgetClass_Main == nullptr) return false;
		Widget_Main = CreateWidget<UMainWidget>(this, WidgetClass_Main);
		if (Widget_Main == nullptr) return false;
		Widget_Main->AddToViewport();
		Widget_Main->SetVisibility(ESlateVisibility::Visible);
		Widget_Main->OnUIStateChanged(CurrentMainUIState);
		if (Delgeate_OnGameStartResponse.IsBound() && DelegateHandle_OnGameStartResponse.IsValid())
		{
			Delgeate_OnGameStartResponse.Remove(DelegateHandle_OnGameStartResponse);
			DelegateHandle_OnGameStartResponse.Reset();
		}
		DelegateHandle_OnGameStartResponse = Delgeate_OnGameStartResponse.AddUFunction(
			Widget_Main, TEXT("Callback_OnGameStartResponse")
		);
		if (!DelegateHandle_OnGameStartResponse.IsValid() || !Delgeate_OnGameStartResponse.IsBound()) return false;
		bLoad_MainWidget = true;
	}

	if (!bLoad_IngameCursor)
	{
		if (WidgetClass_IngameCursor == nullptr) return false;
		Widget_IngameCursor = CreateWidget<UIngameCursor>(this, WidgetClass_IngameCursor);
		if (Widget_IngameCursor == nullptr) return false;
		// TODO Move To GameStart
		SetMouseCursorWidget(EMouseCursor::Default, Widget_IngameCursor);
		bShowMouseCursor = true;
		bLoad_IngameCursor = true;
	}

	if (GetGameInstance() != nullptr)
	{
		EMainUIState SucceededUIState;
		EMainUIState FailedUIState; // ?
		UGoSoccerPlayManager::GetClientTravelCallbackUIState(GetGameInstance(), SucceededUIState, FailedUIState);

		if (SucceededUIState != EMainUIState::EMUIS_Default)
		{
#if !UE_BUILD_SHIPPING
			if (GEngine)
			{
				FString S_UIState = UEnum::GetValueAsString(SucceededUIState);
				FString LogString = FString::Printf(TEXT("Load Widget_SetUIState : %s"), *S_UIState);
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, LogString);
			}
#endif
			Widget_Main->OnUIStateChanged(SucceededUIState);
			Widget_HUD->OnUIStateChanged(SucceededUIState);
		}
		else
		{
			EMainUIState CurrMainUIState;
			if (!UGoSoccerPlayManager::GetMainUIState(GetGameInstance(), CurrMainUIState)) return false;
			Widget_Main->OnUIStateChanged(CurrMainUIState);
			Widget_HUD->OnUIStateChanged(CurrMainUIState);
		}

		// 세션 생성/조인 중에 트래블로 넘어온 경우, 이전 PC 가 띄웠던 로딩 UI 를 여기서 다시 세운다.
		// (Load() 가 모두 끝나면 해제된다)
		ESessionLoadingPhase PendingSessionLoadingPhase = ESessionLoadingPhase::ESLP_None;
		if (UGoSoccerPlayManager::GetSessionLoadingPhase(GetGameInstance(), PendingSessionLoadingPhase) &&
			PendingSessionLoadingPhase != ESessionLoadingPhase::ESLP_None &&
			CurrentSessionLoadingPhase != PendingSessionLoadingPhase)
		{
			CurrentSessionLoadingPhase = PendingSessionLoadingPhase;
			Widget_HUD->OnSetSessionLoadingScreen(true, PendingSessionLoadingPhase);
		}
	}
	return true;
}

bool AGoPlayerController::Load_GameInstance()
{
	if (!IsLocalController()) return true; // Client Only

	if (!bLoad_Widget || !Widget_Main) return false;

	// Move To Completed Callback
	//if (AudioComponent_BGM == nullptr) return false;
	//AudioComponent_BGM->OnAudioFinished.RemoveDynamic(this, &AGoPlayerController::PlayLobbyBGM);
	//AudioComponent_BGM->OnAudioFinished.AddDynamic(this, &AGoPlayerController::PlayLobbyBGM);
	if (AudioComponent_SFX_Flicker == nullptr) return false;
	AudioComponent_SFX_Flicker->SetSound(SB_SFX_Flicker);
	Client_SwitchBGM(SB_BGM, 1.f, 0.f);
	//FInputModeGameAndUI InputMode;
	//InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	//SetInputMode(InputMode);
	return UGoSoccerPlayManager::BindDelegate_Widget(GetGameInstance(), Widget_Main);
}

bool AGoPlayerController::Load_SessionInterface()
{
	// NetId 복제 / Steam 초기화를 기다리는 최대 틱 수. (Load() 는 매 틱 재시도된다)
	static constexpr int32 POSTSTEAMID_MAX_RETRY = 300;

	if (!IsLocalController()) return true; // Client Only
	// PlayerJoinedSessionComplete -> Client_PlayerJoinedSessionComplete 가 Widget_Main 을 참조하므로 대기
	if (!bLoad_Widget || !Widget_Main) return false;
	// PlayerState 가 아직 없으면 ChangePlayerName 이 조용히 실패해서
	// 호스트 쪽 PlayerCard 의 이름이 비어있는 채로 남는다. 준비될 때까지 기다린다.
	if (GetPlayerState<APlayerState>() == nullptr) return false;
	if (!bSent_PlayerNickname) Client_DebugPlayerNickname();
	//Server_DebugPlayerNickname();

	// "조인으로 원격 호스트에 붙은 클라이언트인지" 는 NetMode 로 판단한다.
	// GameInstance 의 PlayGameMode 는 트래블 전 UI 흐름(뒤로가기 / 초대 수락 경로)에서
	// EPGM_Default 로 되돌아갈 수 있고, 그러면 아래 통지가 통째로 스킵되어
	// 클라이언트의 MainUIState 가 InLobby 로 바뀌지 않고 호스트의 PlayerCard 도 갱신되지 않는다.
	const bool bJoinedClient = (GetNetMode() == ENetMode::NM_Client);

	EPlayGameMode CurrentPlayGameMode = EPlayGameMode::EPGM_Default;
	UGoSoccerPlayManager::GetPlayGameMode(GetGameInstance(), CurrentPlayGameMode);
	if (bJoinedClient && CurrentPlayGameMode != EPlayGameMode::EPGM_OnlineSession)
	{
		// 접속은 이미 성공했으므로 어긋난 PlayGameMode 를 여기서 바로잡는다.
		UGoSoccerPlayManager::SetPlayGameMode(GetGameInstance(), EPlayGameMode::EPGM_OnlineSession);
		CurrentPlayGameMode = EPlayGameMode::EPGM_OnlineSession;
	}
	if (CurrentPlayGameMode != EPlayGameMode::EPGM_OnlineSession) return true;

	// 아바타용 SteamID 등록. 실패해도 조인 통지를 막지 않는다.
	// (여기서 무조건 false 를 리턴하면 Load() 가 끝나지 않아 세션 로딩 UI 가 걸린 채로 남고
	//  호스트에도 조인 통지가 가지 않는다)
	if (!bPosted_SteamID && UGoSoccerPlayManager::GetSteamOSSOnline(GetGameInstance()))
	{
		int64 SteamID = 0;
		if (UGoSoccerPlayManager::GetSteamID(GetGameInstance(), SteamID))
		{
#if !UE_BUILD_SHIPPING
			UE_LOG(LogTemp, Log, TEXT("Load_SessionInterface : CSteamID Accquired - %lld"), SteamID);
#endif
			// PlayerState 의 NetId 가 아직 복제되지 않았으면 false 가 돌아온다.
			bPosted_SteamID = Execute_PlayerJoinedOrCreatedSessionComplete(this, SteamID);
		}
		if (!bPosted_SteamID && PostSteamID_RetryCount < POSTSTEAMID_MAX_RETRY)
		{
			++PostSteamID_RetryCount;
			return false;
		}
	}

	// 호스트가 아니라 조인으로 접속한 클라이언트만 수행한다.
	// (기존에는 GoSoccerGameInstance::OnJoinSessionComplate 에서 호출했으나,
	//  그 시점의 PlayerController 는 접속 직후 파괴되므로 Server RPC 가 호스트에 도달하지 않았다.)
	if (bJoinedClient && !bNotified_PlayerJoinedSession)
	{
		bNotified_PlayerJoinedSession = true;
		UE_LOG(LogTemp, Log, TEXT("Load_SessionInterface : Notify PlayerJoinedSessionComplete (SteamIDPosted : %s)"),
			bPosted_SteamID ? TEXT("True") : TEXT("False"));
		return Execute_PlayerJoinedSessionComplete(this);
	}
	return true;
}

void AGoPlayerController::LMBStarted(const FInputActionValue& Value)
{
	if (Widget_IngameCursor) Widget_IngameCursor->LMBPressed();

	bClicking = true;
	if (!bItsMyTurn) return;

	FHitResult HitResult;
	bool bHit = GetHitResultUnderCursor(
		FingerMode < 1 ? ECollisionChannel::ECC_EngineTraceChannel1 : ECollisionChannel::ECC_Visibility,
		true, HitResult
	);
	if (FingerMode < 1)
	{
		if (!bTakeTurn && !IsPlacable_CoolDown())
		{
			UE_LOG(LogTemp, Log, TEXT("X - Checked TakeTurn_PlaceCoolDown : %f"), TakeTurn_PlaceCoolDown);
			return;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("V - Checked TakeTurn_PlaceCoolDown : %f"), TakeTurn_PlaceCoolDown);
		}
		if (MouseOverObject != nullptr)
		{
			UGoSoccerPlayManager::MouseOverEnd(MouseOverObject.Get());
		}
		if (!UGoSoccerPlayManager::IsGoBoard(HitResult.GetActor())) return;
		MouseOverObject = HitResult.GetActor();
	}
	else
	{
		if (Widget_FlickWidget == nullptr) return;
		bool bIsFlickable = false;
		EPlayGameMode CurrentPlayGameMode;
		UGoSoccerPlayManager::GetPlayGameMode(GetGameInstance(), CurrentPlayGameMode);
		if (bHit)
		{
			if (CurrentPlayGameMode != EPlayGameMode::EPGM_HotSeatGame && CurrentPlayGameMode != EPlayGameMode::EPGM_SingleAIPlay)
			{
				bIsFlickable = UGoSoccerPlayManager::IsGoDoll(HitResult.GetActor());
			}
			else
			{
				uint8 HitDollColor;
				UGoSoccerPlayManager::GetDollColor(HitResult.GetActor(), HitDollColor);
				bIsFlickable = (CurrentPlayGameMode == EPlayGameMode::EPGM_SingleAIPlay ? 0 : HotSeatDollColor) == HitDollColor;
			}
			if (bIsFlickable)
			{
				bool IsFallen = true;
				UGoSoccerPlayManager::GetDollFallFromBoard(HitResult.GetActor(), IsFallen);
				bIsFlickable = !IsFallen;
			}
		}
		if (bHit && bIsFlickable)
		{
			if (MouseOverObject != nullptr)
			{
				UGoSoccerPlayManager::MouseOverEnd(MouseOverObject.Get());
			}
			MouseOverObject = HitResult.GetActor();
			if (HitResult.GetActor()->Owner != this) return;
			FVector2D ScreenPos;
			UGameplayStatics::ProjectWorldToScreen(this, HitResult.GetActor()->GetActorLocation(), ScreenPos);
			Widget_FlickWidget->SetPositionInViewport(ScreenPos);
			Widget_FlickWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
			if (AudioComponent_SFX_Flicker) AudioComponent_SFX_Flicker->Play();
		}
	}
}

void AGoPlayerController::LMBTriggered(const FInputActionValue& Value)
{
	if (!bItsMyTurn) return;

	if (FingerMode < 1)
	{
		FHitResult HitResult;
		if (GetHitResultUnderCursor(ECollisionChannel::ECC_EngineTraceChannel1, true, HitResult))
		{
			if (UGoSoccerPlayManager::IsGoBoard(HitResult.GetActor()))
			{
				TracingLocation = UGoSoccerPlayManager::GetPutLocation(HitResult.GetActor(), HitResult.Location);
			}
		}
		else
		{
			TracingLocation = FVector(0.f, 0.f, -999.f);
		}
	}
	else
	{
		AActor* FlickingDoll = Cast<AActor>(MouseOverObject.Get());
		if (FlickingDoll == nullptr)
		{
#if !UE_BUILD_SHIPPING
			if (GEngine)
			{
				FString LogString = FString::Printf(TEXT("Doll Not Found"));
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, LogString);
			}
#endif
			return;
		}
		if (FlickingDoll->Owner != this) return;
		FVector2D MousePos;
		FVector2D DollPos;
		if (!GetMousePosition(MousePos.X, MousePos.Y) || !ProjectWorldLocationToScreen(FlickingDoll->GetActorLocation(), DollPos)) return;
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("MoustPos : %f, %f\n DollPos : %f, %f"), MousePos.X, MousePos.Y, DollPos.X, DollPos.Y);
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, LogString);
		}
#endif
		FVector2D AimDirection2D = (DollPos - MousePos);
		AimDirection2D  = AimDirection2D.GetRotated(90.f);
		FVector AimDirection{ AimDirection2D.X, AimDirection2D.Y, 0.f };
		Delegate_OnAimFlicker.Broadcast(AimDirection);
	}
}

void AGoPlayerController::LMBCompleted(const FInputActionValue& Value)
{
	//float V = Value.Get<float>();

	if (Widget_IngameCursor) Widget_IngameCursor->LMBReleased();
	if (AudioComponent_SFX_Flicker) AudioComponent_SFX_Flicker->Stop();

	bClicking = false;

	if (!bEnablePlay)
	{
		if (FingerMode < 1)
		{
			if (MouseOverObject != nullptr)
			{
				if (UGoSoccerPlayManager::IsGoBoard(MouseOverObject.Get()))
				{
					UGoSoccerPlayManager::SetTranclucentDollLocation(MouseOverObject.Get(), FVector(0.f, 0.f, -999.f));
				}
				UGoSoccerPlayManager::MouseOverEnd(MouseOverObject.Get());
				TracingLocation = FVector(0.f, 0.f, -999.f);
			}
		}
		else
		{
			if (Widget_FlickWidget != nullptr)
			{
				Widget_FlickWidget->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		return;
	}
	bool bMouseOverObjectNullCheckFailed = false;
	bool bIsGoBoardCheckFailed = false;
	bool bOverrlapingCheckFailed = false;

	//bool bActionSucceeded = false;
	if (FingerMode < 1)
	{
		if (MouseOverObject != nullptr)
		{
			// Overlapping Test
			if (UGoSoccerPlayManager::IsGoBoard(MouseOverObject.Get()))
			{
				UObject* TranclucentDollObject = nullptr;
				bool bOverrlapingDoll = false;
				if (UGoSoccerPlayManager::GetTranclucentDoll(MouseOverObject.Get(), TranclucentDollObject))
				{
					bOverrlapingDoll = UGoSoccerPlayManager::GetOverlapingDoll_TrancluentDoll(TranclucentDollObject);
				}
				UGoSoccerPlayManager::SetTranclucentDollLocation(MouseOverObject.Get(), FVector(0.f, 0.f, -999.f));
				if (bOverrlapingDoll)
				{
#if !UE_BUILD_SHIPPING
					if (GEngine)
					{
						FString LogString = FString::Printf(TEXT("InValid Space : OverlapingDoll"));
						GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, LogString);
					}
#endif
					bOverrlapingCheckFailed = true;
				}
			}
			else
			{
#if !UE_BUILD_SHIPPING
				if (GEngine)
				{
					FString LogString = FString::Printf(TEXT("InValid Space : No GoBoard Found"));
					GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, LogString);
				}
#endif
				bIsGoBoardCheckFailed = true;
			}
			UGoSoccerPlayManager::MouseOverEnd(MouseOverObject.Get());
		}
		else
		{
#if !UE_BUILD_SHIPPING
			if (GEngine)
			{
				FString LogString = FString::Printf(TEXT("InValid Space : No Object Should've been GoBoard Found"));
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, LogString);
			}
#endif
			bMouseOverObjectNullCheckFailed = true;
		}

		if (bMouseOverObjectNullCheckFailed || bIsGoBoardCheckFailed || bOverrlapingCheckFailed)
		{
			TracingLocation = FVector(0.f, 0.f, -999.f);
			return;
		}
		FHitResult HitResult;
		if (GetHitResultUnderCursor(ECollisionChannel::ECC_EngineTraceChannel1, true, HitResult))
		{
			if (UGoSoccerPlayManager::IsGoBoard(HitResult.GetActor()))
			{
				if (IsLocalController())
				{
					if (bTakeTurn || (!bTakeTurn && TakeTurn_PlaceCoolDown > TakeTurn_PlaceCoolDownThreshold))
					{
						EDollType tempdt = UGoSoccerPlayManager::ConvertIntToDollType(FMath::Floor(CylinderConfig.ShadowTextureIndex.X));

						Server_SpawnDoll(TracingLocation, tempdt);
						FlickCylinderSpinWeight = 0.f;
						if (!bTakeTurn)
						{
							Cylinder_LastPlaceedTime = GetWorld()->GetTimeSeconds();
							Server_AddRealtimeCooldownBonus(-1.f);
							//if (TakeTurn_PlaceCoolDown > TakeTurn_PlaceCoolDownThreshold * 1.25f) TakeTurn_PlaceCoolDown = 0.f;
							//else TakeTurn_PlaceCoolDown -= TakeTurn_PlaceCoolDownThreshold;
						}
						UE_LOG(LogTemp, Log, TEXT("TakeTurn_PlaceCoolDown : %f"), TakeTurn_PlaceCoolDown);
					}
					//bActionSucceeded = true;
				}
			}
		}
		TracingLocation = FVector(0.f, 0.f, -999.f);
	}
	else
	{
		AActor* FlickingDoll = Cast<AActor>(MouseOverObject.Get());
		int32 DollPlacedIndex = -1;
		if (!UGoSoccerPlayManager::GetDollPlacedIndex(FlickingDoll, DollPlacedIndex))
		{
#if !UE_BUILD_SHIPPING
			if (GEngine)
			{
				FString LogString = FString::Printf(TEXT("Doll Not Found or Doll Counter Failed"));
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, LogString);
			}
#endif
			return;
		}
		bool bFlickable = false;
		EPlayGameMode CurrentPlayGameMode;
		UGoSoccerPlayManager::GetPlayGameMode(GetGameInstance(), CurrentPlayGameMode);
		if (CurrentPlayGameMode != EPlayGameMode::EPGM_HotSeatGame
			&& CurrentPlayGameMode != EPlayGameMode::EPGM_SingleAIPlay)
		{
			bFlickable = FlickingDoll->Owner == this;
		}
		else
		{
			uint8 OverDollColor;
			UGoSoccerPlayManager::GetDollColor(FlickingDoll, OverDollColor);
			bFlickable = OverDollColor == (CurrentPlayGameMode == EPlayGameMode::EPGM_SingleAIPlay ? 0 : HotSeatDollColor);
		}
		if (bFlickable)
		{
			bool IsFallen = true;
			UGoSoccerPlayManager::GetDollFallFromBoard(FlickingDoll, IsFallen);
			bFlickable = !IsFallen;
		}
		if (!bFlickable) return;

		FVector2D MousePos;
		FVector2D DollPos;
		if (!GetMousePosition(MousePos.X, MousePos.Y) || !ProjectWorldLocationToScreen(FlickingDoll->GetActorLocation(), DollPos)) return;
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("MoustPos : %f, %f\n DollPos : %f, %f"), MousePos.X, MousePos.Y, DollPos.X, DollPos.Y);
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, LogString);
		}
#endif
		FVector2D AimDirection2D = (DollPos - MousePos);
		AimDirection2D = AimDirection2D.GetRotated(90.f);
		FVector AimDirection{ AimDirection2D.X, AimDirection2D.Y, 0.f };
		UGoSoccerPlayManager::FlickDoll(FlickingDoll, AimDirection);
		if (Widget_FlickWidget != nullptr)
		{
#if UE_EDITOR
			UE_LOG(LogTemp, Log, TEXT("Widget_FlickWidget->SetVisibility(ESlateVisibility::HitTestInvisible)"));
#endif
			Widget_FlickWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		//bActionSucceeded = true;
	}
	TracingLocation = FVector(0.f, 0.f, -999.f);

	//if (bActionSucceeded) Execute_TurnEnd(this); // UI, Controller boolean Set
}

void AGoPlayerController::RMBStarted()
{
	auto ResetCameraTimer = [&]()->void 
		{
			GetMousePosition(CameraMoveCriteriaLocation.X, CameraMoveCriteriaLocation.Y);
			GetWorldTimerManager().ClearTimer(CameraXYTimerHandle);
			GetWorldTimerManager().ClearTimer(CameraZTimerHandle);
			CameraToMoveDirection = FVector::ZeroVector;
		};

	if (Widget_IngameCursor) Widget_IngameCursor->RMBPressed();

	//if (bTakeTurn)
	//{
	//	if (bClicking) return;
	//	ResetCameraTimer();
	//	//GetMousePosition(CameraMoveCriteriaLocation.X, CameraMoveCriteriaLocation.Y);
	//	//GetWorldTimerManager().ClearTimer(CameraXYTimerHandle);
	//	//GetWorldTimerManager().ClearTimer(CameraZTimerHandle);
	//	//CameraToMoveDirection = FVector::ZeroVector;
	//}
	//else
	//{
	//SetbFlickLocking(false);
	bFlickLocking = false;
	CylinderConfig.bUseClockCurve = true;

	bClicking = true;
	FHitResult HitResult;
	bool bHit = GetHitResultUnderCursor(
		ECollisionChannel::ECC_Visibility,
		true, HitResult
	); 
// Swipe Screen In Realtime Mode
	bool bIsGoDoll = UGoSoccerPlayManager::IsGoDoll(HitResult.GetActor());
	if (!bHit || !bIsGoDoll)
	{
		ResetCameraTimer();
		bTakeTurn_SwipingScreen = true;
	}

	if (CurrentPlayingSessionData.PlayTurnRule == EPlayTurnRule::EPTR_RealTime
		&& TakeTurn_FlickCoolDown < TakeTurn_FlickCoolDownThreshold) return;

	if (!bEnablePlay) return;
// Flick In Realtime Mode
	if (Widget_FlickWidget == nullptr) return;
	bool bIsFlickable = false;
	EPlayGameMode CurrentPlayGameMode;
	UGoSoccerPlayManager::GetPlayGameMode(GetGameInstance(), CurrentPlayGameMode);
	if (bHit)
	{
		if (CurrentPlayGameMode != EPlayGameMode::EPGM_HotSeatGame && CurrentPlayGameMode != EPlayGameMode::EPGM_SingleAIPlay)
		{
			bIsFlickable = bIsGoDoll;
		}
		else
		{
			uint8 HitDollColor;
			UGoSoccerPlayManager::GetDollColor(HitResult.GetActor(), HitDollColor);
			bIsFlickable = (CurrentPlayGameMode == EPlayGameMode::EPGM_SingleAIPlay ? 0 : HotSeatDollColor) == HitDollColor;
		}
		if (bIsFlickable)
		{
			bool IsFallen = true;
			UGoSoccerPlayManager::GetDollFallFromBoard(HitResult.GetActor(), IsFallen);
			bIsFlickable = !IsFallen;
		}
	}
	if (bHit && bIsFlickable)
	{
		if (MouseOverObject != nullptr)
		{
			UGoSoccerPlayManager::MouseOverEnd(MouseOverObject.Get());
		}
		MouseOverObject = HitResult.GetActor();
		if (HitResult.GetActor()->Owner != this) return;
		//SetbFlickLocking(true);
		bFlickLocking = true;
		CylinderConfig.bUseClockCurve = false;

		FVector2D ScreenPos;
		UGameplayStatics::ProjectWorldToScreen(this, HitResult.GetActor()->GetActorLocation(), ScreenPos);
		Widget_FlickWidget->SetPositionInViewport(ScreenPos);
#if UE_EDITOR
		UE_LOG(LogTemp, Log, TEXT("Widget_FlickWidget->SetVisibility(ESlateVisibility::HitTestInvisible)"));
#endif
		Widget_FlickWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (AudioComponent_SFX_Flicker) AudioComponent_SFX_Flicker->Play();
	}
	//}
}

void AGoPlayerController::RMBTriggered()
{
	auto UpdateMousePosition = [&]()->void
		{
			FVector2D DraggingLocation;
			if (GetMousePosition(DraggingLocation.X, DraggingLocation.Y))
			{
				FVector2D Dir = DraggingLocation - CameraMoveCriteriaLocation;
				CameraToMoveDirection.Y = FMath::Clamp(CameraToMoveDirection.Y - Dir.X * 0.0125f, -50.f, 50.f);
				CameraToMoveDirection.X = FMath::Clamp(CameraToMoveDirection.X + Dir.Y * 0.0125f, -50.f, 50.f);
			}
		};

	//if (bTakeTurn)
	//{
	//	if (bClicking) return;
	//	UpdateMousePosition();
	//}
	//else
	if (bTakeTurn_SwipingScreen) UpdateMousePosition();

	AActor* FlickingDoll = Cast<AActor>(MouseOverObject.Get());
	if (FlickingDoll == nullptr)
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("Doll Not Found"));
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, LogString);
		}
#endif
		return;
	}
	if (FlickingDoll->Owner != this) return;
	FVector2D MousePos;
	FVector2D DollPos;
	if (!GetMousePosition(MousePos.X, MousePos.Y) || !ProjectWorldLocationToScreen(FlickingDoll->GetActorLocation(), DollPos)) return;
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("MoustPos : %f, %f\n DollPos : %f, %f"), MousePos.X, MousePos.Y, DollPos.X, DollPos.Y);
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, LogString);
	}
#endif
	FVector2D AimDirection2D = (DollPos - MousePos);
	AimDirection2D = AimDirection2D.GetRotated(90.f);
	FVector AimDirection{ AimDirection2D.X, AimDirection2D.Y, 0.f };
	Delegate_OnAimFlicker.Broadcast(AimDirection);
}

void AGoPlayerController::RMBCompleted()
{
	auto WindupMousePosition = [&]()->void 
		{
			GetWorldTimerManager().ClearTimer(CameraXYTimerHandle);
			GetWorldTimerManager().SetTimer(
				CameraXYTimerHandle,
				[&]()
				{
					CameraToMoveDirection.X *= 0.65f;
					CameraToMoveDirection.Y *= 0.65f;
					if (FMath::IsNearlyZero(CameraToMoveDirection.X, 0.1f) &&
						FMath::IsNearlyZero(CameraToMoveDirection.Y, 0.1f))
					{
						CameraToMoveDirection.X = 0.f;
						CameraToMoveDirection.Y = 0.f;
						if (GetWorld())
						{
							GetWorldTimerManager().ClearTimer(CameraXYTimerHandle);
						}
					}
				},
				0.1f,
				true
			);
		};

	if (Widget_IngameCursor) Widget_IngameCursor->RMBReleased();
	if (AudioComponent_SFX_Flicker) AudioComponent_SFX_Flicker->Stop();

	//if (bTakeTurn)
	//{
	//	if (bClicking) return;
	//	WindupMousePosition();
	//}
	//else


	//SetbFlickLocking(false);
	bFlickLocking = false;
	CylinderConfig.bUseClockCurve = true;

	bClicking = false;
	if (bTakeTurn_SwipingScreen)
	{
		bTakeTurn_SwipingScreen = false;
		WindupMousePosition();
	}

	if (!bEnablePlay) return;
	if (CurrentPlayingSessionData.PlayTurnRule == EPlayTurnRule::EPTR_RealTime
		&& TakeTurn_FlickCoolDown < TakeTurn_FlickCoolDownThreshold) return;

	AActor* FlickingDoll = Cast<AActor>(MouseOverObject.Get());
	int32 DollPlacedIndex = -1;
	if (!UGoSoccerPlayManager::GetDollPlacedIndex(FlickingDoll, DollPlacedIndex))
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("Doll Not Found or Doll Counter Failed"));
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, LogString);
		}
#endif
		return;
	}
	bool bFlickable = false;
	EPlayGameMode CurrentPlayGameMode;
	UGoSoccerPlayManager::GetPlayGameMode(GetGameInstance(), CurrentPlayGameMode);
	if (CurrentPlayGameMode != EPlayGameMode::EPGM_HotSeatGame
		&& CurrentPlayGameMode != EPlayGameMode::EPGM_SingleAIPlay)
	{
		bFlickable = FlickingDoll->Owner == this;
	}
	else
	{
		uint8 OverDollColor;
		UGoSoccerPlayManager::GetDollColor(FlickingDoll, OverDollColor);
		bFlickable = OverDollColor == (CurrentPlayGameMode == EPlayGameMode::EPGM_SingleAIPlay ? 0 : HotSeatDollColor);
	}
	if (bFlickable)
	{
		bool IsFallen = true;
		UGoSoccerPlayManager::GetDollFallFromBoard(FlickingDoll, IsFallen);
		bFlickable = !IsFallen;
	}
	if (!bFlickable) return;

	FVector2D MousePos;
	FVector2D DollPos;
	if (!GetMousePosition(MousePos.X, MousePos.Y) || !ProjectWorldLocationToScreen(FlickingDoll->GetActorLocation(), DollPos)) return;
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("MoustPos : %f, %f\n DollPos : %f, %f"), MousePos.X, MousePos.Y, DollPos.X, DollPos.Y);
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, LogString);
	}
#endif
	FVector2D AimDirection2D = (DollPos - MousePos);
	AimDirection2D = AimDirection2D.GetRotated(90.f);
	FVector AimDirection{ AimDirection2D.X, AimDirection2D.Y, 0.f };
	if (UGoSoccerPlayManager::FlickDoll(FlickingDoll, AimDirection))
	{
		if (CurrentPlayingSessionData.PlayTurnRule == EPlayTurnRule::EPTR_RealTime)
		{
			TakeTurn_FlickCoolDown -= TakeTurn_FlickCoolDownThreshold;
		}
		else if (CurrentPlayingSessionData.PlayTurnRule == EPlayTurnRule::EPTR_TakeTurn)
		{
			//Server_Flicked_InvertCylinderCharge(CylinderConfig.ChargePercent.X > 0 ? 1 : 0);
		}
	}
	if (Widget_FlickWidget != nullptr)
	{
		Widget_FlickWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	TracingLocation = FVector(0.f, 0.f, -999.f);
}

void AGoPlayerController::ScrollStarted()
{
	GetWorldTimerManager().ClearTimer(CameraZTimerHandle);
	CameraToMoveDirection.Z = 0.f;
}

void AGoPlayerController::ScrollTriggered(const FInputActionValue& Value)
{
	if (Widget_Main == nullptr || !Widget_Main->IsInPlay()) return;

	float V = Value.Get<float>();
	CameraToMoveDirection.Z = FMath::Clamp(CameraToMoveDirection.Z - V * 10.f, -50.f, 50.f);
}

void AGoPlayerController::ScrollCompleted()
{
	if (Widget_Main == nullptr || !Widget_Main->IsInPlay()) return;

	GetWorldTimerManager().ClearTimer(CameraZTimerHandle);
	GetWorldTimerManager().SetTimer(
		CameraZTimerHandle,
		[this]()
		{
			CameraToMoveDirection.Z *= 0.35f;
			if (FMath::IsNearlyZero(CameraToMoveDirection.Z, 0.1f))
			{
				CameraToMoveDirection.Z = 0.f;
				GetWorldTimerManager().ClearTimer(CameraZTimerHandle);
			}
		},
		0.1f,
		true
	);
}

void AGoPlayerController::TabTriggered()
{
}

void AGoPlayerController::TabCompleted()
{
	if (!bTakeTurn) return;
	if (bClicking)
	{
		OnClickCanceled(FingerMode);
	}
	FingerMode += 1;
	FingerMode %= 2;

	Delegate_OnSwitchFingerMode.Broadcast(FingerMode);
}

void AGoPlayerController::ESCCompleted()
{
	EMainUIState CurrMainUIState;
	UGoSoccerPlayManager::GetMainUIState(GetGameInstance(), CurrMainUIState);
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("ESCCompleted : %s, %d"),
			CurrMainUIState == EMainUIState::EMUIS_GameStarted ? TEXT("GameStarted") : TEXT("Not GameStarted"),
			CurrMainUIState
			);
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, LogString);
	}
#endif
	if (CurrMainUIState == EMainUIState::EMUIS_GameStarted)
	{
		Client_SwitchInGameSettingsWidget(true);
		//Widget_Main->SetSettingWidget_InGameManuPanelVisibility(true); // Enable InGame Settings Widget Mode(Add Give Up Button)
		//Execute_SetMainUIState(this, bESCSettingOn ? CurrMainUIState : EMainUIState::EMUIS_Settings);
		//bESCSettingOn = !bESCSettingOn;
		//Client_SetbEnablePlay(!bESCSettingOn);
	}
	//Widget_Main->SetSettingWidget_InGameManuPanelVisibility(false);
}

void AGoPlayerController::EnterCompleted()
{
	if (Widget_HUD) Widget_HUD->OnEnterCompleted();
}

void AGoPlayerController::MoveTriggered(const FInputActionValue& Value)
{
}

void AGoPlayerController::OnClickCanceled(uint8 InFingerMode)
{
	if (InFingerMode < 1)
	{
		if (MouseOverObject != nullptr)
		{
			if (UGoSoccerPlayManager::IsGoBoard(MouseOverObject.Get()))
			{
				UGoSoccerPlayManager::SetTranclucentDollLocation(MouseOverObject.Get(), FVector(0.f, 0.f, -999.f));
			}
			UGoSoccerPlayManager::MouseOverEnd(MouseOverObject.Get());
		}
	}
	else
	{
		if (Widget_FlickWidget != nullptr)
		{
			Widget_FlickWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	TracingLocation = FVector(0.f, 0.f, -999.f);
}

TWeakObjectPtr<UObject> AGoPlayerController::GetGoBoardObjectPtr()
{
	if (!GoBoardObjectPtr.IsValid())
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("GoBoard"), FoundActors);
		for (AActor* FoundActor : FoundActors)
		{
			if (UGoSoccerPlayManager::IsGoBoard(FoundActor))
			{
				GoBoardObjectPtr = FoundActor;
				GoBoardLocation = FoundActor->GetActorLocation();
				break;
			}
		}
	}
	return GoBoardObjectPtr;
}

void AGoPlayerController::Client_SetBGMVolume_Implementation(float InVolume)
{
	if (AudioComponent_BGM == nullptr) return;
	AudioComponent_BGM->SetVolumeMultiplier(InVolume);
}

void AGoPlayerController::PlayLobbyBGM()
{
	Client_SwitchBGM(SB_BGM);
}

AGameStateBase* AGoPlayerController::GetCachedGameState()
{
	if (!CachedGameState.IsValid()) CachedGameState = GetWorld()->GetGameState();
	return CachedGameState.IsValid() ? CachedGameState.Get() : nullptr;
}

bool AGoPlayerController::IsPlacable_CoolDown()
{
	if (TakeTurn_PlaceCoolDown < TakeTurn_PlaceCoolDownThreshold ||
		Cylinder_LastPlaceedTime + TakeTurn_PlaceInsideCoolDownThreshold > GetWorld()->GetTimeSeconds())
	{
		return false;
	}
	return true;
}

void AGoPlayerController::SetbFlickLocking(bool e)
{
	bFlickLocking = e;
	if (HasAuthority()) OnRep_FlickLocking();
}

void AGoPlayerController::Client_SwitchBGM_Implementation(USoundBase* ToPlay, float fFadeOut, float fFadeIn, float fFadeOutLevel, float fVolume = 1.f)
{
	if (AudioComponent_BGM == nullptr) return;
	AudioComponent_BGM->SetVolumeMultiplier(fVolume);
	AudioComponent_BGM->FadeOut(fFadeOut, fFadeOutLevel);
	AudioComponent_BGM->SetSound(ToPlay);
	if (ToPlay != nullptr)
	{
		AudioComponent_BGM->FadeIn(fFadeIn, 1.f, 0.f, EAudioFaderCurve::Logarithmic);
	}
	//if (AudioComponent_BGM->IsPlaying() && AudioComponent_BGM->GetSound() == ToPlay)
	//{
	//	AudioComponent_BGM->FadeIn(fFadeIn, 1.f, 0.f, EAudioFaderCurve::Logarithmic);
	//}
}

void AGoPlayerController::Server_PlayerJoinedSessionComplete_Implementation()
{
	if (GetWorld())
	{
		UGoSoccerPlayManager::Notify_PlayerJoinedSessionComplete(GetWorld()->GetAuthGameMode(), this);
	}
}

void AGoPlayerController::Server_DebugPlayerNickname_Implementation()
{
	FString PlayerNickName;
	UGoSoccerPlayManager::GetOnlineIdentityPlayerNickName(GetGameInstance(), PlayerNickName);
	FName tempName{ PlayerNickName };
	//Execute_ChangePlayerName(this, tempName);

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("Server PlayerNickName : %s"), *PlayerNickName);
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::White, LogString);
	}
#endif
}

void AGoPlayerController::Client_DebugPlayerNickname_Implementation ()
{
	FString PlayerNickName;
	// 닉네임을 못 가져온 상태에서 그대로 넘기면 PlayerCard 이름을 빈 값으로 덮어쓴다.
	if (!UGoSoccerPlayManager::GetOnlineIdentityPlayerNickName(GetGameInstance(), PlayerNickName) ||
		PlayerNickName.IsEmpty())
	{
		return;
	}
	bSent_PlayerNickname = true;
	FName tempName{ PlayerNickName };
	Execute_ChangePlayerName(this, tempName);

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("Client PlayerNickName : %s"), *PlayerNickName);
		//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::White, LogString);
	}
#endif
}

void AGoPlayerController::Server_ChangePlayerName_Implementation(const FName& NewPlayerName)
{
	APlayerState* tempPlayerState = GetPlayerState<APlayerState>();
	if (tempPlayerState == nullptr) return;
	UGoSoccerPlayManager::ChangePlayerName(tempPlayerState, NewPlayerName);
}

void AGoPlayerController::Client_PlayerJoinedSessionComplete_Implementation()
{
	if (Widget_Main)
	{
		Widget_Main->OnJoinSelectedSessionCompleted();
	}
	UGoSoccerPlayManager::SetPlayerSessionHost(PlayerState, false);
	UGoSoccerPlayManager::SetPlayerSessionHost(GetGameInstance(), false);
}

void AGoPlayerController::Server_PlayerJoinedOrCreatedSessionComplete_Implementation(int64 RawSteamID)
{
	UGoSoccerPlayManager::PostSteamID(GetWorld()->GetGameState(), this, RawSteamID);
}

void AGoPlayerController::Client_PlayerCreateSessionComplete_Implementation()
{
	Widget_Main->OnHostGameCompleted();
	UGoSoccerPlayManager::SetPlayerSessionHost(PlayerState, true);
	UGoSoccerPlayManager::SetPlayerSessionHost(GetGameInstance(), true);
}

void AGoPlayerController::Server_GiveUpGame_Implementation()
{
	if (!GetWorld()) return;
	UGoSoccerPlayManager::PlayerWalkOver(GetWorld()->GetAuthGameMode(), this);
}

void AGoPlayerController::Client_SetCameraPosition_Implementation(ECameraPosition CameraPosition)
{
	if (CurrentCameraPosition == CameraPosition) return;
	FName Tag;
	if (CameraPosition == ECameraPosition::ECP_InPlay)
	{
		Tag = TEXT("InPlayCamera");
	}
	else if (CameraPosition == ECameraPosition::ECP_VeryFirst)
	{
		Tag = TEXT("DefaultCamera");
	}
	else if (CameraPosition == ECameraPosition::ECP_Session)
	{
		Tag = TEXT("SessionCamera");
	}
	else if (CameraPosition == ECameraPosition::ECP_Lobby)
	{
		Tag = TEXT("InLobbyCamera");
	}
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ACameraActor::StaticClass(), Tag, FoundActors);
	if (FoundActors.IsValidIndex(0) && FoundActors[0] != nullptr)
	{
		FViewTargetTransitionParams ViewTargetTransitionParams;
		ViewTargetTransitionParams.BlendTime = 1.5f;
		ViewTargetTransitionParams.BlendFunction = EViewTargetBlendFunction::VTBlend_EaseIn;
		SetViewTarget(FoundActors[0], ViewTargetTransitionParams);
		CurrentPossessingCamera = FoundActors[0];
		CurrentCameraPosition = CameraPosition;
	}
}

void AGoPlayerController::Server_PlayerCreateSessionComplete_Implementation()
{
	UGoSoccerPlayManager::SetReadyState(GetPlayerState<APlayerState>(), true);
}

void AGoPlayerController::Server_Request_UpdateTooltip_SessionConfig_Implementation()
{
	UGoSoccerPlayManager::Request_UpdateTooltip_SessionConfig(GetWorld()->GetAuthGameMode(), this);
}

void AGoPlayerController::Server_Notify_UpdateTooltip_SessionConfig_Implementation(const FSessionCreateData& InHostSessionCreateData)
{
	UGoSoccerPlayManager::Notify_UpdateTooltip_SessionConfig(GetWorld()->GetAuthGameMode(), InHostSessionCreateData);
}

void AGoPlayerController::Server_SendChattingMessage_Implementation(const FText& TextMessage)
{
	FName tempPlayerName;
	UGoSoccerPlayManager::GetPlayerName(PlayerState, tempPlayerName);
	UGoSoccerPlayManager::AddChattingMessage(GetWorld()->GetAuthGameMode(), TextMessage, tempPlayerName);
}

void AGoPlayerController::Client_AddRealtimeCooldownBonus_Flick_Implementation(float Portion_MaxCooldown)
{
	float tempAdded = TakeTurn_FlickCoolDown + Portion_MaxCooldown * TakeTurn_FlickCoolDownThreshold; // *1.1f;
	TakeTurn_FlickCoolDown = FMath::Clamp(tempAdded, 0.f, TakeTurn_FlickCoolDownThreshold * 1.1f);
	if (Widget_IngameCursor)
	{
		Widget_IngameCursor->SetCoolDown_R(TakeTurn_FlickCoolDown / TakeTurn_FlickCoolDownThreshold);
		//UE_LOG(LogTemp, Log, TEXT("%s - Client_AddRealtimeCooldownBonus_Flick : %f"), *GetFName().ToString(), TakeTurn_FlickCoolDown);
	}
}

void AGoPlayerController::Server_AddRealtimeCooldownBonus_Implementation(float Portion_MaxCooldown)
{
	float tempAdded = TakeTurn_PlaceCoolDown + Portion_MaxCooldown * TakeTurn_PlaceCoolDownThreshold;
	TakeTurn_PlaceCoolDown = FMath::Clamp(tempAdded, -TakeTurn_PlaceCoolDownThreshold, TakeTurn_PlaceCoolDownThreshold * EXTRA_COOLDOWN_SAVE_PERCENT);
	if (IsLocalController()) OnRep_PlaceCoolDown();
}

void AGoPlayerController::Client_SetCurrentPlayingSessionData_Implementation(const FSessionCreateData& InSessionData)
{
	CurrentPlayingSessionData = InSessionData;
}

void AGoPlayerController::Server_SetCurrentPlayingSessionData_Implementation(const FSessionCreateData& InSessionData)
{
	CurrentPlayingSessionData = InSessionData;
}

void AGoPlayerController::Client_SetTurnMode_CylinderConfig_Implementation(const TArray<EDollType>& ItemDollArr)
{
	CylinderConfig.ShadowTextureIndex.X = ItemDollArr.IsValidIndex(0) ? StaticCast<uint8>(ItemDollArr[0]) : 0;
	CylinderConfig.ShadowTextureIndex.Y = ItemDollArr.IsValidIndex(1) ? StaticCast<uint8>(ItemDollArr[1]) : 0;
	CylinderConfig.ShadowTextureIndex.Z = ItemDollArr.IsValidIndex(2) ? StaticCast<uint8>(ItemDollArr[2]) : 0;
	CylinderConfig.ShadowTextureIndex.W = ItemDollArr.IsValidIndex(3) ? StaticCast<uint8>(ItemDollArr[3]) : 0;
}

//void AGoPlayerController::NetMulticast_NextDollType_Received_Implementation(const EDollType InDollType)
//{
//	Client_NextDollType_Received(InDollType);
//	//Client_SetTurnMode_CylinderConfig(InDollType);
//}

void AGoPlayerController::Client_Flicked_InvertCylinderCharge_Implementation(int32 InFlickCylinderInvert)
{
	FlickCylinderInvert = InFlickCylinderInvert;
}

void AGoPlayerController::Client_Flicked_bFlickLock_Implementation(bool bLocked)
{
	bFlickLocking = bLocked;
}

void AGoPlayerController::Client_WindupMatchWidget_Implementation(const FMatchScoreLampStateConfig& InMatchScoreLampStateConfig)
{
	if (Widget_HUD)
	{
		Widget_HUD->WindupMatchWidget(InMatchScoreLampStateConfig);
	}
}

void AGoPlayerController::Client_SetMainUIState_Implementation(const EMainUIState& NewMainUIState)
{
	if (Widget_Main == nullptr || Widget_HUD == nullptr) return;
	Widget_Main->OnUIStateChanged(NewMainUIState);
	Widget_HUD->OnUIStateChanged(NewMainUIState);
	if (NewMainUIState == EMainUIState::EMUIS_InLobby)
	{
		Client_SwitchBGM(SB_BGM_InGame);
		Client_SetCameraPosition(ECameraPosition::ECP_Lobby);
	}
	else if (NewMainUIState == EMainUIState::EMUIS_GameConfigSetting)
	{
		Client_SetCameraPosition(ECameraPosition::ECP_Session);
	}
}
