#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "OnlineSessionSettings.h"
#include "Widget/CustomButtonWidget.h"
#include "MainWidget.generated.h"

class UButton;
class UEditableText;
class UComboBoxString;
class UListView;
class USettingWidget;
class UGameConfigSettingWidget;
class UCheckButtonWidget;
class UCheckBox;
class USelectButtonWidget;
class UTileView;
class UGameUserSettings;

UCLASS()
class GOSOCCER_API UMainWidget : public UUserWidget
{
	GENERATED_BODY()

public:

 /*
	MainUIPanel =========================================================
 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_OpenSinglePlayPanel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_OpenMultiPlayPanel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_Settings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_EndProcess;
 /*
	MainUIPanel End	=====================================================
 */

/*
	MultiPlayUIPanel	=================================================
*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_QuickMatch;

	/* Open Game Ccnfig Setting */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_CreateSessionUIOn;

	/* SearchSessions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_SessionUIOn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_MultiPlayUI_Back;
/*
	MultiPlayUIPanel End	=============================================
*/

/*
	SinglePlayUIPanel	=================================================
*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_AIPlayUIOn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_HotSeatPlayUIOn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_SinglePlayUI_Back;
/*
	SinglePlayUIPanel End	=============================================
*/

/*
	LobbyUIPanel ========================================================
*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_Invite;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_Ready;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_Start;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_Out_LobbyUIPanel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_ToggleSetting;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTileView> TileView_Invitables;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_SendRSVP;
/*
	LobbyUIPanel End	=================================================
*/

/*
	SessionUIPanel	=====================================================
*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_Out_SessionUIPanel;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_JoinSelectedSession;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_RefreshOnlineSession;
/*
	SessionUIPanel End	=================================================
*/
	/*Debug Only*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_OpenLocalHost;

	/*Debug Only*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_JoinLocalHost;
	
	/*Deprecated*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_CreateSession;
	
	/*Deprecated*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_Out_CreateSessionUIPanel;

	/*Debug Only*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_Debug_CustomButton;

	/*Deprecated*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_HotSeatPlay;

	/*Deprecated*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_HotSeatPlay_5plus5;

/*
	GameEndNotifyPanel	=================================================
*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_GameEnd_OutSession;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_GameEnd_Regame;
/*
	GameEndNotifyPanel End	==============================================
*/

