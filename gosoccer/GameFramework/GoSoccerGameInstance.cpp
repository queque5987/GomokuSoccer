#include "GameFramework/GoSoccerGameInstance.h"
#include "Materials/MaterialParameterCollection.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "HAL/IConsoleManager.h"
#include "OnlineSessionSettings.h"
#include "Widget/Data/GameSessionData.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "GoSoccerPlayManager.h"
//#include "Steamworks/Steamv153/sdk/public/steam/steam_api.h"
#include "ThirdParty/Steamworks/Steamv153/sdk/public/steam/steam_api.h"
#include "OnlineSubsystemSteam.h"
#include "OnlineSubsystemUtils.h"
#include "Components/AudioComponent.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/Base64.h"
#include "Interface/IGoSoccerGameUserSettings.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"

static const FName Key_SessionName			= FName("SessionName");
static const FName Key_HostName				= FName("HostName");
static const FName Key_PlayGameRule			= FName("PlayGameRule");
static const FName Key_DollType				= FName("DollType");
static const FName Key_BoardPreset			= FName("BoardPreset");
static const FName Key_PlayTurnRule			= FName("PlayTurnRule");
static const FName Key_TimeLimit			= FName("TimeLimit");
static const FName Key_AngularThreshold		= FName("AngularThreshold");
static const FName Key_DistanceThreshold	= FName("DistanceThreshold");
static const FName Key_EnableItem			= FName("EnableItem");
static const FName Key_MatchRule			= FName("MatchRule");
static const FName Key_IsSecret				= FName("IsSecret");
static const TCHAR* Key_TextBase64Prefix = TEXT("b64|");

/** 세션 설정에 넣기 전 인코딩. ASCII 만 있으면 그대로 둔다. */
static FString EncodeSessionText(const FString& InText)
{
	bool bAsciiOnly = true;
	for (const TCHAR Iter : InText)
	{
		if (Iter > 127)
		{
			bAsciiOnly = false;
			break;
		}
	}
	// 우연히 접두사로 시작하는 ASCII 값도 감싸 둬야 디코딩 쪽 판정이 어긋나지 않는다.
	if (bAsciiOnly && !InText.StartsWith(Key_TextBase64Prefix, ESearchCase::CaseSensitive)) return InText;

	const FTCHARToUTF8 Utf8Text(*InText);
	return FString(Key_TextBase64Prefix) + FBase64::Encode(reinterpret_cast<const uint8*>(Utf8Text.Get()), Utf8Text.Length());
}

/** 세션 설정에서 읽은 값 디코딩. 접두사가 없으면 인코딩되지 않은 값이므로 그대로 돌려준다. */
static FString DecodeSessionText(const FString& InText)
{
	if (!InText.StartsWith(Key_TextBase64Prefix, ESearchCase::CaseSensitive)) return InText;

	TArray<uint8> Utf8Bytes;
	if (!FBase64::Decode(InText.RightChop(FCString::Strlen(Key_TextBase64Prefix)), Utf8Bytes)) return InText;

	Utf8Bytes.Add(0); // UTF8_TO_TCHAR 에 넘길 널 종단
	return FString(UTF8_TO_TCHAR(reinterpret_cast<const ANSICHAR*>(Utf8Bytes.GetData())));
}

UGoSoccerGameInstance::UGoSoccerGameInstance()
	//: Delegate_OnCreateSessionComplete(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete))
	//, Delegate_OnFindSessionsComplete(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionComplete))
	//, Delegate_OnJoinSessionComplete(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplate))
{
	//AudioComponent_BGM = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent_BGM"));
}

void UGoSoccerGameInstance::Client_SetCurrentMainUIState_Implementation(const EMainUIState& NewMainUIState)
{
	MainUIState = NewMainUIState;
}

void UGoSoccerGameInstance::NotifySessionLoadingScreen(bool bEnable, ESessionLoadingPhase InLoadingPhase)
{
	SessionLoadingPhase = bEnable ? InLoadingPhase : ESessionLoadingPhase::ESLP_None;

	UE_LOG(LogTemp, Log, TEXT("NotifySessionLoadingScreen : %s (%s)"),
		bEnable ? TEXT("Enable") : TEXT("Disable"),
		*UEnum::GetValueAsString(SessionLoadingPhase)
	);

	// PC 가 없어도(초대 부팅 등) SessionLoadingPhase 는 남으므로,
	// 이후 생성되는 PC 의 Load_Widget 에서 로딩 UI 가 복원된다.
	UGoSoccerPlayManager::SetSessionLoadingScreen(GetFirstLocalPlayerController(), bEnable, SessionLoadingPhase);
}

bool UGoSoccerGameInstance::Init_OSS()
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem == nullptr) return false;
	OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!OnlineSessionInterface.IsValid()) return false;

	if (DelegateHandle_OnCreateSessionComplete.IsValid())
	{
		//Delegate_OnCreateSessionComplete.Unbind();
		OnlineSessionInterface->OnCreateSessionCompleteDelegates.Remove(DelegateHandle_OnCreateSessionComplete);
		DelegateHandle_OnCreateSessionComplete.Reset();
	}
	//DelegateHandle_OnCreateSessionComplete = OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(Delegate_OnCreateSessionComplete);
	DelegateHandle_OnCreateSessionComplete = OnlineSessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UGoSoccerGameInstance::OnCreateSessionComplete);	
	if (DelegateHandle_OnFindSessionsCompleteDelegate.IsValid())
	{
		//Delegate_OnFindSessionsComplete.Unbind();
		OnlineSessionInterface->OnFindSessionsCompleteDelegates.Remove(DelegateHandle_OnFindSessionsCompleteDelegate);
		DelegateHandle_OnFindSessionsCompleteDelegate.Reset();
	}
	//DelegateHandle_OnFindSessionsCompleteDelegate = OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(Delegate_OnFindSessionsComplete);
	DelegateHandle_OnFindSessionsCompleteDelegate = OnlineSessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UGoSoccerGameInstance::OnFindSessionComplete);
	if (DelegateHandle_OnJoinSessionCompleteDelegate.IsValid())
	{
		//Delegate_OnJoinSessionComplete.Unbind();
		OnlineSessionInterface->OnJoinSessionCompleteDelegates.Remove(DelegateHandle_OnJoinSessionCompleteDelegate);
		DelegateHandle_OnJoinSessionCompleteDelegate.Reset();
	}
	//DelegateHandle_OnJoinSessionCompleteDelegate = OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(Delegate_OnJoinSessionComplete);
	DelegateHandle_OnJoinSessionCompleteDelegate = OnlineSessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UGoSoccerGameInstance::OnJoinSessionComplate);

	if (DelegateHandle_OnSessionFailure.IsValid())
	{
		OnlineSessionInterface->OnSessionFailureDelegates.Remove(DelegateHandle_OnSessionFailure);
		DelegateHandle_OnSessionFailure.Reset();
	}
	DelegateHandle_OnSessionFailure = OnlineSessionInterface->OnSessionFailureDelegates.AddUObject(this, &UGoSoccerGameInstance::OnSessionFailure);

	if (DelegateHandle_OnUpdateOnlineSessionComplete.IsValid())
	{
		OnlineSessionInterface->OnUpdateSessionCompleteDelegates.Remove(DelegateHandle_OnUpdateOnlineSessionComplete);
		DelegateHandle_OnUpdateOnlineSessionComplete.Reset();
	}
	DelegateHandle_OnUpdateOnlineSessionComplete = OnlineSessionInterface->OnUpdateSessionCompleteDelegates.AddUObject(this, &UGoSoccerGameInstance::OnUpdateSessionComplete);

	// PlayerCard 의 플레이어 이름 조회(GetIdentityPlayerNickName)에 쓰이므로
	// Shipping 빌드에서도 반드시 잡아둬야 한다.
	OnlineIdentity = OnlineSubsystem->GetIdentityInterface();
#if !UE_BUILD_SHIPPING
	if (OnlineIdentity.IsValid())
	{
		FString PlayerName = OnlineIdentity->GetPlayerNickname(0);
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("PlayerName : %s"), *PlayerName);
			GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Green, LogString);
		}
	}
	else
	{
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("Player Identity Load Failed"));
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
		}
	}
#endif
	return true;
}

void UGoSoccerGameInstance::Init()
{
	Super::Init();

	if (Init_OSS())
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("OnlineSubsystemLoaded"));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
		}
#endif
	}

	Init_SteamInvite();		// Steam Invite

	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UGoSoccerGameInstance::HandleNetworkFailure);
	}
}

void UGoSoccerGameInstance::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
	UGameplayStatics::PushSoundMixModifier(NewWorld, SM_DefaultSoundMix);

	//if (!AudioComponent_BGM || AudioComponent_BGM->GetWorld() != NewWorld)
	//{
	//	AudioComponent_BGM = NewObject<UAudioComponent>(NewWorld);
	//	AudioComponent_BGM->RegisterComponent();
	//}
	//if (AudioComponent_BGM)
	//{
	//	AudioComponent_BGM->SetSound(SB_BGM);
	//	AudioComponent_BGM->Play();
	//}
}

void UGoSoccerGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		// 생성 실패 시 NamedSession 이 없을 수 있으므로 역참조 전에 확인한다.
		FNamedOnlineSession* NamedSession = OnlineSessionInterface.IsValid() ? OnlineSessionInterface->GetNamedSession(SessionName) : nullptr;
		FString LogString = FString::Printf(TEXT("Session Create : %s - %s > %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"), *SessionName.ToString(), NamedSession != nullptr ? *NamedSession->GetSessionIdStr() : TEXT("None"));
		GEngine->AddOnScreenDebugMessage(-1, 3.f, bWasSuccessful ? FColor::Green : FColor::Red, LogString);
	}
#endif
	//UpdateSteamIDToGameState();

	if (!bWasSuccessful || GetWorld() == nullptr)
	{
		// 실패했으면 여기서 로딩 UI 를 내린다. 성공 시에는 트래블 이후까지 유지한다.
		NotifySessionLoadingScreen(false, ESessionLoadingPhase::ESLP_None);
	}

	if (bWasSuccessful && GetWorld())
	{
		UGameplayStatics::OpenLevel(GetWorld(), FName("/Game/Asset/GoStage"), false, TEXT("listen"));
		//GetWorld()->ServerTravel("/Game/Asset/GoStage?listen");
		if (APlayerController* PC = GetFirstLocalPlayerController())
		{
			UGoSoccerPlayManager::PlayerCreateSessionComplete(PC);
			UGoSoccerPlayManager::PlayerJoinedOrCreatedSessionComplete(PC, Execute_GetCSteamID(this));
			UGoSoccerPlayManager::SetHostingSessionCreateData(PC, HostingSessionCreateData);
			UGoSoccerPlayManager::UpdateTooltip_SessionConfig(PC, HostingSessionCreateData);
		}
		//Execute_ApplySavedSettings(this);
	}
}

