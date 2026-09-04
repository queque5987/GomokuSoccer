#include "GoSoccerPlayMode.h"
#include "Interface/IGoBoard.h"
#include "GoSoccerPlayManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "Interface/IGoSoccerPlayerState.h"
#include "DrawDebugHelpers.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GoSoccerAISubsystem.h"

constexpr float REALTIME_STREAK_MAINTAIN_TIMESEC = 3.f;
constexpr float REALTIME_BOARDCHECK_INTERVAL = 0.125f;

constexpr float REALTIME_FALLEN_COOLDOWN_BONUS = 0.25f;
constexpr float REALTIME_FLICK_COOLDOWN_BONUS = 0.05f;

constexpr float REALTIME_FALLEN_COOLDOWN_BONUS_FLICK = 0.35f;
constexpr float REALTIME_PLACE_COOLDOWN_BONUS_FLICK = 0.25f;

AGoSoccerPlayMode::AGoSoccerPlayMode()
{
}

void AGoSoccerPlayMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

#if !UE_BUILD_SHIPPING
	if (GEngine && Exiting != nullptr)
	{
		FString LogString = FString::Printf(TEXT("Out Player : %s"), *Exiting->GetFName().ToString());
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, LogString);
		UE_LOG(LogTemp, Log, TEXT("Out Player : %s"), *Exiting->GetFName().ToString());
	}
#endif

	Execute_Notify_PlayerLeftSession(this, Exiting);
}

void AGoSoccerPlayMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (GoBoardActor == nullptr)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UIGoBoard::StaticClass(), FoundActors);
		for (AActor* FoundActor : FoundActors)
		{
			if (UGoSoccerPlayManager::IsGoBoard(FoundActor))
			{
				GoBoardActor = FoundActor;
				break;
			}
		}
	}
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FUniqueNetIdRepl UniqueId = NewPlayer->PlayerState->GetUniqueId();
			if (UniqueId.IsValid())
			{
				SessionInterface->RegisterPlayer(NAME_GameSession, *UniqueId, false);
			}
		}
	}
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; ++PCIter)
		{
			APlayerController* PC = PCIter->Get();
			if (PC == nullptr) continue;
			FString LogString = FString::Printf(TEXT("Current Players : %s"), *PC->GetFName().ToString());
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, LogString);
			UE_LOG(LogTemp, Log, TEXT("Current Players : %s"), *PC->GetFName().ToString());
		}
	}
#endif
}

bool AGoSoccerPlayMode::StartGame_IfAllReady_Implementation(const FSessionCreateData& SessionCreateData)
{
	bool bAllReady = true;
	uint8 ColorCounter = 0;
	AGameStateBase* GS = GetGameState<AGameStateBase>();
	bool bIndivDollColor = true;
	//FSessionCreateData tempSessionConfig;
	//APlayerController* HostPlayer = nullptr;
	if (GS != nullptr)
	{
		for (APlayerState* PSIter : GS->PlayerArray)
		{
			FPlayerCard tempPlayerCard;
			IIGoSoccerPlayerState::Execute_GetPlayerCard(PSIter, tempPlayerCard);
			uint8 DollColor = tempPlayerCard.PlayerDollColor;
			if (ColorCounter & (1 << DollColor))
			{
				bIndivDollColor = false;
			}
			ColorCounter |= (1 << DollColor);
			if (!tempPlayerCard.bGameReady)
			{
				bAllReady = false;
			}
			//if (HostPlayer == nullptr)
			//{
			//	if (UGoSoccerPlayManager::GetHostingSessionCreateData(PSIter, tempSessionConfig))
			//	{
			//		HostPlayer = PSIter->GetPlayerController();
			//	}
			//}
		}
	}
//#if !UE_BUILD_SHIPPING
//	if (GEngine)
//	{
//		FString LogString = FString::Printf(TEXT("StartGame_IfAllReady : %s, Host : %s"),
//			bAllReady ? TEXT("Start Game") : TEXT("Nono Start Game"),
//			HostPlayer != nullptr ? *HostPlayer->GetFName().ToString() : TEXT("Not Found")
//		);
//		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, LogString);
//	}
//#endif
	if (!bAllReady || !bIndivDollColor)
	{
		for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; ++PCIter)
		{
			APlayerController* PC = PCIter->Get();
			if (PC == nullptr) continue;
			if (!bAllReady)
			{
				UGoSoccerPlayManager::ThrowTemporalMessage(PC, WarningMessage_NotAllReady.ToString(), ETemporalMessageType::ETMT_Warning);
			}
			if (!bIndivDollColor)
			{
				UGoSoccerPlayManager::ThrowTemporalMessage(PC, WarningMessage_SameDollColor.ToString(), ETemporalMessageType::ETMT_Warning);
			}
		}

		return bAllReady && bIndivDollColor;
	}

	uint8 tempFaster = (uint8)(EDollColor::EDC_Max);
	APlayerController* FasterPlayer = nullptr;
	for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; ++PCIter)
	{
		APlayerController* PC = PCIter->Get();
		if (PC == nullptr) continue;
		APlayerState* PS = PC->GetPlayerState<APlayerState>();
		if (PS == nullptr) continue;
		FPlayerCard tempPlayerCard;
		IIGoSoccerPlayerState::Execute_GetPlayerCard(PS, tempPlayerCard);
		if (tempPlayerCard.PlayerDollColor < tempFaster)
		{
			tempFaster = tempPlayerCard.PlayerDollColor;
			FasterPlayer = PC;
		}
	}
	if (FasterPlayer == nullptr)
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("Unable to Find Player"));
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, LogString);
		}
#endif
		return false;
	}

	for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; ++PCIter)
	{
		APlayerController* PC = PCIter->Get();
		if (PC == nullptr) continue;
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("GameStarted : %s"), *PC->GetFName().ToString());
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, LogString);
		}
#endif
		if (SessionCreateData.PlayGameRule == EPlayGameRule::EPGR_Default)
		{
#if !UE_BUILD_SHIPPING
			if (GEngine)
			{
				FString LogString = FString::Printf(TEXT("Session Create Data Not Found Playing 5or5 Mode"));
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, LogString);
			}
#endif
			//SessionCreateData.PlayGameRule = EPlayGameRule::EPGR_5or5Mode;
			UGoSoccerPlayManager::GameStarted(PC, SessionCreateData);
		}
		else
		{
#if !UE_BUILD_SHIPPING
			if (GEngine)
			{
				FString LogString = FString::Printf(TEXT("Playing Game Rule Based : %s"),
					SessionCreateData.PlayGameRule == EPlayGameRule::EPGR_5or5Mode ? TEXT("EPGR_5or5Mode")
					: (SessionCreateData.PlayGameRule == EPlayGameRule::EPGR_5plus5Mode ? TEXT("EPGR_5plus5Mode")
						: TEXT("Default"))
					);
				GEngine->AddOnScreenDebugMessage(-1, 7.5f, FColor::Green, LogString);
			}
#endif
			UGoSoccerPlayManager::GameStarted(PC, SessionCreateData);
		}
		if (!bTakeTurn) UGoSoccerPlayManager::TurnStart(PC);
	}
	if (SessionCreateData.PlayGameRule == EPlayGameRule::EPGR_Default)
	{
		PlayingGameRule = SessionCreateData.PlayGameRule;
	}

	if (bTakeTurn)
	{
		GetWorld()->GetTimerManager().SetTimer(
			TurnTimerHandle,
			FTimerDelegate::CreateLambda(
				[this, FasterPlayer]()
				{
					if (FasterPlayer != nullptr)
					{
						AGoSoccerPlayMode::Execute_TurnStart(this, FasterPlayer);
					}
				}
			), 1.f, false
		);
	}
	
	ResetMatchBoard(SessionCreateData);
	ResetPlayBoard(SessionCreateData);
	//int32 SelectedMatchRule = static_cast<int32>(SessionCreateData.MatchRule);
	//MatchCount = 1 + (SelectedMatchRule - 1) * 2;
	//Color0MatchCount = 0;
	//Color1MatchCount = 0;
	//for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; ++PCIter)
	//{
	//	UGoSoccerPlayManager::SetMatchScoreLampCount(PCIter->Get(), MatchCount);
	//}
	PlayGameMode = EPlayGameMode::EPGM_OnlineSession;
	return bAllReady;
}

bool AGoSoccerPlayMode::StartLocalHostGame_Implementation(const FSessionCreateData& SessionCreateData)
{
	APlayerController* PC = nullptr;
	for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; ++PCIter)
	{
		PC = PCIter->Get();
		if (PC == nullptr) continue;
		UGoSoccerPlayManager::GameStarted(PC, SessionCreateData);
	}
	UE_LOG(LogTemp, Log, TEXT("StartLocalHostGame - ResetPlayBoard Start"));
	FSessionCreateData tempSessionCreateData = SessionCreateData;
	tempSessionCreateData.PlayGameMode = EPlayGameMode::EPGM_OnlineSession;
	ResetMatchBoard(SessionCreateData);
	ResetPlayBoard(tempSessionCreateData);
	UE_LOG(LogTemp, Log, TEXT("StartLocalHostGame - ResetPlayBoard End"));
	if (bTakeTurn)
	{
		APlayerController* FasterPlayer = nullptr;
		uint8 tempFaster = (uint8)(EDollColor::EDC_Max);
		for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; ++PCIter)
		{
			PC = PCIter->Get();
			if (PC == nullptr) continue;
			APlayerState* PS = PC->GetPlayerState<APlayerState>();
			if (PS == nullptr) continue;
			FPlayerCard tempPlayerCard;
			IIGoSoccerPlayerState::Execute_GetPlayerCard(PS, tempPlayerCard);
			if (tempPlayerCard.PlayerDollColor < tempFaster)
			{
				tempFaster = tempPlayerCard.PlayerDollColor;
				FasterPlayer = PC;
			}
		}
		GetWorld()->GetTimerManager().SetTimer(
			TurnTimerHandle,
			FTimerDelegate::CreateLambda(
				[this, FasterPlayer]()
				{
					if (FasterPlayer != nullptr)
					{
						AGoSoccerPlayMode::Execute_TurnStart(this, FasterPlayer);
					}
				}
			), 1.f, false
		);
	}
	else
	{
		for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; ++PCIter)
		{
			APlayerController* iterPC = PCIter->Get();
			if (iterPC == nullptr) continue;
			UGoSoccerPlayManager::TurnStart(iterPC);
		}
	}
	PlayGameMode = EPlayGameMode::EPGM_OnlineSession;
	PlayingGameRule = SessionCreateData.PlayGameRule;
	return true;
}

bool AGoSoccerPlayMode::HostHotSeatGame_Implementation(const FSessionCreateData& SessionCreateData)
{
	APlayerController* PC = nullptr;
	for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; ++PCIter)
	{
		PC = PCIter->Get();
		if (PC == nullptr) return false;
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("HotSeatGameStarted : %s"), *PC->GetFName().ToString());
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, LogString);
		}
#endif
		UGoSoccerPlayManager::GameStarted(PC, SessionCreateData);
	}
	ResetMatchBoard(SessionCreateData);
	ResetPlayBoard(SessionCreateData);
	if (bTakeTurn)
	{
		GetWorld()->GetTimerManager().SetTimer(
			TurnTimerHandle,
			FTimerDelegate::CreateLambda(
				[this, PC]()
				{
					if (PC != nullptr)
					{
						AGoSoccerPlayMode::Execute_TurnStart(this, PC);
					}
				}
			), 1.f, false
		);
	}
	else
	{
		for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; ++PCIter)
		{
			APlayerController* iterPC = PCIter->Get();
			if (iterPC == nullptr) continue;
			UGoSoccerPlayManager::TurnStart(iterPC);
		}
	}
	PlayGameMode = EPlayGameMode::EPGM_HotSeatGame;
	PlayingGameRule = SessionCreateData.PlayGameRule;
	return true;
}

//bool AGoSoccerPlayMode::HostSingleAIGame_Implementation(EPlayGameRule PlayGameRule, int32 PlayAsDollColor)
bool AGoSoccerPlayMode::HostSingleAIGame_Implementation(const FSessionCreateData& SessionCreateData)
{
	APlayerController* PC = nullptr;
	for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; ++PCIter)
	{
		PC = PCIter->Get();
		if (PC == nullptr) return false;
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("SingleAIGameStarted : %s"), *PC->GetFName().ToString());
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, LogString);
		}
