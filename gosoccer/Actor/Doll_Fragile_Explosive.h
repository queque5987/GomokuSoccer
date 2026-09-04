#pragma once

#include "CoreMinimal.h"
#include "Actor/Doll_Fragile.h"
#include "Doll_Fragile_Explosive.generated.h"

class USpaherComponent;

UCLASS()
class GOSOCCER_API ADoll_Fragile_Explosive : public ADoll_Fragile
{
	GENERATED_BODY()

	ADoll_Fragile_Explosive();
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> Explosive_Radius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ExplodeImpulsePower;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ExplodeImpulsePower_Friendly;

protected:
	virtual void ComponentHit_V(const FHitResult& Hit) override;
	virtual void SendDamageToHP_Implementation(const FDollDamageConfig& DollDamageConfig) override;
	virtual void NetMulticast_Debris_FireImpulse(FVector FireDirection, float Power, FVector CauserLocation) override;
	virtual void DollBroken(FVector HitDirection, FVector CauserLocation) override;
};
