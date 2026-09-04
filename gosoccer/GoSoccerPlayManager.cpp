#include "GoSoccerPlayManager.h"
#include "Interface/IGoBoard.h"
#include "Interface/IGoHUD.h"
#include "Interface/IGoDoll.h"
#include "Interface/IGoSoccerPlayerController.h"
#include "Interface/IGoSoccerPlayerState.h"
#include "Interface/IGoSoccerPlayMode.h"
#include "Interface/IGoSoccerGameInstance.h"
#include "Interface/IGoSoccerGameState.h"
#include "Interface/IGoWidget.h"

bool UGoSoccerPlayManager::IsGoBoard(UObject* CheckObject)
{
	if (!NullCheck(CheckObject, TEXT("BoardObject"), TEXT("IsGoBoard"))) return false;
	return CheckObject->Implements<UIGoBoard>();
}

bool UGoSoccerPlayManager::IsGoDoll(UObject* CheckObject)
{
	if (!NullCheck(CheckObject, TEXT("DollObject"), TEXT("IsGoBoard"))) return false;
	return CheckObject->Implements<UIGoDoll>();
}

bool UGoSoccerPlayManager::IsDollFallen(UObject* BoardObject, UObject* DollObject)
{
	if (!IsGoBoard(BoardObject) || !IsGoDoll(DollObject)) return false;
	if (IIGoDoll::Execute_GetDollFallFromBoard(DollObject)) return true;
	bool R = IIGoBoard::Execute_IsDollFallen(BoardObject, DollObject);
	if (R)
	{
		IIGoDoll::Execute_SetDollFallFromBoard(DollObject, R, true);
	}
	return R;
}

bool UGoSoccerPlayManager::GetDollFallFromBoard(UObject* DollObject, bool& bIsFallen)
{
	if (!IsGoDoll(DollObject)) return false;
	bIsFallen = IIGoDoll::Execute_GetDollFallFromBoard(DollObject);
	return true;
}

bool UGoSoccerPlayManager::GetDollColor(UObject* DollObject, uint8& OutDollColor)
{
	if (!IsGoDoll(DollObject)) return false;
	return IIGoDoll::Execute_GetDollcolor(DollObject, OutDollColor);
}

bool UGoSoccerPlayManager::GetGoBoardBorderLine(UObject* BoardObject, float& U, float& D, float& L, float& R)
{
	if (!IsGoBoard(BoardObject)) return false;
	return IIGoBoard::Execute_GetBorderLine(BoardObject, U, D, L, R);
}

bool UGoSoccerPlayManager::IsDollConsideredStationary(UObject* DollObject)
{
	if (!IsGoDoll(DollObject)) return false;
	return IIGoDoll::Execute_IsDollConsideredStationary(DollObject);
}

bool UGoSoccerPlayManager::GameStarted(UObject* PlayerObject, const FSessionCreateData& SessionCreateData)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("GameStarted"))) return false;
	return IIGoSoccerPlayerController::Execute_GameStarted(PlayerObject, SessionCreateData);
}

FVector UGoSoccerPlayManager::GetPutLocation(UObject* BoardObject, FVector CursorLocation)
{
	if (!NullCheck(BoardObject, TEXT("BoardObject"), TEXT("GetPutLocation"))) return FVector::ZeroVector;
	if (!IsGoBoard(BoardObject)) return FVector::ZeroVector;
	return IIGoBoard::Execute_GetPutLocation(BoardObject, CursorLocation);
}

bool UGoSoccerPlayManager::PutDoll(UObject* GameModeObject, UObject* PlayerControllerObject, FVector Location, uint8 DollColor, const EDollType& SpawnDollType)
{
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("PutDoll")) ||
		!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("PutDoll")))
	{
		return false;
	}
	return IIGoSoccerPlayMode::Execute_SpawnDoll(GameModeObject, PlayerControllerObject, Location, DollColor, SpawnDollType);
}

bool UGoSoccerPlayManager::SetDollPlacedIndex(UObject* DollObject, int32 InPlacedIndex)
{
	if (!NullCheck(DollObject, TEXT("DollObject"), TEXT("SetDollPlacedIndex"))) return false;
	if (!IsGoDoll(DollObject)) return false;
	return IIGoDoll::Execute_SetPlacedIndex(DollObject, InPlacedIndex);
}

bool UGoSoccerPlayManager::GetDollPlacedIndex(UObject* DollObject, int32& OutPlacedIndex)
{
	if (!NullCheck(DollObject, TEXT("DollObject"), TEXT("GetDollPlacedIndex"))) return false;
	if (!IsGoDoll(DollObject)) return false;
	OutPlacedIndex = IIGoDoll::Execute_GetPlacedIndex(DollObject);
	return OutPlacedIndex >= 0;
}

bool UGoSoccerPlayManager::MouseOver(UObject* MouseOverObject, EDollStencilValue DollStencilValue)
{
	if (!NullCheck(MouseOverObject, TEXT("MouseOverObject"), TEXT("MouseOver"))) return false;
	if (!IsGoDoll(MouseOverObject)) return false;
	return IIGoDoll::Execute_SetDollCustomStencil(MouseOverObject, (uint8)DollStencilValue);
}

bool UGoSoccerPlayManager::MouseOverEnd(UObject* MouseOverEndObject)
{
	//if (!NullCheck(MouseOverEndObject, TEXT("MouseOverEndObject"), TEXT("MouseOverEnd"))) return false;
	if (MouseOverEndObject == nullptr) return false;
	if (!IsGoDoll(MouseOverEndObject)) return false;
	return IIGoDoll::Execute_SetDollCustomStencil(MouseOverEndObject, (uint8)EDollStencilValue::EDSV_Default);
}

bool UGoSoccerPlayManager::FlickDoll(UObject* DollObject, FVector FlickDirection)
{
	if (!NullCheck(DollObject, TEXT("DollObject"), TEXT("FlickDoll"))) return false;
	return IIGoDoll::Execute_FlickDoll(DollObject, FlickDirection);
}

bool UGoSoccerPlayManager::FlickDoll_Completed(UObject* GameModeObject, UObject* DollOwnerControllerObject)
{
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("FlickDoll_Completed"))) return false;
	if (!NullCheck(DollOwnerControllerObject, TEXT("DollOwnerControllerObject"), TEXT("FlickDoll_Completed"))) return false;
	return IIGoSoccerPlayMode::Execute_TurnEnd_FlickDoll(GameModeObject, DollOwnerControllerObject);
}

//bool UGoSoccerPlayManager::CheckWinning(UObject* CheckPlayerObject, UObject* GoBoardObject)
//{
//	return false;
//}

bool UGoSoccerPlayManager::TurnStart(UObject* StartPlayerObject)
{
	if (!NullCheck(StartPlayerObject, TEXT("StartPlayerObject"), TEXT("TurnStart"))) return false;
	if (StartPlayerObject->Implements<UIGoSoccerPlayerController>())
	{
		return IIGoSoccerPlayerController::Execute_TurnStart(StartPlayerObject);
	}
	else if (StartPlayerObject->Implements<UIGoSoccerPlayerState>())
	{
		return IIGoSoccerPlayerState::Execute_TurnStart(StartPlayerObject);
	}
	return false;
}

bool UGoSoccerPlayManager::TurnEnd(UObject* EndPlayerObject)
{
	if (!NullCheck(EndPlayerObject, TEXT("EndPlayerObject"), TEXT("TurnEnd"))) return false;
	if (EndPlayerObject->Implements<UIGoSoccerPlayerController>())
	{
		return IIGoSoccerPlayerController::Execute_TurnEnd(EndPlayerObject);
	}
	else if (EndPlayerObject->Implements<UIGoSoccerPlayerState>())
	{
		return IIGoSoccerPlayerState::Execute_TurnEnd(EndPlayerObject);
	}
	return false;
}

bool UGoSoccerPlayManager::MarkDollAsMoved(UObject* GameModeObject, UObject* DollObject)
{
	//if (!NullCheck(DollObject, TEXT("DollObject"), TEXT("MarkDollAsMoved"))) return false;
	if (!IsGoDoll(DollObject)) return false;
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("MarkDollAsMoved"))) return false;
	return IIGoSoccerPlayMode::Execute_MarkDollAsMoved(GameModeObject, DollObject);
}