#endif
		UGoSoccerPlayManager::GameStarted(PC, SessionCreateData);
	}
	GetWorld()->GetTimerManager().SetTimer(
		TurnTimerHandle,
		FTimerDelegate::CreateLambda(
			[this, PC]()
			{
				if (PC != nullptr)
				{
					AGoSoccerPlayMode::Execute_TurnStart(this, PC);
				}
			}
		), 1.f, false
	);
	ResetMatchBoard(SessionCreateData);
	ResetPlayBoard(SessionCreateData);
	PlayGameMode = EPlayGameMode::EPGM_SingleAIPlay;
	PlayingGameRule = SessionCreateData.PlayGameRule;
	AIPlayingTurn = false;
	AIGame_PlayerDollColor = (uint8)SessionCreateData.PlayAsDollColor;
	return true;
}

bool AGoSoccerPlayMode::TurnStart_Implementation(APlayerController* Player)
{
	if (PlayGameMode == EPlayGameMode::EPGM_SingleAIPlay)
	{
		for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; ++PCIter)
		{
			APlayerController* PC = PCIter->Get();
			if (PC == nullptr) continue;
			UGoSoccerPlayManager::NotifyPlayerTurn(PC, AIPlayingTurn ? TEXT("AI Turn") : TEXT("Player Turn"));
		}
		if (AIPlayingTurn)
		{
			AIPlayingTurn = false;
			AITurnStart(Player);
			return true;
		}
		else
		{
			AIPlayingTurn = !AIPlayingTurn;
		}
	}
	else 
	{
		if (bTakeTurn)
		{
			FName PlayerName;
			if (UGoSoccerPlayManager::GetPlayerName(Player ? Player->GetPlayerState<APlayerState>() : nullptr, PlayerName))
			{ 
				for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; ++PCIter)
				{
					APlayerController* PC = PCIter->Get();
					if (PC == nullptr) continue;
					UGoSoccerPlayManager::NotifyPlayerTurn(PC, PlayerName);
				}
			}
		}
	}
	return bTakeTurn ? UGoSoccerPlayManager::TurnStart(Player) : true;
}

bool AGoSoccerPlayMode::TurnEnd_FlickDoll_Implementation(UObject* PlayerObject)
{
	APlayerController* Player = Cast<APlayerController>(PlayerObject);
	if (Player == nullptr) return false;
	UE_LOG(LogTemp, Log, TEXT("Player %s : Flick Turn End"), *PlayerObject->GetFName().ToString());
	TurnEnd_FlickDoll(Player);
	return true;
}

bool AGoSoccerPlayMode::WinCheck_Implementation(APlayerController* LatestPlayer)
{
	APlayerController* WinPlayer = nullptr;
	uint8 tempWinDollColor = 0;
	bool MatchEndFlag = false;
	bool ContinueGameFlag = false;
	if (PlayGameMode == EPlayGameMode::EPGM_HotSeatGame || PlayGameMode == EPlayGameMode::EPGM_SingleAIPlay)
	{
		FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator();
		APlayerController* PC = PCIter->Get();
		if (WinPlayerDollColorSet_HotSeat.Num() > 1)
		{
			// Plz No Draw
			//bGameOver = MatchEnd();
			//return UGoSoccerPlayManager::PlayerDraw(PC);
		}
		else if (WinPlayerDollColorSet_HotSeat.Num() > 0)
		{
			for (uint8 WinDollColor : WinPlayerDollColorSet_HotSeat)
			{
				if (PlayGameMode == EPlayGameMode::EPGM_HotSeatGame)
				{
					MatchEndFlag = MatchEnd(WinDollColor, ContinueGameFlag, WinPlayer, tempWinDollColor);
					break;
					//return UGoSoccerPlayManager::PlayerWin_HotSeat(PC, WinDollColor);
				}
				else
				{
					if (AIGame_PlayerDollColor == WinDollColor)
					{
						MatchEndFlag = MatchEnd(WinDollColor, ContinueGameFlag, WinPlayer, tempWinDollColor);
						//return UGoSoccerPlayManager::PlayerWin(PC);
					}
					else
					{
						MatchEndFlag = MatchEnd(WinDollColor, ContinueGameFlag, WinPlayer, tempWinDollColor);
						//return UGoSoccerPlayManager::PlayerLose(PC);
					}
					break;
				}
			}
		}
	}
	else
	{
		//uint8 iFinished = 0;
		for (auto& Iter : WinPlayerSet)
		{
			if (!Iter.IsValid()) continue;
			AActor* PC = Iter.Get();
			if (PC == nullptr) continue;
			if (WinPlayerSet.Num() > 1) // Draw
			{
				// Plz No Draw
				//if (UGoSoccerPlayManager::PlayerDraw(PC))
				//{
				//	iFinished++;
				//}
			}
			else if (WinPlayerSet.Num() > 0) // Win
			{
				uint8 tempPCDollColor = 0;
				GetPlayerColor(Cast<APlayerController>(PC), tempPCDollColor);
				MatchEndFlag = MatchEnd(tempPCDollColor, ContinueGameFlag, WinPlayer, tempWinDollColor);
				//if (UGoSoccerPlayManager::PlayerWin(PC))
				//{
				//	APlayerController* tempPlayer = Cast<APlayerController>(PC);
				//	UGoSoccerPlayManager::PlayerLose(GetNextPlayer(tempPlayer));
				//	iFinished++;
				//}
			}
		}
//		if (iFinished > 1)
//		{
//			return true;
//		}
//		else if (iFinished == 1)
//		{
//#if !UE_BUILD_SHIPPING
//			if (GEngine && WinPlayerSet.Num() > 1)
//			{
//				FString LogString = FString::Printf(TEXT("WinCheck : Draw - OnePlayerMissing"));
//				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, LogString);
//			}
//#endif
//			return true;
//		}
	}
	// Continue Game
	if (LatestPlayer == nullptr) return false;
	APlayerController* NextPlayer = GetNextPlayer(LatestPlayer);
	if (NextPlayer == nullptr) return false;
	//if (CurrentPlayingSessionData.PlayTurnRule == EPlayTurnRule::EPTR_TakeTurn)
	//{
	//	TArray<EDollType> Turn_DollArr;
	//	for (int32 i = 0; i < 4; i++)
	//	{
	//		if (DollTypeQueue.IsEmpty()) break;
	//		EDollType OutDollType;
	//		if (DollTypeQueue.Dequeue(OutDollType))
	//		{
	//			Turn_DollArr.Emplace(OutDollType);
	//		}
	//	}
	//	UGoSoccerPlayManager::SetTurnMode_CylinderConfig(LatestPlayer, Turn_DollArr);
	//	// TODO Push 1 EDollType Element -> Inside { Idx++ Every Element + Cylinder Turn Animation}
	//}
	if (MatchEndFlag)
	{
		auto* SomePC = GetWorld()->GetFirstPlayerController();
		uint8 SomePlayerColor;
		GetPlayerColor(SomePC, SomePlayerColor);
		if (ContinueGameFlag) // Continue Nexe Match
		{
			FMatchScoreLampStateConfig MatchScoreLampStateConfig;
			if (CurrentPlayingSessionData.PlayGameMode == EPlayGameMode::EPGM_HotSeatGame)
			{
				UGoSoccerPlayManager::ThrowContinueMatchWidget(
					SomePC, true, FMatchScoreLampStateConfig(
						Color0MatchCount + Color1MatchCount - 1,
						tempWinDollColor,
						EMatchScoreLampState::EMSLS_Green
					)
				);
				//UGoSoccerPlayManager::SetMatchScoreLampState(SomePC, Color0MatchCount + Color1MatchCount - 1, EMatchScoreLampState::EMSLS_Green, tempWinDollColor);
			}
			else if (CurrentPlayingSessionData.PlayGameMode == EPlayGameMode::EPGM_SingleAIPlay)
			{
				UGoSoccerPlayManager::ThrowContinueMatchWidget(
					SomePC, true, FMatchScoreLampStateConfig(
						Color0MatchCount + Color1MatchCount - 1, 
						tempWinDollColor,
						tempWinDollColor == 0 ? EMatchScoreLampState::EMSLS_Green : EMatchScoreLampState::EMSLS_Broken
					)
				);
				//UGoSoccerPlayManager::SetMatchScoreLampState(
				//	SomePC,
				//	Color0MatchCount + Color1MatchCount - 1, 
				//	tempWinDollColor == 0 ? EMatchScoreLampState::EMSLS_Green : EMatchScoreLampState::EMSLS_Broken,
				//	tempWinDollColor
				//);
			}
			else if (CurrentPlayingSessionData.PlayGameMode == EPlayGameMode::EPGM_OnlineSession)
			{
				UGoSoccerPlayManager::ThrowContinueMatchWidget(
					SomePC, true, FMatchScoreLampStateConfig(
						Color0MatchCount + Color1MatchCount - 1,
						tempWinDollColor,
						tempWinDollColor == SomePlayerColor ? EMatchScoreLampState::EMSLS_Green : EMatchScoreLampState::EMSLS_Broken
					)
				);
				//UGoSoccerPlayManager::SetMatchScoreLampState(
				//	SomePC,
				//	Color0MatchCount + Color1MatchCount - 1,
				//	tempWinDollColor == SomePlayerColor ? EMatchScoreLampState::EMSLS_Green : EMatchScoreLampState::EMSLS_Broken,
				//	tempWinDollColor
				//);

				UGoSoccerPlayManager::ThrowContinueMatchWidget(
					GetNextPlayer(SomePC), false, FMatchScoreLampStateConfig(
						Color0MatchCount + Color1MatchCount - 1,
						tempWinDollColor,
						tempWinDollColor != SomePlayerColor ? EMatchScoreLampState::EMSLS_Green : EMatchScoreLampState::EMSLS_Broken
					)
				);
				//UGoSoccerPlayManager::SetMatchScoreLampState(
				//	GetNextPlayer(SomePC),
				//	Color0MatchCount + Color1MatchCount - 1,
				//	tempWinDollColor != SomePlayerColor ? EMatchScoreLampState::EMSLS_Green : EMatchScoreLampState::EMSLS_Broken,
				//	tempWinDollColor
				//);
			}
			//UGoSoccerPlayManager::ThrowContinueMatchWidget(SomePC, true);
			//auto* NP = GetNextPlayer(SomePC);
			//if (NP != SomePC)
			//{
			//	UGoSoccerPlayManager::ThrowContinueMatchWidget(NP, false);
			//}
			//ResetPlayBoard(CurrentPlayingSessionData);
			return true;
		}
		else // End Whole Match
		{
			//GetWorld()->GetTimerManager().ClearTimer(WholeMatchEnd_TimerHandle);
			//GetWorld()->GetTimerManager().SetTimer(
			//	WholeMatchEnd_TimerHandle,
			//	FTimerDelegate::CreateLambda([&]()
			//		{
			//			if (MatchCount < Color0MatchCount + Color1MatchCount);
			//		}
			//	),
			//	0.15f, false
			//);
			//auto* SomePC = GetWorld()->GetFirstPlayerController();
			uint8 SomeColor = 0;
			GetPlayerColor(SomePC, SomeColor);
			UGoSoccerPlayManager::WindupMatchWidget(
				SomePC,
				FMatchScoreLampStateConfig(
					Color0MatchCount + Color1MatchCount - 1,
					tempWinDollColor,
					tempWinDollColor == SomeColor ? EMatchScoreLampState::EMSLS_Green : EMatchScoreLampState::EMSLS_Broken
				)
			);
			APlayerController* SomeNextPlayer = GetNextPlayer(SomePC);
			if (CurrentPlayingSessionData.PlayGameMode == EPlayGameMode::EPGM_OnlineSession)
			{
				uint8 SomeNextPlayerColor;
				GetPlayerColor(SomeNextPlayer, SomeNextPlayerColor);
				UGoSoccerPlayManager::WindupMatchWidget(
					SomeNextPlayer,
					FMatchScoreLampStateConfig(
						Color0MatchCount + Color1MatchCount - 1,
						tempWinDollColor,
						tempWinDollColor != SomeColor ? EMatchScoreLampState::EMSLS_Green : EMatchScoreLampState::EMSLS_Broken
					)
				);
			}
			//UGoSoccerPlayManager::ThrowContinueMatchWidget(
			//	SomePC, false, FMatchScoreLampStateConfig(
			//		Color0MatchCount + Color1MatchCount - 1,
			//		tempWinDollColor,
			//		tempWinDollColor == SomePlayerColor ? EMatchScoreLampState::EMSLS_Green : EMatchScoreLampState::EMSLS_Broken
			//	)
			//);
			//if (PlayGameMode == EPlayGameMode::EPGM_OnlineSession)
			//{
			//	uint8 SomeNextPlayerColor;
			//	GetPlayerColor(SomeNextPlayer, SomeNextPlayerColor);
			//	UGoSoccerPlayManager::ThrowContinueMatchWidget(
			//		SomeNextPlayer, false, FMatchScoreLampStateConfig(
			//			Color0MatchCount + Color1MatchCount - 1,
			//			tempWinDollColor,
			//			tempWinDollColor == SomeNextPlayerColor ? EMatchScoreLampState::EMSLS_Green : EMatchScoreLampState::EMSLS_Broken
			//		)
			//	);
			//}

			if (CurrentPlayingSessionData.PlayGameMode == EPlayGameMode::EPGM_HotSeatGame)
			{
				UGoSoccerPlayManager::PlayerWin_HotSeat(WinPlayer, tempWinDollColor);
			}
			else if (CurrentPlayingSessionData.PlayGameMode == EPlayGameMode::EPGM_SingleAIPlay)
			{
				if (tempWinDollColor == 0) UGoSoccerPlayManager::PlayerWin(WinPlayer);
				else UGoSoccerPlayManager::PlayerLose(WinPlayer);
			}
			else if (CurrentPlayingSessionData.PlayGameMode == EPlayGameMode::EPGM_OnlineSession)
			{
				if (UGoSoccerPlayManager::PlayerWin(WinPlayer))
				{
					auto* NP = GetNextPlayer(WinPlayer);
					UGoSoccerPlayManager::PlayerLose(NP);
				}
			}
			else return false;
			return true;
		}
	}
	return Execute_TurnStart(this, NextPlayer); // Continue Turn
}

