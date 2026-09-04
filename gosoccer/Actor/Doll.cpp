#include "Actor/Doll.h"
#include "Net/UnrealNetwork.h"
#include "GoSoccerPlayManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "GameFramework/GoSoccer_PCH.h"

constexpr float HIT_CAMERASHAKE_WEIGHT = 0.25;
ADoll::ADoll()
{
	PrimaryActorTick.bCanEverTick = true;

	Initialize_ComponentAttachment();

	bReplicates = true;
	//MaxHP = 100.f;
	//CurrHP = MaxHP;
}

void ADoll::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		if (DollMeshComponent)
		{
			DollMeshComponent->OnComponentWake.AddDynamic(this, &ADoll::Callback_OnComponentWake);
			DollMeshComponent->OnComponentHit.AddDynamic(this, &ADoll::Callback_OnComponentHit);
			DollMeshComponent->SetNotifyRigidBodyCollision(true);
		}
	}
	if (DollMeshComponent != nullptr)
	{
		DollMeshComponent->SetRenderCustomDepth(true);
		//DollMeshComponent->SetMassOverrideInKg(NAME_None, 0.1f, true);
		DollMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_EngineTraceChannel1, ECollisionResponse::ECR_Ignore);
		DollMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_EngineTraceChannel2, ECollisionResponse::ECR_Block);
	}
	if (Ciraular_StaticMeshComponent != nullptr)
	{
		Ciraular_StaticMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_EngineTraceChannel1, ECollisionResponse::ECR_Ignore);
	}
	if (MouseHitTraceComponent != nullptr)
	{
		MouseHitTraceComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		MouseHitTraceComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Block);
	}

	FPlayerCard tempPlayerCard;
	auto* tempPC = GetOwner<APlayerController>();
	if (UGoSoccerPlayManager::GetPlayerCard(tempPC, tempPlayerCard))
	{
		Client_DollColor = tempPlayerCard.PlayerDollColor;
	}
}

void ADoll::Initialize_ComponentAttachment()
{
	DollMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DollComponent"));
	Ciraular_StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ciraular_StaticMeshComponent"));
	MouseHitTraceComponent = CreateDefaultSubobject<USphereComponent>(TEXT("MouseHitTraceComponent"));
	if (DollMeshComponent != nullptr)
	{
		SetRootComponent(DollMeshComponent);
	}
	if (Ciraular_StaticMeshComponent != nullptr)
	{
		Ciraular_StaticMeshComponent->SetupAttachment(DollMeshComponent);
		Ciraular_StaticMeshComponent->SetVisibility(false);
	}
	if (MouseHitTraceComponent != nullptr)
	{
		MouseHitTraceComponent->SetupAttachment(DollMeshComponent);
	}
}

void ADoll::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADoll, PlacedIndex);
	DOREPLIFETIME(ADoll, bFallen);
}

void ADoll::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if !UE_BUILD_SHIPPING
	float V = GetVelocity().Size2D();
	//if (CurrHP > 0.01f)
	if (V > 0.01f)
	{
		FString DebugMessage = FString::Printf(TEXT("%s : %f"), bFallen? TEXT("F") : TEXT("V"), V);
		//FString DebugMessage = FString::Printf(TEXT("%.1f / %.1f"), CurrHP, MaxHP);
		DrawDebugString(
			GetWorld(),
			GetActorLocation() + FVector(0.f, 0.f, 8.f),
			DebugMessage, nullptr,
			bFallen ? FColor::Red : FColor::Green,
			DeltaTime * 3.f, false, 1.f
		);
	}
	if (DollMeshComponent && GetVelocity().Size() > 100.f && !bFallen)
	{
		FPredictProjectilePathParams PredictParams;
		PredictParams.StartLocation = GetActorLocation();
		PredictParams.LaunchVelocity = GetVelocity();
		PredictParams.bTraceWithChannel = true;
		PredictParams.TraceChannel = DollMeshComponent->GetCollisionObjectType();
		PredictParams.MaxSimTime = 0.25f;
		PredictParams.SimFrequency = 2;
		FPredictProjectilePathResult PredictResults;

		UGameplayStatics::PredictProjectilePath(GetWorld(), PredictParams, PredictResults);
		int32 N = PredictResults.PathData.Num();
		if (N > 0)
		{
			const FVector ZAxisAdjust{ 0.f, 0.f, 4.f };
			const FVector PredicedLocation{ PredictResults.PathData[N - 1].Location.X, PredictResults.PathData[N - 1].Location.Y, GetActorLocation().Z + 4.f};
			DrawDebugDirectionalArrow(GetWorld(), GetActorLocation() + ZAxisAdjust,PredicedLocation,3.f, FColor::Cyan, false, DeltaTime * 2.f);
		}
		//UE_LOG(LogTemp, Log, TEXT("Predicted : %s"), *GetName());
		//for (const FPredictProjectilePathPointData& Point : PredictResults.PathData)
		//{
		//	DrawDebugSphere(GetWorld(), FVector(Point.Location.X, Point.Location.Y, GetActorLocation().Z) + FVector(0.f, 0.f, 4.f), 3.f, 32, FColor::Cyan, false, DeltaTime);
		//	UE_LOG(LogTemp, Log, TEXT("Predicted : %s"), *Point.Location.ToString());
		//}
	}