bool UGoSoccerPlayManager::SetDollStreakSpline(UObject* BoardObject, const TArray<FVector> DollPosArr)
{
	if (!NullCheck(BoardObject, TEXT("BoardObject"), TEXT("SetDollStreakSpline"))) return false;
	return IIGoBoard::Execute_SetDollStreakSpline(BoardObject, DollPosArr);
}

bool UGoSoccerPlayManager::SetWinDollCircularEffect(UObject* DollObject, bool bOnEffect)
{
	if (!NullCheck(DollObject, TEXT("DollObject"), TEXT("SetWinDollCircularEffect"))) return false;
	if (!IsGoDoll(DollObject)) return false;
	return IIGoDoll::Execute_SetWinDollCircularEffect(DollObject, bOnEffect);
}

bool UGoSoccerPlayManager::SetDollCircularEffect_Progressive(UObject* DollObject, float CurrentTimeSec, float MaxTimeSec)
{
	if (!IsGoDoll(DollObject)) return false;
	return IIGoDoll::Execute_SetDollCircularEffect_Progressive(DollObject, CurrentTimeSec, MaxTimeSec);
}

bool UGoSoccerPlayManager::UpdateFallenDollScore(UObject* PlayerControllerObject, int32 FallenDollScore)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("UpdateFallenDollScore"))) return false;
	return IIGoSoccerPlayerController::Execute_NotifyFallenDollScore(PlayerControllerObject, FallenDollScore);
}

bool UGoSoccerPlayManager::SetTranclucentDollLocation(UObject* GoBoardObject, FVector TDollLocation)
{
	if (!IsGoBoard(GoBoardObject)) return false;
	return IIGoBoard::Execute_SetTranclucentDollLocation(GoBoardObject, TDollLocation);
}

bool UGoSoccerPlayManager::GetTranclucentDoll(UObject* GoBoardObject, UObject*& OutTranclucentDollObject)
{
	if (!IsGoBoard(GoBoardObject)) return false;
	return IIGoBoard::Execute_GetTranclucentDoll(GoBoardObject, OutTranclucentDollObject);
}

bool UGoSoccerPlayManager::GetOverlapingDoll_TrancluentDoll(UObject* TranclucentDollObject)
{
	if (!IsGoDoll(TranclucentDollObject)) return false;
	return IIGoDoll::Execute_Transpalent_GetIsPuttable(TranclucentDollObject);
}

bool UGoSoccerPlayManager::SetDollFallenFromBoard(UObject* DollObject, bool bFallen)
{
	if (!IsGoDoll(DollObject)) return false;
	return IIGoDoll::Execute_SetDollFallFromBoard(DollObject, bFallen, true);
}

bool UGoSoccerPlayManager::SetHostingSessionCreateData(UObject* PlayerControllerObject, const FSessionCreateData& InHostSessionCreateData)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("SetHostingSessionCreateData"))) return false;
	UObject* tempPlayerState = IIGoSoccerPlayerController::Execute_GetPlayerStateAsObject(PlayerControllerObject);
	if (!NullCheck(tempPlayerState, TEXT("tempPlayerState"), TEXT("SetHostingSessionCreateData"))) return false;
	return IIGoSoccerPlayerState::Execute_SetHostingSessionCreateData(tempPlayerState, InHostSessionCreateData);
}

bool UGoSoccerPlayManager::GetHostingSessionCreateData(UObject* PlayerStateObject, FSessionCreateData& OutHostSessionCreateData)
{
	if (!NullCheck(PlayerStateObject, TEXT("PlayerStateObject"), TEXT("GetHostingSessionCreateData"))) return false;
	return IIGoSoccerPlayerState::Execute_GetHostingSessionCreateData(PlayerStateObject, OutHostSessionCreateData);
}

bool UGoSoccerPlayManager::ClearHostingSessionCreateData(UObject* PlayerControllerObject)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("ClearHostingSessionCreateData"))) return false;
	UObject* tempPlayerState = IIGoSoccerPlayerController::Execute_GetPlayerStateAsObject(PlayerControllerObject);
	if (!NullCheck(tempPlayerState, TEXT("tempPlayerState"), TEXT("ClearHostingSessionCreateData"))) return false;
	return IIGoSoccerPlayerState::Execute_ClearHostingSessionCreateData(tempPlayerState);
}

bool UGoSoccerPlayManager::GetPossibleCameraLocation(UObject* GoBoardObject, FVector& ToMoveLocation)
{
	if (!IsGoBoard(GoBoardObject)) return false;
	return IIGoBoard::Execute_GetPossibleCameraLocation(GoBoardObject, ToMoveLocation);
}

bool UGoSoccerPlayManager::AlertDollBoardStationary(UObject* DollObejct)
{
	if (!IsGoDoll(DollObejct)) return false;
	return IIGoDoll::Execute_OnBoardGetStationary(DollObejct);
}

bool UGoSoccerPlayManager::AIPlay_SpawnDollAtLocation(UObject* GameModeObject, UObject* NextPlayerObject, FVector PlaceLocation)
{
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("AIPlay_SpawnDollAtLocation"))) return false;
	return IIGoSoccerPlayMode::Execute_AIPlay_SpawnDollAtLocation(GameModeObject, NextPlayerObject, PlaceLocation);
}

bool UGoSoccerPlayManager::AIPlay_FlickDoll(UObject* GameModeObject, UObject* NextPlayerObject, UObject* FlickDollObject, FVector FlickDirection)
{
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("AIPlay_FlickDoll"))) return false;
	if (!NullCheck(NextPlayerObject, TEXT("NextPlayerObject"), TEXT("AIPlay_FlickDoll"))) return false;
	if (!IsGoDoll(FlickDollObject)) return false;
	if (!GameModeObject->Implements<UIGoSoccerPlayMode>()) return false;
	return IIGoSoccerPlayMode::Execute_AIPlay_FlickDoll(GameModeObject, NextPlayerObject, FlickDollObject, FlickDirection);
}

bool UGoSoccerPlayManager::AIPlay_ThrowLoadingScreen(UObject* PlayerObject, bool bEnable)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("AIPlay_ThrowLoadingScreen"))) return false;
	return IIGoSoccerPlayerController::Execute_SetLoadingScreen(PlayerObject, bEnable);
}

bool UGoSoccerPlayManager::AIPlay_ConsiderTurnPassed(UObject* PlayerObject)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("AIPlay_ConsiderTurnPassed"))) return false;
	return IIGoSoccerPlayerController::Execute_AI_ConsiderTurnPassed(PlayerObject);
}

bool UGoSoccerPlayManager::AddFallenScore(UObject* GameModeObject, UObject* FallenDollObject)
{
	if (!IsGoDoll(FallenDollObject)) return false;
	return IIGoSoccerPlayMode::Execute_AddFallenScore(GameModeObject, FallenDollObject);
}

bool UGoSoccerPlayManager::AddRealtimeCooldownBonus(UObject* PlayerControllerObject, float CooldownBonusPortion, bool bFlick)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("AddRealtimeCooldownBonus"))) return false;
	return IIGoSoccerPlayerController::Execute_AddRealtimeCoolDownBonus(PlayerControllerObject, CooldownBonusPortion, bFlick);
}

bool UGoSoccerPlayManager::SetOminousHUDPercent(UObject* PlayerControllerObject, float CurrentTimeSec, float MaxTimeSec)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("SetOminousHUDPercent"))) return false;
	return IIGoSoccerPlayerController::Execute_SetOminousHUDPercent(PlayerControllerObject, CurrentTimeSec, MaxTimeSec);
}

bool UGoSoccerPlayManager::StartGameTimer(UObject* PlayerControllerObject, double InStartTimeStamp, double InTimeLimit)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("StartGameTimer"))) return false;
	return IIGoSoccerPlayerController::Execute_StartGameTimer(PlayerControllerObject, InStartTimeStamp, InTimeLimit);
}

bool UGoSoccerPlayManager::ThrowScoringBoard(UObject* PlayerControllerObject, const TArray<FScoreBoard>& ScoreArr)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("ThrowScoringBoard"))) return false;
	return IIGoSoccerPlayerController::Execute_ThrowScoringBoard(PlayerControllerObject, ScoreArr);
}

