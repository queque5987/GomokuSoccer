#include "Actor/Doll_Fragile_Explosive.h"
#include "Components/SphereComponent.h"
#include "GoSoccerPlayManager.h"

constexpr float EXPLOSIVE_DOLL_DAMAGE = 20.f;

ADoll_Fragile_Explosive::ADoll_Fragile_Explosive() : Super()
{
	Explosive_Radius = CreateDefaultSubobject<USphereComponent>(TEXT("ExeplosiveRadius"));
	if (Explosive_Radius != nullptr)
	{
		Explosive_Radius->SetupAttachment(GetRootComponent());
		Explosive_Radius->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel10);
		Explosive_Radius->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void ADoll_Fragile_Explosive::ComponentHit_V(const FHitResult& Hit)
{
	// Self Damage -> Brake When It Hit
	if (Hit.GetActor() && Hit.GetActor()->Implements<UIGoDoll>())
	{
		FDollDamageConfig DollDamageConfig{
			EXPLOSIVE_DOLL_DAMAGE,
			Hit.GetActor() ? Hit.GetActor()->GetVelocity() : Hit.ImpactNormal, 
			GetActorLocation(),
			Hit.Location,
			0.f
		};
		//Execute_SendDamageToHP(this, 10.f, Hit.GetActor() ? Hit.GetActor()->GetVelocity() : Hit.ImpactNormal, GetActorLocation(), Hit.Location, 0.f);
		Execute_SendDamageToHP(this, DollDamageConfig);
	}
}

//void ADoll_Fragile_Explosive::SendDamageToHP_Implementation(float DamageAmount, FVector HitDirection, FVector InHitLocation, FVector InImpulseLocation, float KnockBackAmount)
void ADoll_Fragile_Explosive::SendDamageToHP_Implementation(const FDollDamageConfig& DollDamageConfig)
{
	CurrHP = FMath::Clamp(CurrHP - DollDamageConfig.DamageAmount, 0.f, MaxHP);

	//NetMulticast_DollCrack((MaxHP - CurrHP) / MaxHP * 2.f);

	if (CurrHP <= 0.f)
	{
		BrokenLocation = DollDamageConfig.HitLocation;
		ImpulseDirection = DollDamageConfig.HitDirection;
		ImpulseLocation = DollDamageConfig.ImpulseLocation;
		Server_SetDollAsFallen();
		//DollBroken(Collided_HitVelocity.Size2D() > Self_HitVelocity.Size2D() ? Collided_HitVelocity : Self_HitVelocity, CollidedActor->GetActorLocation());
	}
}

void ADoll_Fragile_Explosive::NetMulticast_Debris_FireImpulse(FVector FireDirection, float Power, FVector CauserLocation)
{
	Super::NetMulticast_Debris_FireImpulse_Implementation(FireDirection, Power, CauserLocation);
	UE_LOG(LogTemp, Log, TEXT("NetMulticast_Debris_FireImpulse_Implementation"));
}

void ADoll_Fragile_Explosive::DollBroken(FVector HitDirection, FVector CauserLocation)
{
	Super::DollBroken(HitDirection, CauserLocation);

	if (Explosive_Radius == nullptr) return;
	Explosive_Radius->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	Explosive_Radius->SetWorldLocation(CauserLocation);
	TArray<AActor*> OverrlappingActorArr;
	Explosive_Radius->GetOverlappingActors(OverrlappingActorArr);
	
	//DrawDebugSphere(GetWorld(), CauserLocation + FVector(0.f, 0.f, 5.f), 12.5f, 32, FColor::Cyan, false, 3.f);
#if UE_EDITOR
	DrawDebugSphere(GetWorld(), Explosive_Radius->GetComponentLocation() + FVector(0.f, 0.f, 5.f), 12.5f, 32, FColor::Red, false, 3.f);
	UE_LOG(LogTemp, Log, TEXT("DollBroken_Eplosive - OverrlappingActorArr -----"));
#endif
	uint8 ThisDollColor;
	bool bCheckColorFlag = Execute_GetDollcolor(this, ThisDollColor);
	int32 idx = 0;
	for (auto Iter : OverrlappingActorArr)
	{
		if (Iter == nullptr ||
			!Iter->Implements<UIGoDoll>() || 
			Iter == this || 
			IIGoDoll::Execute_GetDollFallFromBoard(Iter)) continue;
		bool bIsFriendly = !bCheckColorFlag;
		if (bCheckColorFlag)
		{
			uint8 IterDollColor;
			if (IIGoDoll::Execute_GetDollcolor(Iter, IterDollColor))
			{
				bIsFriendly = ThisDollColor == IterDollColor;
			}
		}
#if UE_EDITOR
		UE_LOG(LogTemp, Log, TEXT("DollBroken_Eplosive - InRange : [%d] %s"), idx++, *Iter->GetFName().ToString());
		UE_LOG(LogTemp, Log, TEXT("DollBroken_Eplosive - Dist From Center : %f - Radius : %f"), FVector::Dist2D(CauserLocation, Iter->GetActorLocation()), Explosive_Radius->GetScaledSphereRadius());
#endif
		FVector tempImpulseDirection = Iter->GetActorLocation() - CauserLocation;
		float R = Explosive_Radius->GetScaledSphereRadius();
		float D = FVector::Dist2D(CauserLocation, Iter->GetActorLocation());
		float RD = FMath::Pow((R - D) / R, 2.f) / 5.f;
		tempImpulseDirection.Z += RD * D;
		//FVector Direction = tempImpulseDirection.GetSafeNormal();
		//IIGoDoll::Execute_FlickDoll(Iter, bIsFriendly ? tempImpulseDirection * 2.f : tempImpulseDirection * 5.f);
		IIGoDoll::Execute_Simple_AddImpulse(Iter, tempImpulseDirection, bIsFriendly ? ExplodeImpulsePower_Friendly : ExplodeImpulsePower);
		if (!bIsFriendly)
		{
			FDollDamageConfig DollDamageConfig{
				EXPLOSIVE_DOLL_DAMAGE,
				tempImpulseDirection,
				Iter->GetActorLocation(),
				CauserLocation,
				0.f
			};
			IIGoDoll::Execute_SendDamageToHP(Iter, DollDamageConfig);
		}
		UGoSoccerPlayManager::MarkDollAsMoved(GetWorld()->GetAuthGameMode(), Iter);
		//DrawDebugDirectionalArrow(GetWorld(), CauserLocation, Dest, 12.f, FColor::Green, false, 3.f);
		//DrawDebugSphere(GetWorld(), Iter->GetActorLocation() + FVector(0.f, 0.f, 5.f), 5.f, 32, FColor::Green, false, 3.f);
	}
#if UE_EDITOR
	UE_LOG(LogTemp, Log, TEXT("DollBroken_Eplosive - OverrlappingActorArr End"));
#endif
}
