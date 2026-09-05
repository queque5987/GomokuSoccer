#include "Widget/MainWidget.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableText.h"
#include "Components/ListView.h"
#include "Components/CheckBox.h"
#include "Components/TileView.h"
#include "GoSoccerPlayManager.h"
#include "Widget/Data/GameSessionData.h"
#include "Widget/CheckButtonWidget.h"
#include "Widget/SettingWidget.h"
#include "Widget/GameConfigSettingWidget.h"
#include "Widget/SelectButtonWidget.h"
#include "Components/Slider.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Widget/Data/PlayerCardData.h"
#include "GameFramework/GameUserSettings.h"


void UMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BP_Button_OpenSinglePlayPanel)		BP_Button_OpenSinglePlayPanel->GetOnReleased().AddDynamic		(this, &UMainWidget::OnOpenSinglePlayPanel);
	if (BP_Button_OpenMultiPlayPanel)		BP_Button_OpenMultiPlayPanel->GetOnReleased().AddDynamic		(this, &UMainWidget::OnOpenMultiPlayPanel);

	if (BP_Button_MultiPlayUI_Back)			BP_Button_MultiPlayUI_Back->GetOnReleased().AddDynamic			(this, &UMainWidget::OnToVeryFirstUI);
	if (BP_Button_SinglePlayUI_Back)		BP_Button_SinglePlayUI_Back->GetOnReleased().AddDynamic			(this, &UMainWidget::OnToVeryFirstUI);

	if (BP_Button_QuickMatch)				BP_Button_QuickMatch->GetOnReleased().AddDynamic				(this, &UMainWidget::OnOpenGameConfigSettingWidget_QuickMatch);
	if (BP_Button_CreateSessionUIOn)		BP_Button_CreateSessionUIOn->GetOnReleased().AddDynamic			(this, &UMainWidget::OnOpenGameConfigSettingWidget_CreateSession);
	if (BP_Button_AIPlayUIOn)				BP_Button_AIPlayUIOn->GetOnReleased().AddDynamic				(this, &UMainWidget::OnOpenGameConfigSettingWidget_AI);
	if (BP_Button_HotSeatPlayUIOn)			BP_Button_HotSeatPlayUIOn->GetOnReleased().AddDynamic			(this, &UMainWidget::OnOpenGameConfigSettingWidget_HotSeat);

	if (BP_GameConfigSetting)
	{
		if (BP_GameConfigSetting->GetButtonCancle())	BP_GameConfigSetting->GetButtonCancle()->GetOnReleased().AddDynamic(this, &UMainWidget::OnGameConfigSettingCanceled);
		if (BP_GameConfigSetting->GetButtonComplete())	BP_GameConfigSetting->GetButtonComplete()->GetOnReleased().AddDynamic(this, &UMainWidget::OnGameConfigSettingCompleted);
	}

	if (BP_LobbyGameConfigSetting)
	{
		if (BP_LobbyGameConfigSetting->GetButtonCancle()) BP_LobbyGameConfigSetting->GetButtonCancle()->GetOnReleased().AddDynamic(this, &UMainWidget::OnLobbyConfigSettingCanceled);
		if (BP_LobbyGameConfigSetting->GetButtonComplete()) BP_LobbyGameConfigSetting->GetButtonComplete()->GetOnReleased().AddDynamic(this, &UMainWidget::OnLobbyConfigSettingCompleted);
	}

	if (BP_DollColor_Select)
	{
		if (BP_DollColor_Select->GetValueChangedDelegate()) BP_DollColor_Select->GetValueChangedDelegate()->BindUFunction(this, TEXT("OnDollColorChanged"));
	}

	//if (BP_Button_CreateSessionUIOn)		BP_Button_CreateSessionUIOn->GetOnReleased().AddDynamic			(this, &UMainWidget::OnCreateSessionUIOn);
	if (BP_Button_SessionUIOn)				BP_Button_SessionUIOn->GetOnReleased().AddDynamic				(this, &UMainWidget::OnJoinGame);
	if (BP_Button_Debug_CustomButton)		BP_Button_Debug_CustomButton->GetOnReleased().AddDynamic		(this, &UMainWidget::OnDebug);
	if (BP_Button_Start)					BP_Button_Start->GetOnReleased().AddDynamic						(this, &UMainWidget::OnStartGame);
	if (BP_Button_Ready)					BP_Button_Ready->GetOnReleased().AddDynamic						(this, &UMainWidget::OnReadyGame);
	if (BP_Button_Invite)					BP_Button_Invite->GetOnReleased().AddDynamic					(this, &UMainWidget::OnInviteUIOn);
	if (BP_Button_SendRSVP)					BP_Button_SendRSVP->GetOnReleased().AddDynamic					(this, &UMainWidget::OnSendRSVP);
	if (BP_Button_ToggleSetting)			BP_Button_ToggleSetting->GetOnReleased().AddDynamic				(this, &UMainWidget::OnLobbyToggleSetting);
	if (Button_OpenLocalHost)				Button_OpenLocalHost->OnReleased.AddDynamic						(this, &UMainWidget::OnOpenLocalHost);
	if (Button_JoinLocalHost)				Button_JoinLocalHost->OnReleased.AddDynamic						(this, &UMainWidget::OnJoinLocalHost);
	//if (BP_Button_CreateSession)			BP_Button_CreateSession->GetOnReleased().AddDynamic				(this, &UMainWidget::OnHostGame);
	if (BP_Button_JoinSelectedSession)		BP_Button_JoinSelectedSession->GetOnReleased().AddDynamic		(this, &UMainWidget::OnJoinSelectedSession);
	if (BP_Button_Settings)					BP_Button_Settings->GetOnReleased().AddDynamic					(this, &UMainWidget::OnOpenSettingsPanel);
	if (BP_Button_Out_CreateSessionUIPanel)	BP_Button_Out_CreateSessionUIPanel->GetOnReleased().AddDynamic	(this, &UMainWidget::OnToVeryFirstUI);
	if (BP_Button_Out_SessionUIPanel)		BP_Button_Out_SessionUIPanel->GetOnReleased().AddDynamic		(this, &UMainWidget::OnToVeryFirstUI);
	if (BP_Button_HotSeatPlay)				BP_Button_HotSeatPlay->GetOnReleased().AddDynamic				(this, &UMainWidget::OnHotSeatPlay);
	if (BP_Button_HotSeatPlay_5plus5)		BP_Button_HotSeatPlay_5plus5->GetOnReleased().AddDynamic		(this, &UMainWidget::OnHotSeatPlay_5plus5);
	if (BP_Button_RefreshOnlineSession)		BP_Button_RefreshOnlineSession->GetOnReleased().AddDynamic		(this, &UMainWidget::OnRefreshOnlineSession);
	if (BP_Button_Out_LobbyUIPanel)			BP_Button_Out_LobbyUIPanel->GetOnReleased().AddDynamic			(this, &UMainWidget::OnOutSession);
	if (BP_Button_EndProcess)				BP_Button_EndProcess->GetOnReleased().AddDynamic				(this, &UMainWidget::OnEndProcess);

	if (BP_Button_GameEnd_OutSession)		BP_Button_GameEnd_OutSession->GetOnReleased().AddDynamic		(this, &UMainWidget::OnOutSession);
	if (BP_Button_GameEnd_Regame)			BP_Button_GameEnd_Regame->GetOnReleased().AddDynamic			(this, &UMainWidget::OnGameEnd_Regame);

	if (Settings_Widget)
	{
		if (Settings_Widget->GetButton_SaveOut())			Settings_Widget->GetButton_SaveOut()->GetOnReleased().AddDynamic				(this, &UMainWidget::CloseSettingsWidget);
		if (Settings_Widget->GetSlider_Shadow())			Settings_Widget->GetSlider_Shadow()->OnValueChanged.AddDynamic					(this, &UMainWidget::OnSlider_ShadowChanged);
		if (Settings_Widget->GetBP_Button_GGQuitSession())	Settings_Widget->GetBP_Button_GGQuitSession()->GetOnReleased().AddDynamic		(this, &UMainWidget::OnOutSession);
		if (Settings_Widget->GetBP_Button_GG())				Settings_Widget->GetBP_Button_GG()->GetOnReleased().AddDynamic					(this, &UMainWidget::OnGG);

		if (Settings_Widget->GetSlider_Volume_BGM_ValueChangedDelegate())	Settings_Widget->GetSlider_Volume_BGM_ValueChangedDelegate()->BindUFunction(this, TEXT("OnSlider_BGMVolumeChanged"));
		if (Settings_Widget->GetSlider_Volume_SFX_ValueChangedDelegate())	Settings_Widget->GetSlider_Volume_SFX_ValueChangedDelegate()->BindUFunction(this, TEXT("OnSlider_FXVolumeChanged"));
		if (Settings_Widget->GetSlider_Volume_Master_ValueChangedDelegate())	Settings_Widget->GetSlider_Volume_Master_ValueChangedDelegate()->BindUFunction(this, TEXT("OnSlider_MasterVolumeChanged"));

		//if (Settings_Widget->GetSlider_Graphic_Total_ValueChangedDelegate())				Settings_Widget->GetSlider_Graphic_Total_ValueChangedDelegate()->BindUFunction				(this, TEXT("OnSlider_Graphic_Total_Changed"));
		if (Settings_Widget->GetSlider_Graphic_Shadow_ValueChangedDelegate())				Settings_Widget->GetSlider_Graphic_Shadow_ValueChangedDelegate()->BindUFunction				(this, TEXT("OnSlider_Graphic_Shadow_Changed"));
		if (Settings_Widget->GetSlider_Graphic_GlobalIllumination_ValueChangedDelegate())	Settings_Widget->GetSlider_Graphic_GlobalIllumination_ValueChangedDelegate()->BindUFunction	(this, TEXT("OnSlider_Graphic_GlobalIllumination_Changed"));
		if (Settings_Widget->GetSlider_Graphic_Reflection_ValueChangedDelegate())			Settings_Widget->GetSlider_Graphic_Reflection_ValueChangedDelegate()->BindUFunction			(this, TEXT("OnSlider_Graphic_Reflection_Changed"));
		if (Settings_Widget->GetSlider_Graphic_PostProcess_ValueChangedDelegate())			Settings_Widget->GetSlider_Graphic_PostProcess_ValueChangedDelegate()->BindUFunction		(this, TEXT("OnSlider_Graphic_PostProcess_Changed"));
		if (Settings_Widget->GetSlider_Graphic_Texture_ValueChangedDelegate())				Settings_Widget->GetSlider_Graphic_Texture_ValueChangedDelegate()->BindUFunction			(this, TEXT("OnSlider_Graphic_Texture_Changed"));
		if (Settings_Widget->GetSlider_Graphic_Effect_ValueChangedDelegate())				Settings_Widget->GetSlider_Graphic_Effect_ValueChangedDelegate()->BindUFunction				(this, TEXT("OnSlider_Graphic_Effect_Changed"));

	}

	if (BP_Radio_Mode0)
	{
		if (BP_Radio_Mode0->GetButton_BG() && BP_Radio_Mode0->GetButton_BG().Get() != nullptr)	BP_Radio_Mode0->GetButton_BG().Get()->OnReleased.AddDynamic(this, &UMainWidget::OnSelectMode0);
		Radio_Mode0 = BP_Radio_Mode0->GetCheckBox();
	}

	if (BP_Radio_Mode1)
	{
		if (BP_Radio_Mode1->GetButton_BG() && BP_Radio_Mode1->GetButton_BG().Get() != nullptr)	BP_Radio_Mode1->GetButton_BG().Get()->OnReleased.AddDynamic(this, &UMainWidget::OnSelectMode1);
		Radio_Mode1 = BP_Radio_Mode1->GetCheckBox();
	}

	//if (EditableText_PlayerName)	EditableText_PlayerName->OnTextCommitted.AddDynamic	(this, &UMainWidget::OnPlayerNameCommitted);
	if (EditableText_SessionName)	EditableText_SessionName->OnTextCommitted.AddDynamic(this, &UMainWidget::OnSessionNameCommitted);

	//if (ComboBox_DollColor)			ComboBox_DollColor->OnSelectionChanged.AddDynamic	(this, &UMainWidget::OnDollColorChanged);
}