#endif
}

void ADoll::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
#if !UE_BUILD_SHIPPING
	//DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 5.f), 3.f, 32, FColor::Green, false, 1.f);
#endif
	//if (HasAuthority())
	//{
	//	UObject* GameModeObject = Cast<UObject>(GetWorld()->GetAuthGameMode());
	//	UGoSoccerPlayManager::MarkDollAsMoved(GameModeObject, this);
	//}
}

bool ADoll::SetPlacedIndex_Implementation(int32 InPlacedIndex)
{
	Server_SetPlacedIndex(InPlacedIndex);
	NetMulticast_PlaySoundAtActorLocation(PutDollSoundFX, 1.f);
	return true;
}

bool ADoll::SetDollCustomStencil_Implementation(uint8 StencilValue)
{
	if (DollMeshComponent == nullptr) return false;
	DollMeshComponent->SetCustomDepthStencilValue(StencilValue);
	return true;
}

bool ADoll::FlickDoll_Implementation(FVector FlickDirection)
{
	if (DollMeshComponent == nullptr) return false;
	Server_FlickDoll(FlickDirection);
	return true;
}

bool ADoll::SetWinDollCircularEffect_Implementation(bool bOnEffect)
{
	NetMulticast_SetWinDollCircularEffect(bOnEffect);
	return Ciraular_StaticMeshComponent != nullptr;
}

bool ADoll::SetDollCircularEffect_Progressive_Implementation(float CurrentTimeSec, float MaxTimeSec)
{
	NetMulticast_SetDollCircularEffect_Progressive(CurrentTimeSec, MaxTimeSec);
	return Ciraular_StaticMeshComponent != nullptr;
}

bool ADoll::GetDollcolor_Implementation(uint8& OutDollColor)
{
	OutDollColor = DollColor;
	return true;
}

bool ADoll::SetDollFallFromBoard_Implementation(bool e, bool bPlaySoundFx)
{
	Server_SetDollFallenFromBoard(e, bPlaySoundFx);
	return true;
}

bool ADoll::OnBoardGetStationary_Implementation()
{
	return true;
}

bool ADoll::GetCurrHP_Implementation(float& OutHP)
{
	return false;
}

bool ADoll::SetDollStencil_Scoring_Implementation()
{
	NetMulticast_SetDollStencil_Scoring();
	return true;
}

bool ADoll::IsDollConsideredStationary_Implementation()
{
	bool rtn = GetVelocity().Size() < 1.f || bFallen;
	//DrawDebugString(
	//	GetWorld(), FVector(0.f, 0.f, 5.f),
	//	rtn ? TEXT("Stable") : TEXT("Unstable"), 
	//	this, rtn ? FColor::Green : FColor::Red, 
	//	rtn ? 1.f : 3.f, false, 1.5f);

	return rtn;
}

void ADoll::Server_SetDollFallenFromBoard_Implementation(bool e, bool bPlaySoundFx)
{
	if (bFallen == e) return;
	bFallen = e;
	if (!bFallen) return;
	UObject* GameModeObject = Cast<UObject>(GetWorld()->GetAuthGameMode());
	UGoSoccerPlayManager::AddFallenScore(GameModeObject, this);
	if (bPlaySoundFx)
	{
		NetMulticast_PlaySoundAtActorLocation(DollFallSoundFX, 1.f);

#if !UE_BUILD_SHIPPING
		DrawDebugSphere(
			GetWorld(),
			GetActorLocation() + FVector(0.f, 0.f, 5.f),
			3.f, 32, FColor::Green, false, 1.f
		);
#endif
	}
}

