#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "Interface/IGoSoccerPlayMode.h"
#include "GoSoccerPlayMode.generated.h"

struct FStreakDollContainer_Counts
{
	FStreakDollContainer_Counts(FStreakDollContainer& InStreakDollContainer, float InCount)
		: StreakDollContainer(InStreakDollContainer), Count(InCount) {};
	FStreakDollContainer StreakDollContainer;
	float Count;
};

UCLASS()
class GOSOCCER_API AGoSoccerPlayMode : public AGameModeBase, public IIGoSoccerPlayMode
{
	GENERATED_BODY()

	AGoSoccerPlayMode();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	UClass* WhiteDoll_AntiFragile;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	UClass* BlackDoll_AntiFragile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	UClass* WhiteDoll_Fragile;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	UClass* BlackDoll_Fragile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DollClass, meta = (AllowPrivateAccess = "true"))
	UClass* WhiteDoll;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DollClass, meta = (AllowPrivateAccess = "true"))
	UClass* WhiteDoll_Explosive;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DollClass, meta = (AllowPrivateAccess = "true"))
	UClass* BlackDoll;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DollClass, meta = (AllowPrivateAccess = "true"))
	UClass* BlackDoll_Explosive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DollClass, meta = (AllowPrivateAccess = "true"))
	TMap<EDollType, UClass*> Black_Doll_ClassMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DollClass, meta = (AllowPrivateAccess = "true"))
	TMap<EDollType, UClass*> White_Doll_ClassMap;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Debug, meta = (AllowPrivateAccess = "true"))
	float Distance_Threshold = 22.5f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Debug, meta = (AllowPrivateAccess = "true"))
	float Radian_Threshold = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Debug, meta = (AllowPrivateAccess = "true"))
	FText WarningMessage_NotAllReady;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Debug, meta = (AllowPrivateAccess = "true"))
	FText WarningMessage_SameDollColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTextureRenderTarget2D> RT_GoBoardState;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ItemPa, meta = (AllowPrivateAccess = "true"))
	TMap<EDollType, float> DollType_SpawnRate;

	UPROPERTY()
	float TotalSpawnRateValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ItemPa, meta = (AllowPrivateAccess = "true"))
	float ItemPa_Default = 0.7f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ItemPa, meta = (AllowPrivateAccess = "true"))
	float ItemPa_Explosive = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	AActor* GoBoardActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TMap<FVector, uint8> ChinaPreset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TMap<FVector, uint8> KoreaPreset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TMap<FVector, uint8> TibetPreset;

	FTimerHandle TurnTimerHandle;
	FTimerHandle Realtime_BoardCheck_TimerHandle;
	FTimerHandle WholeMatchEnd_TimerHandle;
	TArray<FTimerHandle> WholeMatchEnd_TimerHandle_Arr;
	int32 PlacedDollCounter = 0;

	UPROPERTY()
	bool AIPlayingTurn;
	UPROPERTY()
	double TimeLimit;
	UPROPERTY()
	double StartTimeStamp;

	UPROPERTY()
	TMap<uint8, FScoreBoard> ScoreMap_Color;
	UPROPERTY()
	bool ScoringTime;
	UPROPERTY()
	EScoreType ScorinBoardSequence;
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> PlacedDollArr;
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> ScoreTemp_PlacedDollArr;

	UPROPERTY()
	uint8 AIGame_PlayerDollColor;
	UPROPERTY()
	int8 FallenDollScore = 0;
	UPROPERTY()
	EPlayGameRule PlayingGameRule;

	TSet<TWeakObjectPtr<AActor>> RecentlyMovedDollSet;
	TSet<uint8> WinPlayerDollColorSet_HotSeat;
	TSet<TWeakObjectPtr<AActor>> WinPlayerSet;
	TSet<TWeakObjectPtr<UObject>> FallenDollSet;

	bool bStationaryOnce = false;
	float Stationary_StartTime;
	uint32 TurnCounter = 0;
	EPlayGameMode PlayGameMode;

	FTimerHandle DebugTickTimer;

	UPROPERTY()
	bool bTakeTurn = false;

	virtual void Logout(AController* Exiting) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