void UGoSoccerGameInstance::OnFindSessionComplete(bool bWasSuccessful)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem == nullptr)
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("Subsystem Not Found"));
			GEngine->AddOnScreenDebugMessage(-1, 3.f, bWasSuccessful ? FColor::Red : FColor::Red, LogString);
		}
#endif
		return;
	}
	FString CurrentSubsystem = OnlineSubsystem->GetSubsystemName().ToString();

	if (!OnlineSessionInterface.IsValid() || !bWasSuccessful || SessionSearch == nullptr)
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("Session Not Found (Subsystem : %s)"), *CurrentSubsystem);
			GEngine->AddOnScreenDebugMessage(-1, 3.f, bWasSuccessful ? FColor::Red : FColor::Red, LogString);
		}
#endif
		return;
	}
	TArray<FSessionDisplayData> SearchSessionResults;

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("SearchResults Start"));
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
	}
#endif

	for (auto Result : SessionSearch->SearchResults)
	{
		FString ID = Result.Session.GetSessionIdStr();
		FString User = Result.Session.OwningUserName;

		FString SessionName;
		FString HostName;
		int PlayGameRule;
		Result.Session.SessionSettings.Get(Key_SessionName, SessionName);
		Result.Session.SessionSettings.Get(Key_HostName, HostName);
		SessionName = DecodeSessionText(SessionName);
		HostName = DecodeSessionText(HostName);
		Result.Session.SessionSettings.Get(Key_PlayGameRule, PlayGameRule);

#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(
				TEXT("Session ID : %s, Host : %s, SessionName : %s, PlayRule : %d, IsValid: %s, Ping: %d"),
				*ID,
				*HostName,
				*SessionName,
				PlayGameRule,
				Result.IsValid() ? TEXT("True") : TEXT("False"),
				Result.PingInMs
			);
			GEngine->AddOnScreenDebugMessage(-1, 3.f, Result.IsValid() ? FColor::Green : FColor::Red, LogString);
		}
#endif
		FSessionDisplayData SessionDisplayData;
		FBlueprintSessionResult BlueprintSessionResult;
		BlueprintSessionResult.OnlineResult = Result;

		SessionDisplayData.RawResult = BlueprintSessionResult;
		SessionDisplayData.ServerName = SessionName;
		SessionDisplayData.Ping = Result.PingInMs;
		SessionDisplayData.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
		SessionDisplayData.CurrentPlayers = SessionDisplayData.MaxPlayers - Result.Session.NumOpenPublicConnections;
		SessionDisplayData.HostName = HostName;
		SessionDisplayData.PlayGameRule = (PlayGameRule == 1 ? EPlayGameRule::EPGR_5or5Mode : EPlayGameRule::EPGR_5plus5Mode);
		SearchSessionResults.Emplace(SessionDisplayData);
		
		//OnlineSessionInterface->JoinSession(0, NAME_GameSession, Result);
	}

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("SearchResults End"));
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
	}
#endif
	Delegate_OnUpdateOnlineSessionList.Broadcast(SearchSessionResults);
}

void UGoSoccerGameInstance::OnJoinSessionComplate(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!OnlineSessionInterface.IsValid())
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("Join Session Failed : OnlineSessionInterface Invalid"));
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, LogString);
		}
#endif
		// 조인 요청 시 미리 세팅해 둔 온라인 세션 상태를 되돌린다. (트래블이 없으므로 그대로 두면 남는다)
		Execute_SetPlayGameMode(this, EPlayGameMode::EPGM_Default);
		NotifySessionLoadingScreen(false, ESessionLoadingPhase::ESLP_None);
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
		Execute_SetPlayGameMode(this, EPlayGameMode::EPGM_Default);
		NotifySessionLoadingScreen(false, ESessionLoadingPhase::ESLP_None);
		return;
	}
	HostingSessionCreateData = FSessionCreateData();
	FString Address;
	if (OnlineSessionInterface->GetResolvedConnectString(SessionName, Address))
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("Join Session Address: %s"), *Address);
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
			UE_LOG(LogTemp, Log, TEXT("Join Session Address: %s"), *Address);
		}
#endif
		if (APlayerController* PC = GetFirstLocalPlayerController())
		{
			PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute, false);
			//PC->ClientTravel(Address, ETravelType::TRAVEL_Relative, true);
			//Execute_ApplySavedSettings(this);
			// 여기서 호출하던 PC 는 접속이 완료되면 UNetConnection::HandleClientPlayer 에서 파괴된다.
			// (로컬 placeholder PC 이므로 Server RPC 가 호스트에 도달하지 않는다.)
			// 두 호출 모두 접속 후 새 PC 의 AGoPlayerController::Load_SessionInterface() 로 이동했다.
			//UGoSoccerPlayManager::PlayerJoinedSessionComplete(PC);
			//UGoSoccerPlayManager::PlayerJoinedOrCreatedSessionComplete(PC, Execute_GetCSteamID(this));
			//UGoSoccerPlayManager::Request_UpdateTooltip_SessionConfig()
			//UGoSoccerPlayManager::SetHostingSessionCreateData(PC, nullptr);
		}
		else
		{
			// 트래블을 태울 PC 가 없으면 조인이 진행되지 않으므로 로딩 UI 를 내린다.
			Execute_SetPlayGameMode(this, EPlayGameMode::EPGM_Default);
			NotifySessionLoadingScreen(false, ESessionLoadingPhase::ESLP_None);
		}
		//UpdateSteamIDToGameState();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Join Session : GetResolvedConnectString Failed"));
		Execute_SetPlayGameMode(this, EPlayGameMode::EPGM_Default);
		NotifySessionLoadingScreen(false, ESessionLoadingPhase::ESLP_None);
	}
}

void UGoSoccerGameInstance::OnDestroySessionComplate(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("=== OnDestroySessionComplate ==="));
	if (OnlineSessionInterface.IsValid())
	{
		if (DelegateHandle_OnDestroySessionComplete.IsValid())
		{
			OnlineSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DelegateHandle_OnDestroySessionComplete);
		}
	}
	TravelToStandAlone();
}

void UGoSoccerGameInstance::OnSessionFailure(const FUniqueNetId& UniqueNetId, ESessionFailure::Type SessionFailure)
{
	// ??? No Use
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		if (OnlineIdentity.IsValid())
		{
			FString PlayerName = OnlineIdentity->GetPlayerNickname(UniqueNetId);
			FString LogString = FString::Printf(TEXT("ServerConnectionLost %s - %s"),
				*PlayerName,
				*UniqueNetId.ToString()
			);
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, LogString);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("=== ServerConnectionLost ==="));
	if (OnlineIdentity.IsValid())
	{
		FString PlayerName = OnlineIdentity->GetPlayerNickname(UniqueNetId);
		UE_LOG(LogTemp, Warning, TEXT("SteamNickName : %s, NetID : %s"),
			*PlayerName,
			*UniqueNetId.ToString()
		);
		UE_LOG(LogTemp, Warning, TEXT("=== ServerConnectionLost ==="));
	}
#endif
	Server_OnSessionFailure();
}

void UGoSoccerGameInstance::OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful)
{
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("UpdateSession Completed Callback- %s"),
			bWasSuccessful ? TEXT("Success") : TEXT("Failed")
		);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, bWasSuccessful ? FColor::Green : FColor::Red, LogString);
	}
#endif
}

//bool UGoSoccerGameInstance::HostGame_Implementation(const FUniqueNetIdRepl& UniqueNetIdRepl, const FName& CreateSessionName)
//{
//	UWorld* W = GetWorld();
//	if (W == nullptr) return false;
//	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
//	if (OnlineSubsystem == nullptr) return false;
//	FString CurrentSubsystem = OnlineSubsystem->GetSubsystemName().ToString();
//
//#if !UE_BUILD_SHIPPING
//	if (GEngine)
//	{
//		FString LogString = FString::Printf(
//			TEXT("HostGame (Subsystem : %s) - SessionName : %s"), *CreateSessionName.ToString(), *CurrentSubsystem);
//		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
//	}
//#endif
//	FName SessionName = NAME_GameSession;
//	if (FNamedOnlineSession* ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession))
//	{
//		OnlineSessionInterface->DestroySession(SessionName);
//		return false;
//	}
//
//	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
//	SessionSettings->NumPublicConnections = 2;
//	SessionSettings->bShouldAdvertise = true;
//	SessionSettings->bAllowJoinInProgress = true;
//
//	if (CurrentSubsystem != FString("NULL"))
//	{
//		SessionSettings->bIsLANMatch = false;
//	}
//	else
//	{
//		SessionSettings->bIsLANMatch = true;
//	}
//	SessionSettings->bIsDedicated = false;
//	SessionSettings->bUsesPresence = true;
//	SessionSettings->bAllowJoinViaPresence = true;
//	SessionSettings->bUseLobbiesIfAvailable = true;
//	SessionSettings->Set(FName("SessionName"), CreateSessionName.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);
//	FString HostPlayerName;
//	Execute_GetIdentityPlayerNickName(this, HostPlayerName);
//	SessionSettings->Set(FName("HostName"), HostPlayerName, EOnlineDataAdvertisementType::ViaOnlineService);
//	//SessionSettings->Set(FName("Game_Key"), FString("GoSoccer"), EOnlineDataAdvertisementType::ViaOnlineService);
//	
//	return OnlineSessionInterface->CreateSession(*UniqueNetIdRepl, SessionName, *SessionSettings);
//}

bool UGoSoccerGameInstance::HostGame_Implementation(const FUniqueNetIdRepl& UniqueNetIdRepl, const FSessionCreateData& SessionCreateData)
{
	UWorld* W = GetWorld();
	if (W == nullptr) return false;
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem == nullptr) return false;
	FString CurrentSubsystem = OnlineSubsystem->GetSubsystemName().ToString();

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString DebugStringPreset = TEXT("None");
		if (SessionCreateData.BoardPreset == EBoardPreset::EBP_Korea)
		{
			DebugStringPreset = TEXT("Korea");
		}
		else if (SessionCreateData.BoardPreset == EBoardPreset::EBP_China)
		{
			DebugStringPreset = TEXT("China");
		}
		else if (SessionCreateData.BoardPreset == EBoardPreset::EBP_Tibet)
		{
			DebugStringPreset = TEXT("Tibet");
		}

		FString LogString = FString::Printf(
			TEXT("HostGame (Subsystem : %s) - SessionName : %s, BoardPreset : %s"),
			*SessionCreateData.SessionName,
			*CurrentSubsystem,
			*DebugStringPreset
		);
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
	}