void ADoll::NetMulticast_SetWinDollCircularEffect_Implementation(bool bOnEffect)
{
	UE_LOG(LogTemp, Log, TEXT("NetMulticast_SetWinDollCircularEffect : %s"), bOnEffect ? TEXT("Enable") : TEXT("Disable"));

	SetDollCircularMID_Auto(bOnEffect);
	SetDollCircularMIDParam(1.f, 1.f, true);

	//if (Ciraular_StaticMeshComponent != nullptr)
	//{
	//	Ciraular_StaticMeshComponent->SetVisibility(bOnEffect);
	//}
}

void ADoll::NetMulticast_PlaySoundAtActorLocation_Implementation(USoundBase* PlaySound, float Volume, float CameraShakeScale)
{
	if (CameraShakeScale > 0.f && GetWorld())
	{
		UGoSoccerPlayManager::StartCameraShake(GetWorld()->GetFirstPlayerController(), CameraShakeScale);
	}

	UGameplayStatics::PlaySoundAtLocation(
		GetWorld(),
		PlaySound,
		GetActorLocation(),
		Volume
	);
}

void ADoll::NetMulticast_SetDollCircularEffect_Progressive_Implementation(float CurrentTimeSec, float MaxTimeSec)
{
	SetDollCircularMIDParam(CurrentTimeSec, MaxTimeSec, DollColor == Client_DollColor);
}

void ADoll::NetMulticast_SetDollStencil_Scoring_Implementation()
{
	auto* ClientPC = GetWorld()->GetFirstPlayerController();
	FPlayerCard tempPlayerCard;
	uint8 tempDollColor;
	if (!UGoSoccerPlayManager::GetPlayerCard(ClientPC, tempPlayerCard) || 
		!Execute_GetDollcolor(this, tempDollColor)) return;
	bool bIsMine = (tempDollColor == tempPlayerCard.PlayerDollColor);
	Execute_SetDollCustomStencil(this, (uint8)(bIsMine ? EDollStencilValue::EDSV_MouseOver : EDollStencilValue::EDSV_MouseOver_NotMine));
}

void ADoll::Callback_OnComponentWake(UPrimitiveComponent* WakingComponent, FName BoneName)
{
	if (HasAuthority())
	{
		//UObject* GameModeObject = Cast<UObject>(GetWorld()->GetAuthGameMode());
		//UGoSoccerPlayManager::MarkDollAsMoved(GameModeObject, this);
	}
}

