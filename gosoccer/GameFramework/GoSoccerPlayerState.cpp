#include "GameFramework/GoSoccerPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "GoSoccerPlayManager.h"

AGoSoccerPlayerState::AGoSoccerPlayerState()
{
	bReplicates = true;
}

void AGoSoccerPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGoSoccerPlayerState, PlayerCard);
	DOREPLIFETIME(AGoSoccerPlayerState, HostingSessionCreateData);
	DOREPLIFETIME(AGoSoccerPlayerState, bIsHost);
}

void AGoSoccerPlayerState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//auto* W = GetCachedWorld();
	//if (W == nullptr) return;
	//double CurrentTime = W->GetTimeSeconds() - GameStartTimeStamp;
	//if (CurrentTime > 0.f)
	//{
	//	// TODO
	//}
}

void AGoSoccerPlayerState::OnRep_Owner()
{
	Super::OnRep_Owner();
}

void AGoSoccerPlayerState::Destroyed()
{
	Super::Destroyed();

#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Log, TEXT("Destroyed : %s"), *GetFName().ToString());
#endif

}

bool AGoSoccerPlayerState::GetPlayerCard_Implementation(FPlayerCard& OutPlayerCard)
{
	OutPlayerCard = PlayerCard;
	return true;
}

bool AGoSoccerPlayerState::SetPlayerCard_Implementation(FPlayerCard& InPlayerCard)
{
	Server_SetPlayerCard(InPlayerCard);
	return true;
}

bool AGoSoccerPlayerState::GetDollColor_Implementation(uint8& OutDollColor)
{
	OutDollColor = PlayerCard.PlayerDollColor;
	return true;
}

bool AGoSoccerPlayerState::SetDollColor_Implementation(uint8 DollColor)
{
	PlayerCard.PlayerDollColor = DollColor;
	return true;
}

bool AGoSoccerPlayerState::TurnStart_Implementation()
{
	PlayerCard.IsMyTurn = true;
	if (HasAuthority())
	{
		OnRep_PlayerCard();
	}
	return true;
}

bool AGoSoccerPlayerState::TurnEnd_Implementation()
{
	PlayerCard.IsMyTurn = false;
	if (HasAuthority())
	{
		OnRep_PlayerCard();
	}
	return true;
}

bool AGoSoccerPlayerState::ReadyGame_Implementation()
{
	Server_SwitchReadyGame();
	return true;
}

bool AGoSoccerPlayerState::SetReadyState_Implementation(bool bReady)
{
	Server_SetReadyState(bReady);
	return true;
}

bool AGoSoccerPlayerState::ClientLoadingCompleted_Implementation()
{
	for (FConstPlayerControllerIterator Iter = GetWorld()->GetPlayerControllerIterator(); Iter; ++Iter)
	{
		if (Iter->Get() == nullptr) continue;
		UE_LOG(LogTemp, Log, TEXT("AGoSoccerPlayerState : ClientLoadingCompleted - %s"), Iter->Get() != nullptr ? *Iter->Get()->GetFName().ToString() : TEXT("Nullptr"));
	}
	return false;
}

bool AGoSoccerPlayerState::SetPlayerName_Implementation(const FName& NewPlayerName, bool bUpdate)
{
	Server_SetPlayerName(NewPlayerName);
	return true;
}

bool AGoSoccerPlayerState::SetPlayerDollColor_Implementation(uint8 NewDollColor, bool bUpdate)
{
	Server_SetPlayerDollColor(NewDollColor);
	return true;
}

bool AGoSoccerPlayerState::GetPlayerName_Implementation(FName& OutPlayerName)
{
	OutPlayerName = PlayerCard.PlayerName;
	return true;
}

bool AGoSoccerPlayerState::SetHostingSessionCreateData_Implementation(const FSessionCreateData& InHostingSessionCreateData)
{
	Server_SetHostingSessionCreateData(InHostingSessionCreateData);
	//HostingSessionCreateData = InHostingSessionCreateData;
	return true;
}

bool AGoSoccerPlayerState::GetHostingSessionCreateData_Implementation(FSessionCreateData& OutHostingSessionCreateData)
{
	OutHostingSessionCreateData = HostingSessionCreateData;
	return HostingSessionCreateData.PlayGameRule != EPlayGameRule::EPGR_Default;
}

bool AGoSoccerPlayerState::ClearHostingSessionCreateData_Implementation()
{
	HostingSessionCreateData = FSessionCreateData();
	return true;
}

bool AGoSoccerPlayerState::SetPlayerSessionHost_Implementation(bool InbIsHost)
{
	bIsHost = InbIsHost;
	return true;
}

bool AGoSoccerPlayerState::GetPlayerSessionHost_Implementation(bool& OutbIsHost)
{
	OutbIsHost = bIsHost;
	return true;
}

void AGoSoccerPlayerState::Server_SwitchReadyGame_Implementation()
{
	PlayerCard.bGameReady = !PlayerCard.bGameReady;
	if (HasAuthority())
	{
		OnRep_PlayerCard();
	}
}

void AGoSoccerPlayerState::Server_SetHostingSessionCreateData_Implementation(const FSessionCreateData& InHostingSessionCreateData)
{
	HostingSessionCreateData = InHostingSessionCreateData;
}

void AGoSoccerPlayerState::OnRep_PlayerCard()
{
	UWorld* W = GetWorld();
	if (W == nullptr) return;
	// 서버에서도 이 함수를 직접 부르는데, GetFirstPlayerController() 는 리슨 서버에서
	// 원격 클라이언트의 PC 를 집을 수 있다. 원격 클라이언트는 자기 쪽 OnRep 으로 갱신되므로
	// 여기서는 이 머신의 로컬 PlayerController 만 갱신한다.
	for (FConstPlayerControllerIterator Iter = W->GetPlayerControllerIterator(); Iter; ++Iter)
	{
		APlayerController* PC = Iter->Get();
		if (PC == nullptr || !PC->IsLocalController()) continue;
		UGoSoccerPlayManager::RefreshPlayerCardListView(PC);
	}
}

UWorld* AGoSoccerPlayerState::GetCachedWorld()
{
	if (CachedWorld.IsValid()) return CachedWorld.Get();
	CachedWorld = GetWorld();
	return CachedWorld.Get();
}

void AGoSoccerPlayerState::Server_SetReadyState_Implementation(bool bReady)
{
	PlayerCard.bGameReady = bReady;
	if (HasAuthority())
	{
		OnRep_PlayerCard();
	}
}

void AGoSoccerPlayerState::Server_SetPlayerName_Implementation(const FName& NewPlayerName)
{
	PlayerCard.PlayerName = NewPlayerName;
	if (HasAuthority())
	{
		OnRep_PlayerCard();
	}
}

void AGoSoccerPlayerState::Server_SetPlayerDollColor_Implementation(uint8 NewDollColor)
{
	PlayerCard.PlayerDollColor = NewDollColor;
	if (HasAuthority())
	{
		OnRep_PlayerCard();
	}
}

void AGoSoccerPlayerState::Server_SetPlayerCard_Implementation(const FPlayerCard& InPlayerCard)
{
	PlayerCard = InPlayerCard;
	if (HasAuthority())
	{
		OnRep_PlayerCard();
	}
}

void AGoSoccerPlayerState::Server_OnRep_PlayerCard_Implementation()
{
}