bool UGoSoccerPlayManager::UpdateScoringBoard(UObject* PlayerControllerObject, uint8 DollColor, EScoreType ScoreType, int32 UpdatedScore)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("UpdateScoringBoard"))) return false;
	return IIGoSoccerPlayerController::Execute_UpdateScoringBoard(PlayerControllerObject, DollColor, ScoreType, UpdatedScore);
}

bool UGoSoccerPlayManager::GetDollHP(UObject* DollObject, float& OutHP)
{
	if (!IsGoDoll(DollObject)) return false;
	return IIGoDoll::Execute_GetCurrHP(DollObject, OutHP);
}

bool UGoSoccerPlayManager::SetDollStencil_Scoring(UObject* DollObject)
{
	if (!IsGoDoll(DollObject)) return false;
	return IIGoDoll::Execute_SetDollStencil_Scoring(DollObject);
}

bool UGoSoccerPlayManager::GetNextDollType(UObject* GameModeObject, EDollType& OutNextDollType)
{
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("GetNextDollType"))) return false;
	return IIGoSoccerPlayMode::Execute_GetNextDollType(GameModeObject, OutNextDollType);
}

bool UGoSoccerPlayManager::SendCurrentSessionData(UObject* PlayerControllerObject, const FSessionCreateData& InSessionCreateData)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("SendCurrentSessionData"))) return false;
	return IIGoSoccerPlayerController::Execute_SetCurrentPlayingSessionData(PlayerControllerObject, InSessionCreateData);
}

bool UGoSoccerPlayManager::GetPlacedDollCount(UObject* GameModeObject, int32& OutDollCount)
{
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("GetPlacedDollCount"))) return false;
	return IIGoSoccerPlayMode::Execute_GetPlacedDollCount(GameModeObject, OutDollCount);
}

bool UGoSoccerPlayManager::SetTurnMode_CylinderConfig(UObject* PlayerControllerObject, const TArray<EDollType>& InItemDollArr)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("Set_TurnMode_CylinderConfig"))) return false;
	return IIGoSoccerPlayerController::Execute_SetTurnMode_CylinderConfig(PlayerControllerObject, InItemDollArr);
}

bool UGoSoccerPlayManager::SetMatchScoreLampState(UObject* PlayerControllerObject, int32 Index, EMatchScoreLampState InMatchScoreLampState, uint8 FillDollColor)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("SetMatchScoreLampState"))) return false;
	return IIGoSoccerPlayerController::Execute_SetMatchScoreLampState(PlayerControllerObject, Index, InMatchScoreLampState, FillDollColor);
}

bool UGoSoccerPlayManager::ResetMatchScoreLampState(UObject* PlayerControllerObject)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("ResetMatchScoreLampState"))) return false;
	return IIGoSoccerPlayerController::Execute_ResetMatchScoreLamp(PlayerControllerObject);
}

bool UGoSoccerPlayManager::SetMatchScoreLampCount(UObject* PlayerControllerObject, int32 MaxCount)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("SetMatchScoreLampCount"))) return false;
	return IIGoSoccerPlayerController::Execute_SetMatchScoreLampCount(PlayerControllerObject, MaxCount);
}

bool UGoSoccerPlayManager::HostGame(UObject* HostPlayerObject, const FUniqueNetIdRepl& UniqueNetIdRepl, FSessionCreateData& SessionCreateData)
{
	if (!NullCheck(HostPlayerObject, TEXT("HostPlayerObject"), TEXT("HostGame"))) return false;
	if (HostPlayerObject->Implements<UIGoSoccerPlayerController>())
	{
		return IIGoSoccerPlayerController::Execute_HostGame(HostPlayerObject, SessionCreateData);
	}
	//else if (HostPlayerObject->Implements<UIGoSoccerGameInstance>())
	//{
	//	return IIGoSoccerGameInstance::Execute_HostGame(HostPlayerObject, UniqueNetIdRepl, CreateSessionName);
	//}
	return false;
}

bool UGoSoccerPlayManager::RequestHostGame(UObject* GameInstanceObject, const FUniqueNetIdRepl& UniqueNetIdRepl, const FSessionCreateData& SessionCreateData)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("RequestHostGame"))) return false;
	return IIGoSoccerGameInstance::Execute_HostGame(GameInstanceObject, UniqueNetIdRepl, SessionCreateData);
}

bool UGoSoccerPlayManager::RefreshOnlineSessions(UObject* JoinPlayerControllerObject)
{
	if (!NullCheck(JoinPlayerControllerObject, TEXT("JoinPlayerControllerObject"), TEXT("RefreshOnlineSessions"))) return false;
	if (!JoinPlayerControllerObject->Implements<UIGoSoccerPlayerController>()) return false;
	return IIGoSoccerPlayerController::Execute_RefreshOnlineSessions(JoinPlayerControllerObject);
}

bool UGoSoccerPlayManager::FindSession(UObject* GameInstanceObject, const FUniqueNetIdRepl& UniqueNetIdRepl)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("FindSession"))) return false;
	return IIGoSoccerGameInstance::Execute_FindSession(GameInstanceObject, UniqueNetIdRepl);
}

bool UGoSoccerPlayManager::JoinSelectedSession(UObject* PlayerObject, UObject* SelectedOnlineSessionData, const FUniqueNetIdRepl& UniqueNetIdRepl)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("JoinSelectedSession"))) return false;
	if (!NullCheck(SelectedOnlineSessionData, TEXT("SelectedOnlineSessionData"), TEXT("JoinSelectedSession"))) return false;
	if (PlayerObject->Implements<UIGoSoccerPlayerController>())
	{
		return IIGoSoccerPlayerController::Execute_JoinSelectedSession(PlayerObject, SelectedOnlineSessionData);
	}
	else if (PlayerObject->Implements<UIGoSoccerGameInstance>())
	{
		return IIGoSoccerGameInstance::Execute_JoinSelectedSession(PlayerObject, SelectedOnlineSessionData, UniqueNetIdRepl);
	}
	return false;
}

bool UGoSoccerPlayManager::StartGame(UObject* PlayerObject)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("StartGame"))) return false;
	if (PlayerObject->Implements<UIGoSoccerPlayerController>())
	{
		return IIGoSoccerPlayerController::Execute_StartGame(PlayerObject);
	}
	return false;
}

bool UGoSoccerPlayManager::StartGameIfAllReady(UObject* GameModeObejct, const FSessionCreateData& SessionCreateData)
{
	if (!NullCheck(GameModeObejct, TEXT("GameModeObejct"), TEXT("StartGame"))) return false;
	if (GameModeObejct->Implements<UIGoSoccerPlayMode>())
	{
		return IIGoSoccerPlayMode::Execute_StartGame_IfAllReady(GameModeObejct, SessionCreateData);
	}
	return false;
}

bool UGoSoccerPlayManager::ReadyGame(UObject* PlayerObject)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("ReadyGame"))) return false;
	if (PlayerObject->Implements<UIGoSoccerPlayerController>())
	{
		return IIGoSoccerPlayerController::Execute_ReadyGame(PlayerObject);
	}
	else if (PlayerObject->Implements<UIGoSoccerPlayerState>())
	{
		return IIGoSoccerPlayerState::Execute_ReadyGame(PlayerObject);
	}
	//else if (PlayerObject->Implements<UIGoSoccerPlayMode>())
	//{
	//	return IIGoSoccerPlayMode::Execute_ReadyGame(PlayerObject);
	//}
	return false;
}

bool UGoSoccerPlayManager::LeaveGame(UObject* PlayerObject)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("LeaveGame"))) return false;
	//return IIGoSoccerPlayerController::Execute_LeaveGame(PlayerObject);
	return IIGoSoccerGameInstance::Execute_LeaveSession(PlayerObject);
}

bool UGoSoccerPlayManager::Steam_CreateSession(UObject* PlayerObject)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("Steam_CreateSession"))) return false;
	if (!PlayerObject->Implements<UIGoSoccerGameInstance>()) return false;
	return IIGoSoccerGameInstance::Execute_Steam_CreateSession(PlayerObject);
}