bool AGoSoccerPlayMode::SpawnDoll_Implementation(UObject* PlayerObject, FVector SpawnLocation, uint8 DollColor, const EDollType& SpawnDollType)
{
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("Server_SpawnDoll: %s"), *SpawnLocation.ToString());
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, LogString);
	}
#endif
	APlayerController* Player = Cast<APlayerController>(PlayerObject);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Player;
	if (GetWorld() == nullptr || Player == nullptr) return false;
	AActor* PlacedDoll = nullptr;
	bool bBlackDoll = DollColor < 1;
	
	auto bFound = bBlackDoll ? Black_Doll_ClassMap.Find(SpawnDollType) : White_Doll_ClassMap.Find(SpawnDollType);
	PlacedDoll = bFound ? GetWorld()->SpawnActor<AActor>(*bFound, SpawnLocation, FRotator::ZeroRotator, SpawnParams) : nullptr;
	//switch (SpawnDollType)
	//{
	//case EDollType::EDT_Default:
	//	PlacedDoll = GetWorld()->SpawnActor<AActor>(bBlackDoll ? BlackDoll : WhiteDoll, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	//	break;
	//case EDollType::EDT_Explosive:
	//	PlacedDoll = GetWorld()->SpawnActor<AActor>(bBlackDoll ? BlackDoll_Explosive : WhiteDoll_Explosive, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	//	break;
	//default:
	//	break;
	//}

	if (PlacedDoll != nullptr)
	{
		UGoSoccerPlayManager::SetDollPlacedIndex(PlacedDoll, PlacedDollCounter);
		UGoSoccerPlayManager::SetDollFallenFromBoard(PlacedDoll, false);
		PlacedDollArr.Emplace(PlacedDoll);
		UF_Parent.Emplace(PlacedDollArr.Num() - 1);
		UF_Accessable.Emplace(TSet<int32>());
		UF_Visited.Emplace(false);
		PlacedDollCounter++;
		return TurnEnd_SpawnDoll(Player, PlacedDoll);
	}
	return false;
}

bool AGoSoccerPlayMode::DebugOnlyFunction_Implementation()
{
	return false;
}

bool AGoSoccerPlayMode::ClientLoadingCompleted_Implementation(UObject* PlayerObject)
{
	APlayerController* Player = Cast<APlayerController>(PlayerObject);
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("Server_ClientLoadingCompleted : %s"), *Player->GetFName().ToString());
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
	}
#endif
	Execute_RefreshPlayerCardArrWidgets(this);
	return (Player != nullptr);
}

int32 AGoSoccerPlayMode::GetPlacedDollCounter_Implementation()
{
	return PlacedDollCounter;
}

bool AGoSoccerPlayMode::MarkDollAsMoved_Implementation(UObject* DollObject)
{
	AActor* Doll = Cast<AActor>(DollObject);
	if (Doll == nullptr) return false;
#if WITH_EDITOR
	//DrawDebugSphere(GetWorld(), Doll->GetActorLocation(), 10.f, 32, FColor::Blue, false, 2.f);
#endif
	RecentlyMovedDollSet.Emplace(Doll);
	return true;
}

bool AGoSoccerPlayMode::GetGoBoardActor_Implementation(UObject*& OutGoBoardObject)
{
	OutGoBoardObject = GoBoardActor;
	return GoBoardActor != nullptr;
}

bool AGoSoccerPlayMode::WalkOverPlayer_Implementation(UObject* GaveUpPlayerObject)
{
	APlayerController* GiveupPlayer = Cast<APlayerController>(GaveUpPlayerObject);
	if (GiveupPlayer == nullptr) return false;
	APlayerController* WalkOverPlayer = GetNextPlayer(GiveupPlayer);
	if (WalkOverPlayer == nullptr) return false;
	UGoSoccerPlayManager::PlayerWin(WalkOverPlayer);
	UGoSoccerPlayManager::PlayerLose(GaveUpPlayerObject);
	return true;
}

bool AGoSoccerPlayMode::Notify_PlayerJoinedSessionComplete_Implementation(UObject* JoinedPlayerObject)
{
	if (JoinedPlayerObject == nullptr) return false;
	// 조인한 클라이언트가 이름/SteamID 를 서버에 올린 뒤 도착하는 통지.
	// 호스트를 포함한 모든 접속자의 PlayerCard 목록을 다시 만든다.
	Execute_RefreshPlayerCardArrWidgets(this);
	// 조인한 클라이언트는 호스트의 세션 설정을 모르므로 여기서 툴팁을 내려준다.
	Execute_Request_UpdateTooltip_SessionConfig(this, JoinedPlayerObject);
	return true;
}

bool AGoSoccerPlayMode::Notify_PlayerLeftSession_Implementation(UObject* LeftPlayerObject)
{
	UWorld* W = GetWorld();
	if (W == nullptr) return false;

	// Logout 시점에는 나간 플레이어의 PlayerState 가 아직 GameState->PlayerArray 에 남아 있다.
	// (Logout -> Controller 파괴 -> APlayerState::Destroyed -> RemovePlayerState 순서)
	// 지금 갱신하면 나간 사람의 카드가 그대로 다시 만들어지므로 다음 틱으로 미룬다.
	const TWeakObjectPtr<UObject> LeftPlayerWeak = LeftPlayerObject;
	W->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateWeakLambda(this, [this, LeftPlayerWeak]()
			{
				// 이미 파괴됐으면 Get() 이 nullptr 이고, 그 경우 PC 목록에도 남아 있지 않다.
				Execute_RefreshPlayerCardArrWidgets_Except(this, LeftPlayerWeak.Get());
			}
		)
	);
	return true;
}

bool AGoSoccerPlayMode::AIPlay_SpawnDollAtLocation_Implementation(UObject* NextPlayerObject, FVector PlaceLocation)
{
	// Thudo
	// Try Random Locations
	AIPlayingTurn = false;
	Execute_SpawnDoll(this, NextPlayerObject, PlaceLocation, ((uint8)1 ^ AIGame_PlayerDollColor), EDollType::EDT_Default);
	APlayerController* tempPC = Cast<APlayerController>(NextPlayerObject);
	//UGoSoccerPlayManager::AIPlay_ConsiderTurnPassed(NextPlayerObject);
	//Execute_WinCheck(this, tempPC);
	return NextPlayerObject != nullptr;
}

bool AGoSoccerPlayMode::AIPlay_FlickDoll_Implementation(UObject* NextPlayerObject, UObject* FlickDollObject, FVector FlickDirection)
{
	if (NextPlayerObject == nullptr || FlickDollObject == nullptr) return false;
	AIPlayingTurn = false;
	bool FlickResult = UGoSoccerPlayManager::FlickDoll(FlickDollObject, FlickDirection);
	if (!FlickResult)
	{
		AIPlayingTurn = true;
		return false;
	}
	//UGoSoccerPlayManager::AIPlay_ConsiderTurnPassed(NextPlayerObject);
	//TurnEnd_FlickDoll(tempPC);

	//Execute_TurnEnd_FlickDoll(this, tempPC);
	//Execute_WinCheck(this, tempPC);
	return NextPlayerObject != nullptr;
}

bool AGoSoccerPlayMode::AddFallenScore_Implementation(UObject* FallenDollObject)
{
	//AActor* tempDollActor = Cast<AActor>(FallenDollObject);
	if (FallenDollSet.Contains(FallenDollObject) || bTakeTurn) return false; // Using IsBoardStaionary To Score Instead In TakeTurn Mode
	FallenDollSet.Emplace(FallenDollObject);
	uint8 DollColor = 0;
	UGoSoccerPlayManager::GetDollColor(FallenDollObject, DollColor);
	AddFallenDollScore(DollColor);
	FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator();
	if (DollColor == 0)
	{
		Delegate_OnPlayerColor0_AddRealtimeColldownBonus.ExecuteIfBound(REALTIME_FALLEN_COOLDOWN_BONUS);
		Delegate_OnPlayerColor0_AddRealtimeColldownBonus_Flick.ExecuteIfBound(REALTIME_FALLEN_COOLDOWN_BONUS_FLICK);
	}
	if (DollColor == 1)
	{
		Delegate_OnPlayerColor1_AddRealtimeColldownBonus.ExecuteIfBound(REALTIME_FALLEN_COOLDOWN_BONUS);
		Delegate_OnPlayerColor1_AddRealtimeColldownBonus_Flick.ExecuteIfBound(REALTIME_FALLEN_COOLDOWN_BONUS_FLICK);
	}

	//if (PlayGameMode == EPlayGameMode::EPGM_HotSeatGame)
	//{
	//	bool bFlag = false;
	//	for (; PCIter; ++PCIter)
	//	{
	//		APlayerController* PC = PCIter->Get();
	//		FPlayerCard tempPlayerCard;
	//		if (UGoSoccerPlayManager::GetPlayerCard(PC->GetPlayerState<APlayerState>(), tempPlayerCard))
	//		{
	//			if (tempPlayerCard.PlayerDollColor != DollColor)
	//			{
	//				UGoSoccerPlayManager::AddRealtimeCooldownBonus(PC, 0.5f);
	//			}
	//		}
	//	}
	//}
	return true;
}

bool AGoSoccerPlayMode::DollBrokenAdvantage_Implementation(UObject* BrokenDollObject)
{
	//if (PlayingGameRule == EPlayGameMode::EPGM_HotSeatGame)
	//{
	//	// debug only
	//	uint8 tempDollColor;
	//	if (UGoSoccerPlayManager::GetDollColor(BrokenDollObject, tempDollColor))
	//	{
	//		if (tempDollColor == 1)
	//		{
	//		}
	//	}

	//}
	//else if (PlayingGameRule == EPlayGameMode::EPGM_OnlineSession)
	//{

	//}
	//else if (PlayingGameRule == EPlayGameMode::EPGM_SingleAIPlay)
	//{
	//	// AI yet developed
	//}
	return false;
}

bool AGoSoccerPlayMode::SetHostingSessionCreateData_Implementation(const FSessionCreateData& InHostingSessionCreateData)
{
	if (!InHostingSessionCreateData.IsValidSessionConfig()) return false;
	HostingSessionCreateData = InHostingSessionCreateData;
	return true;
}

bool AGoSoccerPlayMode::GetHostingSessionCreateData_Implementation(FSessionCreateData& OutHostingSessionCreateData)
{
	if (HostingSessionCreateData.IsValidSessionConfig())
	{
		OutHostingSessionCreateData = HostingSessionCreateData;
		return true;
	}
	// 로비에서 아직 설정을 바꾼 적이 없으면 호스트가 세션 생성에 쓴 원본을 가져온다.
	// GameMode 는 언제나 호스트(서버) 쪽에 있으므로 호스트의 GameInstance 를 그대로 참조할 수 있다.
	// (호스트의 PlayerState 는 리슨 서버 트래블 때 새로 만들어져서 값이 남아있지 않다)
	if (UGoSoccerPlayManager::GetHostingSessionCreateData_FromGameInstance(GetGameInstance(), OutHostingSessionCreateData))
	{
		HostingSessionCreateData = OutHostingSessionCreateData;
		return true;
	}
	return false;
}