void ADoll::Callback_OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	auto Calculate_Velocity_Bonus = [&](float InOtherVelocity) -> float
		{
			const float MaxVelocityCapacity = 1000.f;
			InOtherVelocity = FMath::Clamp(InOtherVelocity, 0.f, MaxVelocityCapacity);
			InOtherVelocity *= 0.001f;
			return 1.f + FMath::Pow(InOtherVelocity, 0.3f);
		};
	ComponentHit_V(Hit);
	if (OtherActor != this)
	{
#if !UE_BUILD_SHIPPING
		//if (UGoSoccerPlayManager::IsGoDoll(OtherActor))
		//{
		//	DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 4.f), 3.f, 32, FColor::Red, false, 3.f);
		//	DrawDebugSphere(GetWorld(), OtherActor->GetActorLocation() + FVector(0.f, 0.f, 4.f), 3.f, 32, FColor::Orange, false, 3.f);
		//	DrawDebugSphere(GetWorld(), Hit.Location + FVector(0.f, 0.f, 4.f), 3.f, 32, FColor::Green, false, 3.f);
		//}
#endif
		//if (GoBoardClass != nullptr && OtherActor->IsA(GoBoardClass))
		//{
		//	StaticMeshComponent->SetConstraintMode(EDOFMode::XYPlane);
		//	FString DebugMessage = FString::Printf(TEXT("Hit-%s"), *OtherActor->GetFName().ToString());
		//	DrawDebugString(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 5.f), DebugMessage, nullptr, FColor::Green, 5.f, false, 1.f);
		//}

		if (NormalImpulse.Size() > 10.f)
		{
			UObject* GameModeObject = Cast<UObject>(GetWorld()->GetAuthGameMode());
			UGoSoccerPlayManager::MarkDollAsMoved(GameModeObject, this);
		}
		if (NormalImpulse.Size() > 50.f) // 100 < NormalImpulse.Size() < 1000
		{
			Doll_TakeDamage(OtherActor, Hit);
			if (OtherActor->Implements<UIGoDoll>())
			{
				bool bDamaged = GetVelocity().Size() < OtherActor->GetVelocity().Size();
#if UE_EDITOR
				if (!bDamaged) UGoSoccerPlayManager::AddRealtimeCooldownBonus(GetOwner(), HIT_COOLDOWN_ADVANTAGE);
#endif
				if (!bDamaged && OtherActor->GetOwner() != GetOwner())
				{
					UGoSoccerPlayManager::AddRealtimeCooldownBonus(GetOwner(), HIT_COOLDOWN_ADVANTAGE);
					UGoSoccerPlayManager::AddRealtimeCooldownBonus(GetOwner(), HIT_COOLDOWN_ADVANTAGE, true);
				}
#if !UE_BUILD_SHIPPING
				DrawDebugDirectionalArrow(
					GetWorld(), (bDamaged ? OtherActor->GetActorLocation() : GetActorLocation()) + FVector(0.f, 0.f, 20.f),
					(bDamaged ? GetActorLocation() : OtherActor->GetActorLocation()) + FVector(0.f, 0.f, 5.f),
					15.f, FColor::Green, false, 2.f
				);
#endif
				//for (int32 i = 0; i < 1 + NormalImpulse.Size() / 200.f; ++i)
				//{
				//	DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + NormalImpulse.GetSafeNormal() * i * 10.f, 5.f, FColor::Green, false, 2.f);
				//}

				// Damage
				//float Hit_Velocity = FMath::Max(IIGoDoll::Execute_GetFlickedDirectionSize(OtherActor), OtherActor->GetVelocity().Size());
				//float Velocity_Bonus = Calculate_Velocity_Bonus(Hit_Velocity);
				//float DamageAmount = DamageWeight * Velocity_Bonus * NORMAL_DAMAGE;
				//CurrHP = FMath::Clamp(CurrHP - DamageAmount, 0.f, MaxHP);


				//UE_LOG(LogTemp, Log, TEXT("%s Doll Took %.1f Damage"), DollColor == 0 ? TEXT("Black") : TEXT("White"), DamageAmount);
				////UE_LOG(LogTemp, Log, TEXT("Component Hit By Doll W/ Velocity : %f"), OtherActor->GetVelocity().Size());

				//FString DebugMessage = FString::Printf(TEXT("Damaged %.1f"), DamageAmount);
				//DrawDebugString(
				//	GetWorld(),
				//	//GetActorLocation() + FVector(0.f, 0.f, 8.f),
				//	FVector(0.f, 0.f, 8.f),
				//	DebugMessage, this,
				//	FColor::Red,
				//	2.f, false, 1.f
				//);
			}
			USoundBase* PlaySound = nullptr;
			for (auto& Iter : HitSoundFXMap)
			{
				UClass* KeyClass = Iter.Key;
				if (!OtherActor->GetClass()->IsChildOf(KeyClass)) continue;
				PlaySound = Iter.Value;
			}
			if (PlaySound != nullptr)
			{
				float VolumeSize = (FMath::Loge(NormalImpulse.Size() / 100.f) + 1.f) * 0.6f;
				NetMulticast_PlaySoundAtActorLocation(PlaySound, VolumeSize, VolumeSize * HIT_CAMERASHAKE_WEIGHT);
#if !UE_BUILD_SHIPPING
				if (GEngine)
				UE_LOG(LogTemp, Log, TEXT("Impulse Size : %f, Volume : %f"), NormalImpulse.Size(), VolumeSize);
				{
					FString LogString = FString::Printf(TEXT("Play Sound : %s"), *PlaySound->GetFName().ToString());
					GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, LogString);
				}
#endif
			}
		}
	}
}

void ADoll::Callback_OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void ADoll::Server_SetPlacedIndex_Implementation(int32 InPlacedIndex)
{
	PlacedIndex = InPlacedIndex;
}

void ADoll::Server_FlickDoll_Implementation(FVector FlickDirection)
{
	if (DollMeshComponent == nullptr) return;
	//FlickDirection.Z *= 0.f;
	FlickDirection *= ImpulseWeight;
	DollMeshComponent->AddImpulse(FlickDirection, NAME_None, true);
	UObject* GameModeObject = Cast<UObject>(GetWorld()->GetAuthGameMode());
	UGoSoccerPlayManager::MarkDollAsMoved(GameModeObject, this);
	UGoSoccerPlayManager::FlickDoll_Completed(GameModeObject, Owner);
}

void ADoll::Simple_AddImpulse_Implementation(FVector InDirection, float Power)
{
	if (DollMeshComponent && GetWorld())
	{
		DollMeshComponent->AddImpulse(InDirection * Power, NAME_None, true);
		UObject* GameModeObject = Cast<UObject>(GetWorld()->GetAuthGameMode());
		UGoSoccerPlayManager::MarkDollAsMoved(GameModeObject, this);
	}
}