bool UGoSoccerPlayManager::SetReadyState(UObject* PlayerStateObject, bool bReady)
{
	if (!NullCheck(PlayerStateObject, TEXT("PlayerStateObject"), TEXT("SetReadyState"))) return false;
	return IIGoSoccerPlayerState::Execute_SetReadyState(PlayerStateObject, bReady);
}

bool UGoSoccerPlayManager::GetPlayerCard(UObject* PlayerStateObject, FPlayerCard& OutPlayerCard)
{
	if (!NullCheck(PlayerStateObject, TEXT("PlayerStateObject"), TEXT("GetPlayerCard"))) return false;
	if (PlayerStateObject->Implements<UIGoSoccerPlayerController>())
	{
		return IIGoSoccerPlayerState::Execute_GetPlayerCard(
			IIGoSoccerPlayerController::Execute_GetPlayerStateAsObject(PlayerStateObject),
			OutPlayerCard
		);
	}
	else if (!PlayerStateObject->Implements<UIGoSoccerPlayerState>()) return false;
	return IIGoSoccerPlayerState::Execute_GetPlayerCard(PlayerStateObject, OutPlayerCard);
}

bool UGoSoccerPlayManager::SetPlayerCard(UObject* PlayerStateObject, FPlayerCard& InPlayerCard)
{
	if (!NullCheck(PlayerStateObject, TEXT("PlayerStateObject"), TEXT("SetPlayerCard"))) return false;
	if (!PlayerStateObject->Implements<UIGoSoccerPlayerState>()) return false;
	return IIGoSoccerPlayerState::Execute_SetPlayerCard(PlayerStateObject, InPlayerCard);
}

//bool UGoSoccerPlayManager::StartHotSeatGame(UObject* PlayerObject, EPlayGameRule PlayGameRule)
bool UGoSoccerPlayManager::StartHotSeatGame(UObject* PlayerObject, const FSessionCreateData& SessionCreateData)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("StartHotSeatGame"))) return false;
	if (PlayerObject->Implements<UIGoSoccerPlayerController>())
	{
		return IIGoSoccerPlayerController::Execute_HostHotSeatGame(PlayerObject, SessionCreateData);
	}
	else if (PlayerObject->Implements<UIGoSoccerPlayMode>())
	{
		return IIGoSoccerPlayMode::Execute_HostHotSeatGame(PlayerObject, SessionCreateData);
	}
	return false;
}

//bool UGoSoccerPlayManager::StartSingleAIGame(UObject* PlayerObject, EPlayGameRule PlayGameRule, int32 PlayAsDollColor)
bool UGoSoccerPlayManager::StartSingleAIGame(UObject* PlayerObject, const FSessionCreateData& SessionCreateData)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("StartSingleAIGame"))) return false;
	if (PlayerObject->Implements<UIGoSoccerPlayerController>())
	{
		return IIGoSoccerPlayerController::Execute_HostSingleAIGame(PlayerObject, SessionCreateData);
	}
	else if (PlayerObject->Implements<UIGoSoccerPlayMode>())
	{
		return IIGoSoccerPlayMode::Execute_HostSingleAIGame(PlayerObject, SessionCreateData);
	}
	return false;
}

//bool UGoSoccerPlayManager::RefreshPlayerCardArr(UObject* PlayerStateObject, TArray<FPlayerCard>& NewPlayerCardArr)
//{
//	if (!NullCheck(PlayerStateObject, TEXT("PlayerStateObject"), TEXT("RefreshPlayerCardArr"))) return false;
//
//	if (GEngine)
//	{
//		FString LogString = FString::Printf(TEXT("RefreshPlayerCardArr Num : %d"), NewPlayerCardArr.Num());
//		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, LogString);
//	}
//	return IIGoSoccerPlayerState::Execute_RefreshPlayerCardArr(PlayerStateObject, NewPlayerCardArr);
//}

bool UGoSoccerPlayManager::SetDollColor(UObject* PlayerObject, uint8 DollColor)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("SetDollColor"))) return false;
	if (PlayerObject->Implements<UIGoSoccerPlayerState>())
	{
		return IIGoSoccerPlayerState::Execute_SetDollColor(PlayerObject, DollColor);
	}
	else if (PlayerObject->Implements<UIGoSoccerPlayerController>())
	{
		return IIGoSoccerPlayerController::Execute_SetDollColor(PlayerObject, DollColor);
	}
	return false;
}

//bool UGoSoccerPlayManager::NotifyPlayerCardChange(UObject* GameModeObject, const FPlayerCard& NewPlayerCard)
//{
//	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("NotifyPlayerCardChange"))) return false;
//	return IIGoSoccerPlayMode::Execute_NotifyPlayerCardChange(GameModeObject, NewPlayerCard);
//}

bool UGoSoccerPlayManager::ClientLoadingCompleted(UObject* PlayerControllerObject, UObject* GameModeObject)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("ClientLoadingCompleted"))) return false;
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("ClientLoadingCompleted"))) return false;
	return IIGoSoccerPlayMode::Execute_ClientLoadingCompleted(GameModeObject, PlayerControllerObject);
}

bool UGoSoccerPlayManager::RefreshPlayerCardListView(UObject* PlayerControllerObject)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("RefreshPlayerCardListView"))) return false;
	return IIGoSoccerPlayerController::Execute_RefreshPlayerCardListView(PlayerControllerObject);
}

bool UGoSoccerPlayManager::RefreshAllPlayerCardListView(UObject* GameModeObject)
{
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("RefreshAllPlayerCardListView"))) return false;
	return IIGoSoccerPlayMode::Execute_RefreshPlayerCardArrWidgets(GameModeObject);
}

bool UGoSoccerPlayManager::RefreshAllPlayerCardListView_Except(UObject* GameModeObject, UObject* ExceptPlayerControllerObject)
{
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("RefreshAllPlayerCardListView_Except"))) return false;
	if (!GameModeObject->Implements<UIGoSoccerPlayMode>()) return false;
	return IIGoSoccerPlayMode::Execute_RefreshPlayerCardArrWidgets_Except(GameModeObject, ExceptPlayerControllerObject);
}

bool UGoSoccerPlayManager::PlayerWin_HotSeat(UObject* PlayerObject, uint8 InDollColor)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("PlayerWin_HotSeat"))) return false;
	return IIGoSoccerPlayerController::Execute_WinGame_HotSeat(PlayerObject, InDollColor);
}

bool UGoSoccerPlayManager::PlayerWin(UObject* PlayerObject)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("PlayerWin"))) return false;
	return IIGoSoccerPlayerController::Execute_WinGame(PlayerObject);
}

bool UGoSoccerPlayManager::PlayerLose(UObject* PlayerObject)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("PlayerLose"))) return false;
	return IIGoSoccerPlayerController::Execute_LoseGame(PlayerObject);
}

bool UGoSoccerPlayManager::PlayerDraw(UObject* PlayerObject)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("PlayerDraw"))) return false;
	return IIGoSoccerPlayerController::Execute_DrawGame(PlayerObject);
}

bool UGoSoccerPlayManager::PlayerGiveUp(UObject* PlayerObject)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("PlayerWalkOver"))) return false;
	return IIGoSoccerPlayerController::Execute_GiveUpGame(PlayerObject);
}

bool UGoSoccerPlayManager::PlayerWalkOver(UObject* GameModeObject, UObject* GaveUpPlayerObject)
{
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("PlayerWalkOver"))) return false;
	if (!NullCheck(GaveUpPlayerObject, TEXT("GaveUpPlayerObject"), TEXT("PlayerWalkOver"))) return false;
	return IIGoSoccerPlayMode::Execute_WalkOverPlayer(GameModeObject, GaveUpPlayerObject);
}

bool UGoSoccerPlayManager::NotifyPlayerTurn(UObject* PlayerControllerObject, const FName& TurnPlayerName)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("NotifyPlayerTurn"))) return false;
	return IIGoSoccerPlayerController::Execute_NotifyPlayerNameOfTurnPlayer(PlayerControllerObject, TurnPlayerName);
}

