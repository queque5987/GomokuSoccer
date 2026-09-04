#pragma once

#include "CoreMinimal.h"
#include "Actor/Doll.h"
#include "Doll_Fragile.generated.h"

class UGeometryCollectionComponent;
class URadialForceComponent;
class UFieldSystemComponent;

UCLASS()
class GOSOCCER_API ADoll_Fragile : public ADoll
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGeometryCollectionComponent> Doll_GeometryCollectionComponent;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	//TObjectPtr<URadialForceComponent> Doll_RadialForceComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFieldSystemComponent> Doll_FieldSystemComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> DollBrokeSoundFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Material, meta = (AllowPrivateAccess = "true"))
	FVector4 CrackOverlayColor;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MID_DollCrack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Parameters, meta = (AllowPrivateAccess = "true"))
	float BrokentLocationThreshold;

	UPROPERTY(Replicated)
	float CurrHP;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Parameters, meta = (AllowPrivateAccess = "true"))
	float MaxHP;
	UPROPERTY()
	TSet<int32> DealtDollIndexSet;
	UPROPERTY()
	float FlickedDirectionSize;
	UPROPERTY()
	FVector FlickedDirection;
	UPROPERTY()
	float DamageWeight;
	UPROPERTY()
	bool bFragile;
	UPROPERTY()
	bool bFlicked;
	UPROPERTY()
	FVector BrokenLocation;
	UPROPERTY()
	FVector ImpulseDirection;
	UPROPERTY()
	FVector ImpulseLocation;
	UPROPERTY()
	bool bInvincible = false;

	UPROPERTY()
	FVector LastDollSweepedLocation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Debug, meta = (AllowPrivateAccess = "true"))
	float DollSweepedDistThreshold = 5.f;
public:
	ADoll_Fragile();
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	virtual float GetFlickedDirectionSize_Implementation() override { return FlickedDirectionSize; };
	virtual FVector GetFlickedDirection_Implementation() override { return FlickedDirection; };
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool OnBoardGetStationary_Implementation() override;
	virtual void Doll_TakeDamage(AActor* CollidedActor, const FHitResult& Hit) override;
	virtual bool SetDollCustomStencil_Implementation(uint8 StencilValue) override;
	virtual bool SetDollFallFromBoard_Implementation(bool e, bool bPlaySoundFx = true) override;

	virtual bool FlickDoll_Implementation(FVector FlickDirection) override;
	virtual bool GetCurrHP_Implementation(float& OutHP) override;
	virtual bool IsDollConsideredStationary_Implementation() override { return Super::IsDollConsideredStationary_Implementation(); };

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCrackPercent(float fCrackPercent);
	UFUNCTION(BlueprintImplementableEvent)
	void Debris_FireImpulse(FVector FireDirection, float Power, FVector CauserLocation);
	UFUNCTION(Server, Unreliable)
	void Server_SetDollAsFallen();
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_DollCrack(float fCrackPercent);
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_DollBroken(FVector HitDirection, FVector CauserLocation);
	UFUNCTION(Server, Reliable)
	void Server_DollBroken(FVector HitDirection, FVector CauserLocation);
	UFUNCTION(NetMulticast, Reliable)
	virtual void NetMulticast_Debris_FireImpulse(FVector FireDirection, float Power, FVector CauserLocation);
protected:
	virtual void ComponentHit_V(const FHitResult& Hit) override {};
	//virtual void SendDamageToHP_Implementation(float DamageAmount, FVector HitDirection, FVector InHitLocation, FVector InImpulseLocation, float KnockBackAmount = 0.f) override;
	virtual void SendDamageToHP_Implementation(const FDollDamageConfig& DollDamageConfig) override;
	virtual void DollBroken(FVector HitDirection, FVector CauserLocation);
};