void UMainWidget::OnGameConfigSettingCompleted()
{
	if (!BP_GameConfigSetting) return;
	APlayerController* PC = GetOwningPlayer();

	FSessionCreateData SessionCreateData = BP_GameConfigSetting->GetSessionCreateData();
	if (SessionCreateData.PlayGameMode == EPlayGameMode::EPGM_HotSeatGame)
	{
		//HotSeatSelectedMode = SessionCreateData.PlayGameRule;
		UGoSoccerPlayManager::StartHotSeatGame(PC, SessionCreateData);
		UGoSoccerPlayManager::SetPlayGameMode(GetGameInstance(), EPlayGameMode::EPGM_HotSeatGame);
		UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_GameStarted);
	}
	else if (SessionCreateData.PlayGameMode == EPlayGameMode::EPGM_OnlineSession)
	{
		//UGoSoccerPlayManager::SetHostingSessionCreateData()
		EMainUIState PrevMainUIState;
		UGoSoccerPlayManager::GetMainUIState(GetGameInstance(), PrevMainUIState);
		UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_Loading);
		//if (UGoSoccerPlayManager::SetHostingSessionCreateData(GetGameInstance(), SessionCreateData))
		//{
		if (!UGoSoccerPlayManager::HostGame(PC, PC->GetLocalPlayer()->GetPreferredUniqueNetId(), SessionCreateData))
		{
			UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, PrevMainUIState);
		}
		//}
	}
	else if (SessionCreateData.PlayGameMode == EPlayGameMode::EPGM_SingleAIPlay)
	{
		//int32 PlayAs = SessionCreateData.PlayAsDollColor;
		UGoSoccerPlayManager::StartSingleAIGame(PC, SessionCreateData);
		UGoSoccerPlayManager::SetPlayGameMode(GetGameInstance(), EPlayGameMode::EPGM_SingleAIPlay);
		UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_GameStarted);
	}
	//UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_GameStarted);
}