bool UGoSoccerPlayManager::GetCameraObject(UObject* GameModeObject, UObject*& CameraObject)
{
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("GetCameraObject"))) return false;
	UObject* GoBoardActor = nullptr;
	IIGoSoccerPlayMode::Execute_GetGoBoardActor(GameModeObject, GoBoardActor);
	if (!NullCheck(GoBoardActor, TEXT("GoBoardActor"), TEXT("Execute_GetGoBoardActor"))) return false;
	return IIGoBoard::Execute_GetStageCameraObject(GoBoardActor, CameraObject);
}

bool UGoSoccerPlayManager::SetGlobalTimeDilation(UObject* GameStateObject, float DilationScale)
{
	if (!NullCheck(GameStateObject, TEXT("GameStateObject"), TEXT("SetGlobalTimeDilation"))) return false;
	return IIGoSoccerGameState::Execute_SetGlobalTimeDilation(GameStateObject, DilationScale);
}

bool UGoSoccerPlayManager::ThrowContinueMatchWidget(UObject* PlayerControllerObject, bool bRespond, const FMatchScoreLampStateConfig& InMatchScoreLampStateConfig)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("ThrowContinueMatchWidget"))) return false;
	return IIGoSoccerPlayerController::Execute_ThrowContinueMatchWidget(PlayerControllerObject, bRespond, InMatchScoreLampStateConfig);
}

bool UGoSoccerPlayManager::WindupMatchWidget(UObject* PlayerControllerObject, const FMatchScoreLampStateConfig& InMatchScoreLampStateConfig)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("WindupMatchWidget"))) return false;
	return IIGoSoccerPlayerController::Execute_WindupMatchWidget(PlayerControllerObject, InMatchScoreLampStateConfig);
}

bool UGoSoccerPlayManager::ContinueMatch(UObject* GameModeObject)
{
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("ContinueMatch"))) return false;
	return IIGoSoccerPlayMode::Execute_ContinueMatch(GameModeObject);
}

bool UGoSoccerPlayManager::SendChattingMessage(UObject* PlayerControllerObject, const FText& TextMessage)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("SendChattingMessage"))) return false;
	return IIGoSoccerPlayerController::Execute_SendChattingMessage(PlayerControllerObject, TextMessage);
}

bool UGoSoccerPlayManager::AddChattingMessage(UObject* GameModeObject, const FText& InTextMessage, const FName& InPlayerName)
{
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("AddChattingMessage"))) return false;
	return IIGoSoccerPlayMode::Execute_AddChattingMessage(GameModeObject, InTextMessage, InPlayerName);
}

bool UGoSoccerPlayManager::ReceiveChattingMessage(UObject* PlayerControllerObject, const FName& SendPlayerName, const FText& TextMessage)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("ReceiveChattingMessage"))) return false;
	return IIGoSoccerPlayerController::Execute_ReceiveChattingMessage(PlayerControllerObject, SendPlayerName, TextMessage);
}

bool UGoSoccerPlayManager::StackChattingMessage(UObject* GameInstanceObject, UObject* ChattingBoxObject)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("StackChattingMessage"))) return false;
	return IIGoSoccerGameInstance::Execute_StackChattingMessage(GameInstanceObject, ChattingBoxObject);
}

bool UGoSoccerPlayManager::GetSteamID(UObject* GameInstanceObject, int64& OutSteamID)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("GetSteamID"))) return false;
	OutSteamID = IIGoSoccerGameInstance::Execute_GetCSteamID(GameInstanceObject);
	return OutSteamID >= 0;
}

bool UGoSoccerPlayManager::PostSteamID(UObject* GameStateObject, UObject* PlayerControllerObject, int64 RawSteamID)
{
	if (!NullCheck(GameStateObject, TEXT("GameStateObject"), TEXT("PostSteamID"))) return false;
	return IIGoSoccerGameState::Execute_PostSteamID(GameStateObject, PlayerControllerObject, RawSteamID);
}

bool UGoSoccerPlayManager::GetSteamAvatar(UObject* GameStateObject, FUniqueNetIdRepl& FindNetID, UObject*& OutAvatarTexture2DObject)
{
	if (!NullCheck(GameStateObject, TEXT("GameStateObject"), TEXT("GetSteamAvatar"))) return false;
	return IIGoSoccerGameState::Execute_GetSteamAvatar(GameStateObject, FindNetID, OutAvatarTexture2DObject);
}

bool UGoSoccerPlayManager::RequestSteamAvatar(UObject* GameStateObject, int64 RawSteamID, const FOnSteamAvatarLoaded& OnAvatarLoaded)
{
	if (!NullCheck(GameStateObject, TEXT("GameStateObject"), TEXT("RequestSteamAvatar"))) return false;
	return IIGoSoccerGameState::Execute_RequestSteamAvatar(GameStateObject, RawSteamID, OnAvatarLoaded);
}

bool UGoSoccerPlayManager::GetSteamOSSOnline(UObject* GameInstanceObject)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("GetSteamOSSOnline"))) return false;
	return IIGoSoccerGameInstance::Execute_GetSteamOSSOnline(GameInstanceObject);
}

bool UGoSoccerPlayManager::Steam_InviteFriend(UObject* GameInstanceObject, int64 FriendSteamID)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("Steam_InviteFriend"))) return false;
	return IIGoSoccerGameInstance::Execute_Steam_InviteFriend(GameInstanceObject, FriendSteamID);
}

bool UGoSoccerPlayManager::Steam_GetInvitableFriends(UObject* GameInstanceObject, TArray<FSteamFriendData>& InInvitableArr)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("Steam_GetInvitableFriend"))) return false;
	return IIGoSoccerGameInstance::Execute_Steam_GetInvitableFriends(GameInstanceObject, InInvitableArr);
}

bool UGoSoccerPlayManager::PlayerJoinedOrCreatedSessionComplete(UObject* PlayerObject, uint64 RawSteamID)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("PlayerJoinedOrCreatedSessionComplete"))) return false;
	if (PlayerObject->Implements<UIGoSoccerPlayerController>())
	{
		int64 iRawSteamID = static_cast<int64>(RawSteamID);
		return IIGoSoccerPlayerController::Execute_PlayerJoinedOrCreatedSessionComplete(PlayerObject, iRawSteamID);
	}
	return false;
}

bool UGoSoccerPlayManager::PlayerJoinedSessionComplete(UObject* PlayerObject)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("PlayerJoinedSession"))) return false;
	if (PlayerObject->Implements<UIGoSoccerPlayerController>())
	{
		return IIGoSoccerPlayerController::Execute_PlayerJoinedSessionComplete(PlayerObject);
	}
	return false;
}

bool UGoSoccerPlayManager::PlayerCreateSessionComplete(UObject* PlayerObject)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("PlayerCompletedSessionComplete"))) return false;
	if (PlayerObject->Implements<UIGoSoccerPlayerController>())
	{
		return IIGoSoccerPlayerController::Execute_PlayerCreateSessionComplete(PlayerObject);
	}
	return false;
}

bool UGoSoccerPlayManager::OnlineSubsystemLoaded(UObject* PlayerObject)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("OnlineSubsystemLoaded"))) return false;
	return IIGoSoccerPlayerController::Execute_OnlineSubsystemLoaded(PlayerObject);
}

bool UGoSoccerPlayManager::Notify_PlayerJoinedSessionComplete(UObject* GameModeObject, UObject* JoinedPlayerObject)
{
	if (!NullCheck(JoinedPlayerObject, TEXT("JoinedPlayerObject"), TEXT("Notify_PlayerJoinedSessionComplete"))) return false;
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("PlayerJoinedSession"))) return false;

	if (GameModeObject->Implements<UIGoSoccerPlayMode>())
	{
		return IIGoSoccerPlayMode::Execute_Notify_PlayerJoinedSessionComplete(GameModeObject, JoinedPlayerObject);
	}
	return false;
}

bool UGoSoccerPlayManager::Notify_PlayerLeftSession(UObject* GameModeObject, UObject* LeftPlayerObject)
{
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("Notify_PlayerLeftSession"))) return false;
	if (!GameModeObject->Implements<UIGoSoccerPlayMode>()) return false;
	return IIGoSoccerPlayMode::Execute_Notify_PlayerLeftSession(GameModeObject, LeftPlayerObject);
}