#endif
	FName SessionName = NAME_GameSession;
	if (FNamedOnlineSession* ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession))
	{
		OnlineSessionInterface->DestroySession(SessionName);
		return false;
	}

	// 비공개 세션은 세션 목록(FindSessions)에 잡히지 않아야 한다.
	// bShouldAdvertise 를 끄면
	//  - Steam : 로비 타입이 Private 이 되어 RequestLobbyList 결과에서 빠진다. (초대로만 입장)
	//  - NULL(LAN) : LAN 검색 패킷에 응답하지 않는다.
	const bool bAdvertiseSession = !SessionCreateData.IsSecret;

	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->NumPublicConnections = 2;
	SessionSettings->bShouldAdvertise = bAdvertiseSession;
	SessionSettings->bAllowJoinInProgress = true;

	if (CurrentSubsystem != FString("NULL"))
	{
		SessionSettings->bIsLANMatch = false;
	}
	else
	{
		SessionSettings->bIsLANMatch = true;
	}
	SessionSettings->bIsDedicated = false;
	SessionSettings->bUsesPresence = true;
	SessionSettings->bAllowJoinViaPresence = bAdvertiseSession;
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->bAllowInvites = true;		// Steam Invite, 비공개 세션도 초대로는 들어올 수 있다
	SessionSettings->Set(Key_IsSecret, SessionCreateData.IsSecret, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings->Set(Key_SessionName, EncodeSessionText(SessionCreateData.SessionName), EOnlineDataAdvertisementType::ViaOnlineService);
	FString HostPlayerName;
	Execute_GetIdentityPlayerNickName(this, HostPlayerName);
	SessionSettings->Set(Key_HostName, EncodeSessionText(HostPlayerName), EOnlineDataAdvertisementType::ViaOnlineService);
	int PlayGameRule = static_cast<int>(SessionCreateData.PlayGameRule);
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("HostGame : PlayGameRule : %d"), PlayGameRule);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
	}
#endif
	SessionSettings->Set(Key_PlayGameRule, PlayGameRule, EOnlineDataAdvertisementType::ViaOnlineService);

	int DollType = static_cast<int>(SessionCreateData.PlayDollType);
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("HostGame : DollType : %d"), DollType);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
	}
#endif
	SessionSettings->Set(Key_DollType, DollType, EOnlineDataAdvertisementType::ViaOnlineService);

	int BoardPreset = static_cast<int>(SessionCreateData.BoardPreset);
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("HostGame : BoardPreset : %d"), BoardPreset);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
	}
#endif
	SessionSettings->Set(Key_BoardPreset, BoardPreset, EOnlineDataAdvertisementType::ViaOnlineService);

	int PlayTurnRule = static_cast<int>(SessionCreateData.PlayTurnRule);
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("HostGame : PlayTurnRule : %d"), PlayTurnRule);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
	}
#endif
	SessionSettings->Set(Key_PlayTurnRule, PlayTurnRule, EOnlineDataAdvertisementType::ViaOnlineService);

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("HostGame : TimeLimit : %f"), SessionCreateData.TimeLimit);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
	}
#endif
	SessionSettings->Set(Key_TimeLimit, SessionCreateData.TimeLimit, EOnlineDataAdvertisementType::ViaOnlineService);

	SessionSettings->Set(Key_AngularThreshold, SessionCreateData.AngularThreshold, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings->Set(Key_DistanceThreshold, SessionCreateData.DistanceThreshold, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings->Set(Key_EnableItem, SessionCreateData.bEnableItem, EOnlineDataAdvertisementType::ViaOnlineService);
	int MatchRule = static_cast<int>(SessionCreateData.MatchRule);
	SessionSettings->Set(Key_MatchRule, MatchRule, EOnlineDataAdvertisementType::ViaOnlineService);


	HostingSessionCreateData = SessionCreateData;

	// Create 요청 시작 -> 로딩 UI On. (해제는 새 레벨의 PC 가 Load() 를 마칠 때)
	NotifySessionLoadingScreen(true, ESessionLoadingPhase::ESLP_CreateSession);

	const bool bRequested = OnlineSessionInterface->CreateSession(*UniqueNetIdRepl, SessionName, *SessionSettings);
	if (!bRequested)
	{
		// 요청 자체가 실패하면 OnCreateSessionComplete 가 안 올 수 있으므로 여기서 내린다.
		NotifySessionLoadingScreen(false, ESessionLoadingPhase::ESLP_None);
	}
	return bRequested;
}

bool UGoSoccerGameInstance::FindSession_Implementation(const FUniqueNetIdRepl& UniqueNetIdRepl)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem == nullptr)
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("Subsystem Not Found"));
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, LogString);
		}
#endif
		return false;
	}
	FString CurrentSubsystem = OnlineSubsystem->GetSubsystemName().ToString();

	if (!OnlineSessionInterface.IsValid())
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("OnlineSessionInterface Invalied (Subsystem : %s)"), *CurrentSubsystem);
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, LogString);
		}
#endif
		return false;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = 20;
	if (CurrentSubsystem != FString("NULL"))
	{
		SessionSearch->bIsLanQuery = false;
	}
	else
	{
		SessionSearch->bIsLanQuery = true;
	}
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	//SessionSearch->QuerySettings.Set(FName("Game_Key"), FString("GoSoccer"), EOnlineComparisonOp::Equals);


#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("FindSessions"));
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
	}
#endif
	
	return OnlineSessionInterface->FindSessions(*UniqueNetIdRepl, SessionSearch.ToSharedRef());
}

bool UGoSoccerGameInstance::JoinSelectedSession_Implementation(UObject* SelectedOnlineSessionData, const FUniqueNetIdRepl& UniqueNetIdRepl)
{
	if (SelectedOnlineSessionData == nullptr) return false;
	UGameSessionData* GameSessionData = Cast<UGameSessionData>(SelectedOnlineSessionData);
	if (GameSessionData == nullptr) return false;
	FOnlineSessionSearchResult OnlineSessionSearchResult;
	GameSessionData->GetOnlineSessionSearchResult(OnlineSessionSearchResult);
	if (!OnlineSessionSearchResult.IsValid()) return false;
	if (!OnlineSessionInterface.IsValid()) return false;

	// Join 요청 시작 -> 로딩 UI On. (해제는 접속 후 새 PC 가 Load() 를 마칠 때)
	NotifySessionLoadingScreen(true, ESessionLoadingPhase::ESLP_JoinSession);

	// 트래블 후 새 PC 의 Load_SessionInterface() 는 PlayGameMode 로 온라인 세션 여부를 판단한다.
	// PlayGameMode 를 InLobby 진입 콜백(OnJoinSelectedSessionCompleted)에서 세팅하면
	// "조인 완료 통지 -> PlayGameMode 세팅" 순환이 되어 통지 자체가 실행되지 않으므로,
	// 초대 조인 경로(ExecutePendingInviteJoin)와 동일하게 요청 시점에 미리 세팅한다.
	Execute_SetPlayerSessionHost(this, false);
	Execute_SetPlayGameMode(this, EPlayGameMode::EPGM_OnlineSession);

	const bool bRequested = OnlineSessionInterface->JoinSession(
		*UniqueNetIdRepl,
		NAME_GameSession,
		OnlineSessionSearchResult
	);
	if (!bRequested)
	{
		// 요청 자체가 실패하면 OnJoinSessionComplate 가 안 올 수 있으므로 여기서 내린다.
		Execute_SetPlayGameMode(this, EPlayGameMode::EPGM_Default);
		NotifySessionLoadingScreen(false, ESessionLoadingPhase::ESLP_None);
	}
	return bRequested;
}

bool UGoSoccerGameInstance::LeaveSession_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("=== GoSoccerGameInstance : LeaveSession ==="));
	if (OnlineSessionInterface.IsValid())
	{
		if (DelegateHandle_OnDestroySessionComplete.IsValid())
		{
			OnlineSessionInterface->OnDestroySessionCompleteDelegates.Remove(DelegateHandle_OnDestroySessionComplete);
			DelegateHandle_OnDestroySessionComplete.Reset();
			//OnlineSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DelegateHandle_OnDestroySessionComplete);
		}
		DelegateHandle_OnDestroySessionComplete = OnlineSessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UGoSoccerGameInstance::OnDestroySessionComplate);
		OnlineSessionInterface->DestroySession(NAME_GameSession);
		return true;
	}
	TravelToStandAlone();
	return false;
}

bool UGoSoccerGameInstance::GetCurrentMainUIState_Implementation(EMainUIState& OutMainUIState)
{
	OutMainUIState = MainUIState;
	return true;
}

bool UGoSoccerGameInstance::SetMaxFPS_Implementation(float NewMaxFPS)
{
	Client_SetMaxFPS(NewMaxFPS, false);
	return true;
}

bool UGoSoccerGameInstance::SetbUseLowTexture_Implementation(bool NewbUseLowTexture)
{
	Client_SetbUseLowTexture(NewbUseLowTexture, false);
	return true;
}

bool UGoSoccerGameInstance::SetfShadowLevel_Implementation(float NewfShadowLevel)
{
	int32 tempShadowResolution = FMath::CeilToInt(NewfShadowLevel);
	Client_SetfShadowLevel(tempShadowResolution, false);
	return true;
}

bool UGoSoccerGameInstance::SetAAType_Implementation(int32 NewAAType)
{
	Client_SetAAType(NewAAType, false);
	return true;
}

bool UGoSoccerGameInstance::SetSettingFloat_Implementation(const ESettingConfigParamType SettingConfigParamType, float fNewLevel)
{
	Client_SetSettingFloat(SettingConfigParamType, fNewLevel, false);
	return true;
}

bool UGoSoccerGameInstance::SetScreenResolution_Implementation(FIntPoint NewResolution)
{
	Client_SetScreenResolution(NewResolution);
	return true;
}

bool UGoSoccerGameInstance::SetWindowMode_Implementation(EWindowMode::Type NewWindowMode)
{
	Client_SetWindowMode(NewWindowMode);
	return true;
}

bool UGoSoccerGameInstance::SetCultureSetting_Implementation(const FString& NewCulture)
{
	FInternationalization::Get().SetCurrentCulture(NewCulture);

	UGameUserSettings* UserSettingsObj = UGameUserSettings::GetGameUserSettings();
	if (UserSettingsObj == nullptr) return false;

	// Client_ApplySavedSettings_Implementation 이 시작 시 쓰는 것과 같은 매핑이다. (ko <-> ELS_Korean, 그 외 <-> ELS_English)
	const ELanguageSetting NewLanguageSetting = (NewCulture == TEXT("ko")) ? ELanguageSetting::ELS_Korean : ELanguageSetting::ELS_English;
	IIGoSoccerGameUserSettings::Execute_SetLanguageSetting(UserSettingsObj, NewLanguageSetting);
	UserSettingsObj->SaveSettings();
	return true;
}