void UMainWidget::OnGameConfigSettingCanceled()
{
	if (BP_GameConfigSetting)
	{
		EPlayGameMode SavedGameMode = BP_GameConfigSetting->GetPlayGameMode();
		if (SavedGameMode == EPlayGameMode::EPGM_HotSeatGame || SavedGameMode == EPlayGameMode::EPGM_SingleAIPlay)
		{
			OnOpenSinglePlayPanel();
			return;
		}
		else if (SavedGameMode == EPlayGameMode::EPGM_OnlineSession)
		{
			OnOpenMultiPlayPanel();
			return;
		}
	}
	OnToVeryFirstUI();
}

void UMainWidget::OnLobbyConfigSettingCompleted_Implementation()
{
	if (BP_LobbyGameConfigSetting == nullptr) return;
	
	FSessionCreateData SessionCreateData = BP_LobbyGameConfigSetting->GetSessionCreateData();
	UGoSoccerPlayManager::SetSessionCreateData_ToGameInstance(GetGameInstance(), SessionCreateData);
	// 확정된 설정을 서버(GameMode)에도 남겨서 접속 중인 플레이어와 이후 조인자가 같은 툴팁을 보게 한다.
	UGoSoccerPlayManager::Request_UpdateTooltip_SessionConfig_ToAll(GetOwningPlayer(), SessionCreateData);
}

void UMainWidget::OnLobbyConfigSettingCanceled_Implementation()
{
	if (BP_LobbyGameConfigSetting == nullptr) return;

	// 설정을 만지는 동안 모든 플레이어의 툴팁이 미리 바뀌어 있으므로,
	// 취소하면 실제 세션 설정으로 되돌려서 다시 뿌린다. (ApplySessionCreateData 가 툴팁 갱신까지 태운다)
	FSessionCreateData CurrentSessionCreateData;
	if (UGoSoccerPlayManager::GetSessionCreateData_FromSessionInterface(GetGameInstance(), CurrentSessionCreateData))
	{
		BP_LobbyGameConfigSetting->ApplySessionCreateData(CurrentSessionCreateData);
		UGoSoccerPlayManager::Request_UpdateTooltip_SessionConfig_ToAll(GetOwningPlayer(), CurrentSessionCreateData);
	}
}