bool AGoSoccerPlayMode::Notify_UpdateTooltip_SessionConfig_Implementation(const FSessionCreateData& InHostingSessionCreateData)
{
	if (!Execute_SetHostingSessionCreateData(this, InHostingSessionCreateData)) return false;
	UWorld* W = GetWorld();
	if (W == nullptr) return false;

	for (FConstPlayerControllerIterator PCIter = W->GetPlayerControllerIterator(); PCIter; ++PCIter)
	{
		APlayerController* PC = PCIter->Get();
		if (PC == nullptr) continue;
		UGoSoccerPlayManager::UpdateTooltip_SessionConfig(PC, InHostingSessionCreateData);
	}
	return true;
}

bool AGoSoccerPlayMode::Request_UpdateTooltip_SessionConfig_Implementation(UObject* PlayerControllerObject)
{
	FSessionCreateData HostSessionCreateData;
	if (!Execute_GetHostingSessionCreateData(this, HostSessionCreateData)) return false;
	return UGoSoccerPlayManager::UpdateTooltip_SessionConfig(PlayerControllerObject, HostSessionCreateData);
}

bool AGoSoccerPlayMode::AddChattingMessage_Implementation(const FText& InTextMessage, const FName& InPlayerName)
{
	auto PlayerIter = GetWorld()->GetPlayerControllerIterator();
	for (; PlayerIter; ++PlayerIter)
	{
		auto* PC = PlayerIter->Get();
		if (PC == nullptr) continue;
		UGoSoccerPlayManager::ReceiveChattingMessage(PC, InPlayerName, InTextMessage);
	}
	return false;
}

bool AGoSoccerPlayMode::GetNextDollType_Implementation(EDollType& OutNextDollType)
{
	auto GetSR = [&](const EDollType& InDollType)->float 
		{
			auto F = DollType_SpawnRate.Find(InDollType);
			if (F) return *F;
			return 0.f;
		};
	auto CalSR = [&](float& CurrRNG, const EDollType& CheckDollType, EDollType& OutDollType) -> bool
		{
			float CheckSR = GetSR(CheckDollType);
			if (CurrRNG < CheckSR)
			{
				OutDollType = CheckDollType;
				return true;
			}
			CurrRNG -= CheckSR;
			return false;
		};
	if (!CurrentPlayingSessionData.bEnableItem)
	{
		OutNextDollType = EDollType::EDT_Default;
		return false;
	}
	float RNG = FMath::FRandRange(0.f, TotalSpawnRateValue);
	if (CalSR(RNG, EDollType::EDT_Explosive, OutNextDollType)) return true;
	if (CalSR(RNG, EDollType::EDT_Beam, OutNextDollType)) return true;
	OutNextDollType = EDollType::EDT_Default;
	return true;
}

bool AGoSoccerPlayMode::ContinueMatch_Implementation()
{
	ResetPlayBoard(CurrentPlayingSessionData);

	bool bColor0Aheads = Color0MatchCount >= Color1MatchCount;

	APlayerController* StartPlayer = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (bColor0Aheads)
	{
		uint8 tempColor;
		//StartPlayer = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
		GetPlayerColor(StartPlayer, tempColor);
		if (tempColor != 0) StartPlayer = GetNextPlayer(StartPlayer);
	}
	return Execute_TurnStart(this, StartPlayer);
}

bool AGoSoccerPlayMode::IsBoardStationary()
{
	for (auto& DollIter : PlacedDollArr)
	{
		if (AActor* Doll = DollIter.Get())
		{
			if (!UGoSoccerPlayManager::IsGoDoll(Doll)) continue;
			if (GoBoardActor != nullptr && UGoSoccerPlayManager::IsDollFallen(GoBoardActor, Doll))
			{
				if (!FallenDollSet.Contains(Doll))
				{
					uint8 DollColor = 0;
					UGoSoccerPlayManager::GetDollColor(Doll, DollColor);
					AddFallenDollScore(DollColor);
					FallenDollSet.Emplace(Doll);
				}
				continue;
			}
			//if (Doll->GetVelocity().Size() > 1.f)
			if (!UGoSoccerPlayManager::IsDollConsideredStationary(Doll))
			{
				return false;
			}
		}
	}
	if (!bStationaryOnce)
	{
		bStationaryOnce = true;
		Stationary_StartTime = GetWorld()->GetTimeSeconds();
		return false;
	}
	else if (GetWorld()->GetTimeSeconds() > Stationary_StartTime + 1.f)
	{
		return true;
	}
	return false;
}

bool AGoSoccerPlayMode::AddWinPlayer(AActor* WinPlayer)
{
	if (!bTakeTurn)
	{
		// TODO Realtime Win Widget
		// Throw A Proper Widget To Winner(Soon) and Loser(Soon)
		// + Beep Sound, Warning Effects
		return true;
	}
	if (WinPlayerSet.Contains(WinPlayer)) return false;
	WinPlayerSet.Emplace(WinPlayer);
	return true;
}

bool AGoSoccerPlayMode::AddWinPlayerDollColor_HotSeat(uint8 WinPlayerDollColor)
{
	if (WinPlayerDollColorSet_HotSeat.Contains(WinPlayerDollColor)) return false;
	WinPlayerDollColorSet_HotSeat.Emplace(WinPlayerDollColor);
	return true;
}

APlayerController* AGoSoccerPlayMode::GetNextPlayer(APlayerController* LatestPlayer)
{
	// If More Than 2 Players, Must Be Fixed
	FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator();
	APlayerController* NextPlayerController = PCIter->Get();
	bool bFlag = false;
	for (; PCIter; ++PCIter)
	{
		APlayerController* PC = PCIter->Get();
		if (PC == nullptr) continue;
		if (bFlag)
		{
			NextPlayerController = PC;
		}
		if (PC == LatestPlayer)
		{
			bFlag = true;
		}
	}
	return NextPlayerController;
}

void AGoSoccerPlayMode::ResetMatchBoard(const FSessionCreateData& SessionCreateData)
{
	int32 SelectedMatchRule = static_cast<int32>(SessionCreateData.MatchRule);
	MatchCount = FMath::Max(1 + (SelectedMatchRule - 1) * 2, 0);
	Color0MatchCount = 0;
	Color1MatchCount = 0;
	for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; ++PCIter)
	{
		UGoSoccerPlayManager::SetMatchScoreLampCount(PCIter->Get(), MatchCount);
	}
}

void AGoSoccerPlayMode::ResetPlayBoard(const FSessionCreateData& SessionCreateData)
{
	auto JustSpawnDoll = [&](APlayerController* Player, uint8 SpawnDollColor, FVector SpawnLocation)
		->bool 
		{
			if (GetWorld() == nullptr || Player == nullptr) return false;
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = Player;
			AActor* PlacedDoll = GetWorld()->SpawnActor<AActor>(SpawnDollColor < 1 ? BlackDoll : WhiteDoll, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
			if (PlacedDoll != nullptr)
			{
				UGoSoccerPlayManager::SetDollPlacedIndex(PlacedDoll, PlacedDollCounter);
				UGoSoccerPlayManager::SetDollFallenFromBoard(PlacedDoll, false);
				PlacedDollArr.Emplace(PlacedDoll);
				UF_Parent.Emplace(PlacedDollArr.Num() - 1);
				UF_Accessable.Emplace(TSet<int32>());
				UF_Visited.Emplace(false);
				PlacedDollCounter++;
				return UF_JoinDollIntoGroup(PlacedDoll);
			}
			return false;
		};

	CurrentPlayingSessionData = SessionCreateData;
	PlacedDollCounter = 0;
	TurnCounter = 0;
	TotalSpawnRateValue = 0.f;
	//auto Doll_SR_Iter = DollType_SpawnRate.CreateConstIterator();
	for (auto Doll_SR_Iter : DollType_SpawnRate)
	{
		float tempSR = Doll_SR_Iter.Value;
#if UE_EDITOR
		UEnum* EnumPtr = StaticEnum<EDollType>();
		if (EnumPtr)
		{
			UE_LOG(LogTemp, Log, TEXT("%s : %.4f"), *EnumPtr->GetNameStringByValue((uint8)Doll_SR_Iter.Key), tempSR);
		}
#endif
		TotalSpawnRateValue += tempSR;
	}
#if UE_EDITOR
	UE_LOG(LogTemp, Log, TEXT("TotalSpawnRateValue : %.4f"), TotalSpawnRateValue);
#endif

	if (PlacedDollArr.Num() > 0)
	{
		for (auto& DollIter : PlacedDollArr)
		{
			if (!DollIter.IsValid()) continue;
			if (AActor* Doll = DollIter.Get())
			{
				Doll->Destroy();
			}
		}
		PlacedDollArr.Empty();
	}
	RecentlyMovedDollSet.Empty();
	WinPlayerDollColorSet_HotSeat.Empty();
	WinPlayerSet.Empty();
	FallenDollSet.Empty();
	UF_Parent.Empty();
	UF_Accessable.Empty();
	UF_Visited.Empty();
	FallenDollScore = 0;
	UpdateFallenDollScore();
	StreakContingDollMap.Empty();
	DollColorHashingMap.Empty();
	ScoreMap_Color.Empty();
	ScoringTime = false;
	ScorinBoardSequence = EScoreType::EST_Default;
	//DollTypeQueue.Empty();
	//if (SessionCreateData.bEnableItem && SessionCreateData.PlayTurnRule == EPlayTurnRule::EPTR_TakeTurn)
	//{
	//	DollTypeQueue.Enqueue(EDollType::EDT_Default);
	//	DollTypeQueue.Enqueue(EDollType::EDT_Default);
	//	DollTypeQueue.Enqueue(EDollType::EDT_Default);
	//	DollTypeQueue.Enqueue(EDollType::EDT_Default);
	//}
	//PlayerCardHashingMap.Empty();
	//GetWorld()->GetTimerManager().ClearTimer(TurnTimerHandle);

	Radian_Threshold = SessionCreateData.AngularThreshold;
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("ResetPlayBoard : Radian_Threshold : %f"), Radian_Threshold);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, LogString);
	}
#endif
	UE_LOG(LogTemp, Log, TEXT("ResetPlayBoard : Radian_Threshold : %f"), Radian_Threshold);

	Distance_Threshold = SessionCreateData.DistanceThreshold;
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("ResetPlayBoard : Distance_Threshold : %f"), Distance_Threshold);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, LogString);
	}
#endif
	UE_LOG(LogTemp, Log, TEXT("ResetPlayBoard : Distance_Threshold : %f"), Distance_Threshold);

	//if (SessionCreateData.BoardPreset == EBoardPreset::EBP_Free || SessionCreateData.BoardPreset == EBoardPreset::EBP_Default)
	TMap<FVector, uint8>* PresetArray = nullptr;
	if (SessionCreateData.BoardPreset == EBoardPreset::EBP_Korea)
	{
		PresetArray = &KoreaPreset;
	}
	else if (SessionCreateData.BoardPreset == EBoardPreset::EBP_China)
	{
		PresetArray = &ChinaPreset;
	}
	else if (SessionCreateData.BoardPreset == EBoardPreset::EBP_Tibet)
	{
		PresetArray = &TibetPreset;
	}

	WhiteDoll = SessionCreateData.PlayDollType == EPlayDollType::EPDT_Fragile ? WhiteDoll_Fragile : WhiteDoll_AntiFragile;
	BlackDoll = SessionCreateData.PlayDollType == EPlayDollType::EPDT_Fragile ? BlackDoll_Fragile : BlackDoll_AntiFragile;

	TMap<uint8, APlayerController*> DollColor_Controller;
	bool bIsOnlineSession = SessionCreateData.PlayGameMode == EPlayGameMode::EPGM_OnlineSession;
#if !UE_BUILD_SHIPPING
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
	UE_LOG(LogTemp, Log, TEXT("Board Preset : %s"), *DebugStringPreset);
	UE_LOG(LogTemp, Log, TEXT("bIsOnlineSession : %s"), bIsOnlineSession ? TEXT("True") : TEXT("False"));
#endif
	for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; ++PCIter)
	{
		APlayerController* PC = PCIter->Get();
		if (PC == nullptr) continue;
		
#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Log, TEXT("%d - PlayerController : %s"), PCIter.GetIndex(), *PC->GetFName().ToString());
#endif
		if (bIsOnlineSession)
		{
			APlayerState* PS = PC->GetPlayerState<APlayerState>();
			if (PS == nullptr) continue;
			FPlayerCard tempPlayerCard;
			if (IIGoSoccerPlayerState::Execute_GetPlayerCard(PS, tempPlayerCard))
			{
#if !UE_BUILD_SHIPPING
				UE_LOG(LogTemp, Log, TEXT("DollColor : %d"), tempPlayerCard.PlayerDollColor);
#endif
				DollColor_Controller.Add(tempPlayerCard.PlayerDollColor, PC);
			}
		}
		else
		{
			DollColor_Controller.Add(0, PC);
			DollColor_Controller.Add(1, PC);
		}
		UGoSoccerPlayManager::SendCurrentSessionData(PC, SessionCreateData);
	}

	if (PresetArray != nullptr)
	{
		for (auto Iter = PresetArray->CreateConstIterator(); Iter; ++Iter)
		{
			FVector PlaceLocation = Iter->Key;
			uint8 DollColor = Iter->Value;
			APlayerController** PC = DollColor_Controller.Find(DollColor);
			if (PC == nullptr) continue;
			JustSpawnDoll(*PC, DollColor, PlaceLocation);
		}
	}

	bTakeTurn = SessionCreateData.PlayTurnRule == EPlayTurnRule::EPTR_TakeTurn;

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("bTakeTurn : %s"), bTakeTurn ? TEXT("True") : TEXT("False"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, LogString);
		UE_LOG(LogTemp, Log, TEXT("bTakeTurn : %s"), bTakeTurn ? TEXT("True") : TEXT("False"));
	}