bool UGoSoccerGameInstance::ApplySavedSettings_Implementation()
{
	Client_LoadSavedSettings();
	Client_ApplySavedSettings();
	return true;
}

bool UGoSoccerGameInstance::GetPlayGameMode_Implementation(EPlayGameMode& OutPlayGameMode)
{
	OutPlayGameMode = PlayGameMode;
	return true; // TODO return false if Server
}

void UGoSoccerGameInstance::SetPlayGameMode_Implementation(EPlayGameMode InPlayGameMode)
{
	Client_SetPlayGameMode(InPlayGameMode);
}

//TObjectPtr<UTexture2D> UGoSoccerGameInstance::GetSteamAvatar_Implementation(int32 ImageRGBAHandle)
//{
//	uint32 Width, Height;
//	if (ImageRGBAHandle == 0 || !SteamUtils()->GetImageSize(ImageRGBAHandle, &Width, &Height))
//	{
//		return nullptr;
//	}
//
//	uint32 BufferSize = Width * Height * 4;
//	uint8* RawBuffer = new uint8[BufferSize];
//
//	if (SteamUtils()->GetImageRGBA(ImageRGBAHandle, RawBuffer, BufferSize))
//	{
//		UTexture2D* NewTexture = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);
//
//		if (NewTexture)
//		{
//			void* TextureData = NewTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
//			FMemory::Memcpy(TextureData, RawBuffer, BufferSize);
//			NewTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
//
//			NewTexture->UpdateResource();
//		}
//
//		delete[] RawBuffer;
//		return NewTexture;
//	}
//
//	delete[] RawBuffer;
//	return nullptr;
//}

bool UGoSoccerGameInstance::BindDelegate_Widget_Implementation(UObject* WidgetDelegate)
{
	if (Delegate_OnUpdateOnlineSessionList.IsBound() && DelegateHandle_OnUpdateOnlineSessionList.IsValid())
	{
		Delegate_OnUpdateOnlineSessionList.Remove(DelegateHandle_OnUpdateOnlineSessionList);
		DelegateHandle_OnUpdateOnlineSessionList.Reset();
	}
	DelegateHandle_OnUpdateOnlineSessionList = Delegate_OnUpdateOnlineSessionList.AddUFunction(WidgetDelegate, TEXT("Callback_OnUpdateOnlineSessionList"));

	if (Delegate_OnClearOnlineSessionList.IsBound() && DelegateHandle_OnClearOnlineSessionList.IsValid())
	{
		Delegate_OnClearOnlineSessionList.Remove(DelegateHandle_OnUpdateOnlineSessionList);
		DelegateHandle_OnClearOnlineSessionList.Reset();
	}
	DelegateHandle_OnClearOnlineSessionList = Delegate_OnClearOnlineSessionList.AddUFunction(WidgetDelegate, TEXT("Callback_OnClearOnlineSessionList"));

	return DelegateHandle_OnUpdateOnlineSessionList.IsValid() && DelegateHandle_OnClearOnlineSessionList.IsValid();
}

bool UGoSoccerGameInstance::GetIdentityPlayerNickName_Implementation(FString& OutPlayerNickName)
{
	if (OnlineIdentity.IsValid())
	{
		OutPlayerNickName = OnlineIdentity->GetPlayerNickname(0);
	}
	return OnlineIdentity.IsValid();
}

bool UGoSoccerGameInstance::SetSessionCreateData_Implementation(const FSessionCreateData& InSessionCreateData)
{
	HostingSessionCreateData = InSessionCreateData;

	if (OnlineSessionInterface == nullptr)
	{
		IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
		if (OnlineSubsystem == nullptr) return false;
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
	}
	if (OnlineSessionInterface == nullptr) return false;
	FOnlineSessionSettings* SessionSettings = OnlineSessionInterface->GetSessionSettings(NAME_GameSession);
	if (SessionSettings != nullptr)
	{
		ApplySessionSettings(SessionSettings, InSessionCreateData);
		OnlineSessionInterface->UpdateSession(NAME_GameSession, *SessionSettings);
	}

	return true;
}

bool UGoSoccerGameInstance::MarkUIState_ClientTravelCallback_Implementation(EMainUIState InSucceededUIState, EMainUIState InFailededUIState)
{
	ClientTravel_Succeeded_CallbakUIState = InSucceededUIState;
	ClientTravel_Failed_CallbakUIState = InFailededUIState;
	return true;
}

bool UGoSoccerGameInstance::GetClientTravelCallbackUIState_Implementation(EMainUIState& OutSucceededUIState, EMainUIState& OutFailededUIState)
{
	OutSucceededUIState = ClientTravel_Succeeded_CallbakUIState;
	OutFailededUIState = ClientTravel_Failed_CallbakUIState;
	return true;
}

bool UGoSoccerGameInstance::StackChattingMessage_Implementation(UObject* ChattingBoxObject)
{
	if (ChattingBoxObject != nullptr)
	{
		StackedChattingMessage.Emplace(ChattingBoxObject);
		return true;
	}
	return false;
}

void UGoSoccerGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Log, TEXT("NetWork Error : %s"), *ErrorString);
#endif
	HostingSessionCreateData = FSessionCreateData();

	//if (FailureType == ENetworkFailure::ConnectionLost || FailureType == ENetworkFailure::FailureReceived)
	//{
	//	GetPrimaryPlayerController
	//}
	// TODO Thudo
	// Throw Message
	// Go Standalone
	//
	TravelToStandAlone();
}

void UGoSoccerGameInstance::Client_SetfShadowLevel_Implementation(int32 NewShadowResolution, bool bForce)
{
	// ShadowMaxResolution 은 Graphic_Shadow(ESCPT_Graphic_Shadow) 와 같은 값이므로
	// GetShadowQuality()/SetShadowQuality() 저장소를 그대로 공유한다.
	auto UserSettings = UGameUserSettings::GetGameUserSettings();
	if (!UserSettings) return;
	if (UserSettings->GetShadowQuality() != NewShadowResolution || bForce)
	{
		IConsoleVariable* CVarMaxResolution = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.MaxResolution"));
		IConsoleVariable* CVarShadowQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ShadowQuality"));
		if (CVarMaxResolution && CVarShadowQuality)
		{
			CVarMaxResolution->Set(
				NewShadowResolution == 0 ? NewShadowResolution : (
					NewShadowResolution == 1 ? 128 : (
						NewShadowResolution == 2 ? 256 : (
							NewShadowResolution == 3 ? 512 : (
								1024
								)
							)
						)
					), EConsoleVariableFlags::ECVF_SetByConsole
			);
			CVarShadowQuality->Set(NewShadowResolution, EConsoleVariableFlags::ECVF_SetByConsole);
			// 주의: 여기서 UserSettings->SetShadowQuality()+ApplySettings() 를 같이 부르지 않는다.
			// ApplySettings()->ApplyNonResolutionSettings() 가 Scalability::SetQualityLevels() 를 호출해서
			// 위에서 커스텀으로 매핑한 128/256/512/1024 해상도 값을 엔진 기본 스케일러빌리티 값으로 덮어써버린다.
			// 예전부터 이 경로는 재시작 시 저장되지 않는 인스턴트 프리뷰였고, 그 동작은 그대로 유지한다.
#if !UE_BUILD_SHIPPING
			if (GEngine)
			{
				FString LogString = FString::Printf(TEXT("Client_SetfShadowLevel: r.ShadowQuality %d"), NewShadowResolution);
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, LogString);
			}
#endif
		}
	}
}

void UGoSoccerGameInstance::Client_ApplySavedSettings_Implementation()
{
	UGameUserSettings* UserSettingsObj = UGameUserSettings::GetGameUserSettings();
	if (UserSettingsObj == nullptr) return;

	Client_SetMaxFPS(UserSettingsObj->GetFrameRateLimit(), true);
	//Client_SetfShadowLevel(UserSettingsObj->GetShadowQuality(), true);
	Client_SetAAType(UserSettingsObj->GetAntiAliasingQuality(), true);

	float Volume_Master = 1.f, Volume_BGM = 1.f, Volume_SFX = 1.f;
	ELanguageSetting LanguageSetting = ELanguageSetting::ELS_Default;

	IIGoSoccerGameUserSettings::Execute_GetVolume_Master(UserSettingsObj, Volume_Master);
	IIGoSoccerGameUserSettings::Execute_GetVolume_BGM(UserSettingsObj, Volume_BGM);
	IIGoSoccerGameUserSettings::Execute_GetVolume_SFX(UserSettingsObj, Volume_SFX);
	IIGoSoccerGameUserSettings::Execute_GetLanguageSetting(UserSettingsObj, LanguageSetting);

	Client_SetSettingFloat(ESettingConfigParamType::ESCPT_MasterVolume,	Volume_Master,	true);
	Client_SetSettingFloat(ESettingConfigParamType::ESCPT_BGMVolume,	Volume_BGM,		true);
	Client_SetSettingFloat(ESettingConfigParamType::ESCPT_FXVolume,		Volume_SFX,		true);

	FString Culture = TEXT("en-US");
	if (LanguageSetting == ELanguageSetting::ELS_Korean) Culture = TEXT("ko");
	FInternationalization::Get().SetCurrentCulture(Culture);

	Client_SetSettingFloat(ESettingConfigParamType::ESCPT_Graphic_Texture,				UserSettingsObj->GetTextureQuality(),				true);
	Client_SetSettingFloat(ESettingConfigParamType::ESCPT_Graphic_Shadow,				UserSettingsObj->GetShadowQuality(),				true);
	Client_SetSettingFloat(ESettingConfigParamType::ESCPT_Graphic_Reflection,			UserSettingsObj->GetReflectionQuality(),			true);
	Client_SetSettingFloat(ESettingConfigParamType::ESCPT_Graphic_PostProcess,			UserSettingsObj->GetPostProcessingQuality(),		true);
	Client_SetSettingFloat(ESettingConfigParamType::ESCPT_Graphic_GlobalIllumination,	UserSettingsObj->GetGlobalIlluminationQuality(),	true);
	Client_SetSettingFloat(ESettingConfigParamType::ESCPT_Graphic_Effect,				UserSettingsObj->GetVisualEffectQuality(),			true);

	Client_SetScreenResolution(UserSettingsObj->GetScreenResolution());
	Client_SetWindowMode(UserSettingsObj->GetFullscreenMode());
}

void UGoSoccerGameInstance::Client_SetbUseLowTexture_Implementation(bool NewbUseLowTexture, bool bForce)
{
	//if (MPC_MaterialSettings == nullptr) return;
	//if (GameSettings.bUseLowTexture != NewbUseLowTexture || bForce)
	//{
	//	UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), MPC_MaterialSettings, FName("bUseLowTexture"), NewbUseLowTexture ? 1.f : 0.f);
	//	GameSettings.bUseLowTexture = NewbUseLowTexture;
	//}
}