void UMainWidget::OnLobbyToggleSetting_Implementation()
{
	FSessionCreateData CurrentSessionCreateData;
	if (UGoSoccerPlayManager::GetSessionCreateData_FromSessionInterface(GetGameInstance(), CurrentSessionCreateData))
	{
		BP_LobbyGameConfigSetting->ApplySessionCreateData(CurrentSessionCreateData);
	}
	//TODO Load SessionCreate Data From GameInstance - BP -> Override Toggle Config UI
}

void UMainWidget::OnOpenSinglePlayPanel()
{
	APlayerController* PC = GetOwningPlayer();
	UGoSoccerPlayManager::SetMainUIState(
		GetGameInstance(), PC, EMainUIState::EMUIS_SinglePlay
	);
}

void UMainWidget::OnOpenMultiPlayPanel()
{
	APlayerController* PC = GetOwningPlayer();
	UGoSoccerPlayManager::SetMainUIState(
		GetGameInstance(), PC, EMainUIState::EMUIS_MultiPlay
	);
}

void UMainWidget::OnOpenGameConfigSettingWidget_QuickMatch()
{
	APlayerController* PC = GetOwningPlayer();
	if (BP_GameConfigSetting)
	{
		BP_GameConfigSetting->SetPlayGameMode(EPlayGameMode::EPGM_Default);
			UGoSoccerPlayManager::SetMainUIState(
				GetGameInstance(), PC, EMainUIState::EMUIS_GameConfigSetting
			);
	}
}

void UMainWidget::OnOpenGameConfigSettingWidget_CreateSession()
{
	APlayerController* PC = GetOwningPlayer();
	if (BP_GameConfigSetting)
	{
		BP_GameConfigSetting->SetPlayGameMode(EPlayGameMode::EPGM_OnlineSession);
		UGoSoccerPlayManager::SetMainUIState(
			GetGameInstance(), PC, EMainUIState::EMUIS_GameConfigSetting
		);
	}
}

void UMainWidget::OnOpenGameConfigSettingWidget_AI()
{
	APlayerController* PC = GetOwningPlayer();
	if (BP_GameConfigSetting)
	{
		BP_GameConfigSetting->SetPlayGameMode(EPlayGameMode::EPGM_SingleAIPlay);
		UGoSoccerPlayManager::SetMainUIState(
			GetGameInstance(), PC, EMainUIState::EMUIS_GameConfigSetting
		);
	}
}

void UMainWidget::OnOpenGameConfigSettingWidget_HotSeat()
{
	APlayerController* PC = GetOwningPlayer();
	if (BP_GameConfigSetting)
	{
		BP_GameConfigSetting->SetPlayGameMode(EPlayGameMode::EPGM_HotSeatGame);
		UGoSoccerPlayManager::SetMainUIState(
			GetGameInstance(), PC, EMainUIState::EMUIS_GameConfigSetting
		);
	}
}

void UMainWidget::OnDebug()
{
#if !UE_BUILD_SHIPPING
	if (GetWorld() != nullptr)
	{
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("Refreshing Players : %d"), GetWorld()->GetNumPlayerControllers());
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, LogString);
		}
	}
#endif
	TArray<FIntPoint> SupportedFullscreenResolutios;
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(SupportedFullscreenResolutios);
	int idx = 0;
	for (auto Iter : SupportedFullscreenResolutios)
	{
		UE_LOG(LogTemp, Log, TEXT("Resolution[%d] : %d, %d"), idx++, Iter.X, Iter.Y);
	}
}

void UMainWidget::OnOpenLocalHost()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC == nullptr)
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("OnOpenLocalHost: %s Nullptr"), *PC->GetFName().ToString());
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, LogString);
		}
#endif
	}
	if (UGoSoccerPlayManager::OpenLocalHost(PC))
	{
		UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_InLobby);
		UGoSoccerPlayManager::SetPlayGameMode(GetGameInstance(), EPlayGameMode::EPGM_Debug);
	}
}

void UMainWidget::OnJoinLocalHost()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC == nullptr)
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("OnJoinLocalHost: %s Nullptr"), *PC->GetFName().ToString());
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, LogString);
		}
#endif
	}
	if (UGoSoccerPlayManager::JoinLocalHost(PC))
	{
		UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_InLobby);
		UGoSoccerPlayManager::SetPlayGameMode(GetGameInstance(), EPlayGameMode::EPGM_OnlineSession);
	}
}

void UMainWidget::OnHostGame()
{
//	APlayerController* PC = GetOwningPlayer();
//	if (PC == nullptr)
//	{
//#if !UE_BUILD_SHIPPING
//		if (GEngine)
//		{
//			FString LogString = FString::Printf(TEXT("OnHostGame: %s Nullptr"), *PC->GetFName().ToString());
//			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, LogString);
//		}
//#endif
//	}
//	EMainUIState PrevMainUIState;
//	UGoSoccerPlayManager::GetMainUIState(GetGameInstance(), PrevMainUIState);
//	UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_Loading);
//	if (!UGoSoccerPlayManager::HostGame(PC, PC->GetLocalPlayer()->GetPreferredUniqueNetId()))
//	{
//		UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, PrevMainUIState);
//	}
}

void UMainWidget::OnHostGameCompleted()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_InLobby);
		UGoSoccerPlayManager::SetPlayGameMode(GetGameInstance(), EPlayGameMode::EPGM_OnlineSession);
	}
}