#endif

	if (!bTakeTurn && GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(Realtime_BoardCheck_TimerHandle);
		GetWorld()->GetTimerManager().SetTimer(Realtime_BoardCheck_TimerHandle, this, &AGoSoccerPlayMode::Realtime_CheckStreak5, REALTIME_BOARDCHECK_INTERVAL, true, 3.f);

		//double StartTimeStamp = GetWorld()->GetTimeSeconds();
		auto* GS = GetGameState<AGameStateBase>();
		if (GS == nullptr)
		{
			// TODO Exceptions
			return;
		}
		StartTimeStamp = GS->GetServerWorldTimeSeconds();
		TimeLimit = SessionCreateData.TimeLimit;
		auto PCIter = GetWorld()->GetPlayerControllerIterator();
		for (; PCIter; ++PCIter)
		{
			APlayerController* PC = PCIter->Get();
			if (PC == nullptr) continue;
			FPlayerCard tempPlayerCard;
			auto* PS = PC->GetPlayerState<APlayerState>();
			if (UGoSoccerPlayManager::GetPlayerCard(PS, tempPlayerCard))
			{
				if (SessionCreateData.PlayTurnRule == EPlayTurnRule::EPTR_RealTime && SessionCreateData.TimeLimit > 0.f)
				{
					UGoSoccerPlayManager::StartGameTimer(PC, StartTimeStamp, SessionCreateData.TimeLimit);
				}
#if !UE_BUILD_SHIPPING
				if (GEngine)
				{
					FString LogString = FString::Printf(TEXT("ResetPlayBoard !TakeTurn - Bind Delegate(%d) - %s"), tempPlayerCard.PlayerDollColor, *PC->GetFName().ToString());
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, LogString);
					UE_LOG(LogTemp, Warning, TEXT("ResetPlayBoard !TakeTurn - Bind Delegate(%d) - %s"), tempPlayerCard.PlayerDollColor, *PC->GetFName().ToString());
				}
#endif
				if (tempPlayerCard.PlayerDollColor == 0)
				{
					Delegate_OnPlayerColor0_SetMinousHUDPercet.Unbind();
					Delegate_OnPlayerColor0_SetMinousHUDPercet.BindUFunction(PC, TEXT("OnSetOminousHUDPercent"));
					Delegate_OnPlayerColor0_SetMinousHUDPercet.ExecuteIfBound(0.f, REALTIME_STREAK_MAINTAIN_TIMESEC);

					Delegate_OnPlayerColor0_AddRealtimeColldownBonus.Unbind();
					Delegate_OnPlayerColor0_AddRealtimeColldownBonus.BindUFunction(PC, TEXT("OnAddRealtimeCooldownBonus"));
					Delegate_OnPlayerColor0_AddRealtimeColldownBonus.ExecuteIfBound(-1.f);

					Delegate_OnPlayerColor0_AddRealtimeColldownBonus_Flick.Unbind();
					Delegate_OnPlayerColor0_AddRealtimeColldownBonus_Flick.BindUFunction(PC, TEXT("OnAddRealtimeCooldownBonus_Flick"));
					Delegate_OnPlayerColor0_AddRealtimeColldownBonus_Flick.ExecuteIfBound(-1.f);

				}
				else if (tempPlayerCard.PlayerDollColor == 1)
				{
					Delegate_OnPlayerColor1_SetMinousHUDPercet.Unbind();
					Delegate_OnPlayerColor1_SetMinousHUDPercet.BindUFunction(PC, TEXT("OnSetOminousHUDPercent"));
					Delegate_OnPlayerColor1_SetMinousHUDPercet.ExecuteIfBound(0.f, REALTIME_STREAK_MAINTAIN_TIMESEC);

					Delegate_OnPlayerColor1_AddRealtimeColldownBonus.Unbind();
					Delegate_OnPlayerColor1_AddRealtimeColldownBonus.BindUFunction(PC, TEXT("OnAddRealtimeCooldownBonus"));
					Delegate_OnPlayerColor1_AddRealtimeColldownBonus.ExecuteIfBound(-1.f);

					Delegate_OnPlayerColor1_AddRealtimeColldownBonus_Flick.Unbind();
					Delegate_OnPlayerColor1_AddRealtimeColldownBonus_Flick.BindUFunction(PC, TEXT("OnAddRealtimeCooldownBonus_Flick"));
					Delegate_OnPlayerColor1_AddRealtimeColldownBonus_Flick.ExecuteIfBound(-1.f);
				}
			}
			else
			{
#if !UE_BUILD_SHIPPING
				if (GEngine)
				{
					FString LogString = FString::Printf(TEXT("ResetPlayBoard !TakeTurn - Failed To Load PlayerCard - %s"), *PC->GetFName().ToString());
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, LogString);
					UE_LOG(LogTemp, Warning, TEXT("Failed To Load PlayerCard - %s"), *PC->GetFName().ToString());
				}
#endif
			}
		}
	}
}

int32 AGoSoccerPlayMode::UF_Find(int32 Doll_idx)
{
	if (!UF_Parent.IsValidIndex(Doll_idx)) return INDEX_NONE;
	if (UF_Parent[Doll_idx] == Doll_idx)
	{
		return Doll_idx;
	}
	return UF_Parent[Doll_idx] = UF_Find(UF_Parent[Doll_idx]);
}

void AGoSoccerPlayMode::UF_Union(int32 Doll_idx_i, int32 Doll_idx_j)
{
	int32 Root_i = UF_Find(Doll_idx_i);
	int32 Root_j = UF_Find(Doll_idx_j);

	if (Root_i < Root_j)
	{
		UF_Parent[Root_j] = Root_i;
	}
	else if (Root_i > Root_j)
	{
		UF_Parent[Root_i] = Root_j;
	}
}

bool AGoSoccerPlayMode::UF_GroupStreakCheck(int32 RootIdx)
{
	TArray<int32> CheckDollArr;
	for (int32 idx = 0; idx < PlacedDollArr.Num(); idx++)
	{
		if (!PlacedDollArr.IsValidIndex(idx) || !UF_Parent.IsValidIndex(idx) || !UF_Accessable.IsValidIndex(idx)) continue;
		AActor* CheckDoll = PlacedDollArr[idx].Get();
		if (FallenDollSet.Contains(CheckDoll)) continue;
		if (UF_Find(idx) == RootIdx)
		{
			CheckDollArr.Emplace(idx);
		}
	}

	for (int32 Iter : CheckDollArr)
	{
		if (!UF_Visited.IsValidIndex(Iter)) continue;
		UF_Visited[Iter] = true;
		TArray<AActor*> WinDollArr;
		if (GoBoardActor && Travel(Iter, 0.f, 0, WinDollArr))
		{
#if !UE_BUILD_SHIPPING
			//for (const auto& Pos : WinDollPosArr)
			//{
			//	DrawDebugSphere(GetWorld(), Pos + FVector(0.f, 0.f, 5.f), 3.f, 32, FColor::Green, false, 5.f);
			//}
#endif
			//UGoSoccerPlayManager::SetDollStreakSpline(GoBoardActor, WinDollPosArr);
			FStreakDollContainer StreakDollContainer{PlacedDollArr.Num() / 64 + 1};
			for (auto* WinDoll_Iter : WinDollArr)
			{
				if (bTakeTurn)
				{
					UGoSoccerPlayManager::SetWinDollCircularEffect(WinDoll_Iter, true);
				}
				else
				{
					StreakDollContainer[PlacedDollArr.IndexOfByKey(WinDoll_Iter)] = true;
					//WinDoll_IndexArr.AddUnique(PlacedDollArr.IndexOfByKey(WinDoll_Iter));
				}
			}
			if (!bTakeTurn) Realtime_StartCount(StreakDollContainer);
			else break;
		}
		UF_Visited[Iter] = false;
	}
	return false;
}

bool AGoSoccerPlayMode::Travel(int32 idx, float StartRadian, int32 depth, TArray<AActor*>& WinDollArr)
{
	if (!PlacedDollArr.IsValidIndex(idx)) return false;
	AActor* CurrentDoll = PlacedDollArr[idx].Get();
	if (CurrentDoll == nullptr || CurrentDoll->Owner == nullptr) return false;
	FVector CurrLocation = CurrentDoll->GetActorLocation();
	uint8 CurrentDollColor;
	UGoSoccerPlayManager::GetDollColor(CurrentDoll, CurrentDollColor);

	int32 DepthThreshold = 3;
	if (PlayingGameRule == EPlayGameRule::EPGR_5plus5Mode)
	{
#if !UE_BUILD_SHIPPING
		//UE_LOG(LogTemp, Log, TEXT("5plus5Mode Current FallenDollScore : %d"), FallenDollScore);
#endif
		if (CurrentDollColor == 0 && FallenDollScore > 0 ||
			CurrentDollColor == 1 && FallenDollScore < 0)
		{
			DepthThreshold -= FMath::Abs(FallenDollScore);
		}
	}
	if (depth > DepthThreshold)
	{
#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Log, TEXT("Player %s Win"), *CurrentDoll->Owner.GetFName().ToString());
#endif
		WinDollArr.Emplace(CurrentDoll);
		if (!bTakeTurn)
		{
			return AddWinPlayer(CurrentDoll->Owner);
		}
		else if (PlayGameMode != EPlayGameMode::EPGM_HotSeatGame && PlayGameMode != EPlayGameMode::EPGM_SingleAIPlay)
		{
			return AddWinPlayer(CurrentDoll->Owner);
		}
		else
		{
			return AddWinPlayerDollColor_HotSeat(CurrentDollColor);
		}
		//return PlayGameMode != EPlayGameMode::EPGM_HotSeatGame && PlayGameMode != EPlayGameMode::EPGM_SingleAIPlay
		//	? AddWinPlayer(CurrentDoll->Owner) : AddWinPlayerDollColor_HotSeat(CurrentDollColor);
	}
	if (UF_Accessable.IsValidIndex(idx))
	{
		for (int32 Iter_Acc : UF_Accessable[idx])
		{
			if (!UF_Visited.IsValidIndex(Iter_Acc)) continue;
			if (UF_Visited[Iter_Acc]) continue;
			if (!PlacedDollArr.IsValidIndex(Iter_Acc)) continue;
			AActor* ToDoll = PlacedDollArr[Iter_Acc].Get();
			if (ToDoll == nullptr || ToDoll->Owner == nullptr) continue;
			if (FallenDollSet.Contains(ToDoll))
			{
				continue;
			}
			FVector DestLocation = ToDoll->GetActorLocation();
			float Distance = FVector::Dist2D(CurrLocation, DestLocation);
			if (!IsDollInRange(idx, Iter_Acc)) continue;
			FVector Direction = (DestLocation - CurrLocation).GetSafeNormal2D();
			float DestRadian = FMath::Atan2(Direction.Y, Direction.X);

			if (depth != 0)
			{
				float Delta = FMath::FindDeltaAngleRadians(StartRadian, DestRadian);
				if (FMath::Abs(Delta) > Radian_Threshold) continue;
			}
			UF_Visited[Iter_Acc] = true;
			bool bWin = Travel(Iter_Acc, depth == 0 ? DestRadian : StartRadian, depth + 1, WinDollArr);
#if !UE_BUILD_SHIPPING
			DrawDebugLine(GetWorld(), CurrLocation + FVector(0.f, 0.f, depth + 1.f), DestLocation + FVector(0.f, 0.f, depth + 2.f), bWin ? FColor::Green : FColor::Red, false, 5.f);
#endif
			UF_Visited[Iter_Acc] = false;
			if (bWin)
			{
				WinDollArr.Emplace(CurrentDoll);
				//WinDollPosArr.Emplace(CurrentDoll->GetActorLocation());
				return bWin;
			}
		}
	}
	return false;
}

