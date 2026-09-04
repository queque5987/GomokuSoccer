#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Interface/IGoSoccerPlayerState.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "GoSoccerPlayerState.generated.h"

UCLASS()
class GOSOCCER_API AGoSoccerPlayerState : public APlayerState, public IIGoSoccerPlayerState
{
	GENERATED_BODY()

	AGoSoccerPlayerState();

	UPROPERTY(ReplicatedUsing = OnRep_PlayerCard)
	FPlayerCard PlayerCard;

	UPROPERTY(Replicated)
	FSessionCreateData HostingSessionCreateData;

	UPROPERTY(Replicated)
	bool bIsHost;

	TWeakObjectPtr<UWorld> CachedWorld;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnRep_Owner() override;
	virtual void Destroyed() override;
public:
	virtual bool GetPlayerCard_Implementation(FPlayerCard& OutPlayerCard) override;
	virtual bool SetPlayerCard_Implementation(FPlayerCard& InPlayerCard) override;

	//virtual bool RefreshPlayerCardArr_Implementation(TArray<FPlayerCard>& NewPlayerCardArr) override;
	virtual bool GetDollColor_Implementation(uint8& OutDollColor) override;
	virtual bool SetDollColor_Implementation(uint8 DollColor) override;
	virtual bool TurnStart_Implementation() override;
	virtual bool TurnEnd_Implementation() override;
	virtual bool ReadyGame_Implementation() override;
	virtual bool SetReadyState_Implementation(bool bReady) override;
	virtual bool ClientLoadingCompleted_Implementation() override;

	virtual bool SetPlayerName_Implementation(const FName& NewPlayerName, bool bUpdate) override;
	virtual bool SetPlayerDollColor_Implementation(uint8 NewDollColor, bool bUpdate) override;
	virtual bool GetPlayerName_Implementation(FName& OutPlayerName) override;
	virtual bool SetHostingSessionCreateData_Implementation(const FSessionCreateData& InHostingSessionCreateData) override;
	virtual bool GetHostingSessionCreateData_Implementation(FSessionCreateData& OutHostingSessionCreateData) override;
	virtual bool ClearHostingSessionCreateData_Implementation() override;
	virtual bool SetPlayerSessionHost_Implementation(bool InbIsHost) override;
	virtual bool GetPlayerSessionHost_Implementation(bool& OutbIsHost) override;

	//UFUNCTION(Client, Reliable)
	//void Client_RefreshPlayerCardListView(const TArray<FPlayerCard>& NewPlayerCardArr);
	UFUNCTION(Server, Reliable)
	void Server_SetHostingSessionCreateData(const FSessionCreateData& InHostingSessionCreateData);
	UFUNCTION(Server, Reliable)
	void Server_SwitchReadyGame();
	UFUNCTION()
	void OnRep_PlayerCard();
	UFUNCTION(Server, Reliable)
	void Server_OnRep_PlayerCard();
	UFUNCTION(Server, Reliable)
	void Server_SetPlayerCard(const FPlayerCard& InPlayerCard);
	UFUNCTION(Server, Reliable)
	void Server_SetPlayerName(const FName& NewPlayerName);
	UFUNCTION(Server, Reliable)
	void Server_SetPlayerDollColor(uint8 NewDollColor);
	UFUNCTION(Server, Reliable)
	void Server_SetReadyState(bool bReady);
private:
	UWorld* GetCachedWorld();
};