void UMainWidget::OnJoinGame()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC == nullptr)
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("OnJoinGame: %s Nullptr"), *PC->GetFName().ToString());
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, LogString);
		}
#endif
	}

	if (UGoSoccerPlayManager::RefreshOnlineSessions(PC))
	{
		UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_SessionSearch);
	}
}

void UMainWidget::OnRefreshOnlineSession()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		UGoSoccerPlayManager::RefreshOnlineSessions(PC);
	}
}

void UMainWidget::OnStartGame()
{
	APlayerController* PC = GetOwningPlayer();

	if (SetButtonDisableTimer(0.25f))
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("OnStartGame : Call Start Game"));
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, LogString);
		}
#endif
		//EMainUIState PrevMainUIState;
		//UGoSoccerPlayManager::GetMainUIState(GetGameInstance(), PrevMainUIState);
		//UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_Loading);
		EPlayGameMode tempPlayGAmeMode;
		if (UGoSoccerPlayManager::GetPlayGameMode(GetGameInstance(), tempPlayGAmeMode))
		{
			if (tempPlayGAmeMode == EPlayGameMode::EPGM_Debug)
			{
				UGoSoccerPlayManager::StartLocalHostGame(GetWorld()->GetAuthGameMode(), BP_LobbyGameConfigSetting->GetSessionCreateData());
				return;
			}
		}
		if (!UGoSoccerPlayManager::StartGame(PC))
		{
			//UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, PrevMainUIState);
			// Move To Delegate -- Client Always Return True
			//UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_GameStarted);
		}
	}
}

void UMainWidget::OnInviteUIOn_Implementation()
{
	TArray<FSteamFriendData> tempFriends;
	UGoSoccerPlayManager::Steam_GetInvitableFriends(GetGameInstance(), tempFriends);

	TileView_Invitables->ClearListItems();
	int32 idx = 0;
	for (const FSteamFriendData& IterFriends : tempFriends)
	{
		UPlayerCardData* tempCardData = NewObject<UPlayerCardData>();
		if (tempCardData == nullptr)
		{
#if !UE_BUILD_SHIPPING
			UE_LOG(LogTemp, Log, TEXT("On Invite UI On : Unable To Create Card Object"));
#endif
			continue;
		}
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("On Invite UI On : Add Object To List - [%d] %s"), idx, *IterFriends.PersonaName);
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, LogString);
		}
		UE_LOG(LogTemp, Log, TEXT("On Invite UI On : Add Object To List - [%d] %s"), idx, *IterFriends.PersonaName);
#endif
		tempCardData->SetSteamFriendData(IterFriends);
		if (tempCardData->GetOnWidgetSelected()) tempCardData->GetOnWidgetSelected()->BindUFunction(this, TEXT("OnInviteItemSelectionChanged"));
		TileView_Invitables->AddItem(tempCardData);
		idx++;
	}
	//TileView_Invitables
}

void UMainWidget::OnInviteItemSelectionChanged(UObject* SelectedWidget, UObject* SelectedItem)
{
	if (TileView_Invitables == nullptr)
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("OnInviteItemSelectionChanged : SelectedItem Invalid"));
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, LogString);
		}
#endif
		return;
	}
	//const TArray<UObject*> Invitables = TileView_Invitables->GetListItems();
	
	for (auto* Iter : TileView_Invitables->GetDisplayedEntryWidgets())
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("OnInviteItemSelectionChanged : SelectedItem %s : %s"),
				*Iter->GetFName().ToString(),
				SelectedWidget == Iter ? TEXT("True") : TEXT("False")
				);
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, LogString);
		}
#endif
		UGoSoccerPlayManager::ChangeWidgetSelected(Iter, SelectedWidget == Iter);
	}

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("OnInviteItemSelectionChanged End"));
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, LogString);
	}
#endif

	TileView_Invitables->SetSelectedItem(SelectedItem);
}

void UMainWidget::OnSendRSVP()
{
	if (TileView_Invitables == nullptr) 
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("OnSendRSVP : TileView Invalid"));
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, LogString);
		}
#endif
		return;
	}
	UPlayerCardData* tempPlayerCardData = TileView_Invitables->GetSelectedItem<UPlayerCardData>();
	if (tempPlayerCardData == nullptr) 
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("OnSendRSVP : Selected Item Not Selected or Invalid"));
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, LogString);
		}
#endif
		return;
	}
	FSteamFriendData tempData;
	tempPlayerCardData->GetSteamFriendData(tempData);

	UE_LOG(LogTemp, Log, TEXT("Send RSVP To : %s"), *tempData.PersonaName);
	bool bSendFlag = UGoSoccerPlayManager::Steam_InviteFriend(GetGameInstance(), tempData.SteamID);
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("Send RSVP To [%s] %s"), *tempData.PersonaName, bSendFlag ? TEXT("Succeeded") : TEXT("Failed"));
		GEngine->AddOnScreenDebugMessage(-1, 2.f, bSendFlag ? FColor::Green : FColor::Red, LogString);
	}
#endif
}

void UMainWidget::OnReadyGame()
{
	APlayerController* PC = GetOwningPlayer();
	if (SetButtonDisableTimer(0.25f))
	{
		UGoSoccerPlayManager::ReadyGame(PC);
	}
}

void UMainWidget::OnCreateSessionUIOn()
{
	APlayerController* PC = GetOwningPlayer();
	UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_SessionCreate);
}