bool AGoSoccerPlayMode::IsDollInRange(int32 x, int32 y)
{
	if (!PlacedDollArr.IsValidIndex(x) || !PlacedDollArr.IsValidIndex(y)) return false;
	AActor* X = PlacedDollArr[x].Get();
	AActor* Y = PlacedDollArr[y].Get();
	if (X == nullptr || Y == nullptr) return false;
	FVector X_Loc = X->GetActorLocation();
	FVector Y_Loc = Y->GetActorLocation();
	FVector Diff = X_Loc - Y_Loc;
	return FMath::Abs(Diff.X) <= Distance_Threshold && FMath::Abs(Diff.Y) <= Distance_Threshold;
}

bool AGoSoccerPlayMode::UF_JoinDollIntoGroup(AActor* NewDoll)
{
	if (NewDoll == nullptr) return false;
	int32 P_idx = PlacedDollArr.IndexOfByKey(NewDoll);
	if (!UF_Accessable.IsValidIndex(P_idx))
	{
		return false;
	}
	for (int32 idx = 0; idx < PlacedDollArr.Num(); idx++)
	{
		if (!PlacedDollArr.IsValidIndex(idx) || !UF_Parent.IsValidIndex(idx) || !UF_Accessable.IsValidIndex(idx)) continue;
		auto& PlacedDoll_Iter = PlacedDollArr[idx];
		if (!PlacedDoll_Iter.IsValid()) continue;
		AActor* Doll_Iter = PlacedDoll_Iter.Get();
		if (Doll_Iter == nullptr || Doll_Iter == NewDoll) continue;
		FVector P_Location = NewDoll->GetActorLocation();
		FVector I_Location = Doll_Iter->GetActorLocation();
		float DollDist = FVector::Dist2D(P_Location, I_Location);

		bool bIsSamePlayer = (NewDoll->Owner == Doll_Iter->Owner);
		if (PlayGameMode == EPlayGameMode::EPGM_HotSeatGame || PlayGameMode == EPlayGameMode::EPGM_SingleAIPlay)
		{
			uint8 NewDollColor = 0;
			uint8 Doll_IterColor = 0;
			UGoSoccerPlayManager::GetDollColor(NewDoll, NewDollColor);
			UGoSoccerPlayManager::GetDollColor(Doll_Iter, Doll_IterColor);
			bIsSamePlayer = (NewDollColor == Doll_IterColor);
		}

		if (
			IsDollInRange(P_idx, idx) && 
			NewDoll->Owner != nullptr && 
			bIsSamePlayer
			)
		{
			UF_Union(P_idx, idx);
			UF_Accessable[P_idx].Add(idx);
			UF_Accessable[idx].Add(P_idx);
		}
	}
	return UF_GroupStreakCheck(UF_Find(P_idx));
}

void AGoSoccerPlayMode::AddFallenDollScore(uint8 InDollColor)
{
	FallenDollScore += InDollColor == 0 ? -1 : 1;
	UpdateFallenDollScore();

	if (FMath::Abs(FallenDollScore) > 4)
	{
		if (PlayGameMode == EPlayGameMode::EPGM_HotSeatGame || PlayGameMode == EPlayGameMode::EPGM_SingleAIPlay)
		{
			AddWinPlayerDollColor_HotSeat((InDollColor + 1) % 2);
		}
		else
		{
			FConstPlayerControllerIterator PC_Iter = GetWorld()->GetPlayerControllerIterator();
			for (; PC_Iter; ++PC_Iter)
			{
				APlayerController* PC = PC_Iter->Get();
				if (PC == nullptr) continue;
				APlayerState* PS = PC->GetPlayerState<APlayerState>();
				if (PS == nullptr) continue;
				FPlayerCard tempPlayerCard;
				IIGoSoccerPlayerState::Execute_GetPlayerCard(PS, tempPlayerCard);
				if (tempPlayerCard.PlayerDollColor != InDollColor)
				{
					AddWinPlayer(PC);
				}
			}
		}
	}
}

void AGoSoccerPlayMode::UpdateFallenDollScore()
{
	FConstPlayerControllerIterator Iter = GetWorld()->GetPlayerControllerIterator();
	UE_LOG(LogTemp, Log, TEXT("UpdateFallenDollScore : %d"), FallenDollScore);
	for (; Iter; ++Iter)
	{
		APlayerController* PC = Iter->Get();
		if (PC == nullptr) continue;
		UGoSoccerPlayManager::UpdateFallenDollScore(PC, FMath::Min(FallenDollScore, 5));
	}
}

bool AGoSoccerPlayMode::ImmediateWinPlayer_LoseAnother(uint8 InColor, bool bDraw)
{
	FConstPlayerControllerIterator Iter = GetWorld()->GetPlayerControllerIterator();
	for (; Iter; ++Iter)
	{
		APlayerController* PC = Iter->Get();
		if (PC == nullptr) continue;
		uint8 PCDollColor;
		if (PlayGameMode == EPlayGameMode::EPGM_HotSeatGame)
		{
			if (UGoSoccerPlayManager::PlayerWin_HotSeat(PC, InColor))
			{
				GameSet();
				return true;
			}
		}
		if (GetPlayerColor(PC, PCDollColor) && InColor == PCDollColor)
		{
			bool bFlag = bDraw ? UGoSoccerPlayManager::PlayerDraw(PC) : UGoSoccerPlayManager::PlayerWin(PC);
			if (bFlag)
			{
				if (PlayGameMode != EPlayGameMode::EPGM_OnlineSession) return true;
				auto NextPC = GetNextPlayer(PC);
				bFlag = bDraw ? UGoSoccerPlayManager::PlayerDraw(NextPC) : UGoSoccerPlayManager::PlayerLose(NextPC);
				if (bFlag)
				{
					GameSet();
					return true;
				}
			}
		}
	}
	return false;
}

void AGoSoccerPlayMode::AITurnStart(APlayerController* WaitPlayer)
{
	 /*
	 TODO Thudo
	 0. !!! In ASync !!!
-40% 1. Calculate Place
	1.5. Awaits
	 2. Place Doll or Flick Doll
	 3. Call Execute_TurnStart(WaitPlayer)
	 */
	float U;
	float D;
	float L;
	float R;
	UGoSoccerPlayManager::GetGoBoardBorderLine(GoBoardActor, U, D, L, R);
	UGoSoccerAISubsystem* SubSystem = GetWorld()->GetSubsystem<UGoSoccerAISubsystem>();	
	if (SubSystem)
	{
		SubSystem->Request_Calculate(
			WaitPlayer, PlacedDollArr,
			Distance_Threshold, Radian_Threshold,
			U, D, L, R, ((uint8)1 ^ AIGame_PlayerDollColor),
			PlayingGameRule == EPlayGameRule::EPGR_5or5Mode
			? (AIGame_PlayerDollColor == 0
				? -FallenDollScore : FallenDollScore) : 0
		);
	}
}