bool UGoSoccerPlayManager::GetSessionCreateData_FromSessionInterface(UObject* GameInstanceObject, FSessionCreateData& OutSessionCreateData)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("GetSessionCreateData_FromSessionInterface"))) return false;
	return IIGoSoccerGameInstance::Execute_GetSessionCreateData_FromSessionInterface(GameInstanceObject, OutSessionCreateData);
}

bool UGoSoccerPlayManager::SetSessionCreateData_ToGameInstance(UObject* GameInstanceObject, const FSessionCreateData& InSessionCreateData)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("SetSessionCreateData_ToGameInstance"))) return false;
	return IIGoSoccerGameInstance::Execute_SetSessionCreateData(GameInstanceObject, InSessionCreateData);
}

bool UGoSoccerPlayManager::SwitchDollColor(UObject* PlayerControllerObject)// ?
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("SwitchDollColor"))) return false;
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("UGoSoccerPlayManager::SwitchDollColor"));
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, LogString);
	}
#endif
	return true;
}

//bool UGoSoccerPlayManager::ChangePlayerName(UObject* GameInstanceObject, UObject* PlayerControllerObject, FName NewPlayerName)
//{
//	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("ChangePlayerName"))) return false;
//	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("ChangePlayerName"))) return false;
//	if (!GameInstanceObject->Implements<UIGoSoccerGameInstance>()) return false;
//	IIGoSoccerGameInstance::Execute_SetPlayerCard_PlayerName(GameInstanceObject, NewPlayerName);
//	return IIGoSoccerPlayerController::Execute_LoadPlayerCardFromGameInstance(PlayerControllerObject);
//}

bool UGoSoccerPlayManager::ChangePlayerName(UObject* PlayerObject, const FName& NewPlayerName)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("ChangePlayerName"))) return false;
	if (PlayerObject->Implements<UIGoSoccerPlayerController>())
	{
		return IIGoSoccerPlayerController::Execute_ChangePlayerName(PlayerObject, NewPlayerName);
	}
	if (PlayerObject->Implements<UIGoSoccerPlayerState>())
	{
		return IIGoSoccerPlayerState::Execute_SetPlayerName(PlayerObject, NewPlayerName, true);
	}
	return false;
}

//bool UGoSoccerPlayManager::ChangePlayerDollColor(UObject* GameInstanceObject, UObject* PlayerControllerObject, uint8 DollColor)
//{
//	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("ChangePlayerDollColor"))) return false;
//	if (!GameInstanceObject->Implements<UIGoSoccerGameInstance>()) return false;
//	IIGoSoccerGameInstance::Execute_SetPlayerCard_PlayerDollColor(GameInstanceObject, DollColor);
//	return IIGoSoccerPlayerController::Execute_LoadPlayerCardFromGameInstance(PlayerControllerObject);
//}

bool UGoSoccerPlayManager::ChangePlayerDollColor(UObject* PlayerObject, uint8 DollColor)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("ChangePlayerDollColor"))) return false;
	if (PlayerObject->Implements<UIGoSoccerPlayerController>())
	{
		return IIGoSoccerPlayerController::Execute_ChangePlayerDollColor(PlayerObject, DollColor);
	}
	if (PlayerObject->Implements<UIGoSoccerPlayerState>())
	{
		return IIGoSoccerPlayerState::Execute_SetPlayerDollColor(PlayerObject, DollColor, true);
	}
	return false;
}

bool UGoSoccerPlayManager::SetMainUIState(UObject* GameInstanceObject, UObject* PlayerControllerObject, const EMainUIState& NewMainUIState)
{
	if (NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("SetMainUIState")))
	{
		IIGoSoccerGameInstance::Execute_SetCurrentMainUIState(GameInstanceObject, NewMainUIState);
	}
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("UGoSoccerPlayManager"))) return false;
	IIGoSoccerPlayerController::Execute_SetMainUIState(PlayerControllerObject, NewMainUIState);
	ECameraPosition CamPos = UIStateToCamPosition(NewMainUIState);
	if (CamPos != ECameraPosition::ECP_Default) IIGoSoccerPlayerController::Execute_SetCameraPosition(PlayerControllerObject, CamPos);
	return true;
}

bool UGoSoccerPlayManager::GetMainUIState(UObject* GameInstanceObject, EMainUIState& OutMainUIState)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("GetMainUIState"))) return false;
	return IIGoSoccerGameInstance::Execute_GetCurrentMainUIState(GameInstanceObject, OutMainUIState);
}

bool UGoSoccerPlayManager::ChangeSessionName(UObject* PlayerControllerObject, const FName& NewSessionName)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("ChangeSessionName"))) return false;
	return IIGoSoccerPlayerController::Execute_ChangeCreateSessionName(PlayerControllerObject, NewSessionName);
}

bool UGoSoccerPlayManager::StartCameraShake(UObject* PlayerControllerObject, float Scale)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("StartCameraShake"))) return false;
	return IIGoSoccerPlayerController::Execute_StartCameraShake(PlayerControllerObject, Scale);
}

bool UGoSoccerPlayManager::Flicked_InvertCylinderCharge(UObject* GameStateObject, int32 InFlickCylinderInvert)
{
	if (!NullCheck(GameStateObject, TEXT("GameStateObject"), TEXT("Flicked_InvertCylinderCharge"))) return false;
	return IIGoSoccerGameState::Execute_Flicked_InvertCylinderCharge(GameStateObject, InFlickCylinderInvert);
}

bool UGoSoccerPlayManager::Flicked_InvertCylinderCharge_SetInvertParam(UObject* PlayerControllerObject, int32 InFlickCylinderInvert)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("Flicked_InvertCylinderCharge_SetInvertParam"))) return false;
	return IIGoSoccerPlayerController::Execute_Flicked_InvertCylinderCharge_SetInvertParam(PlayerControllerObject, InFlickCylinderInvert);
}

bool UGoSoccerPlayManager::Flicked_Locking(UObject* PlayerControllerObject, UObject* GameStateObject, bool bLocked)
{
	if (!NullCheck(GameStateObject, TEXT("GameStateObject"), TEXT("Flicked_Locking"))) return false;
	return IIGoSoccerGameState::Execute_Flicked_Locking(GameStateObject, bLocked, PlayerControllerObject);
}

bool UGoSoccerPlayManager::Flicked_Locking_SetBoolParam(UObject* PlayerControllerObject, bool bLocked)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("Flicked_Locking_SetBoolParam"))) return false;
	return IIGoSoccerPlayerController::Execute_Flicked_Locking_SetBoolParam(PlayerControllerObject, bLocked);
}

bool UGoSoccerPlayManager::Placed_NextDollType_Received(UObject* GameStateObject, const EDollType InNextDollType)
{
	if (!NullCheck(GameStateObject, TEXT("GameStateObject"), TEXT("Placed_NextDollType_Received"))) return false;
	return IIGoSoccerGameState::Execute_Placed_NextDollType_Received(GameStateObject, InNextDollType);
}

bool UGoSoccerPlayManager::Placed_NextDollType_Received_SetDollType(UObject* PlayerControllerObject, const EDollType InNextDollType)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("Placed_NextDollType_Received_SetDollType"))) return false;
	return IIGoSoccerPlayerController::Execute_NextDollType_Received_SetDollType(PlayerControllerObject, InNextDollType);
}

bool UGoSoccerPlayManager::ApplySavedSettings(UObject* GameInstanceObject)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("ApplySavedSettings"))) return false;
	return IIGoSoccerGameInstance::Execute_ApplySavedSettings(GameInstanceObject);
}

bool UGoSoccerPlayManager::ChangeSettings_MaxFPS(UObject* GameInstanceObject, float NewMaxFPS)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("ChangeSettings_MaxFPS"))) return false;
	return IIGoSoccerGameInstance::Execute_SetMaxFPS(GameInstanceObject, NewMaxFPS);
}

bool UGoSoccerPlayManager::ChangeSettings_bUseLowTexture(UObject* GameInstanceObject, bool NewbUseLowTexture)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("ChangeSettings_bUseLowTexture"))) return false;
	return IIGoSoccerGameInstance::Execute_SetbUseLowTexture(GameInstanceObject, NewbUseLowTexture);
}

