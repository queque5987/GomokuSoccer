#include "Actor/Doll_Fragile.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GoSoccerPlayManager.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

constexpr float MAX_DAMAGE_WEIGHT = 1.f;
constexpr float MIN_DAMAGE_WEIGHT = 0.1f;
constexpr float DAMAGED_WEIGHT = 0.3f;
constexpr float NORMAL_DAMAGE_MAX = 20.f;
constexpr float NORMAL_DAMAGE = 5.f;
static const FName MID_PARAM_CRACKPERCENT = FName("Crack_Percent");
static const FName MID_PARAM_CRACKCOLOR = FName("Crack_Color");
constexpr float DILATION_SCALE_HIT = 0.65f;
constexpr float DILATION_SCALE_BROKEN = 0.25f;
constexpr float FRENDILY_FIRE_DAMAGE_PERCENT = 0.5f;
constexpr float FLICKED_DOLL_TAKE_DAMAGE_PERCENT = 0.2f;
//constexpr float DILATION_MAX_TIME = 0.125f;
//constexpr float DILATION_TIMER_RATE = 0.016667;
//constexpr float DILATION_INTERP_EXP = 12.f;

ADoll_Fragile::ADoll_Fragile() : Super()
{
	//Initialize_ComponentAttachment();
	Doll_GeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("FragileDollComponent"));
	//Doll_RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComponent"));
	Doll_FieldSystemComponent = CreateDefaultSubobject<UFieldSystemComponent>(TEXT("FieldSystemComponent"));

	if (DollMeshComponent && Doll_GeometryCollectionComponent && Doll_FieldSystemComponent)
	{
		Doll_GeometryCollectionComponent->SetupAttachment(DollMeshComponent);
		Doll_GeometryCollectionComponent->SetRenderCustomDepth(true);
		//Doll_RadialForceComponent->SetupAttachment(Doll_GeometryCollectionComponent);
		//Doll_RadialForceComponent->Radius = 100.f;

		//Doll_RadialForceComponent->ImpulseStrength = 500.f;

		//Doll_RadialForceComponent->Falloff = ERadialImpulseFalloff::RIF_Linear;
		//Doll_RadialForceComponent->AddCollisionChannelToAffect(Doll_GeometryCollectionComponent->GetCollisionObjectType());
		//DollMeshComponent->SetVisibility(false);
		Doll_FieldSystemComponent->SetupAttachment(DollMeshComponent);
		
		DollMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel10, ECollisionResponse::ECR_Overlap);
	}
	//MaxHP = 30.f;
	bFragile = false;
	BrokenLocation = FVector{ TNumericLimits<float>::Max()};
}

void ADoll_Fragile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//FVector V = GetVelocity();
	//if (bFragile)
	//{
	//	DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 4.f), 4.f, 32, FColor::Cyan, false, DeltaTime * 1.1f);
	//}

	if (BrokenLocation.Z < 1000.f && FVector::Dist2D(GetActorLocation(), BrokenLocation) < BrokentLocationThreshold)
	{
		DollBroken(ImpulseDirection, ImpulseLocation);
		BrokenLocation = FVector{ TNumericLimits<float>::Max()};
	}

//	if (HasAuthority() && FVector::Dist2D(LastDollSweepedLocation, GetActorLocation()) > DollSweepedDistThreshold)
//	{
//		LastDollSweepedLocation = GetActorLocation();
//		NetMulticast_Debris_FireImpulse(GetVelocity().GetSafeNormal(), 1.f, LastDollSweepedLocation);
//		// TODO Sweep Debris To Velocity As Direction At LastDollSweepedLocation
//#if !UE_BUILD_SHIPPING
//		DrawDebugSphere(GetWorld(), LastDollSweepedLocation + FVector(0.f, 0.f, 4.f), 4.f, 32, FColor::Orange, false, 3.f);
//#endif
//	}
	//if (DollMeshComponent)
	//{
	//	FPredictProjectilePathParams PredictParams;
	//	PredictParams.StartLocation = GetActorLocation();
	//	PredictParams.LaunchVelocity = GetVelocity();
	//	PredictParams.bTraceWithChannel = true;
	//	PredictParams.TraceChannel = DollMeshComponent->GetCollisionObjectType();
	//	PredictParams.MaxSimTime = 0.25f;
	//	PredictParams.SimFrequency = 10;

	//	FPredictProjectilePathResult PredictResults;

	//	if (UGameplayStatics::PredictProjectilePath(GetWorld(), PredictParams, PredictResults))
	//	{
	//		for (const FPredictProjectilePathPointData& Point : PredictResults.PathData)
	//		{
	//			DrawDebugSphere(GetWorld(), Point.Location, 3.f, 32, FColor::Cyan, false, DeltaTime);
	//		}
	//	}
	//}
}