void AGoSoccerPlayMode::Realtime_CheckStreak5()
{
	//if (RecentlyMovedDollSet.IsEmpty()) return;
	//UE_LOG(LogTemp, Log, TEXT("Realtime_CheckStreak5 : FallenDollScore : %d"), FallenDollScore);
	//if (FallenDollScore >= 5 || FallenDollScore <= -5)
	//{
	//	if (FallenDollScoreExceededTimeStamp > REALTIME_STREAK_MAINTAIN_TIMESEC)
	//	{
	//		if (ImmediateWinPlayer_LoseAnother(FallenDollScore > 0 ? 0 : 1))
	//		{
	//			GetWorld()->GetTimerManager().ClearTimer(Realtime_BoardCheck_TimerHandle);
	//		}
	//	}
	//	else 
	//	{
	//		FallenDollScoreExceededTimeStamp += REALTIME_BOARDCHECK_INTERVAL;
	//		if (FallenDollScore >= 5)
	//		{
	//			if (PlayGameMode == EPlayGameMode::EPGM_HotSeatGame)
	//			{
	//				/*Debug*/
	//				Delegate_OnPlayerColor0_SetMinousHUDPercet.ExecuteIfBound(FallenDollScoreExceededTimeStamp, REALTIME_STREAK_MAINTAIN_TIMESEC);
	//			}
	//			else Delegate_OnPlayerColor1_SetMinousHUDPercet.ExecuteIfBound(FallenDollScoreExceededTimeStamp, REALTIME_STREAK_MAINTAIN_TIMESEC);
	//		}
	//		else if (FallenDollScore <= -5)
	//		{
	//			Delegate_OnPlayerColor0_SetMinousHUDPercet.ExecuteIfBound(FallenDollScoreExceededTimeStamp, REALTIME_STREAK_MAINTAIN_TIMESEC);
	//		}
	//	}
	//}
	//else if (FallenDollScoreExceededTimeStamp > 0.f)
	//{
	//	FallenDollScoreExceededTimeStamp = 0.f;
	//	Delegate_OnPlayerColor0_SetMinousHUDPercet.ExecuteIfBound(FallenDollScoreExceededTimeStamp, REALTIME_STREAK_MAINTAIN_TIMESEC);
	//	Delegate_OnPlayerColor1_SetMinousHUDPercet.ExecuteIfBound(FallenDollScoreExceededTimeStamp, REALTIME_STREAK_MAINTAIN_TIMESEC);
	//}

/*
////////////////////////////////////////////////////////////////////////// Game Timer Over Start
*/
	if (GetWorld()->GetTimeSeconds() - StartTimeStamp >= TimeLimit)
	{
		if (!ScoringTime)
		{
			UE_LOG(LogTemp, Log, TEXT("Scoring Time Init"));
			if (PlayGameMode == EPlayGameMode::EPGM_HotSeatGame || PlayGameMode == EPlayGameMode::EPGM_SingleAIPlay)
			{
				auto PCIter = GetWorld()->GetPlayerControllerIterator();
				auto* PC = PCIter->Get();
				if (PC)
				{
					FName PlayerName;
					if (!UGoSoccerPlayManager::GetPlayerName(PC->GetPlayerState<APlayerState>(), PlayerName)) PlayerName = PC->GetFName();
					uint8 DollColor;
					GetPlayerColor(PC, DollColor);
					ScoreMap_Color.Add(0, FScoreBoard{ 0, DollColor == 0 ? PlayerName : FName("Black") });
					ScoreMap_Color.Add(1, FScoreBoard{ 1, DollColor == 1 ? PlayerName : FName("White") });
					TArray<FScoreBoard> ScoreArr;
					ScoreMap_Color.GenerateValueArray(ScoreArr);
					UGoSoccerPlayManager::ThrowScoringBoard(PC, ScoreArr);
					UGoSoccerPlayManager::TurnEnd(PC);
					ScoringTime = true;
				}
			}
			else
			{
				auto PCIter = GetWorld()->GetPlayerControllerIterator();
				for (; PCIter; ++PCIter)
				{
					if (!PCIter->IsValid()) continue;
					auto* PC = PCIter->Get();
					//UGoSoccerPlayManager::GetSteamID
					uint32 UniqueID = PC->GetUniqueID();
					UE_LOG(LogTemp, Log, TEXT("[%d] Unique ID : %d - %s"), PCIter.GetIndex(), UniqueID, *PC->GetFName().ToString());
					uint8 DollColor;
					if (!GetPlayerColor(PC, DollColor)) continue;
					UE_LOG(LogTemp, Log, TEXT("[%d] DollColor : %d - %s"), PCIter.GetIndex(), DollColor, *PC->GetFName().ToString());
					FName PlayerName;
					if (!UGoSoccerPlayManager::GetPlayerName(PC->GetPlayerState<APlayerState>(), PlayerName)) PlayerName = PC->GetFName();
					ScoreMap_Color.Add(DollColor, FScoreBoard{ DollColor, PlayerName });
					UGoSoccerPlayManager::TurnEnd(PC);
					ScoringTime = true;
				}
				TArray<FScoreBoard> ScoreArr;
				ScoreMap_Color.GenerateValueArray(ScoreArr);
				auto anotherPCIter = GetWorld()->GetPlayerControllerIterator();
				for (; anotherPCIter; ++anotherPCIter)
				{
					if (!anotherPCIter->IsValid()) continue;
					auto* PC = anotherPCIter->Get();
					UGoSoccerPlayManager::ThrowScoringBoard(PC, ScoreArr);
				}
			}
			if (ScoringTime)
			{
				ScoreTemp_PlacedDollArr = PlacedDollArr;
				GetWorld()->GetTimerManager().ClearTimer(Realtime_BoardCheck_TimerHandle);
				GetWorld()->GetTimerManager().SetTimer(Realtime_BoardCheck_TimerHandle, this, &AGoSoccerPlayMode::Realtime_CheckStreak5, REALTIME_BOARDCHECK_INTERVAL);
			}
			return;
		}

		auto DollIter = ScoreTemp_PlacedDollArr.CreateIterator();
		for (; DollIter; ++DollIter)
		{
			if (!DollIter->IsValid()) continue;
			auto* DollActor = DollIter->Get();
			if (DollActor == nullptr) continue;
			float CurrHP = 999.f;
			UGoSoccerPlayManager::GetDollHP(DollActor, CurrHP);
			bool bBroken = CurrHP <= 0.f;
			bool bFall = UGoSoccerPlayManager::IsDollFallen(GoBoardActor, DollActor);
			uint8 DollColor;
			bool bDollColorFlag = UGoSoccerPlayManager::GetDollColor(DollActor, DollColor);
			UE_LOG(LogTemp, Log, TEXT("[%d - %d] %s HP : %f"),
				DollIter.GetIndex(),
				DollColor,
				bFall ? (
					CurrHP > 0.f ? TEXT("Fall") : TEXT("Broken")
					) : TEXT("OnBoard"),
				CurrHP);
			if (!bDollColorFlag)
			{
				UE_LOG(LogTemp, Warning, TEXT("DollColor Not Found"));
			}
#if UE_EDITOR
			DrawDebugSphere(GetWorld(), DollActor->GetActorLocation() + FVector(0.f, 0.f, 5.f), 5.f, 32, FColor::Green, false, REALTIME_BOARDCHECK_INTERVAL * 10.f);
#endif
			for (auto& KV : ScoreMap_Color)
			{
				EScoreType ScoreType = EScoreType::EST_Default;
				float UpdatedScore = 0.f;
				if (bFall && DollColor != KV.Key)
				{
					if (bBroken)
					{
						KV.Value.BreakScore += 1;
						UpdatedScore = KV.Value.BreakScore;
						ScoreType = EScoreType::EST_BreakScore;
					}
					else 
					{
						KV.Value.FallScore += 1;
						UpdatedScore = KV.Value.FallScore;
						ScoreType = EScoreType::EST_FallScore;
					}
				}
				else if (!bFall && DollColor == KV.Key)
				{
					KV.Value.OnBoardScore += 1;
					UpdatedScore = KV.Value.OnBoardScore;
					ScoreType = EScoreType::EST_OnBoardScore;
				}
				if (ScoreType == EScoreType::EST_Default) continue;
				UGoSoccerPlayManager::SetDollStencil_Scoring(DollActor);

				/*
				auto PCIter = GetWorld()->GetPlayerControllerIterator();
				for (; PCIter; ++PCIter)
				{
					if (!PCIter->IsValid()) continue;
					auto* PC = PCIter->Get();
					if (PC == nullptr) continue;
					UGoSoccerPlayManager::UpdateScoringBoard(PC, bFall ? KV.Key : DollColor, ScoreType, UpdatedScore);
				}
				*/
			}
			DollIter.RemoveCurrent();
#if UE_EDITOR
			for (auto KV : ScoreMap_Color)
			{
				UE_LOG(LogTemp, Log, TEXT("%d - OnBoard : %d, Fall : %d, Broken : %d"), KV.Key, KV.Value.OnBoardScore, KV.Value.FallScore, KV.Value.BreakScore);
			}
#endif
			break;
		}


		//GetWorld()->GetTimerManager().ClearTimer(Realtime_BoardCheck_TimerHandle);
		if (ScoreTemp_PlacedDollArr.Num() > 0)
		{
			float Remnant = 1.f - ScoreTemp_PlacedDollArr.Num() / (float)PlacedDollArr.Num();
			float Delta = FMath::Lerp(REALTIME_BOARDCHECK_INTERVAL / 5.f, REALTIME_BOARDCHECK_INTERVAL * 3.f, FMath::Pow(Remnant, 5.f));
			UE_LOG(LogTemp, Log, TEXT("Remnant : %f -> %f sec Num : %d, MaxNum : %d"),
				Remnant, Delta, ScoreTemp_PlacedDollArr.Num(), PlacedDollArr.Num());
			GetWorld()->GetTimerManager().SetTimer(
				Realtime_BoardCheck_TimerHandle,
				this, &AGoSoccerPlayMode::Realtime_CheckStreak5,
				Delta
			);
		}
		else
		{
			GetWorld()->GetTimerManager().ClearTimer(Realtime_BoardCheck_TimerHandle);
			auto UpdateScore = [&](EScoreType InScoreType)->void 
				{
					auto PCIter = GetWorld()->GetPlayerControllerIterator();
					for (; PCIter; ++PCIter)
					{
						if (!PCIter->IsValid()) continue;
						auto* PC = PCIter->Get();
						if (PC == nullptr) continue;
						uint8 PlayerDollColor;
						GetPlayerColor(PC, PlayerDollColor);
						for (auto& KV : ScoreMap_Color)
						{
							int32 UpdateScore = 0;
							if (InScoreType == EScoreType::EST_OnBoardScore) UpdateScore = KV.Value.OnBoardScore;
							else if (InScoreType == EScoreType::EST_FallScore) UpdateScore = KV.Value.FallScore;
							else if (InScoreType == EScoreType::EST_BreakScore) UpdateScore = KV.Value.BreakScore;
							else if (InScoreType == EScoreType::EST_TotalScore)
							{
								UpdateScore += KV.Value.OnBoardScore;
								UpdateScore += KV.Value.FallScore;
								UpdateScore += KV.Value.BreakScore;
							}
#if UE_EDITOR
							auto* SE = StaticEnum<EScoreType>();
							if (SE)
							{
								UE_LOG(LogTemp, Log, TEXT("Updating - Color - %d (%s), score : %d"), KV.Key, *SE->GetDisplayNameTextByValue((int64)InScoreType).ToString(), UpdateScore);
							}
#endif
							UGoSoccerPlayManager::UpdateScoringBoard(
								PC, KV.Key,
								InScoreType,
								UpdateScore
							);
						}
					}
				};
			if (ScorinBoardSequence == EScoreType::EST_Default)				ScorinBoardSequence = EScoreType::EST_OnBoardScore;
			else if (ScorinBoardSequence == EScoreType::EST_OnBoardScore)	ScorinBoardSequence = EScoreType::EST_FallScore;
			else if (ScorinBoardSequence == EScoreType::EST_FallScore)		ScorinBoardSequence = EScoreType::EST_BreakScore;
			else if (ScorinBoardSequence == EScoreType::EST_BreakScore)		ScorinBoardSequence = EScoreType::EST_TotalScore;
			else if (ScorinBoardSequence == EScoreType::EST_TotalScore)		ScorinBoardSequence = EScoreType::EST_Default;
			if (ScorinBoardSequence != EScoreType::EST_Default)
			{
				UpdateScore(ScorinBoardSequence);
				GetWorld()->GetTimerManager().SetTimer(
					Realtime_BoardCheck_TimerHandle,
					this, &AGoSoccerPlayMode::Realtime_CheckStreak5,
					ScorinBoardSequence != EScoreType::EST_TotalScore ? 1.f : 2.f
				);
			}
			else
			{
				uint8 WinColor = 0;
				int32 TopScore = -999;
				int32 CheckedIdx = 0;
				bool bDrawFlag = false;
				for (auto& KV : ScoreMap_Color)
				{
					CheckedIdx++;
					int32 TotlaScore = KV.Value.BreakScore + KV.Value.FallScore + KV.Value.OnBoardScore;
					if (TotlaScore > TopScore)
					{
						TopScore = TotlaScore;
						WinColor = KV.Key;
					}
					else if (CheckedIdx == ScoreMap_Color.Num() && TotlaScore == TopScore)
					{
						bDrawFlag = true;
					}
				}
				ImmediateWinPlayer_LoseAnother(WinColor, bDrawFlag);
				//GetWorld()->GetTimerManager().ClearTimer(Realtime_BoardCheck_TimerHandle);
			}
		}
		return;
	}
/*
////////////////////////////////////////////////////////////////////////// Game Timer Over End
*/

// Streak Count Rutine;
	for (FStreakDollContainer_Counts& StreakIter : StreakContingDollMap)
	{
		StreakIter.Count += REALTIME_BOARDCHECK_INTERVAL;
	}


// Check Recently Moved Dolls If It's Still Moving
	TSet<AActor*> StoppedDollSet;
	TSet<int32> ToResetGroup;
	for (TWeakObjectPtr<AActor>& Iter : RecentlyMovedDollSet)
	{
		AActor* IterActor = Iter.Get();
		if (IterActor == nullptr || !UGoSoccerPlayManager::IsGoDoll(IterActor)) continue;
		bool bSetFree = IterActor->GetVelocity().Size() < 1.f;
#if WITH_EDITOR
		//DrawDebugSphere(GetWorld(), IterActor->GetActorLocation(), 10.f, 32, bSetFree ? FColor::Green : FColor::Red, false, bSetFree ? 1.5f : 0.5f);
#endif

// Check There's Counting Doll Interupted
		int32 idx = PlacedDollArr.IndexOfByKey(IterActor);
		if (!PlacedDollArr.IsValidIndex(idx)) continue;

		if (!bSetFree)
		{
			for (FStreakDollContainer_Counts& StreakIter : StreakContingDollMap)
			{
				if (StreakIter.StreakDollContainer[idx]) StreakIter.Count = -1.f;
			}
		}

		if (bSetFree)
		{
			ToResetGroup.Emplace(UF_Find(idx));
			StoppedDollSet.Emplace(IterActor);
			UGoSoccerPlayManager::AlertDollBoardStationary(IterActor);
		}
	}

// Change Counting Doll's Charge
	float MaxTimerColor0 = 0.f;
	float MaxTimerColor1 = 0.f;
	bool bGameEnded = false;
	for (auto Iter = StreakContingDollMap.CreateIterator(); Iter; ++Iter) // Read And Remove Only
	{
		auto DollContainer = *Iter;

		//if (DollContainer.Count < 0.f)
		//{
		//	int32 curridx = Iter.GetIndex();
		//	if (StreakContingDollMap.IsValidIndex(curridx)) DollContainer.Count += REALTIME_BOARDCHECK_INTERVAL;
		//}

		const TArray<uint64>& DollContainerDat = DollContainer.StreakDollContainer.GetData();
		int32 tempidx = 0;
		APlayerController* OwningPlayerController = nullptr;
		uint8 DollColor = 255;
		for (uint64 Accessibles : DollContainerDat)
		{
		    while (Accessibles != 0)
		    {
		        int32 BitIdx = FMath::CountTrailingZeros64(Accessibles);
		        int32 idx = tempidx * 64 + BitIdx;
		        if (!(Accessibles & (1ULL << BitIdx))) continue;
		        Accessibles &= ~(1ULL << BitIdx);

				if (!PlacedDollArr.IsValidIndex(idx))
				{
					UE_LOG(LogTemp, Warning, TEXT("Realtime_CheckStreak5 : Streak Counter Invalid Index Error"));
				}
				else if (PlacedDollArr[idx].IsValid())
				{
					UGoSoccerPlayManager::SetDollCircularEffect_Progressive(PlacedDollArr[idx].Get(), DollContainer.Count, REALTIME_STREAK_MAINTAIN_TIMESEC);
					if (OwningPlayerController == nullptr && UGoSoccerPlayManager::GetDollColor(PlacedDollArr[idx].Get(), DollColor))
					{
						if (DollColor == 0 && MaxTimerColor0 < DollContainer.Count)
						{
							MaxTimerColor0 = DollContainer.Count;
						}
						else if (DollColor == 1 && MaxTimerColor1 < DollContainer.Count)
						{
							MaxTimerColor1 = DollContainer.Count;
						}
						OwningPlayerController = PlacedDollArr[idx].Get()->GetOwner<APlayerController>();
					}
				}
		    }
		    tempidx++;
		}
		if (DollContainer.Count > REALTIME_STREAK_MAINTAIN_TIMESEC)
		{
			if (PlayGameMode == EPlayGameMode::EPGM_HotSeatGame)
			{
				if (UGoSoccerPlayManager::PlayerWin_HotSeat(OwningPlayerController, DollColor))
				{
					GetWorld()->GetTimerManager().ClearTimer(Realtime_BoardCheck_TimerHandle);
					bGameEnded = true;
					continue;
				}
			}
			else if (PlayGameMode == EPlayGameMode::EPGM_OnlineSession)
			{
				if (UGoSoccerPlayManager::PlayerWin(OwningPlayerController))
				{
					UGoSoccerPlayManager::PlayerLose(GetNextPlayer(OwningPlayerController));
					GetWorld()->GetTimerManager().ClearTimer(Realtime_BoardCheck_TimerHandle);
					bGameEnded = true;
					continue;
				}
			}
		}
		else if (DollContainer.Count < 0.f)
		{
			Iter.RemoveCurrent();
			continue;
		}
	}
	if (bGameEnded)
	{
		MaxTimerColor0 = 0.f;
		MaxTimerColor1 = 0.f;
	}
	bool bNeedUpdate = (PrevTimeColor0 != MaxTimerColor0 || PrevTimeColor1 != MaxTimerColor1);
	if (PrevTimeColor0 != MaxTimerColor0)
	{
		if (PlayGameMode == EPlayGameMode::EPGM_HotSeatGame)
		{
			/*Debug*/
			Delegate_OnPlayerColor0_SetMinousHUDPercet.ExecuteIfBound(MaxTimerColor0, REALTIME_STREAK_MAINTAIN_TIMESEC);
		}
		else Delegate_OnPlayerColor1_SetMinousHUDPercet.ExecuteIfBound(MaxTimerColor0, REALTIME_STREAK_MAINTAIN_TIMESEC);
	}
	if (PrevTimeColor1 != MaxTimerColor1)
	{
		Delegate_OnPlayerColor0_SetMinousHUDPercet.ExecuteIfBound(MaxTimerColor1, REALTIME_STREAK_MAINTAIN_TIMESEC);
	}

	PrevTimeColor0 = MaxTimerColor0;
	PrevTimeColor1 = MaxTimerColor1;

// Nothing To Check return false
	if (StoppedDollSet.IsEmpty()) return;

// Streak Check For Stopped Dolls
	for (AActor* IterActor : StoppedDollSet)
	{
		if (IterActor == nullptr) continue;
		RecentlyMovedDollSet.Remove(IterActor);
	}

// Add Involved Dolls To StreakCheck Set, Remove UnionFind Structure
	TArray<int32> ResetActors;
	for (int32 i = 0; i < PlacedDollArr.Num(); i++)
	{
		AActor* Doll_Iter = PlacedDollArr[i].Get();
		if (!UF_Parent.IsValidIndex(i) || Doll_Iter == nullptr) continue;
		if (ToResetGroup.Contains(UF_Find(i)))
		{
			UF_Parent[i] = i;
			ResetActors.Add(i);
		}
	}

// Reset UnionFind Structure
	for (int32 Iter : ResetActors)
	{
		if (!PlacedDollArr.IsValidIndex(Iter)) continue;
		if (!UF_Accessable.IsValidIndex(Iter)) continue;
		if (AActor* ResetDoll = PlacedDollArr[Iter].Get())
		{
			UF_Accessable[Iter].Reset();
			UF_JoinDollIntoGroup(ResetDoll); // Check Streak 5 Involved
		}
	}

	//Execute_WinCheck(this, nullptr);

	return;
}