bool UGoSoccerPlayManager::ChangeSettings_fShadowLevel(UObject* GameInstanceObject, float NewfShadowLevel)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("ChangeSettings_fShadowLevel"))) return false;
	return IIGoSoccerGameInstance::Execute_SetfShadowLevel(GameInstanceObject, NewfShadowLevel);
}

bool UGoSoccerPlayManager::ChangeSettings_AA(UObject* GameInstanceObject, int32 NewAAType)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("ChangeSettings_AA"))) return false;
	return IIGoSoccerGameInstance::Execute_SetAAType(GameInstanceObject, NewAAType);
}

bool UGoSoccerPlayManager::ChangeSettings_Float(UObject* GameInstanceObject, ESettingConfigParamType SettingConfigParamType, float fNewLevel)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("ChangeSettings_Float"))) return false;
	return IIGoSoccerGameInstance::Execute_SetSettingFloat(GameInstanceObject, SettingConfigParamType, fNewLevel);
}

bool UGoSoccerPlayManager::ChangeSettings_ScreenResolution(UObject* GameInstanceObject, FIntPoint NewResolution)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("ChangeSettings_ScreenResolution"))) return false;
	return IIGoSoccerGameInstance::Execute_SetScreenResolution(GameInstanceObject, NewResolution);
}

bool UGoSoccerPlayManager::ChangeSettings_WindowMode(UObject* GameInstanceObject, EWindowMode::Type NewWindowMode)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("ChangeSettings_WindowMode"))) return false;
	return IIGoSoccerGameInstance::Execute_SetWindowMode(GameInstanceObject, NewWindowMode);
}

bool UGoSoccerPlayManager::ChangeSettings_Language(UObject* GameInstanceObject, const FString& NewCulture)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("ChangeSettings_Language"))) return false;
	return IIGoSoccerGameInstance::Execute_SetCultureSetting(GameInstanceObject, NewCulture);
}

bool UGoSoccerPlayManager::Request_UpdateTooltip_SessionConfig(UObject* GameModeObject, UObject* PlayerControllerObject)
{
	if (!NullCheck(GameModeObject, TEXT("GamemodeObject"), TEXT("Request_UpdateTooltip_SessionConfig"))) return false;
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("Request_UpdateTooltip_SessionConfig"))) return false;
	// 메인 메뉴 등 GoSoccerPlayMode 가 아닌 레벨에서도 불릴 수 있다.
	if (!GameModeObject->Implements<UIGoSoccerPlayMode>()) return false;
	return IIGoSoccerPlayMode::Execute_Request_UpdateTooltip_SessionConfig(GameModeObject, PlayerControllerObject);
}

bool UGoSoccerPlayManager::Request_UpdateTooltip_SessionConfig_ToAll(UObject* PlayerControllerObject, const FSessionCreateData& InHostSessionCreateData)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("Request_UpdateTooltip_SessionConfig_ToAll"))) return false;
	return IIGoSoccerPlayerController::Execute_Request_UpdateTooltip_SessionConfig_ToAll(PlayerControllerObject, InHostSessionCreateData);
}

bool UGoSoccerPlayManager::Notify_UpdateTooltip_SessionConfig(UObject* GameModeObject, const FSessionCreateData& InHostSessionCreateData)
{
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("Notify_UpdateTooltip_SessionConfig"))) return false;
	if (!GameModeObject->Implements<UIGoSoccerPlayMode>()) return false;
	return IIGoSoccerPlayMode::Execute_Notify_UpdateTooltip_SessionConfig(GameModeObject, InHostSessionCreateData);
}

bool UGoSoccerPlayManager::UpdateTooltip_SessionConfig(UObject* PlayerControllerObject, const FSessionCreateData& HostSessionCreateData)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("UpdateTooltip_SessionConfig"))) return false;
	return IIGoSoccerPlayerController::Execute_UpdateTooltip_SessionConfig(PlayerControllerObject, HostSessionCreateData);
}

bool UGoSoccerPlayManager::SetHostingSessionCreateData_ToGameMode(UObject* GameModeObject, const FSessionCreateData& InHostSessionCreateData)
{
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("SetHostingSessionCreateData_ToGameMode"))) return false;
	if (!GameModeObject->Implements<UIGoSoccerPlayMode>()) return false;
	return IIGoSoccerPlayMode::Execute_SetHostingSessionCreateData(GameModeObject, InHostSessionCreateData);
}

bool UGoSoccerPlayManager::GetHostingSessionCreateData_FromGameMode(UObject* GameModeObject, FSessionCreateData& OutHostSessionCreateData)
{
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("GetHostingSessionCreateData_FromGameMode"))) return false;
	if (!GameModeObject->Implements<UIGoSoccerPlayMode>()) return false;
	return IIGoSoccerPlayMode::Execute_GetHostingSessionCreateData(GameModeObject, OutHostSessionCreateData);
}

bool UGoSoccerPlayManager::GetHostingSessionCreateData_FromGameInstance(UObject* GameInstanceObject, FSessionCreateData& OutHostSessionCreateData)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("GetHostingSessionCreateData_FromGameInstance"))) return false;
	return IIGoSoccerGameInstance::Execute_GetHostingSessionCreateData(GameInstanceObject, OutHostSessionCreateData);
}

//bool UGoSoccerPlayManager::GetMainWidgetAsObject(UObject* PlayerControllerObject, UObject*& OutMainWidget)
//{
//	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("GetMainWidgetAsObject"))) return false;
//	return IIGoSoccerPlayerController::Execute_GetMainWidgetAsObject(PlayerControllerObject, OutMainWidget);
//}

bool UGoSoccerPlayManager::ChangeWidgetSelected(UObject* WidgetObjecct, bool bSelected)
{
	if (!NullCheck(WidgetObjecct, TEXT("WidgetObjecct"), TEXT("ChangeWidgetSelected"))) return false;
	if (!WidgetObjecct->Implements<UIGoWidget>()) return false;
	return IIGoWidget::Execute_OnSelectedChanged(WidgetObjecct, bSelected);
}

bool UGoSoccerPlayManager::GetPlayGameMode(UObject* PlayerObject, EPlayGameMode& OutPlayGameMode)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("SetPlayGameMode"))) return false;
	return IIGoSoccerGameInstance::Execute_GetPlayGameMode(PlayerObject, OutPlayGameMode);
}

bool UGoSoccerPlayManager::SetPlayGameMode(UObject* PlayerObject, EPlayGameMode InPlayGameMode)
{
	if (!NullCheck(PlayerObject, TEXT("PlayerObject"), TEXT("SetPlayGameMode"))) return false;
	IIGoSoccerGameInstance::Execute_SetPlayGameMode(PlayerObject, InPlayGameMode);
	return true;
}

bool UGoSoccerPlayManager::EndProcess(UObject* PlayerControllerObject)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("EndProcess"))) return false;
	return IIGoSoccerPlayerController::Execute_EndProcess(PlayerControllerObject);
}

bool UGoSoccerPlayManager::ThrowTemporalMessage(UObject* PlayerControllerObject, const FString& ThrowMessage, ETemporalMessageType TemporalMessageType)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("ThrowTemporalMessage"))) return false;
	return IIGoSoccerPlayerController::Execute_ThrowTemporalMessage(PlayerControllerObject, ThrowMessage, TemporalMessageType);
}

bool UGoSoccerPlayManager::SetCameraPosition(UObject* PlayerControllerObject, ECameraPosition CameraPosition)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("SetCameraPosition"))) return false;
	return IIGoSoccerPlayerController::Execute_SetCameraPosition(PlayerControllerObject, CameraPosition);
}

bool UGoSoccerPlayManager::CloseInGameSettingsWidget(UObject* PlayerControllerObject)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("CloseInGameSettingsWidget"))) return false;
	return IIGoSoccerPlayerController::Execute_CloseInGameSettingsWidget(PlayerControllerObject);
}

bool UGoSoccerPlayManager::ClientTravel_UILoaded(UObject* PlayerControllerObject, EMainUIState SucceededUIState, const FString& URL, ETravelType TravelType, bool bSeamless)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("ClientTravel_UILoaded"))) return false;
	return IIGoSoccerPlayerController::Execute_ClientTravel_UILoaded(PlayerControllerObject, SucceededUIState, URL, TravelType, bSeamless);
}