void UGoSoccerGameInstance::Client_SetMaxFPS_Implementation(float NewMaxFPS, bool bForce)
{
	auto UserSettings = UGameUserSettings::GetGameUserSettings();
	if (!UserSettings) return;
	if (UserSettings->GetFrameRateLimit() != NewMaxFPS || bForce)
	{
		// UGameUserSettings::SetFrameRateLimit 은 값만 저장하고, 실제 t.MaxFPS CVar 반영과 저장은
		// ApplySettings() (-> ApplyNonResolutionSettings() -> SetFrameRateLimitCVar(), SaveSettings()) 가 담당한다.
		UserSettings->SetFrameRateLimit(NewMaxFPS);
		UserSettings->ApplySettings(false);
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("Client_SetMaxFPS: t.MaxFPS %f"), NewMaxFPS);
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, LogString);
		}
#endif
	}
}

void UGoSoccerGameInstance::Client_SetPlayGameMode_Implementation(EPlayGameMode InPlayGameMode)
{
	PlayGameMode = InPlayGameMode;
}

void UGoSoccerGameInstance::Client_SetAAType_Implementation(int32 NewAAType, bool bForce)
{
	auto UserSettings = UGameUserSettings::GetGameUserSettings();
	if (!UserSettings) return;
	if (UserSettings->GetAntiAliasingQuality() != NewAAType || bForce)
	{
		UserSettings->SetAntiAliasingQuality(NewAAType);
		UserSettings->ApplySettings(false);
//		IConsoleVariable* CVarAAType = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AntiAliasingMethod"));
//		if (CVarAAType)
//		{
//			CVarAAType->Set(NewAAType, EConsoleVariableFlags::ECVF_SetByConsole);
//		}
//#if !UE_BUILD_SHIPPING
//		if (GEngine)
//		{
//			FString LogString = FString::Printf(TEXT("Client_SetAAType: r.AntiAliasingMethod %d"), NewAAType);
//			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, LogString);
//		}
//#endif
	}
}

void UGoSoccerGameInstance::Client_SetSettingFloat_Implementation(const ESettingConfigParamType SettingConfigParamType, float fNewLevel, bool bForce)
{
	auto ChangeSettingViaCommandLine = [&](const TCHAR* CommandLine, auto NewLevel)-> bool
		{
			IConsoleVariable* CVarAAType = IConsoleManager::Get().FindConsoleVariable(CommandLine);
			if (CVarAAType)
			{
				CVarAAType->Set(NewLevel, EConsoleVariableFlags::ECVF_SetByConsole);
#if !UE_BUILD_SHIPPING
			if (GEngine)
			{
				
				FString LogString;
				if (NewLevel - static_cast<int32>(NewLevel) > 0.f) LogString = FString::Printf(TEXT("Client %s - %f"), CommandLine, NewLevel);
				else LogString = FString::Printf(TEXT("Client %s - %d"), CommandLine, NewLevel);
				//FString LogString = FString::Printf(TEXT("Client %s - %f ? %d"), CommandLine, NewLevel, NewLevel);
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, LogString);
			}
#endif
				return true;
			}
			return false;
		};
#if UE_EDITOR
	UEnum* EnumPtr = StaticEnum<ESettingConfigParamType>();
	if (EnumPtr)
	{
		FString tempS = EnumPtr->GetNameStringByValue((int64)SettingConfigParamType);
		UE_LOG(LogTemp, Log, TEXT("Client_SetSettingFloat %s : %f"), *tempS, fNewLevel);
	}
#endif
	if (SettingConfigParamType == ESettingConfigParamType::ESCPT_MasterVolume ||
		SettingConfigParamType == ESettingConfigParamType::ESCPT_BGMVolume ||
		SettingConfigParamType == ESettingConfigParamType::ESCPT_FXVolume)
	{
		// 볼륨은 더 이상 GameSettings 에 있지 않고 UGoSoccerGameUserSettings 가 들고 있다.
		UGameUserSettings* UserSettingsObj = UGameUserSettings::GetGameUserSettings();
		if (SM_DefaultSoundMix == nullptr || UserSettingsObj == nullptr) return;

		USoundClass* ChangeSC = nullptr;
		float CurrentVolume = 0.f;
		switch (SettingConfigParamType)
		{
		case ESettingConfigParamType::ESCPT_MasterVolume:
			IIGoSoccerGameUserSettings::Execute_GetVolume_Master(UserSettingsObj, CurrentVolume);
			ChangeSC = SC_MasterVolume.Get();
			break;
		case ESettingConfigParamType::ESCPT_BGMVolume:
			IIGoSoccerGameUserSettings::Execute_GetVolume_BGM(UserSettingsObj, CurrentVolume);
			ChangeSC = SC_BGMVolume.Get();
			break;
		case ESettingConfigParamType::ESCPT_FXVolume:
			IIGoSoccerGameUserSettings::Execute_GetVolume_SFX(UserSettingsObj, CurrentVolume);
			ChangeSC = SC_FXVolume.Get();
			break;
		default:
			break;
		}

		if (CurrentVolume != fNewLevel || bForce)
		{
			const float ClampedVolume = FMath::Clamp(fNewLevel, 0.f, 1.f);
			switch (SettingConfigParamType)
			{
			case ESettingConfigParamType::ESCPT_MasterVolume:
				IIGoSoccerGameUserSettings::Execute_SetVolume_Master(UserSettingsObj, ClampedVolume);
				break;
			case ESettingConfigParamType::ESCPT_BGMVolume:
				IIGoSoccerGameUserSettings::Execute_SetVolume_BGM(UserSettingsObj, ClampedVolume);
				break;
			case ESettingConfigParamType::ESCPT_FXVolume:
				IIGoSoccerGameUserSettings::Execute_SetVolume_SFX(UserSettingsObj, ClampedVolume);
				break;
			default:
				break;
			}

			UGameplayStatics::PushSoundMixModifier(GetWorld(), SM_DefaultSoundMix);

			UGameplayStatics::SetSoundMixClassOverride(
				GetWorld(),
				SM_DefaultSoundMix,
				ChangeSC,
				ClampedVolume
			);
		}
	}
	else if (SettingConfigParamType == ESettingConfigParamType::ESCPT_Graphic_Total ||
		SettingConfigParamType == ESettingConfigParamType::ESCPT_Graphic_Shadow ||
		SettingConfigParamType == ESettingConfigParamType::ESCPT_Graphic_GlobalIllumination ||
		SettingConfigParamType == ESettingConfigParamType::ESCPT_Graphic_Reflection ||
		SettingConfigParamType == ESettingConfigParamType::ESCPT_Graphic_PostProcess ||
		SettingConfigParamType == ESettingConfigParamType::ESCPT_Graphic_Texture ||
		SettingConfigParamType == ESettingConfigParamType::ESCPT_Graphic_Effect)
	{
		auto UserSettings = UGameUserSettings::GetGameUserSettings();
		if (!UserSettings) return;

		float ClampedLevel = FMath::Clamp(fNewLevel, 0.f, 4.f);
		int32 IntLevel = (int32)ClampedLevel;

		switch (SettingConfigParamType)
		{
		case(ESettingConfigParamType::ESCPT_Graphic_Total):
			Client_SetSettingFloat(ESettingConfigParamType::ESCPT_Graphic_Shadow, fNewLevel, bForce);
			Client_SetSettingFloat(ESettingConfigParamType::ESCPT_Graphic_GlobalIllumination, fNewLevel, bForce);
			Client_SetSettingFloat(ESettingConfigParamType::ESCPT_Graphic_Reflection, fNewLevel, bForce);
			Client_SetSettingFloat(ESettingConfigParamType::ESCPT_Graphic_PostProcess, fNewLevel, bForce);
			Client_SetSettingFloat(ESettingConfigParamType::ESCPT_Graphic_Texture, fNewLevel, bForce);
			Client_SetSettingFloat(ESettingConfigParamType::ESCPT_Graphic_Effect, fNewLevel, bForce);
			break;
		case(ESettingConfigParamType::ESCPT_Graphic_Shadow):
			UserSettings->SetShadowQuality(IntLevel);
			break;
		case(ESettingConfigParamType::ESCPT_Graphic_GlobalIllumination):
			UserSettings->SetGlobalIlluminationQuality(IntLevel);
			break;
		case(ESettingConfigParamType::ESCPT_Graphic_Reflection):
			UserSettings->SetReflectionQuality(IntLevel);
			break;
		case(ESettingConfigParamType::ESCPT_Graphic_PostProcess):
			UserSettings->SetPostProcessingQuality(IntLevel);
			break;
		case(ESettingConfigParamType::ESCPT_Graphic_Texture):
			UserSettings->SetTextureQuality(IntLevel);
			break;
		case(ESettingConfigParamType::ESCPT_Graphic_Effect):
			UserSettings->SetVisualEffectQuality(IntLevel);
			break;
		default:
			break;
		}
		UserSettings->ApplySettings(false);
	}
}

//void UGoSoccerGameInstance::Client_UpdateSteamIDToGameState()
//{
//	if (SteamAPI_Init())
//	{
//		CSteamID SteamID = SteamUser()->GetSteamID();
//		bool bIsValidID = SteamID.IsValid();
//		uint64 RawID = bIsValidID ? SteamID.ConvertToUint64() : 0;
//		FString tempName{ "" };
//		if (SteamFriends())
//		{
//			const char* PersonaName = SteamFriends()->GetFriendPersonaName(SteamID);
//			tempName = UTF8_TO_TCHAR(PersonaName);
//		}
//		UGoSoccerPlayManager::PostSteamID();
//
//#if !UE_BUILD_SHIPPING
//		if (GEngine)
//		{
//			FString LogString = FString::Printf(TEXT("SteamAPI Valid, SteamID %s Raw : %d, Name : %s"),
//				bIsValidID ? TEXT("Valid") : TEXT("Invalid"),
//				RawID,
//				*tempName
//			);
//			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
//		}
//#endif
//	}
//	else
//	{
//
//#if !UE_BUILD_SHIPPING
//		if (GEngine)
//		{
//			FString LogString = FString::Printf(TEXT("SteamAPI InValid"));
//			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, LogString);
//		}
//#endif
//	}
//}

int64 UGoSoccerGameInstance::GetCSteamID_Implementation()
{
	uint64 RawID = 0;
	int64 rtn = -1;
	if (SteamAPI_Init())
	{
		CSteamID SteamID = SteamUser()->GetSteamID();
		bool bIsValidID = SteamID.IsValid();
		if (bIsValidID)
		{
			RawID = SteamID.ConvertToUint64();
			rtn = static_cast<int64>(RawID);
		}
		FString tempName{ "" };
		if (SteamFriends())
		{
			const char* PersonaName = SteamFriends()->GetFriendPersonaName(SteamID);
			tempName = UTF8_TO_TCHAR(PersonaName);
		}
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("SteamAPI Valid, SteamID %s Raw : %d, Name : %s"),
				bIsValidID ? TEXT("Valid") : TEXT("Invalid"),
				RawID,
				*tempName
			);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
		}