void ADoll_Fragile::BeginPlay()
{
	Super::BeginPlay();

	//if (DollMeshComponent)
	//{
	//	auto* CurrMI = DollMeshComponent->GetMaterial(0);
	//	MID_DollCrack = DollMeshComponent->CreateDynamicMaterialInstance(0, CurrMI);
	//	if (MID_DollCrack)
	//	{
	//		MID_DollCrack->SetScalarParameterValue(MID_PARAM_CRACKPERCENT, 0.f);
	//		MID_DollCrack->SetVectorParameterValue(MID_PARAM_CRACKCOLOR, CrackOverlayColor);
	//	}
	//}
	CurrHP = MaxHP;
}

void ADoll_Fragile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADoll_Fragile, CurrHP);
	//DOREPLIFETIME(ADoll_Fragile, MaxHP);
}

bool ADoll_Fragile::OnBoardGetStationary_Implementation()
{
	//bool rtn = Super::OnBoardGetStationary_Implementation();
	DealtDollIndexSet.Empty();
	//FlickedDirectionSize = 0.f;
	//FlickedDirection = FVector::ZeroVector;
	DamageWeight = MAX_DAMAGE_WEIGHT;
	bFlicked = false;
#if UE_EDITOR
	UE_LOG(LogTemp, Log, TEXT("OnBoardGetStationary : %s"), *GetFName().ToString());
	//DrawDebugSphere(GetWorld(), GetActorLocation(), 5.f, 32, FColor::Cyan, false, 3.f);
#endif
	return true;
}

void ADoll_Fragile::Doll_TakeDamage(AActor* CollidedActor, const FHitResult& Hit)
{
	if (CollidedActor->IsValidLowLevel() && !CollidedActor->Implements<UIGoDoll>()) return;


	int32 DealtIndex = IIGoDoll::Execute_GetPlacedIndex(CollidedActor);
	if (DealtDollIndexSet.Contains(DealtIndex) || DealtIndex < 0) return;
	FVector Collided_FlickedDirection = IIGoDoll::Execute_GetFlickedDirection(CollidedActor);
	FVector Collided_Velocity = CollidedActor->GetVelocity();
	FVector Collided_HitVelocity = Collided_FlickedDirection.Size2D() > Collided_Velocity.Size2D() ? Collided_FlickedDirection : Collided_Velocity;
	FVector Self_HitVelocity = FlickedDirection.Size2D() > GetVelocity().Size2D() ? FlickedDirection : GetVelocity();

	//if (GetWorld()) UGoSoccerPlayManager::SetGlobalTimeDilation(GetWorld()->GetGameState<AGameStateBase>(), DILATION_SCALE_HIT);

	float Hit_Velocity = FMath::Max(IIGoDoll::Execute_GetFlickedDirectionSize(CollidedActor), CollidedActor->GetVelocity().Size());

	auto Calculate_Velocity_Alpha = [&](float InOtherVelocity) -> float
		{
			const float MaxVelocityCapacity = 1000.f;
			InOtherVelocity = FMath::Clamp(InOtherVelocity, 0.f, MaxVelocityCapacity);
			InOtherVelocity *= 0.001f;
			return FMath::Pow(InOtherVelocity, 0.5f);
		};
	float Velocity_Alpha = Calculate_Velocity_Alpha(Hit_Velocity);
	float DamageAmount = FMath::Lerp(NORMAL_DAMAGE, NORMAL_DAMAGE_MAX, Velocity_Alpha);
	DamageAmount = FMath::Max(DamageWeight * DamageAmount, NORMAL_DAMAGE);
	//CurrHP = FMath::Clamp(CurrHP - DamageAmount, 0.f, MaxHP);
	DamageWeight = FMath::Clamp(DamageWeight * DAMAGED_WEIGHT, MIN_DAMAGE_WEIGHT, MAX_DAMAGE_WEIGHT);
	DealtDollIndexSet.Add(DealtIndex);
	uint8 CollidedDollColor;
	IIGoDoll::Execute_GetDollcolor(CollidedActor, CollidedDollColor);
	if (bFlicked) 
	{
#if UE_EDITOR
		UE_LOG(LogTemp, Log, TEXT("Flicked Doll Damage * %.1f - %s"), FLICKED_DOLL_TAKE_DAMAGE_PERCENT, *GetFName().ToString());
		DrawDebugSphere(GetWorld(), GetActorLocation(), 5.f, 32, FColor::Cyan, false, 1.f);
#endif
		DamageAmount *= FLICKED_DOLL_TAKE_DAMAGE_PERCENT;
	}
	else if (CollidedDollColor == DollColor)
	{
#if UE_EDITOR
		UE_LOG(LogTemp, Log, TEXT("Friendly Doll Damage * %.1f - %s"), FRENDILY_FIRE_DAMAGE_PERCENT, *GetFName().ToString());
		DrawDebugSphere(GetWorld(), GetActorLocation(), 5.f, 32, FColor::Cyan, false, 1.f);
#endif
		DamageWeight *= FRENDILY_FIRE_DAMAGE_PERCENT;
	}
	UE_LOG(LogTemp, Log, TEXT("Hit_Velocity : %f, DamageAmount : %f"), Hit_Velocity, DamageAmount);
	UE_LOG(LogTemp, Log, TEXT("CurrHP : %f, MaxHP : %f, MID Value : %f"), CurrHP, MaxHP, (MaxHP - CurrHP) / MaxHP * 2.f);

	FDollDamageConfig DollDamageConfig{
		DamageAmount,
		Collided_HitVelocity.Size2D() > Self_HitVelocity.Size2D() ? Collided_HitVelocity : Self_HitVelocity,
		Hit.Location,
		CollidedActor->GetActorLocation(),
		0.f
	};
	//Execute_SendDamageToHP(this, 
	//	DamageAmount,
	//	Collided_HitVelocity.Size2D() > Self_HitVelocity.Size2D() ? Collided_HitVelocity : Self_HitVelocity,
	//	Hit.Location,
	//	CollidedActor->GetActorLocation(), 0.f
	//);
	Execute_SendDamageToHP(this, DollDamageConfig);
	//NetMulticast_DollCrack((MaxHP - CurrHP) / MaxHP * 2.f);

	//if (CurrHP <= 0.f)
	//{
	//	BrokenLocation = Hit.Location;
	//	ImpulseDirection = Collided_HitVelocity.Size2D() > Self_HitVelocity.Size2D() ? Collided_HitVelocity : Self_HitVelocity;
	//	ImpulseLocation = CollidedActor->GetActorLocation();
	//	Server_SetDollAsFallen();
	//	//DollBroken(Collided_HitVelocity.Size2D() > Self_HitVelocity.Size2D() ? Collided_HitVelocity : Self_HitVelocity, CollidedActor->GetActorLocation());
	//}
}