void UMainWidget::OnJoinSelectedSession()
{
	APlayerController* PC = GetOwningPlayer();
	if (ListView_SessionList == nullptr) return;
	UGameSessionData* SelectedSessionData = ListView_SessionList->GetSelectedItem<UGameSessionData>();
	if (IsValid(SelectedSessionData))
	{
		FString SessionName;
		SelectedSessionData->GetSessionName(SessionName);
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("Join Selected Session Name : %s"), *SessionName);
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, LogString);
		}
#endif
		EMainUIState PrevMainUIState;
		UGoSoccerPlayManager::GetMainUIState(GetGameInstance(), PrevMainUIState);
		//UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_Loading);
		if (!UGoSoccerPlayManager::JoinSelectedSession(PC, SelectedSessionData, 0))
		{
			UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, PrevMainUIState);
			//UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_InLobby);
			//UGoSoccerPlayManager::SetPlayGameMode(GetGameInstance(), EPlayGameMode::EPGM_OnlineSession);
		}
	}
}

void UMainWidget::OnJoinSelectedSessionCompleted()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_InLobby);
		UGoSoccerPlayManager::SetPlayGameMode(GetGameInstance(), EPlayGameMode::EPGM_OnlineSession);
	}
}

void UMainWidget::OnToVeryFirstUI()
{
	APlayerController* PC = GetOwningPlayer();
	UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_VeryFirst);
	UGoSoccerPlayManager::SetPlayGameMode(GetGameInstance(), EPlayGameMode::EPGM_Default);
}

void UMainWidget::CloseSettingsWidget()
{
	APlayerController* PC = GetOwningPlayer();
	EMainUIState CurrentMainUIState;
	if (!UGoSoccerPlayManager::GetMainUIState(GetGameInstance(), CurrentMainUIState)) return;
	if (CurrentMainUIState == EMainUIState::EMUIS_GameStarted)
	{
		// In Game Call
		UGoSoccerPlayManager::CloseInGameSettingsWidget(PC);
		//UGoSoccerPlayManager::SetMainUIState(nullptr, PC, EMainUIState::EMUIS_GameStarted);
	}
	else
	{
		OnToVeryFirstUI();
	}
}

void UMainWidget::OnOpenSettingsPanel()
{
	APlayerController* PC = GetOwningPlayer();
	SetSettingWidget_InGameManuPanelVisibility(false);
	UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_Settings);
}

void UMainWidget::OnHotSeatPlay()
{
	//APlayerController* PC = GetOwningPlayer();
	//UGoSoccerPlayManager::StartHotSeatGame(PC, EPlayGameRule::EPGR_5or5Mode);
	//HotSeatSelectedMode = EPlayGameRule::EPGR_5or5Mode;
	//UGoSoccerPlayManager::SetPlayGameMode(GetGameInstance(), EPlayGameMode::EPGM_HotSeatGame);
	//UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_GameStarted);
}

void UMainWidget::OnHotSeatPlay_5plus5()
{
	//APlayerController* PC = GetOwningPlayer();
	//UGoSoccerPlayManager::StartHotSeatGame(PC, EPlayGameRule::EPGR_5plus5Mode);
	//HotSeatSelectedMode = EPlayGameRule::EPGR_5plus5Mode;
	//UGoSoccerPlayManager::SetPlayGameMode(GetGameInstance(), EPlayGameMode::EPGM_HotSeatGame);
	//UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_GameStarted);
}

void UMainWidget::OnGameEnd_Regame()
{
	APlayerController* PC = GetOwningPlayer();
	EPlayGameMode CurrPlayGameMode;
	UGoSoccerPlayManager::GetPlayGameMode(GetGameInstance(), CurrPlayGameMode);
	if (CurrPlayGameMode == EPlayGameMode::EPGM_HotSeatGame)
	{
		OnOpenGameConfigSettingWidget_HotSeat();
		//if (HotSeatSelectedMode == EPlayGameRule::EPGR_5plus5Mode)
		//{
		//	OnHotSeatPlay_5plus5();
		//}
		//else 
		//{
		//	OnHotSeatPlay();
		//}
	}
	else if (CurrPlayGameMode == EPlayGameMode::EPGM_SingleAIPlay)
	{
		OnOpenGameConfigSettingWidget_AI();
	}
	else 
	{
		UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_InLobby);
	}
}

void UMainWidget::OnOutSession()
{
	APlayerController* PC = GetOwningPlayer();
	if (UGoSoccerPlayManager::LeaveGame(GetGameInstance()))
	{
		UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, EMainUIState::EMUIS_VeryFirst);
	}
}

void UMainWidget::OnEndProcess()
{
	APlayerController* PC = GetOwningPlayer();
	UGoSoccerPlayManager::EndProcess(PC);
}

void UMainWidget::OnGG()
{
	APlayerController* PC = GetOwningPlayer();
	UGoSoccerPlayManager::PlayerGiveUp(PC);
}

void UMainWidget::OnSelectMode0()
{
	if (!Radio_Mode0 && BP_Radio_Mode0)
	{
		Radio_Mode0 = BP_Radio_Mode0->GetCheckBox();
	}
	if (!Radio_Mode1 && BP_Radio_Mode1)
	{
		Radio_Mode1 = BP_Radio_Mode1->GetCheckBox();
	}
	if (Radio_Mode0) Radio_Mode0->SetIsChecked(true);
	if (Radio_Mode1) Radio_Mode1->SetIsChecked(false);
}

