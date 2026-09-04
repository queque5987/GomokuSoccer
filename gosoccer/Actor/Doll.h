#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/IGoDoll.h"
#include "Doll.generated.h"

class UBoxComponent;
class USphereComponent;

UCLASS()
class GOSOCCER_API ADoll : public AActor, public IIGoDoll
{
	GENERATED_BODY()
	
public:	
	ADoll();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMeshComponent> DollMeshComponent;
	
	//UStaticMeshComponent* StaticMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Ciraular_StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> MouseHitTraceComponent;

	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	uint8 DollColor = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	float ImpulseWeight = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TMap<TSubclassOf<AActor>, TObjectPtr<USoundBase>> HitSoundFXMap;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> PutDollSoundFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> DollFallSoundFX;
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	//TSubclassOf<AActor> GoBoardClass;

	UPROPERTY(Replicated)
	int32 PlacedIndex = -1;
	UPROPERTY(Replicated)
	bool bFallen = false;
	UPROPERTY()
	bool bEnableDamageKnockBack = true;

	UPROPERTY()
	uint8 Client_DollColor;

	void Initialize_ComponentAttachment();
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit);

	virtual bool SetPlacedIndex_Implementation(int32 InPlacedIndex) override;
	virtual int32 GetPlacedIndex_Implementation() override { return PlacedIndex; };
	virtual bool SetDollCustomStencil_Implementation(uint8 StencilValue) override;
	virtual bool FlickDoll_Implementation(FVector FlickDirection) override;
	virtual bool SetWinDollCircularEffect_Implementation(bool bOnEffect) override;
	virtual bool SetDollCircularEffect_Progressive_Implementation(float CurrentTimeSec, float MaxTimeSec) override;
	virtual bool GetDollcolor_Implementation(uint8& OutDollColor) override;
	virtual bool SetDollFallFromBoard_Implementation(bool e, bool bPlaySoundFx = true) override;
	virtual bool GetDollFallFromBoard_Implementation() override { return bFallen; };
	
	virtual float GetFlickedDirectionSize_Implementation() override { return 0.f; };
	virtual FVector GetFlickedDirection_Implementation() override { return FVector(); };
	virtual bool OnBoardGetStationary_Implementation() override;
	virtual bool GetCurrHP_Implementation(float& OutHP) override;
	virtual bool SetDollStencil_Scoring_Implementation() override;
	virtual bool IsDollConsideredStationary_Implementation() override;

	UFUNCTION(Server, Reliable)
	void Server_SetDollFallenFromBoard(bool e, bool bPlaySoundFx);
	UFUNCTION(Server, Reliable)
	void Server_SetPlacedIndex(int32 InPlacedIndex);
	UFUNCTION(Server, Reliable)
	virtual void Server_FlickDoll(FVector FlickDirection);
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_SetWinDollCircularEffect(bool bOnEffect);
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_SetDollCircularEffect_Progressive(float CurrentTimeSec, float MaxTimeSec);
	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticast_PlaySoundAtActorLocation(USoundBase* PlaySound, float Volume, float CameraShakeScale = 0.f);
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_SetDollStencil_Scoring();
	UFUNCTION()
	void Callback_OnComponentWake(UPrimitiveComponent* WakingComponent, FName BoneName);
	/*Called By Server Only*/
	UFUNCTION()
	void Callback_OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION()
	void Callback_OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//void Callback_OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintImplementableEvent)
	void SetDollCircularMID_Auto(bool bEnable);

	UFUNCTION(BlueprintImplementableEvent)
	void SetDollCircularMIDParam(float CurrentTimeSec, float MaxTimeSec, bool bFriendly);
	virtual void Simple_AddImpulse_Implementation(FVector InDirection, float Power) override;

protected:
	virtual void Doll_TakeDamage(AActor* CollidedActor, const FHitResult& Hit) {};
	//virtual void SendDamageToHP_Implementation(float DamageAmount, FVector HitDirection, FVector InHitLocation, FVector InImpulseLocation, float KnockBackAmount = 0.f) override {};
	virtual void SendDamageToHP_Implementation(const FDollDamageConfig& DollDamageConfig) override {};
	virtual void ComponentHit_V(const FHitResult& Hit) {};
	void SpawnFX(FVector SpawnLocation, FRotator SpawnRotation, UObject* SpawnFX, bool bAttached) {};	//TODO
};