#endif
	}
	else
	{
	
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("SteamAPI InValid"));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, LogString);
		}
#endif
	}
	
	return rtn;
}

bool UGoSoccerGameInstance::GetSteamOSSOnline_Implementation()
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem == nullptr) return false;
	if (OnlineSubsystem->GetSubsystemName() != TEXT("Steam")) return false;
	return SteamAPI_Init() && SteamFriends();
}

bool UGoSoccerGameInstance::GetSessionCreateData_FromSessionInterface_Implementation(FSessionCreateData& OutSessionCreateData)
{
	if (OnlineSessionInterface == nullptr)
	{
		IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
		if (OnlineSubsystem == nullptr) return false;
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
	}
	if (OnlineSessionInterface == nullptr) return false;

	OutSessionCreateData.PlayGameMode = EPlayGameMode::EPGM_OnlineSession;

	FOnlineSessionSettings* SessionSettings = OnlineSessionInterface->GetSessionSettings(NAME_GameSession);
	if (SessionSettings == nullptr) return false;
	FString CurrentSessionName;
	if (SessionSettings->Get(Key_SessionName, CurrentSessionName))
	{
		OutSessionCreateData.SessionName = DecodeSessionText(CurrentSessionName);
	}
	FString CurrentHostName;
	if (SessionSettings->Get(Key_HostName, CurrentHostName))
	{
		OutSessionCreateData.HostName = DecodeSessionText(CurrentHostName);
	}
	int32 CurrentPlayGameRule;
	if (SessionSettings->Get(Key_PlayGameRule, CurrentPlayGameRule))
	{
		EPlayGameRule tempRule = static_cast<EPlayGameRule>(CurrentPlayGameRule);
		OutSessionCreateData.PlayGameRule = tempRule;
	}
	int32 CurrentDollType;
	if (SessionSettings->Get(Key_DollType, CurrentDollType))
	{
		EPlayDollType tempDollType = static_cast<EPlayDollType>(CurrentDollType);
		OutSessionCreateData.PlayDollType = tempDollType;
	}
	int32 CurrentBoardPreset;
	if (SessionSettings->Get(Key_BoardPreset, CurrentBoardPreset))
	{
		EBoardPreset tempBoardPreset = static_cast<EBoardPreset>(CurrentBoardPreset);
		OutSessionCreateData.BoardPreset = tempBoardPreset;
	}
	int32 CurrentPlayTurnRule;
	if (SessionSettings->Get(Key_PlayTurnRule, CurrentPlayTurnRule))
	{
		EPlayTurnRule tempPlayTurnRule = static_cast<EPlayTurnRule>(CurrentPlayTurnRule);
		OutSessionCreateData.PlayTurnRule = tempPlayTurnRule;
	}
	float CurrentTimeLimit;
	if (SessionSettings->Get(Key_TimeLimit, CurrentTimeLimit))
	{
		OutSessionCreateData.TimeLimit = CurrentTimeLimit;
	}
	float CurrentAThreshold;
	if (SessionSettings->Get(Key_AngularThreshold, CurrentAThreshold))
	{
		OutSessionCreateData.AngularThreshold = CurrentAThreshold;
	}
	float CurrentDThreshold;
	if (SessionSettings->Get(Key_DistanceThreshold, CurrentDThreshold))
	{
		OutSessionCreateData.DistanceThreshold = CurrentDThreshold;
	}
	bool bEnableItem;
	if (SessionSettings->Get(Key_EnableItem, bEnableItem))
	{
		OutSessionCreateData.bEnableItem = bEnableItem;
	}
	int MatchRule;
	if (SessionSettings->Get(Key_MatchRule, MatchRule))
	{
		EMatchRule tempMatchRule = static_cast<EMatchRule>(MatchRule);
		OutSessionCreateData.MatchRule = tempMatchRule;
	}
	bool bIsSecret;
	if (SessionSettings->Get(Key_IsSecret, bIsSecret))
	{
		OutSessionCreateData.IsSecret = bIsSecret;
	}
	else
	{
		// 예전에 만들어진 세션 등 키가 없으면 광고 여부로 판단한다.
		OutSessionCreateData.IsSecret = !SessionSettings->bShouldAdvertise;
	}
	return true;
}

bool UGoSoccerGameInstance::SetPlayerSessionHost_Implementation(bool e)
{
	bIsHost = e;
	return true;
}

void UGoSoccerGameInstance::Server_OnSessionFailure_Implementation()
{
}

bool UGoSoccerGameInstance::TravelToStandAlone()
{
	UE_LOG(LogTemp, Warning, TEXT("=== TravelToStandAlone ==="));

	NotifySessionLoadingScreen(false, ESessionLoadingPhase::ESLP_None);
	if (APlayerController* PC = GetFirstLocalPlayerController())
	{
		UE_LOG(LogTemp, Warning, TEXT("TravelToStandAlone : ClientTravel"));
		//PC->ClientTravel(TEXT("/Game/Asset/GoStage"), ETravelType::TRAVEL_Absolute);
		PC->ClientTravel(TEXT("/Game/Asset/GoStage"), ETravelType::TRAVEL_Relative, true);
		UGoSoccerPlayManager::SetMainUIState(this, PC, EMainUIState::EMUIS_VeryFirst);
		//UGoSoccerPlayManager::SetCameraPosition(PC, ECameraPosition::ECP_Lobby);
		return true;
	}
	return false;
}

void UGoSoccerGameInstance::ApplySessionSettings(FOnlineSessionSettings* OutSessionSettings, const FSessionCreateData& InSessionCreateData)
{
	// HostGame_Implementation 과 동일한 규칙. 비공개 세션은 검색에 잡히지 않게 한다.
	const bool bAdvertiseSession = !InSessionCreateData.IsSecret;

	OutSessionSettings->NumPublicConnections = 2;
	OutSessionSettings->bShouldAdvertise = bAdvertiseSession;
	OutSessionSettings->bAllowJoinInProgress = true;

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem != nullptr)
	{
		FString CurrentSubsystem = OnlineSubsystem->GetSubsystemName().ToString();
		OutSessionSettings->bIsLANMatch = CurrentSubsystem == FString("NULL");
	}
	else
	{
		OutSessionSettings->bIsLANMatch = true;
	}
	OutSessionSettings->bIsDedicated = false;
	OutSessionSettings->bUsesPresence = true;
	OutSessionSettings->bAllowJoinViaPresence = bAdvertiseSession;
	OutSessionSettings->bUseLobbiesIfAvailable = true;
	OutSessionSettings->bAllowInvites = true;
	OutSessionSettings->Set(Key_IsSecret, InSessionCreateData.IsSecret, EOnlineDataAdvertisementType::ViaOnlineService);
	OutSessionSettings->Set(Key_SessionName, EncodeSessionText(InSessionCreateData.SessionName), EOnlineDataAdvertisementType::ViaOnlineService);
	FString HostPlayerName;
	Execute_GetIdentityPlayerNickName(this, HostPlayerName);
	OutSessionSettings->Set(Key_HostName, EncodeSessionText(HostPlayerName), EOnlineDataAdvertisementType::ViaOnlineService);
	int PlayGameRule = static_cast<int>(InSessionCreateData.PlayGameRule);
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("HostGame : PlayGameRule : %d"), PlayGameRule);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
	}
#endif
	OutSessionSettings->Set(Key_PlayGameRule, PlayGameRule, EOnlineDataAdvertisementType::ViaOnlineService);

	int DollType = static_cast<int>(InSessionCreateData.PlayDollType);
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("HostGame : DollType : %d"), DollType);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
	}
#endif
	OutSessionSettings->Set(Key_DollType, DollType, EOnlineDataAdvertisementType::ViaOnlineService);

	int BoardPreset = static_cast<int>(InSessionCreateData.BoardPreset);
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("HostGame : BoardPreset : %d"), BoardPreset);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
	}
#endif
	OutSessionSettings->Set(Key_BoardPreset, BoardPreset, EOnlineDataAdvertisementType::ViaOnlineService);

	int PlayTurnRule = static_cast<int>(InSessionCreateData.PlayTurnRule);
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("HostGame : PlayTurnRule : %d"), PlayTurnRule);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
	}
#endif
	OutSessionSettings->Set(Key_PlayTurnRule, PlayTurnRule, EOnlineDataAdvertisementType::ViaOnlineService);

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("HostGame : TimeLimit : %f"), InSessionCreateData.TimeLimit);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
	}
#endif
	OutSessionSettings->Set(Key_TimeLimit, InSessionCreateData.TimeLimit, EOnlineDataAdvertisementType::ViaOnlineService);

	OutSessionSettings->Set(Key_AngularThreshold, InSessionCreateData.AngularThreshold, EOnlineDataAdvertisementType::ViaOnlineService);
	OutSessionSettings->Set(Key_DistanceThreshold, InSessionCreateData.DistanceThreshold, EOnlineDataAdvertisementType::ViaOnlineService);
	OutSessionSettings->Set(Key_EnableItem, InSessionCreateData.bEnableItem, EOnlineDataAdvertisementType::ViaOnlineService);
	int MatchRule = static_cast<int>(InSessionCreateData.MatchRule);
	OutSessionSettings->Set(Key_MatchRule, MatchRule, EOnlineDataAdvertisementType::ViaOnlineService);
}

void UGoSoccerGameInstance::Client_SetScreenResolution_Implementation(FIntPoint NewResolution)
{
	auto UserSettings = UGameUserSettings::GetGameUserSettings();
	if (!UserSettings) return;
	UserSettings->SetScreenResolution(NewResolution);
	UserSettings->ApplyResolutionSettings(false);
	UserSettings->SaveSettings();
}

void UGoSoccerGameInstance::Client_SetWindowMode_Implementation(EWindowMode::Type NewWindowMode)
{
	auto UserSettings = UGameUserSettings::GetGameUserSettings();
	if (!UserSettings) return;
	UserSettings->SetFullscreenMode(NewWindowMode);
	UserSettings->ApplyResolutionSettings(false);
	//UserSettings->ApplySettings(false);
	UserSettings->SaveSettings();
}