bool ADoll_Fragile::FlickDoll_Implementation(FVector FlickDirection)
{
	bool rtn = Super::FlickDoll_Implementation(FlickDirection);

	DamageWeight = MIN_DAMAGE_WEIGHT;
	FlickedDirectionSize = FlickDirection.Size();
	FlickedDirection = FlickDirection;
	bFlicked = true;
#if UE_EDITOR
	DrawDebugSphere(GetWorld(), GetActorLocation(), 5.f, 32, FColor::Red, false, 1.f);
#endif
	//UE_LOG(LogTemp, Log, TEXT("FlickedDirectionSize Set : %d - %f"), Execute_GetPlacedIndex(this), FlickedDirectionSize);
	return rtn;
}

bool ADoll_Fragile::GetCurrHP_Implementation(float& OutHP)
{
	OutHP = CurrHP;
	return true;
}

void ADoll_Fragile::NetMulticast_DollCrack_Implementation(float fCrackPercent)
{
	UpdateCrackPercent(fCrackPercent);
	//if (MID_DollCrack)
	//{
	//	MID_DollCrack->SetScalarParameterValue(MID_PARAM_CRACKPERCENT, fCrackPercent);
	//}
}

bool ADoll_Fragile::SetDollCustomStencil_Implementation(uint8 StencilValue)
{
	bool rtn = Super::SetDollCustomStencil_Implementation(StencilValue);
	if (Doll_GeometryCollectionComponent == nullptr) return false;
	Doll_GeometryCollectionComponent->SetCustomDepthStencilValue(StencilValue);
	return rtn;
}

bool ADoll_Fragile::SetDollFallFromBoard_Implementation(bool e, bool bPlaySoundFx)
{
	bool rtn = Super::SetDollFallFromBoard_Implementation(e, bPlaySoundFx);
	return rtn;
}