public:
	virtual bool StartLocalHostGame_Implementation(const FSessionCreateData& SessionCreateData) override;

	virtual bool HostHotSeatGame_Implementation(const FSessionCreateData& SessionCreateData) override;
	virtual bool HostSingleAIGame_Implementation(const FSessionCreateData& SessionCreateData) override;
	virtual bool StartGame_IfAllReady_Implementation(const FSessionCreateData& SessionCreateData) override;

	virtual bool TurnStart_Implementation(APlayerController* Player) override;
	//virtual bool TurnEnd_Implementation(UObject* PlayerObject) override;
	virtual bool TurnEnd_FlickDoll_Implementation(UObject* PlayerObject) override;
	virtual bool WinCheck_Implementation(APlayerController* LatestPlayer) override;

	//virtual bool CheckBoard_Implementation(APlayerController* LatestPlayer) override;
	virtual bool SpawnDoll_Implementation(UObject* PlayerObject, FVector SpawnLocation, uint8 DollColor, const EDollType& SpawnDollType) override;
	virtual bool DebugOnlyFunction_Implementation() override;
	virtual bool ClientLoadingCompleted_Implementation(UObject* PlayerObject) override;
	virtual int32 GetPlacedDollCounter_Implementation() override;
	virtual bool MarkDollAsMoved_Implementation(UObject* DollObject) override;
	virtual bool GetGoBoardActor_Implementation(UObject*& OutGoBoardObject) override;
	virtual bool WalkOverPlayer_Implementation(UObject* GaveUpPlayerObject) override;

	virtual bool Notify_PlayerJoinedSessionComplete_Implementation(UObject* JoinedPlayerObject) override;
	virtual bool Notify_PlayerLeftSession_Implementation(UObject* LeftPlayerObject) override;

	virtual bool AIPlay_SpawnDollAtLocation_Implementation(UObject* NextPlayerObject, FVector PlaceLocation) override;
	virtual bool AIPlay_FlickDoll_Implementation(UObject* NextPlayerObject, UObject* FlickDollObject, FVector FlickDirection) override;
	
	virtual bool AddFallenScore_Implementation(UObject* FallenDollObject) override;
	virtual bool DollBrokenAdvantage_Implementation(UObject* BrokenDollObject) override;

	virtual bool SetHostingSessionCreateData_Implementation(const FSessionCreateData& InHostingSessionCreateData) override;
	virtual bool GetHostingSessionCreateData_Implementation(FSessionCreateData& OutHostingSessionCreateData) override;
	virtual bool Notify_UpdateTooltip_SessionConfig_Implementation(const FSessionCreateData& InHostingSessionCreateData) override;
	virtual bool Request_UpdateTooltip_SessionConfig_Implementation(UObject* PlayerControllerObject) override;
	virtual bool AddChattingMessage_Implementation(const FText& InTextMessage, const FName& InPlayerName) override;
	virtual bool GetNextDollType_Implementation(EDollType& OutNextDollType) override;
	virtual bool GetPlacedDollCount_Implementation(int32& OutDollCount) override
	{
		OutDollCount = PlacedDollArr.Num();
		return true;
	};
	virtual bool ContinueMatch_Implementation() override;
private:
	FSessionCreateData CurrentPlayingSessionData;
	/**
	 * 호스트가 세션을 만들 때 / 로비에서 바꾼 세션 설정.
	 * 호스트의 PlayerState 는 리슨 서버 트래블 때 새로 만들어지면서 값을 잃으므로,
	 * 세션이 유지되는 동안 살아있는 GameMode 가 이 값을 들고 모든 플레이어에게 공유한다.
	 */
	FSessionCreateData HostingSessionCreateData;
	int32 MatchCount = 1;
	int32 Color0MatchCount = 0;
	int32 Color1MatchCount = 0;
	FOnContinueMatch Delegate_OnContinueMatch;

	virtual bool RefreshPlayerCardArrWidgets_Implementation() override;
	virtual bool RefreshPlayerCardArrWidgets_Except_Implementation(UObject* ExceptPlayerControllerObject) override;
	bool TurnEnd_SpawnDoll(APlayerController* LatestPlayer, TWeakObjectPtr<AActor> PlacedDoll);
	bool TurnEnd_FlickDoll(APlayerController* LatestPlayer);
	bool IsBoardStationary();
	bool AddWinPlayer(AActor* WinPlayer);
	bool AddWinPlayerDollColor_HotSeat(uint8 WinPlayerDollColor);
	APlayerController* GetNextPlayer(APlayerController* LatestPlayer);
	void ResetPlayBoard(const FSessionCreateData& SessionCreateData);
	void ResetMatchBoard(const FSessionCreateData& SessionCreateData);
	TArray<int32> UF_Parent;
	TArray<TSet<int32>> UF_Accessable;
	TArray<bool> UF_Visited;

	int32 UF_Find(int32 Doll_idx);
	void UF_Union(int32 Doll_idx_i, int32 Doll_idx_j);
	bool UF_GroupStreakCheck(int32 RootIdx);

	bool Travel(int32 idx, float StartRadian, int32 depth, TArray<AActor*>& WinDollArr);
	bool IsDollInRange(int32 x, int32 y);
	bool UF_JoinDollIntoGroup(AActor* NewDoll);

	void AddFallenDollScore(uint8 FallenDollColor);
	void UpdateFallenDollScore();

	bool ImmediateWinPlayer_LoseAnother(uint8 InColor, bool bDraw = false);