void UGoSoccerGameInstance::Client_LoadSavedSettings_Implementation()
{
	auto UserSettings = UGameUserSettings::GetGameUserSettings();
	if (!UserSettings) return;
	// 그래픽/해상도/AA 는 UGameUserSettings 자체가 값을 들고 있으므로 LoadSettings() 로 충분하다.
	// (예전에는 여기서 GameSettings 캐시로 복사했지만 GameSettings 가 사라지면서 필요 없어졌다)
	UserSettings->LoadSettings();

//#if UE_EDITOR
	UE_LOG(LogTemp, Log, TEXT("===== Client_Load SavedSettings ====="));
	UE_LOG(LogTemp, Log, TEXT("ScreenResolution : %s"), *UserSettings->GetScreenResolution().ToString());
	UE_LOG(LogTemp, Log, TEXT("Graphic_Shadow : %d"), UserSettings->GetShadowQuality());
	UE_LOG(LogTemp, Log, TEXT("Graphic_GlobalIllumination : %d"), UserSettings->GetGlobalIlluminationQuality());
	UE_LOG(LogTemp, Log, TEXT("Graphic_Reflection : %d"), UserSettings->GetReflectionQuality());
	UE_LOG(LogTemp, Log, TEXT("Graphic_PostProcess : %d"), UserSettings->GetPostProcessingQuality());
	UE_LOG(LogTemp, Log, TEXT("Graphic_Texture : %d"), UserSettings->GetTextureQuality());
	UE_LOG(LogTemp, Log, TEXT("Graphic_Effect : %d"), UserSettings->GetVisualEffectQuality());
	UE_LOG(LogTemp, Log, TEXT("AAType : %d"), UserSettings->GetAntiAliasingQuality());
	UE_LOG(LogTemp, Log, TEXT("===== Client_Load SavedSettings End ====="));
//#endif
}

// ============================================================================
// Steam Invite
//
// [보내기]
//   Steam_ShowInviteOverlay()  : 스팀 오버레이 친구 초대 다이얼로그
//   Steam_InviteFriend(s)()    : 오버레이 없이 SteamID 로 직접 초대
//   Steam_GetInvitableFriends(): 자체 초대 UI 용 친구 목록
//
// [받기]
//   OnSessionUserInviteAccepted()
//     -> (기존 세션이 있으면 DestroySession)
//     -> StartPendingInviteJoin() : 로컬 PC 준비 대기
//     -> ExecutePendingInviteJoin() : JoinSession
//     -> 이후는 기존 OnJoinSessionComplate() 경로를 그대로 탄다.
//
//   오버레이에서 수락한 경우와, 게임이 꺼진 상태에서 초대로 부팅된 경우(+connect_lobby)
//   모두 동일하게 OnSessionUserInviteAccepted 로 들어온다.
// ============================================================================

bool UGoSoccerGameInstance::Init_SteamInvite()
{
	if (!OnlineSessionInterface.IsValid())
	{
		IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
		if (OnlineSubsystem == nullptr) return false;
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
		if (!OnlineSessionInterface.IsValid()) return false;
	}

	if (DelegateHandle_OnSessionUserInviteAccepted.IsValid())
	{
		OnlineSessionInterface->OnSessionUserInviteAcceptedDelegates.Remove(DelegateHandle_OnSessionUserInviteAccepted);
		DelegateHandle_OnSessionUserInviteAccepted.Reset();
	}
	DelegateHandle_OnSessionUserInviteAccepted =
		OnlineSessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &UGoSoccerGameInstance::OnSessionUserInviteAccepted);

	UE_LOG(LogTemp, Log, TEXT("=== Init_SteamInvite : OnSessionUserInviteAccepted Bound ==="));
	return true;
}

void UGoSoccerGameInstance::Shutdown()
{
	Steam_CancelPendingInviteJoin();

	if (OnlineSessionInterface.IsValid())
	{
		if (DelegateHandle_OnSessionUserInviteAccepted.IsValid())
		{
			OnlineSessionInterface->OnSessionUserInviteAcceptedDelegates.Remove(DelegateHandle_OnSessionUserInviteAccepted);
			DelegateHandle_OnSessionUserInviteAccepted.Reset();
		}
		if (DelegateHandle_OnDestroySessionComplete_Invite.IsValid())
		{
			OnlineSessionInterface->OnDestroySessionCompleteDelegates.Remove(DelegateHandle_OnDestroySessionComplete_Invite);
			DelegateHandle_OnDestroySessionComplete_Invite.Reset();
		}
	}

	Super::Shutdown();
}

uint64 UGoSoccerGameInstance::GetCurrentSessionSteamId()
{
	if (!OnlineSessionInterface.IsValid()) return 0;

	FNamedOnlineSession* NamedSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
	if (NamedSession == nullptr) return 0;
	if (!NamedSession->SessionInfo.IsValid() || !NamedSession->SessionInfo->IsValid()) return 0;

	// Steam OSS 는 로비 세션이면 SessionId 에 로비의 CSteamID 를, 광고 세션이면 서버의 CSteamID 를 담는다.
	const FString SessionIdString = NamedSession->SessionInfo->GetSessionId().ToString();
	return FCString::Strtoui64(*SessionIdString, nullptr, 10);
}

bool UGoSoccerGameInstance::Steam_IsInviteAvailable()
{
	if (!Execute_GetSteamOSSOnline(this)) return false;
	if (!OnlineSessionInterface.IsValid()) return false;
	return OnlineSessionInterface->GetNamedSession(NAME_GameSession) != nullptr;
}

FString UGoSoccerGameInstance::Steam_GetCurrentLobbyIdString()
{
	const uint64 SessionSteamId = GetCurrentSessionSteamId();
	if (SessionSteamId == 0) return FString();
	if (!CSteamID(SessionSteamId).IsLobby()) return FString();
	return FString::Printf(TEXT("%llu"), SessionSteamId);
}

bool UGoSoccerGameInstance::Steam_ShowInviteOverlay()
{
	if (!Execute_GetSteamOSSOnline(this))
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Steam Invite : Steam OSS Offline"));
		}
#endif
		return false;
	}

	const uint64 SessionSteamId = GetCurrentSessionSteamId();
	const CSteamID SessionCSteamID(SessionSteamId);
	if (SessionSteamId == 0 || !SessionCSteamID.IsLobby())
	{
		// 오버레이 초대 다이얼로그는 "로비 세션" 에서만 동작한다.
		// 세션이 로비로 만들어지지 않았다면 Steam_InviteFriend() (SendSessionInviteToFriends) 를 사용해야 한다.
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("Steam Invite : Not a Lobby Session (SessionSteamId : %llu)"), SessionSteamId);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, LogString);
		}
#endif
		UE_LOG(LogTemp, Warning, TEXT("Steam_ShowInviteOverlay : Not a Lobby Session (SessionSteamId : %llu)"), SessionSteamId);
		return false;
	}

	if (SteamUtils() != nullptr && !SteamUtils()->IsOverlayEnabled())
	{
		// 오버레이가 아직 로드되지 않았거나 사용자가 꺼둔 경우. 호출은 해보되 경고를 남긴다.
		UE_LOG(LogTemp, Warning, TEXT("Steam_ShowInviteOverlay : Steam Overlay is not enabled"));
	}

	SteamFriends()->ActivateGameOverlayInviteDialog(SessionCSteamID);

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("Steam Invite : Overlay Opened (Lobby : %llu)"), SessionSteamId);
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
	}
#endif
	return true;
}

bool UGoSoccerGameInstance::Steam_InviteFriend_Implementation(int64 FriendSteamID)
{
	TArray<int64> FriendSteamIDs;
	FriendSteamIDs.Emplace(FriendSteamID);
	return Steam_InviteFriends(FriendSteamIDs);
}

bool UGoSoccerGameInstance::Steam_InviteFriends(const TArray<int64>& FriendSteamIDs)
{
	if (FriendSteamIDs.Num() == 0) return false;
	if (!OnlineSessionInterface.IsValid()) return false;

	if (OnlineSessionInterface->GetNamedSession(NAME_GameSession) == nullptr)
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Steam Invite : No Active Session"));
		}
#endif
		return false;
	}

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem == nullptr) return false;
	IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface();
	if (!Identity.IsValid()) return false;

	FUniqueNetIdPtr LocalUserId = Identity->GetUniquePlayerId(0);
	if (!LocalUserId.IsValid()) return false;

	TArray<FUniqueNetIdRef> FriendIds;
	FriendIds.Reserve(FriendSteamIDs.Num());
	for (const int64 RawFriendId : FriendSteamIDs)
	{
		if (RawFriendId <= 0) continue;
		FUniqueNetIdPtr FriendId = Identity->CreateUniquePlayerId(FString::Printf(TEXT("%llu"), static_cast<uint64>(RawFriendId)));
		if (FriendId.IsValid())
		{
			FriendIds.Emplace(FriendId.ToSharedRef());
		}
	}
	if (FriendIds.Num() == 0) return false;

	const bool bResult = OnlineSessionInterface->SendSessionInviteToFriends(*LocalUserId, NAME_GameSession, FriendIds);

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("Steam Invite : Sent to %d friend(s) - %s"),
			FriendIds.Num(), bResult ? TEXT("Success") : TEXT("Failed"));
		GEngine->AddOnScreenDebugMessage(-1, 3.f, bResult ? FColor::Green : FColor::Red, LogString);
	}
#endif
	return bResult;
}

void UGoSoccerGameInstance::Debug_Steam_GetInvitableFriends()
{
	//OutFriends.Reset();
#if !UE_BUILD_SHIPPING
	auto Logging = [&](FString LogString)->void
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
			}
		};
#endif

	if (!Execute_GetSteamOSSOnline(this))
	{
#if !UE_BUILD_SHIPPING
		Logging(FString::Printf(TEXT("InvitableFriends - SteamOSS Offline")));
#endif
		return;
	}

	ISteamFriends* Friends = SteamFriends();
	if (Friends == nullptr)
	{
#if !UE_BUILD_SHIPPING
		Logging(FString::Printf(TEXT("InvitableFriends - SteamFriends Invalid")));
#endif
		return;
	}

	const int32 FriendCount = Friends->GetFriendCount(k_EFriendFlagImmediate);
	if (FriendCount < 0)
	{
#if !UE_BUILD_SHIPPING
		Logging(FString::Printf(TEXT("InvitableFriends - SteamFriends Count Invalid")));
#endif
		return;
	}

	const uint32 ThisAppID = (SteamUtils() != nullptr) ? SteamUtils()->GetAppID() : 0;

#if !UE_BUILD_SHIPPING
	Logging(FString::Printf(TEXT("InvitableFriends Start ==========")));