void UMainWidget::OnSelectMode1()
{
	if (!Radio_Mode0 && BP_Radio_Mode0)
	{
		Radio_Mode0 = BP_Radio_Mode0->GetCheckBox();
	}
	if (!Radio_Mode1 && BP_Radio_Mode1)
	{
		Radio_Mode1 = BP_Radio_Mode1->GetCheckBox();
	}
	if (Radio_Mode0) Radio_Mode0->SetIsChecked(false);
	if (Radio_Mode1) Radio_Mode1->SetIsChecked(true);
}

//void UMainWidget::OnPlayerNameCommitted(const FText& Text, ETextCommit::Type CommitMethod)
//{
//#if !UE_BUILD_SHIPPING
//	if (GEngine)
//	{
//		FString LogString = FString::Printf(TEXT("OnPlayerNameCommitted : %s"), *Text.ToString());
//		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, LogString);
//	}
//#endif
//	FName NewPlayerName = FName(Text.ToString());
//	UGoSoccerPlayManager::ChangePlayerName(GetOwningPlayer(), NewPlayerName);
//}

void UMainWidget::OnSessionNameCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("OnSessionNameCommitted : %s"), *Text.ToString());
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, LogString);
	}
#endif
	FName NewSessionName = FName(Text.ToString());
	UGoSoccerPlayManager::ChangeSessionName(GetOwningPlayer(), NewSessionName);
}

void UMainWidget::OnDollColorChanged(float NewValue)
{
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("OnDollColorChanged : %f"), NewValue);
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, LogString);
	}
#endif
	//UGoSoccerPlayManager::ChangePlayerDollColor(GetOwningPlayer(), SelectedItem == TEXT("Black") ? 0 : 1);
	UGoSoccerPlayManager::ChangePlayerDollColor(GetOwningPlayer(), static_cast<uint8>(NewValue));
}

void UMainWidget::OnCheckBox_UseDollVFX(bool bIsChecked)
{
	//UGoSoccerPlayManager::ChangeSettings_bUseLowTexture(GetGameInstance(), bIsChecked);
}

void UMainWidget::OnSlider_ShadowChanged(float NewLevel)
{
	UGoSoccerPlayManager::ChangeSettings_fShadowLevel(GetGameInstance(), NewLevel);
}

void UMainWidget::OnSlider_BGMVolumeChanged(float NewLevel)
{
	UGoSoccerPlayManager::ChangeSettings_Float(GetGameInstance(), ESettingConfigParamType::ESCPT_BGMVolume, NewLevel / 100.f);
}

void UMainWidget::OnSlider_FXVolumeChanged(float NewLevel)
{
	UGoSoccerPlayManager::ChangeSettings_Float(GetGameInstance(), ESettingConfigParamType::ESCPT_FXVolume, NewLevel / 100.f);
}

void UMainWidget::OnSlider_MasterVolumeChanged(float NewLevel)
{
	UGoSoccerPlayManager::ChangeSettings_Float(GetGameInstance(), ESettingConfigParamType::ESCPT_MasterVolume, NewLevel / 100.f);
}

void UMainWidget::OnSlider_Graphic_Total_Changed(float NewLevel)
{
	//UGoSoccerPlayManager::ChangeSettings_Float(GetGameInstance(), ESettingConfigParamType::ESCPT_Graphic_Total, NewLevel);
}

void UMainWidget::OnSlider_Graphic_Shadow_Changed(float NewLevel)
{
	UGoSoccerPlayManager::ChangeSettings_Float(GetGameInstance(), ESettingConfigParamType::ESCPT_Graphic_Shadow, NewLevel);
}

void UMainWidget::OnSlider_Graphic_GlobalIllumination_Changed(float NewLevel)
{
	UGoSoccerPlayManager::ChangeSettings_Float(GetGameInstance(), ESettingConfigParamType::ESCPT_Graphic_GlobalIllumination, NewLevel);
}

void UMainWidget::OnSlider_Graphic_Reflection_Changed(float NewLevel)
{
	UGoSoccerPlayManager::ChangeSettings_Float(GetGameInstance(), ESettingConfigParamType::ESCPT_Graphic_Reflection, NewLevel);
}

void UMainWidget::OnSlider_Graphic_PostProcess_Changed(float NewLevel)
{
	UGoSoccerPlayManager::ChangeSettings_Float(GetGameInstance(), ESettingConfigParamType::ESCPT_Graphic_PostProcess, NewLevel);
}

void UMainWidget::OnSlider_Graphic_Texture_Changed(float NewLevel)
{
	UGoSoccerPlayManager::ChangeSettings_Float(GetGameInstance(), ESettingConfigParamType::ESCPT_Graphic_Texture, NewLevel);
}

void UMainWidget::OnSlider_Graphic_Effect_Changed(float NewLevel)
{
	UGoSoccerPlayManager::ChangeSettings_Float(GetGameInstance(), ESettingConfigParamType::ESCPT_Graphic_Effect, NewLevel);
}

void UMainWidget::Debug_AddSessionName(FString InSessionName)
{
	if (ListView_SessionList == nullptr) return;
	UGameSessionData* GameSessionData = NewObject<UGameSessionData>(GetOwningLocalPlayer());
	if (GameSessionData == nullptr) return;
	GameSessionData->SetSessionName(InSessionName);
	ListView_SessionList->AddItem(GameSessionData);
}