void ADoll_Fragile::Server_SetDollAsFallen_Implementation()
{
	Execute_SetDollFallFromBoard(this, true, false);
}

void ADoll_Fragile::NetMulticast_DollBroken_Implementation(FVector HitDirection, FVector CauserLocation)
{
	if (Doll_GeometryCollectionComponent && DollMeshComponent && Doll_FieldSystemComponent)
	{
		Doll_GeometryCollectionComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		Doll_GeometryCollectionComponent->SetSimulatePhysics(true);
		Debris_FireImpulse(HitDirection.GetSafeNormal(), HitDirection.Size(), CauserLocation);
		if (GetWorld() == nullptr || GetWorld()->GetFirstPlayerController() == nullptr) return;
		UGoSoccerPlayManager::StartCameraShake(GetWorld()->GetFirstPlayerController(), 1.f);
	}
}

void ADoll_Fragile::Server_DollBroken_Implementation(FVector HitDirection, FVector CauserLocation)
{
	if (Doll_GeometryCollectionComponent && DollMeshComponent && Doll_FieldSystemComponent)
	{
		DollMeshComponent->SetSimulatePhysics(false);
		DollMeshComponent->SetWorldLocation(
			DollMeshComponent->GetComponentLocation() + FVector(0.f, 0.f, -50.f)
		);
		if (GetWorld()) UGoSoccerPlayManager::SetGlobalTimeDilation(GetWorld()->GetGameState(), DILATION_SCALE_BROKEN);
	}
}

void ADoll_Fragile::NetMulticast_Debris_FireImpulse_Implementation(FVector FireDirection, float Power, FVector CauserLocation)
{
	Debris_FireImpulse(FireDirection, Power, CauserLocation);
}

void ADoll_Fragile::SendDamageToHP_Implementation(const FDollDamageConfig& DollDamageConfig)
{
	if (!bInvincible)
	{
		CurrHP = FMath::Clamp(CurrHP - DollDamageConfig.DamageAmount, 0.f, MaxHP);
	}
	UE_LOG(LogTemp, Log, TEXT("%s - DamageAmount : %f"), *GetFName().ToString(), DollDamageConfig.DamageAmount);
	NetMulticast_DollCrack((MaxHP - CurrHP) / MaxHP * 2.f);


	if (CurrHP <= 0.f)
	{
		BrokenLocation = DollDamageConfig.HitLocation; //InHitLocation;
		ImpulseDirection = DollDamageConfig.HitDirection;
		ImpulseLocation = DollDamageConfig.ImpulseLocation;
		Server_SetDollAsFallen();
		//DollBroken(Collided_HitVelocity.Size2D() > Self_HitVelocity.Size2D() ? Collided_HitVelocity : Self_HitVelocity, CollidedActor->GetActorLocation());
	}
	else if (GetWorld())
	{
		if (bEnableDamageKnockBack && DollDamageConfig.KnockBackAmount > 0.f)
		{
			//FVector DamDir = DollDamageConfig.HitDirection * DollDamageConfig.KnockBackAmount;
			Execute_Simple_AddImpulse(this, DollDamageConfig.HitDirection, DollDamageConfig.KnockBackAmount);
			//DollMeshComponent->AddImpulse(DamDir, NAME_None, true);
			//UObject* GameModeObject = Cast<UObject>(GetWorld()->GetAuthGameMode());
			//UGoSoccerPlayManager::MarkDollAsMoved(GameModeObject, this);
		}
		UGoSoccerPlayManager::SetGlobalTimeDilation(GetWorld()->GetGameState<AGameStateBase>(), DILATION_SCALE_HIT);
	}
}

void ADoll_Fragile::DollBroken(FVector HitDirection, FVector CauserLocation)
{
	if (Doll_GeometryCollectionComponent && DollMeshComponent && Doll_FieldSystemComponent)
	{
		NetMulticast_PlaySoundAtActorLocation(DollBrokeSoundFX, 1.f);
		NetMulticast_DollBroken(HitDirection, CauserLocation);
		Server_DollBroken(HitDirection, CauserLocation);
		//Doll_GeometryCollectionComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		//Doll_GeometryCollectionComponent->SetSimulatePhysics(true);
		//Debris_FireImpulse(HitDirection.GetSafeNormal(), HitDirection.Size(), CauserLocation);
		//DollMeshComponent->SetSimulatePhysics(false);
		//DollMeshComponent->SetWorldLocation(
		//	DollMeshComponent->GetComponentLocation() + FVector(0.f, 0.f, - 50.f)
		//);
	}
}
