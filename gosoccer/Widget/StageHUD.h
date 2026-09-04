#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interface/IGoHUD.h"
//#include "Blueprint/IUserObjectListEntry.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "Net/UnrealNetwork.h"
#include "StageHUD.generated.h"

class UTexture2D;
class UImage;
class UListView;
class UTileView;
class UButton;
class UTextBlock;
class UProgressBar;
class UEditableTextBox;

UCLASS()
class GOSOCCER_API UStageHUD : public UUserWidget, public IIGoHUD
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> PutDollTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> FlickDollTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UButton> Button_GameEnd_Regame;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UButton> Button_GameEnd_OutSession;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> TurnNotifyText_PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProgressBar> ProgressBar_FallenDollCounter;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> FingerModePreview;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UListView> ListView_PlayerCards;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTileView> TileView_TemporalMessages;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UListView> ListView_ChattingBoxes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget), meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEditableTextBox> Textbox_ChatInsert;
	
	uint8 FingerMode = 0;

	float TemporalMessage_ManualDestroyCounter;
	FTimerHandle TemporalMessageTimerHandle;
	FTimerManager* TimerManager;
public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintNativeEvent)
	UObject* CreateChattingBoxDataAsObject(const FName& SendPlayerName, const FText& TextMessage, bool bPushToChat = true);

	UFUNCTION()
	void DEBUG_OnDebug();

	UFUNCTION()
	void CallBack_SwitchFingerMode(uint8 SwitchMode);
	UFUNCTION()
	bool AddPlayerCard(const FPlayerCard& AddPlayerCardStruct);
	UFUNCTION()
	bool AddTemporalMessage(const FString& ThrowMessage, ETemporalMessageType TemporalMessageType, float FloatTime);
	UFUNCTION()
	bool ResetCursorPlayerCards();
	UFUNCTION(BlueprintImplementableEvent)
	void OnGameStarted(const FSessionCreateData& InSessionCreateData);
	UFUNCTION(BlueprintImplementableEvent)
	void OnItsMyTurn();
	UFUNCTION(BlueprintImplementableEvent)
	void OnItWasMyTurn();
	UFUNCTION(BlueprintImplementableEvent)
	void OnWinGame_HotSeat(uint8 WinDollColor);
	UFUNCTION(BlueprintImplementableEvent)
	void OnWinGame();
	UFUNCTION(BlueprintImplementableEvent)
	void OnLoseGame();
	UFUNCTION(BlueprintImplementableEvent)
	void OnDrawGame();
	UFUNCTION(BlueprintImplementableEvent)
	void OnLoadingComplete_Lobby();
	UFUNCTION(BlueprintImplementableEvent)
	void OnLoadingComplete_Main();
	UFUNCTION(BlueprintImplementableEvent)
	void OnNotifyTurn(const FName& TurnPlayerName);
	UFUNCTION(BlueprintImplementableEvent)
	void OnNotifyFallenDollScore(int32 InFallenDollScore);
	UFUNCTION(BlueprintImplementableEvent)
	void OnUIStateChanged(const EMainUIState& CurrentState);
	UFUNCTION(BlueprintImplementableEvent)
	void OnSetLoadingScreen(bool bEnable);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSetSessionLoadingScreen(bool bEnable, ESessionLoadingPhase LoadingPhase);
	//bool RefreshPlayerCardListView(const TArray<FPlayerCard>& NewPlayerCardArr);
	//bool RefreshPlayerCardListView(const FPlayerCard& NewPlayerCard);
	UFUNCTION(BlueprintImplementableEvent)
	void OnSetOminousHUD(float CurrentTimeSec, float MaxTimeSec);
	UFUNCTION(BlueprintImplementableEvent)
	void OnEnterCompleted();
	UFUNCTION(BlueprintImplementableEvent)
	void OnSetGameTimer(double InSecondsLeft);
	UFUNCTION(BlueprintImplementableEvent)
	void OnSetScoringBoard(bool bEnable, const TArray<FScoreBoard>& ScoreArr);
	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateScoringBoard(uint8 DollColor, EScoreType UpdateScoreType, int32 UpdatedScore);
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCylinder(FCylinderConfig InCylinderConfig);
	UFUNCTION(BlueprintImplementableEvent)
	void SetMatchScoreLampState(int32 Index, const EMatchScoreLampState NewState, int32 FillDollColor);
	UFUNCTION(BlueprintImplementableEvent)
	void ResetMatchScoreLampState();
	UFUNCTION(BlueprintImplementableEvent)
	void SetMatchScoreLampCount(int32 MaxCount);
	UFUNCTION(BlueprintImplementableEvent)
	void ThrowContinueMatch(float ThrowingTimeSec);
	UFUNCTION(BlueprintImplementableEvent)
	void WindupMatchWidget(const FMatchScoreLampStateConfig& InMatchScoreLampStateConfig);
private:
	void CheckTemporalMessageIter();
	FTimerManager* GetTimerManager_HUD();
};