void UMainWidget::AddOnlineSessionSearchResult(const FOnlineSessionSearchResult& InOnlineSessionSearchResult)
{
	if (ListView_SessionList == nullptr) return;
	UGameSessionData* GameSessionData = NewObject<UGameSessionData>(GetOwningLocalPlayer());
	if (GameSessionData == nullptr) return;
	FString SessionName;
	InOnlineSessionSearchResult.Session.SessionSettings.Get(FName("SessionName"), SessionName);
	GameSessionData->SetSessionName(SessionName);
	GameSessionData->SetOnlineSessionSearchResult(InOnlineSessionSearchResult);
	ListView_SessionList->AddItem(GameSessionData);
}

bool UMainWidget::SetButtonDisableTimer(float DisableTime)
{
	//APlayerController* PC = GetOwningPlayer();
	//if (PC == nullptr) return false;
	//if (Button_Start != nullptr) Button_Start->SetIsEnabled(false);
	//if (Button_Ready != nullptr) Button_Ready->SetIsEnabled(false);
	//if (Button_SessionUIOn != nullptr) Button_SessionUIOn->SetIsEnabled(false);
	//if (Button_CreateSessionUIOn != nullptr) Button_CreateSessionUIOn->SetIsEnabled(false);

	//PC->GetWorld()->GetTimerManager().SetTimer(ButtonTimerHandle, FTimerDelegate::CreateLambda(
	//	[this] {
	//		if (Button_Start != nullptr) Button_Start->SetIsEnabled(true);
	//		if (Button_Ready != nullptr) Button_Ready->SetIsEnabled(true);
	//		if (Button_SessionUIOn != nullptr) Button_SessionUIOn->SetIsEnabled(true);
	//		if (Button_CreateSessionUIOn != nullptr) Button_CreateSessionUIOn->SetIsEnabled(true);
	//	}
	//), DisableTime, false);
	return true;
}

void UMainWidget::OnUIStateChanged_Implementation(const EMainUIState& InCurrentState)
{
	CurrentUIState = InCurrentState;
	if (InCurrentState != EMainUIState::EMUIS_InLobby) return;

	bool IsHost;
	if (UGoSoccerPlayManager::GetPlayerSessionHost(GetGameInstance(), IsHost))
	{
		OnLobbyHostSetting(IsHost);
	}
}

void UMainWidget::SetSettingWidget_InGameManuPanelVisibility(bool e)
{
	if (Settings_Widget)
	{
		Settings_Widget->SetInGameManuPanelVisibility(e);
	}
}

void UMainWidget::Callback_OnClearOnlineSessionList()
{
	if (ListView_SessionList) ListView_SessionList->ClearListItems();
}

void UMainWidget::Callback_OnUpdateOnlineSessionList(const TArray<FSessionDisplayData>& OnlineSessionDataArr)
{
	Callback_OnClearOnlineSessionList();
	for (const auto& Iter : OnlineSessionDataArr)
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(
				TEXT("Session Name : %s"), *Iter.ServerName
			);
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, LogString);
		}
#endif
		//AddOnlineSessionSearchResult(Iter.RawResult.OnlineResult);
		UGameSessionData* GameSessionData = NewObject<UGameSessionData>(GetOwningLocalPlayer());
		if (GameSessionData == nullptr) return;
		GameSessionData->SetSessionName(Iter.ServerName);
		GameSessionData->SetOnlineSessionSearchResult(Iter.RawResult.OnlineResult);
		GameSessionData->SetSessionDisplayData(Iter);
		ListView_SessionList->AddItem(GameSessionData);
	}
}

void UMainWidget::Callback_OnGameStartResponse(bool bWasSuccessful)
{
	APlayerController* PC = GetOwningPlayer();
	UGoSoccerPlayManager::SetMainUIState(GetGameInstance(), PC, bWasSuccessful ? EMainUIState::EMUIS_GameStarted : EMainUIState::EMUIS_InLobby);

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("Callback_OnGameStartResponse : %s"),
			bWasSuccessful ? TEXT("Start Game") : TEXT("Remain Lobby")
			);
		GEngine->AddOnScreenDebugMessage(-1, 3.f, bWasSuccessful ? FColor::Green : FColor::Blue, LogString);
	}
#endif
}

EPlayGameRule UMainWidget::GetSelectedMode_CreateSession()
{
	if (!Radio_Mode0 && BP_Radio_Mode0)
	{
		Radio_Mode0 = BP_Radio_Mode0->GetCheckBox();
	}
	if (!Radio_Mode1 && BP_Radio_Mode1)
	{
		Radio_Mode1 = BP_Radio_Mode1->GetCheckBox();
	}
	if (!Radio_Mode0 || !Radio_Mode1) return EPlayGameRule::EPGR_Default;
	
	ECheckBoxState CheckState0 = Radio_Mode0->GetCheckedState();
	ECheckBoxState CheckState1 = Radio_Mode1->GetCheckedState();
	if (CheckState0 == ECheckBoxState::Checked && CheckState1 == ECheckBoxState::Unchecked)
	{
		return EPlayGameRule::EPGR_5or5Mode;
	}
	if (CheckState0 == ECheckBoxState::Unchecked && CheckState1 == ECheckBoxState::Checked)
	{
		return EPlayGameRule::EPGR_5plus5Mode;
	}
	return EPlayGameRule::EPGR_Default;
}

bool UMainWidget::IsInPlay()
{
	return CurrentUIState == EMainUIState::EMUIS_InLobby || CurrentUIState == EMainUIState::EMUIS_GameStarted;
}

void UMainWidget::ApplyGameSettings(UGameUserSettings* InUserSettings)
{
	if (Settings_Widget == nullptr) return;
	Settings_Widget->ApplyGameSettings(InUserSettings);
}