// AI Mode
private:
	void AITurnStart(APlayerController* WaitPlayer);
// Realtime Mode
public:
	UFUNCTION()
	void Realtime_CheckStreak5();
private:
	//struct FPlayerDollColorContainer
	//{
	//	FPlayerDollColorContainer(TWeakObjectPtr<APlayerController> InPC, InDollColor)
	//		: ContainPC(InPC), ContainDollColor(InDollColor) {};
	//	TWeakObjectPtr<APlayerController> ContainPC;
	//	uint8 ContainDollColor;
	//};
	FOnSetOminousHUDPercent Delegate_OnPlayerColor0_SetMinousHUDPercet;
	FOnSetOminousHUDPercent Delegate_OnPlayerColor1_SetMinousHUDPercet;
	FOnAddRealtimeColldownBonus Delegate_OnPlayerColor0_AddRealtimeColldownBonus;
	FOnAddRealtimeColldownBonus Delegate_OnPlayerColor0_AddRealtimeColldownBonus_Flick;
	FOnAddRealtimeColldownBonus Delegate_OnPlayerColor1_AddRealtimeColldownBonus;
	FOnAddRealtimeColldownBonus Delegate_OnPlayerColor1_AddRealtimeColldownBonus_Flick;
	//FDelegateHandle DelegateHandle_OnPlayerColor0_SetMinousHUDPercet;
	//FDelegateHandle DelegateHandle_OnPlayerColor1_SetMinousHUDPercet;

	float PrevTimeColor0 = 0.f;
	float PrevTimeColor1 = 0.f;
	float FallenDollScoreExceededTimeStamp = 0.f;

	//TMap<APlayerController*, FPlayerCard> PlayerCardHashingMap;
	TMap<APlayerController*, uint8> DollColorHashingMap;

	bool bRealtime_GameSet = false;
	TArray<FStreakDollContainer_Counts> StreakContingDollMap;

	bool Realtime_StartCount(FStreakDollContainer& StreakDollContainer);
	bool GetPlayerColor(APlayerController* CheckPC, uint8& OutDollColor);
	bool GetPlayerName(APlayerController* CheckPC, FName& OutPlayerName);
	bool GameSet();

	void OnSpawnedOrFlicked();

	bool MatchEnd(uint8 WinDollColor, bool& OutContinueGame, APlayerController*& OutFinalWinPlayer, uint8& OutFinalWinDollColor);
public:
	UFUNCTION(Exec, Category = "Debug")
	void SetAngularThreshold(float NewThreshold) { Radian_Threshold = NewThreshold; };

	UFUNCTION(Exec, Category = "Debug")
	void SetDistanceThreshold(float NewThreshold) { Distance_Threshold = NewThreshold; };

	UFUNCTION(Exec, Category = "Debug")
	void GetDistanceThreshold() { UE_LOG(LogTemp, Log, TEXT("Distance_Threshold : %f"), Distance_Threshold); };

	UFUNCTION(Exec, Category = "Debug")
	void SetItemPa_Explosive(float NewPercent) { ItemPa_Explosive = NewPercent; };

	UFUNCTION(Exec, Category = "Debug")
	void SetMatchScoreLamp(int32 Idx, uint8 MatchState, int32 DollColor);

	UFUNCTION(Exec, Category = "Debug")
	void ResetMatchScoreLamp();

	UFUNCTION(Exec, Category = "Debug")
	void SetMatchScoreLampCount(int32 MaxCount);
};