	/*Deprecated*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckButtonWidget> BP_Radio_Mode0;

	/*Deprecated*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckButtonWidget> BP_Radio_Mode1;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	//TObjectPtr<UEditableText> EditableText_PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEditableText> EditableText_SessionName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectButtonWidget> BP_DollColor_Select;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	//TObjectPtr<UComboBoxString> ComboBox_DollColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UListView> ListView_SessionList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USettingWidget> Settings_Widget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UGameConfigSettingWidget> BP_GameConfigSetting;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UGameConfigSettingWidget> BP_LobbyGameConfigSetting;

	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnGameConfigSettingCompleted();
	UFUNCTION()
	void OnGameConfigSettingCanceled();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnLobbyConfigSettingCompleted();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnLobbyConfigSettingCanceled();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnLobbyToggleSetting();

	UFUNCTION()
	void OnOpenSinglePlayPanel();
	UFUNCTION()
	void OnOpenMultiPlayPanel();

	UFUNCTION()
	void OnOpenGameConfigSettingWidget_QuickMatch();
	UFUNCTION()
	void OnOpenGameConfigSettingWidget_CreateSession();
	UFUNCTION()
	void OnOpenGameConfigSettingWidget_AI();
	UFUNCTION()
	void OnOpenGameConfigSettingWidget_HotSeat();

	UFUNCTION()
	void OnDebug();
	UFUNCTION()
	void OnOpenLocalHost();
	UFUNCTION()
	void OnJoinLocalHost();
	UFUNCTION(BlueprintImplementableEvent)
	void Steam_CreateSession();
	UFUNCTION()
	void OnHostGame();
	UFUNCTION()
	void OnHostGameCompleted();

	UFUNCTION()
	void OnJoinGame();

	UFUNCTION()
	void OnRefreshOnlineSession();
	UFUNCTION()
	void OnStartGame();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnInviteUIOn();
	//virtual void OnInviteUIOn_Implementation();
	UFUNCTION()
	void OnInviteItemSelectionChanged(UObject* SelectedWidget, UObject* SelectedItem);
	UFUNCTION()
	void OnSendRSVP();
	UFUNCTION()
	void OnReadyGame();
	UFUNCTION()
	void OnCreateSessionUIOn();
	UFUNCTION()
	void OnJoinSelectedSession();
	UFUNCTION()
	void OnJoinSelectedSessionCompleted();
	UFUNCTION()
	void OnToVeryFirstUI();
	UFUNCTION()
	void CloseSettingsWidget();
	UFUNCTION()
	void OnOpenSettingsPanel();
	UFUNCTION()
	void OnHotSeatPlay();
	UFUNCTION()
	void OnHotSeatPlay_5plus5();
	UFUNCTION()
	void OnGameEnd_Regame();
	UFUNCTION()
	void OnOutSession();
	UFUNCTION()
	void OnEndProcess();
	UFUNCTION()
	void OnGG();

	UFUNCTION()
	void OnSelectMode0();
	UFUNCTION()
	void OnSelectMode1();

	//UFUNCTION()
	//void OnPlayerNameCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	UFUNCTION()
	void OnSessionNameCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnDollColorChanged(float NewValue);
	//void OnDollColorChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnCheckBox_UseDollVFX(bool bIsChecked);
	UFUNCTION()
	void OnSlider_ShadowChanged(float NewLevel);
	UFUNCTION()
	void OnSlider_BGMVolumeChanged(float NewLevel);
	UFUNCTION()
	void OnSlider_FXVolumeChanged(float NewLevel);
	UFUNCTION()
	void OnSlider_MasterVolumeChanged(float NewLevel);

	UFUNCTION()
	void OnSlider_Graphic_Total_Changed(float NewLevel);
	UFUNCTION()
	void OnSlider_Graphic_Shadow_Changed(float NewLevel);
	UFUNCTION()
	void OnSlider_Graphic_GlobalIllumination_Changed(float NewLevel);
	UFUNCTION()
	void OnSlider_Graphic_Reflection_Changed(float NewLevel);
	UFUNCTION()
	void OnSlider_Graphic_PostProcess_Changed(float NewLevel);
	UFUNCTION()
	void OnSlider_Graphic_Texture_Changed(float NewLevel);
	UFUNCTION()
	void OnSlider_Graphic_Effect_Changed(float NewLevel);

	void Debug_AddSessionName(FString InSessionName);
	void AddOnlineSessionSearchResult(const FOnlineSessionSearchResult& InOnlineSessionSearchResult);
private:

	TObjectPtr<UCheckBox> Radio_Mode0;
	TObjectPtr<UCheckBox> Radio_Mode1;
	FTimerHandle ButtonTimerHandle;
	EMainUIState CurrentUIState;

	UPROPERTY()
	EPlayGameRule HotSeatSelectedMode;
	bool SetButtonDisableTimer(float DisableTime);
public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnLoadingComplete_Lobby();
	UFUNCTION(BlueprintImplementableEvent)
	void OnLoadingComplete_Main();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnUIStateChanged(const EMainUIState& InCurrentState);
	UFUNCTION(BlueprintImplementableEvent)
	void OnLobbyHostSetting(bool IsHost);
	UFUNCTION(BlueprintImplementableEvent)
	void OnWinGame_HotSeat(uint8 WinDollColor);
	UFUNCTION(BlueprintImplementableEvent)
	void OnWinGame();
	UFUNCTION(BlueprintImplementableEvent)
	void OnLoseGame();
	UFUNCTION(BlueprintImplementableEvent)
	void OnDrawGame();
	UFUNCTION(BlueprintImplementableEvent)
	void OnTooltipSet(const FSessionCreateData& HostSessionCreateData);

	void SetSettingWidget_InGameManuPanelVisibility(bool e);

	UFUNCTION()
	void Callback_OnClearOnlineSessionList();
	UFUNCTION()
	void Callback_OnUpdateOnlineSessionList(const TArray<FSessionDisplayData>& OnlineSessionDataArr);
	UFUNCTION()
	void Callback_OnGameStartResponse(bool bWasSuccessful);

	EPlayGameRule GetSelectedMode_CreateSession();

	bool IsInPlay();

	void ApplyGameSettings(UGameUserSettings* InUserSettings);
};