#endif
	for (int32 Index = 0; Index < FriendCount; ++Index)
	{
		const CSteamID FriendSteamID = Friends->GetFriendByIndex(Index, k_EFriendFlagImmediate);
		if (!FriendSteamID.IsValid()) continue;
		if (Friends->GetFriendPersonaState(FriendSteamID) == k_EPersonaStateOffline) continue;

		FSteamFriendData FriendData;
		FriendData.SteamID = static_cast<int64>(FriendSteamID.ConvertToUint64());
		FriendData.PersonaName = UTF8_TO_TCHAR(Friends->GetFriendPersonaName(FriendSteamID));
		FriendData.bIsOnline = true;
		FriendGameInfo_t FriendGameInfo;
		if (Friends->GetFriendGamePlayed(FriendSteamID, &FriendGameInfo))
		{
			FriendData.bIsPlayingThisGame = (ThisAppID != 0 && FriendGameInfo.m_gameID.AppID() == ThisAppID);
		}
		//OutFriends.Emplace(FriendData);

#if !UE_BUILD_SHIPPING
		Logging(FString::Printf(TEXT("InvitableFriends [%d] %s"), Index, *FriendData.PersonaName));
#endif

	}
#if !UE_BUILD_SHIPPING
	Logging(FString::Printf(TEXT("InvitableFriends End ==========")));
#endif

	//OutFriends.Sort([](const FSteamFriendData& A, const FSteamFriendData& B)
	//	{
	//		if (A.bIsPlayingThisGame != B.bIsPlayingThisGame) return A.bIsPlayingThisGame;
	//		return A.PersonaName.Compare(B.PersonaName, ESearchCase::IgnoreCase) < 0;
	//	});
}

bool UGoSoccerGameInstance::Steam_GetInvitableFriends_Implementation(TArray<FSteamFriendData>& OutFriends)
{
	OutFriends.Reset();

	if (!Execute_GetSteamOSSOnline(this)) return false;

	ISteamFriends* Friends = SteamFriends();
	if (Friends == nullptr) return false;

	const int32 FriendCount = Friends->GetFriendCount(k_EFriendFlagImmediate);
	if (FriendCount < 0) return false;

	const uint32 ThisAppID = (SteamUtils() != nullptr) ? SteamUtils()->GetAppID() : 0;

	UE_LOG(LogTemp, Log, TEXT("InvitableFriends Start =========="));
	for (int32 Index = 0; Index < FriendCount; ++Index)
	{
		const CSteamID FriendSteamID = Friends->GetFriendByIndex(Index, k_EFriendFlagImmediate);
		if (!FriendSteamID.IsValid()) continue;
		// 오프라인 친구는 초대를 받을 수 없다.
		if (Friends->GetFriendPersonaState(FriendSteamID) == k_EPersonaStateOffline) continue;

		FSteamFriendData FriendData;
		FriendData.SteamID = static_cast<int64>(FriendSteamID.ConvertToUint64());
		FriendData.PersonaName = UTF8_TO_TCHAR(Friends->GetFriendPersonaName(FriendSteamID));
		FriendData.bIsOnline = true;
		FriendGameInfo_t FriendGameInfo;
		if (Friends->GetFriendGamePlayed(FriendSteamID, &FriendGameInfo))
		{
			FriendData.bIsPlayingThisGame = (ThisAppID != 0 && FriendGameInfo.m_gameID.AppID() == ThisAppID);
		}
		OutFriends.Emplace(FriendData);

#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Log, TEXT("InvitableFriends [%d] %s"), Index, *FriendData.PersonaName);
#endif

	}
	UE_LOG(LogTemp, Log, TEXT("InvitableFriends End =========="));

	OutFriends.Sort([](const FSteamFriendData& A, const FSteamFriendData& B)
		{
			if (A.bIsPlayingThisGame != B.bIsPlayingThisGame) return A.bIsPlayingThisGame;
			return A.PersonaName.Compare(B.PersonaName, ESearchCase::IgnoreCase) < 0;
		});

	return true;
}

void UGoSoccerGameInstance::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	FString HostName;
	InviteResult.Session.SessionSettings.Get(Key_HostName, HostName);
	HostName = DecodeSessionText(HostName);

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("Steam Invite Accepted : %s - Host : %s"),
			bWasSuccessful ? TEXT("Success") : TEXT("Failed"), *HostName);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, bWasSuccessful ? FColor::Green : FColor::Red, LogString);
	}
#endif
	UE_LOG(LogTemp, Warning, TEXT("=== Steam Invite Accepted : %s / Host : %s ==="),
		bWasSuccessful ? TEXT("Success") : TEXT("Failed"), *HostName);

	if (!bWasSuccessful || !InviteResult.IsValid())
	{
		NotifySessionLoadingScreen(false, ESessionLoadingPhase::ESLP_None);
		Delegate_OnSteamInviteAccepted.Broadcast(false, HostName);
		return;
	}

	PendingInviteControllerId = (ControllerId >= 0) ? ControllerId : 0;
	PendingInviteSearchResult = InviteResult;
	bHasPendingInviteJoin = true;
	PendingInviteElapsedSeconds = 0.f;

	// 초대 수락도 Join 요청의 시작점이다. (실제 JoinSession 은 PC 준비 후 실행된다)
	NotifySessionLoadingScreen(true, ESessionLoadingPhase::ESLP_JoinSession);

	Delegate_OnSteamInviteAccepted.Broadcast(true, HostName);

	// 이미 세션에 들어가 있으면 먼저 정리한 뒤 조인한다.
	if (OnlineSessionInterface.IsValid() && OnlineSessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
	{
		if (DelegateHandle_OnDestroySessionComplete_Invite.IsValid())
		{
			OnlineSessionInterface->OnDestroySessionCompleteDelegates.Remove(DelegateHandle_OnDestroySessionComplete_Invite);
			DelegateHandle_OnDestroySessionComplete_Invite.Reset();
		}
		DelegateHandle_OnDestroySessionComplete_Invite =
			OnlineSessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UGoSoccerGameInstance::OnDestroySessionComplete_Invite);

		UE_LOG(LogTemp, Warning, TEXT("Steam Invite : Destroying Current Session Before Join"));
		OnlineSessionInterface->DestroySession(NAME_GameSession);
		return;
	}

	StartPendingInviteJoin();
}

void UGoSoccerGameInstance::OnDestroySessionComplete_Invite(FName SessionName, bool bWasSuccessful)
{
	if (OnlineSessionInterface.IsValid() && DelegateHandle_OnDestroySessionComplete_Invite.IsValid())
	{
		OnlineSessionInterface->OnDestroySessionCompleteDelegates.Remove(DelegateHandle_OnDestroySessionComplete_Invite);
		DelegateHandle_OnDestroySessionComplete_Invite.Reset();
	}

	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Steam Invite : DestroySession Failed - Join Canceled"));
		bHasPendingInviteJoin = false;
		NotifySessionLoadingScreen(false, ESessionLoadingPhase::ESLP_None);
		Delegate_OnSteamInviteAccepted.Broadcast(false, FString());
		return;
	}

	StartPendingInviteJoin();
}

void UGoSoccerGameInstance::StartPendingInviteJoin()
{
	if (!bHasPendingInviteJoin) return;

	if (TickerHandle_PendingInviteJoin.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle_PendingInviteJoin);
		TickerHandle_PendingInviteJoin.Reset();
	}

	PendingInviteElapsedSeconds = 0.f;
	TickerHandle_PendingInviteJoin = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UGoSoccerGameInstance::Tick_PendingInviteJoin), 0.f);
}

bool UGoSoccerGameInstance::Tick_PendingInviteJoin(float DeltaTime)
{
	if (!bHasPendingInviteJoin)
	{
		TickerHandle_PendingInviteJoin.Reset();
		return false;	// 티커 해제
	}

	PendingInviteElapsedSeconds += DeltaTime;

	// 게임이 꺼진 상태에서 초대로 부팅된 경우 아직 로컬 PlayerController 가 없을 수 있다.
	if (GetFirstLocalPlayerController() == nullptr)
	{
		if (PendingInviteElapsedSeconds > 30.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("Steam Invite : Timed Out Waiting For Local PlayerController"));
			bHasPendingInviteJoin = false;
			TickerHandle_PendingInviteJoin.Reset();
			NotifySessionLoadingScreen(false, ESessionLoadingPhase::ESLP_None);
			Delegate_OnSteamInviteAccepted.Broadcast(false, FString());
			return false;
		}
		return true;
	}

	if (PendingInviteElapsedSeconds < InviteJoinDelaySeconds)
	{
		return true;
	}

	ExecutePendingInviteJoin();
	TickerHandle_PendingInviteJoin.Reset();
	return false;
}

bool UGoSoccerGameInstance::ExecutePendingInviteJoin()
{
	bHasPendingInviteJoin = false;

	if (!OnlineSessionInterface.IsValid() || !PendingInviteSearchResult.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Steam Invite : Invalid Pending Invite - Join Canceled"));
		NotifySessionLoadingScreen(false, ESessionLoadingPhase::ESLP_None);
		Delegate_OnSteamInviteAccepted.Broadcast(false, FString());
		return false;
	}

	Execute_SetPlayerSessionHost(this, false);
	Execute_SetPlayGameMode(this, EPlayGameMode::EPGM_OnlineSession);

	FUniqueNetIdPtr LocalUserId;
	if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface();
		if (Identity.IsValid())
		{
			LocalUserId = Identity->GetUniquePlayerId(PendingInviteControllerId);
		}
	}

	// 조인 성공 시 이후 흐름은 기존 OnJoinSessionComplate() 가 그대로 처리한다.
	const bool bResult = LocalUserId.IsValid()
		? OnlineSessionInterface->JoinSession(*LocalUserId, NAME_GameSession, PendingInviteSearchResult)
		: OnlineSessionInterface->JoinSession(PendingInviteControllerId, NAME_GameSession, PendingInviteSearchResult);

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("Steam Invite : JoinSession Requested - %s"), bResult ? TEXT("Success") : TEXT("Failed"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, bResult ? FColor::Green : FColor::Red, LogString);
	}
#endif

	if (!bResult)
	{
		Execute_SetPlayGameMode(this, EPlayGameMode::EPGM_Default);
		NotifySessionLoadingScreen(false, ESessionLoadingPhase::ESLP_None);
		Delegate_OnSteamInviteAccepted.Broadcast(false, FString());
	}
	return bResult;
}

void UGoSoccerGameInstance::Steam_CancelPendingInviteJoin()
{
	if (bHasPendingInviteJoin)
	{
		NotifySessionLoadingScreen(false, ESessionLoadingPhase::ESLP_None);
	}
	bHasPendingInviteJoin = false;
	if (TickerHandle_PendingInviteJoin.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle_PendingInviteJoin);
		TickerHandle_PendingInviteJoin.Reset();
	}
}

void UGoSoccerGameInstance::Debug_CurrentMainUIState()
{
	if (GEngine)
	{
		UEnum* EnumPtr = StaticEnum<EMainUIState>();
		//MainUIState;
		FString LogString = FString::Printf(TEXT("Current MainUIState %s"), *EnumPtr->GetNameStringByValue((int64)MainUIState));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
	}
}