bool UGoSoccerPlayManager::MarkUIState_ClientTravelCallback(UObject* GameInstanceObject, EMainUIState SucceededUIState, EMainUIState FailededUIState)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("MarkUIState_ClientTravelCallback"))) return false;
	return IIGoSoccerGameInstance::Execute_MarkUIState_ClientTravelCallback(GameInstanceObject, SucceededUIState, FailededUIState);
}

bool UGoSoccerPlayManager::GetClientTravelCallbackUIState(UObject* GameInstanceObject, EMainUIState& OutSucceededUIState, EMainUIState& OutFailededUIState)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("GetClientTravelCallbackUIState"))) return false;
	return IIGoSoccerGameInstance::Execute_GetClientTravelCallbackUIState(GameInstanceObject, OutSucceededUIState, OutFailededUIState);
}

bool UGoSoccerPlayManager::SetSessionLoadingScreen(UObject* PlayerControllerObject, bool bEnable, ESessionLoadingPhase LoadingPhase)
{
	// 초대로 부팅된 직후처럼 로컬 PlayerController 가 아직 없는 경우가 정상적으로 존재하므로
	// NullCheck 의 에러 로그를 태우지 않고 조용히 빠진다. (상태는 GameInstance 에 남아있다)
	if (PlayerControllerObject == nullptr) return false;
	if (!PlayerControllerObject->Implements<UIGoSoccerPlayerController>()) return false;
	return IIGoSoccerPlayerController::Execute_SetSessionLoadingScreen(PlayerControllerObject, bEnable, LoadingPhase);
}

bool UGoSoccerPlayManager::MarkSessionLoadingPhase(UObject* GameInstanceObject, ESessionLoadingPhase LoadingPhase)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("MarkSessionLoadingPhase"))) return false;
	return IIGoSoccerGameInstance::Execute_MarkSessionLoadingPhase(GameInstanceObject, LoadingPhase);
}

bool UGoSoccerPlayManager::GetSessionLoadingPhase(UObject* GameInstanceObject, ESessionLoadingPhase& OutLoadingPhase)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("GetSessionLoadingPhase"))) return false;
	return IIGoSoccerGameInstance::Execute_GetSessionLoadingPhase(GameInstanceObject, OutLoadingPhase);
}

bool UGoSoccerPlayManager::SetPlayerSessionHost(UObject* GameInstanceObject, bool e)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("SetPlayerSessionHost"))) return false;
	if (GameInstanceObject->Implements<UIGoSoccerGameInstance>())
	{
		return IIGoSoccerGameInstance::Execute_SetPlayerSessionHost(GameInstanceObject, e);
	}
	else if (GameInstanceObject->Implements<UIGoSoccerPlayerState>())
	{
		return IIGoSoccerPlayerState::Execute_SetPlayerSessionHost(GameInstanceObject, e);
	}
	return false;
}

bool UGoSoccerPlayManager::GetPlayerSessionHost(UObject* GameInstanceObject, bool& OutIsHost)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("GetPlayerSessionHost"))) return false;
	if (GameInstanceObject->Implements<UIGoSoccerGameInstance>())
	{
		return IIGoSoccerGameInstance::Execute_GetPlayerSessionHost(GameInstanceObject, OutIsHost);
	}
	else if (GameInstanceObject->Implements<UIGoSoccerPlayerState>())
	{
		return IIGoSoccerPlayerState::Execute_GetPlayerSessionHost(GameInstanceObject, OutIsHost);
	}
	return false;
}

//bool UGoSoccerPlayManager::Steam_GetSteamAvatar(UObject* GameInstance, UObject*& SteamAvatarObjectPtr)
//{
//	if (!NullCheck(GameInstance, TEXT("GameInstance"), TEXT("Steam_GetSteamAvatar"))) return false;
//	//SteamAvatarObjectPtr = IIGoSoccerGameInstance::Execute_GetSteamAvatar(GameInstance);
//	return nullptr;
//}

bool UGoSoccerPlayManager::BindDelegate_Widget(UObject* BindObject, UObject* WidgetObject)
{
	if (!NullCheck(BindObject, TEXT("BindObject"), TEXT("BindDelegate_Widget"))) return false;
	if (!NullCheck(WidgetObject, TEXT("WidgetObject"), TEXT("BindDelegate_Widget"))) return false;
	if (BindObject->Implements<UIGoSoccerGameInstance>())
	{
		return IIGoSoccerGameInstance::Execute_BindDelegate_Widget(BindObject, WidgetObject);
	}
	return false;
}

bool UGoSoccerPlayManager::OpenLocalHost(UObject* PlayerControllerObject)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("OpenLocalHost"))) return false;
	return IIGoSoccerPlayerController::Execute_OpenLocalHost(PlayerControllerObject);
}

bool UGoSoccerPlayManager::JoinLocalHost(UObject* PlayerControllerObject)
{
	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("JoinLocalHost"))) return false;
	return IIGoSoccerPlayerController::Execute_JoinLocalHost(PlayerControllerObject);
}

bool UGoSoccerPlayManager::StartLocalHostGame(UObject* GameModeObject, const FSessionCreateData& SessionCreateData)
{
	if (!NullCheck(GameModeObject, TEXT("GameModeObject"), TEXT("StartLocalHostGame"))) return false;
	return IIGoSoccerPlayMode::Execute_StartLocalHostGame(GameModeObject, SessionCreateData);
}

bool UGoSoccerPlayManager::GetPlayerName(UObject* PlayerStateObject, FName& OutPlayerName)
{
	if (!NullCheck(PlayerStateObject, TEXT("PlayerStateObject"), TEXT("GetPlayerName"))) return false;
	return IIGoSoccerPlayerState::Execute_GetPlayerName(PlayerStateObject, OutPlayerName);
}

bool UGoSoccerPlayManager::GetOnlineIdentityPlayerNickName(UObject* GameInstanceObject, FString& OutPlayerName)
{
	if (!NullCheck(GameInstanceObject, TEXT("GameInstanceObject"), TEXT("GetPlayerName"))) return false;
	return IIGoSoccerGameInstance::Execute_GetIdentityPlayerNickName(GameInstanceObject, OutPlayerName);
}

EDollType UGoSoccerPlayManager::ConvertIntToDollType(int32 In)
{
	if (In < 1) return EDollType::EDT_Default;
	else if (In < 2) return EDollType::EDT_Explosive;
	else if (In < 3) return EDollType::EDT_Beam;
	return EDollType::EDT_Default;
}

//void UGoSoccerPlayManager::Debug_DrawDebugLine(UObject* PlayerControllerObject, FVector StartLocation, FVector EndLocation, FColor DrawColor)
//{
//	if (!NullCheck(PlayerControllerObject, TEXT("PlayerControllerObject"), TEXT("DrawDebugLine"))) return;
//	IIGoSoccerPlayerController::Execute_Debug_DrawDebugLine(PlayerControllerObject, StartLocation, EndLocation, DrawColor);
//}

bool UGoSoccerPlayManager::NullCheck(UObject* CheckObject, FName ObjectName, FName FuncName)
{
	if (CheckObject == nullptr)
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("%s : %s Is Nullptr"), *FuncName.ToString(), *ObjectName.ToString());
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, LogString);
		}
#endif
		return false;
	}
	return true;
}

ECameraPosition UGoSoccerPlayManager::UIStateToCamPosition(EMainUIState MainUIState)
{
	if (MainUIState == EMainUIState::EMUIS_GameStarted)
	{
		return ECameraPosition::ECP_InPlay;
	}
	else if (MainUIState == EMainUIState::EMUIS_SessionCreate ||
		MainUIState == EMainUIState::EMUIS_VeryFirst)
	{
		return ECameraPosition::ECP_Default;
	}
	else if (MainUIState == EMainUIState::EMUIS_SessionSearch ||
		MainUIState == EMainUIState::EMUIS_InLobby)
	{
		return ECameraPosition::ECP_Session;
	}
	return ECameraPosition::ECP_Default;
}