bool AGoSoccerPlayMode::Realtime_StartCount(FStreakDollContainer& StreakDollContainer)
{
	for (auto Iter : StreakContingDollMap)
	{
		if (Iter.StreakDollContainer == StreakDollContainer) return false;
	}
	StreakContingDollMap.Emplace(StreakDollContainer, 0.f);
	//if (!StreakContingDollMap.Contains(StreakDollContainer))
	//{
	//	StreakContingDollMap.Emplace(StreakDollContainer, 0.f);
	//}
	return true;
}

bool AGoSoccerPlayMode::GetPlayerColor(APlayerController* CheckPC, uint8& OutDollColor)
{
	if (CheckPC != nullptr)
	{
		//if (!PlayerCardHashingMap.Contains(CheckPC))
		//{
		//	FPlayerCard tempPlayerCard;
		//	if (UGoSoccerPlayManager::GetPlayerCard(CheckPC, tempPlayerCard))
		//	{
		//		DollColorHashingMap.Emplace(CheckPC, tempPlayerCard);
		//	}
		//}
		//if (PlayerCardHashingMap.Contains(CheckPC))
		//{
		//	auto* FindKey = PlayerCardHashingMap.Find(CheckPC);
		//	if (FindKey != nullptr)
		//	{
		//		OutDollColor = FindKey->PlayerDollColor;
		//		return true;
		//	}
		//}
		if (!DollColorHashingMap.Contains(CheckPC))
		{
			FPlayerCard tempPlayerCard;
			if (UGoSoccerPlayManager::GetPlayerCard(CheckPC, tempPlayerCard))
			{
				DollColorHashingMap.Emplace(CheckPC, tempPlayerCard.PlayerDollColor);
			}
		}
		if (DollColorHashingMap.Contains(CheckPC))
		{
			OutDollColor = *DollColorHashingMap.Find(CheckPC);
			return true;
		}
	}
	return false;
}

bool AGoSoccerPlayMode::GetPlayerName(APlayerController* CheckPC, FName& OutPlayerName)
{
	//if (CheckPC != nullptr)
	//{
	//	if (!PlayerCardHashingMap.Contains(CheckPC))
	//	{
	//		FPlayerCard tempPlayerCard;
	//		if (UGoSoccerPlayManager::GetPlayerCard(CheckPC, tempPlayerCard))
	//		{
	//			DollColorHashingMap.Emplace(CheckPC, tempPlayerCard);
	//		}
	//	}
	//	if (PlayerCardHashingMap.Contains(CheckPC))
	//	{
	//		auto* FindKey = PlayerCardHashingMap.Find(CheckPC);
	//		if (FindKey != nullptr)
	//		{
	//			OutPlayerName = FindKey->PlayerName;
	//			return true;
	//		}
	//	}
	//}
	//OutPlayerName = FName(TEXT("LocalPlayer"));
	return false;
}

bool AGoSoccerPlayMode::GameSet()
{
	if (!GetWorld()) return false;
	GetWorld()->GetTimerManager().ClearTimer(Realtime_BoardCheck_TimerHandle);
	return true;
}

void AGoSoccerPlayMode::OnSpawnedOrFlicked()
{

}

bool AGoSoccerPlayMode::RefreshPlayerCardArrWidgets_Except_Implementation(UObject* ExceptPlayerControllerObject)
{
	UWorld* W = GetWorld();
	if (W == nullptr) return false;

	UE_LOG(LogTemp, Log, TEXT("AGoSoccerPlayMode : RefreshPlayerCardArrWidgets_Except - %s"),
		(ExceptPlayerControllerObject != nullptr ? *ExceptPlayerControllerObject->GetFName().ToString() : TEXT("Already Destroyed"))
	);
	for (FConstPlayerControllerIterator It = W->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* P = It->Get();
		if (P == nullptr) continue;
		if (ExceptPlayerControllerObject != nullptr && P == ExceptPlayerControllerObject) continue;
		UE_LOG(LogTemp, Log, TEXT("AGoSoccerPlayMode : RefreshPlayerCardArrWidgets_Except - %s"), *P->GetFName().ToString());
		UGoSoccerPlayManager::RefreshPlayerCardListView(P);
	}
	return true;
}

bool AGoSoccerPlayMode::RefreshPlayerCardArrWidgets_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("AGoSoccerPlayMode : RefreshPlayerCardArrWidgets - %s"),
		(GetWorld()->GetNetMode() == ENetMode::NM_ListenServer ? TEXT("Listen Server") : TEXT("Client"))
	);
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* P = It->Get();
		UE_LOG(LogTemp, Log, TEXT("AGoSoccerPlayMode : RefreshPlayerCardArrWidgets - %s"),
			(P != nullptr ? *P->GetFName().ToString() : TEXT("Nullptr Controller"))
		);
		if (P != nullptr)
		{
			UGoSoccerPlayManager::RefreshPlayerCardListView(P);
		}
	}
	return true;
}

bool AGoSoccerPlayMode::TurnEnd_SpawnDoll(APlayerController* LatestPlayer, TWeakObjectPtr<AActor> PlacedDoll)
{
	if (LatestPlayer == nullptr) return false;
	AActor* Doll = PlacedDoll.Get();
	if (bTakeTurn)
	{
		UGoSoccerPlayManager::TurnEnd(LatestPlayer);
		if (Doll != nullptr) UF_JoinDollIntoGroup(Doll);
	}
	else
	{
		if (Doll != nullptr)
		{
			RecentlyMovedDollSet.Emplace(Doll);
			UGoSoccerPlayManager::AddRealtimeCooldownBonus(LatestPlayer, REALTIME_PLACE_COOLDOWN_BONUS_FLICK, true);
		}
	}
	TurnCounter++;
	OnSpawnedOrFlicked();
	return bTakeTurn ? Execute_WinCheck(this, LatestPlayer) : true;
}

bool AGoSoccerPlayMode::TurnEnd_FlickDoll(APlayerController* LatestPlayer)
{
	if (LatestPlayer == nullptr) return false;
	if (bTakeTurn && !IsBoardStationary())
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this, LatestPlayer]()
			{
				TurnEnd_FlickDoll(LatestPlayer);
			}
		));
		return true;
	}
	bStationaryOnce = false;
	if (bTakeTurn && PlayGameMode != EPlayGameMode::EPGM_SingleAIPlay) UGoSoccerPlayManager::TurnEnd(LatestPlayer);
	if (!bTakeTurn)
	{
		UGoSoccerPlayManager::AddRealtimeCooldownBonus(LatestPlayer, REALTIME_FLICK_COOLDOWN_BONUS);
		return true;
	}
	TSet<int32> ToResetGroup;
	for (auto& Iter : RecentlyMovedDollSet)
	{
		AActor* Doll_Iter = Iter.Get();
		int32 idx = PlacedDollArr.IndexOfByKey(Doll_Iter);
		if (!PlacedDollArr.IsValidIndex(idx)) continue;
		ToResetGroup.Emplace(UF_Find(idx));
	}
	RecentlyMovedDollSet.Empty();
	TArray<int32> ResetActors;
	for (int32 i = 0; i < PlacedDollArr.Num(); i++)
	{
		AActor* Doll_Iter = PlacedDollArr[i].Get();
		if (!UF_Parent.IsValidIndex(i) || Doll_Iter == nullptr) continue;
		if (ToResetGroup.Contains(UF_Find(i)))
		{
			UF_Parent[i] = i;
			ResetActors.Add(i);
		}
	}

	for (auto& Iter : PlacedDollArr)
	{
		AActor* Iter_Doll = Iter.Get();
		if (Iter_Doll == nullptr) continue;
		UGoSoccerPlayManager::AlertDollBoardStationary(Iter_Doll);
	}

	for (int32 Iter : ResetActors)
	{
		if (!PlacedDollArr.IsValidIndex(Iter)) continue;
		if (!UF_Accessable.IsValidIndex(Iter)) continue;
		if (AActor* ResetDoll = PlacedDollArr[Iter].Get())
		{
			UF_Accessable[Iter].Reset();
			UF_JoinDollIntoGroup(ResetDoll);
		}
	}
	TurnCounter++;
	UE_LOG(LogTemp, Log, TEXT("TurnEnd_FlickDoll"));
	OnSpawnedOrFlicked();
	return Execute_WinCheck(this, LatestPlayer);
}

bool AGoSoccerPlayMode::MatchEnd(uint8 WinDollColor, bool& OutContinueGame, APlayerController*& OutFinalWinPlayer, uint8& OutFinalWinDollColor)
{
	OutFinalWinDollColor = WinDollColor;
	if (WinDollColor == 0) 
	{
		Color0MatchCount++;
	}
	else if (WinDollColor == 1) 
	{
		Color1MatchCount++;
	}
	else 
	{
		return false;
	}
	if (MatchCount / 2 < FMath::Max(Color0MatchCount, Color1MatchCount))
	{
		auto Iter = GetWorld()->GetPlayerControllerIterator();
		for (; Iter; ++Iter)
		{
			APlayerController* PC = Iter->Get();
			uint8 OutColor;
			if (!GetPlayerColor(PC, OutColor)) continue;
			if (OutColor == WinDollColor)
			{
				OutFinalWinPlayer = PC;
			}
		}
		OutContinueGame = false;
	}
	else 
	{
		OutContinueGame = true;
	}
	return true;
}

void AGoSoccerPlayMode::SetMatchScoreLamp(int32 Idx, uint8 MatchState, int32 DollColor)
{
	UGoSoccerPlayManager::SetMatchScoreLampState(GetWorld()->GetFirstPlayerController(), Idx, static_cast<EMatchScoreLampState>(MatchState), DollColor);
}

void AGoSoccerPlayMode::ResetMatchScoreLamp()
{
	UGoSoccerPlayManager::ResetMatchScoreLampState(GetWorld()->GetFirstPlayerController());
}

void AGoSoccerPlayMode::SetMatchScoreLampCount(int32 MaxCount)
{
	UGoSoccerPlayManager::SetMatchScoreLampCount(GetWorld()->GetFirstPlayerController(), MaxCount);
}