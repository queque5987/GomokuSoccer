#include "Actor/Doll_Fragile_Beam.h"
#include "Components/CapsuleComponent.h"
#include "GoSoccerPlayManager.h"

ADoll_Fragile_Beam::ADoll_Fragile_Beam() : Super()
{
	BeamCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BeamCapsuleComponent"));
	if (BeamCapsuleComponent != nullptr)
	{
		//BeamCapsuleComponent->SetupAttachment(GetRootComponent());
		BeamCapsuleComponent->SetCapsuleSize(BEAM_WIDTH, BEAM_LENGTH / 2.f);
#if UE_EDITOR
		BeamCapsuleComponent->SetHiddenInGame(false);
#endif
	}
	bEnableDamageKnockBack = false;
}

void ADoll_Fragile_Beam::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if (Beam_Flag && HasAuthority()) StartTraceAtLocation(DeltaTime); // TODO Change To TimerHandle
}

bool ADoll_Fragile_Beam::FlickDoll_Implementation(FVector FlickDirection)
{
	if (BeamCapsuleComponent == nullptr) return false;
	
	UGoSoccerPlayManager::MarkDollAsMoved(GetWorld()->GetAuthGameMode(), this);

	Beam_Direction = FlickDirection.GetSafeNormal2D();
	FRotator Beam_Rotation = Beam_Direction.Rotation();
	Beam_Rotation.Pitch = 90.f;
	BeamCapsuleComponent->SetWorldLocation(GetActorLocation());
	BeamCapsuleComponent->SetWorldRotation(Beam_Rotation);
	BeamCapsuleComponent->UpdateOverlaps();

	FVector CurrLoc = GetActorLocation();
	Beam_StartLocation = CurrLoc - Beam_Direction * BEAM_LENGTH;
	Beam_EndLocation = CurrLoc + Beam_Direction * BEAM_LENGTH;
	Beam_DealtTime = 0.f;
	Beam_Flag = true;

	Server_OnSpawnBeam(Beam_StartLocation, CurrLoc + Beam_Direction * BEAM_LENGTH, Beam_Rotation);

	int32 CurrDollCount;
	UGoSoccerPlayManager::GetPlacedDollCount(GetWorld()->GetAuthGameMode(), CurrDollCount);
	DealtDoll_IdxContainer.SetNum((2 + (int32)CurrDollCount / 64));

	// TODO
	// Client -> Spawn Beam Actor
	// Server -> Start Check Overlapping Dolls Within Tick / When Ends Kill Beam Actor

	return true;
}

bool ADoll_Fragile_Beam::IsDollConsideredStationary_Implementation()
{
	bool rtn = Super::IsDollConsideredStationary_Implementation();
	//bool this_rtn = GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_BeamSendDamage);
	//return rtn && this_rtn;
	if (Beam_DamageStartFlag != Beam_DamageEndFlag)
	{
		DrawDebugString(
			GetWorld(), FVector(0.f, 0.f, 5.f),
			TEXT("Unstable"),
			this, FColor::Red,
			rtn ? 1.f : 3.f, false, 1.5f);
		return false;
	}
	return rtn;
}

bool ADoll_Fragile_Beam::StartBeamDamage()
{
	Server_StartBeamDamage();
	return true;
}

bool ADoll_Fragile_Beam::EndBeamDamage()
{
	//if (!GetWorld()) return false;
	Server_EndBeamDamage();
	return true;
}

bool ADoll_Fragile_Beam::DisableBeam()
{
	//Beam_Flag = false;
	return true;
}

void ADoll_Fragile_Beam::Server_EndBeamDamage_Implementation()
{
	Beam_DamageEndFlag = true;
	bInvincible = false;
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_BeamSendDamage);
	FDollDamageConfig DamageConfig;
	DamageConfig.DamageAmount = MaxHP * 2.f;
	DamageConfig.HitDirection = FVector::UpVector;
	DamageConfig.HitLocation = GetActorLocation();
	DamageConfig.ImpulseLocation = GetActorLocation();
	DamageConfig.KnockBackAmount = 0.f;
	Execute_SendDamageToHP(this, DamageConfig);
	UGoSoccerPlayManager::FlickDoll_Completed(GetWorld()->GetAuthGameMode(), Owner);
}

void ADoll_Fragile_Beam::Server_StartBeamDamage_Implementation()
{
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_BeamSendDamage,
		this, &ADoll_Fragile_Beam::BeamTraceOnce,
		Beam_DamageTick,
		true,
		Beam_DamageTick
	);
}

void ADoll_Fragile_Beam::Server_OnSpawnBeam_Implementation(FVector StartLocation, FVector EndLocation, FRotator InBeamRotation)
{
	Beam_DamageStartFlag = true;
	bInvincible = true;
	
	//Execute_SetDollFallFromBoard(this, true, false);
	NetMulticast_OnSpawnBeam(StartLocation, EndLocation, InBeamRotation);
}

void ADoll_Fragile_Beam::NetMulticast_OnSpawnBeam_Implementation(FVector StartLocation, FVector EndLocation, FRotator InBeamRotation)
{
	OnSpawnBeam(StartLocation, EndLocation, InBeamRotation);
}

void ADoll_Fragile_Beam::OnBeamOverlapped(AActor* OverlappedActor)
{
	if (!OverlappedActor->Implements<UIGoDoll>()) return;
	FVector HitDirection;
	FDollDamageConfig DollDamageConfig{
		Beam_DamagePerTick,
		Beam_Direction,
		OverlappedActor->GetActorLocation(),
		OverlappedActor->GetActorLocation(),
		Beam_DamageKnockbackAmount
	};
	//Execute_SendDamageToHP(OverlappedActor, Beam_DamagePerTick, Beam_Direction, OverlappedActor->GetActorLocation(), OverlappedActor->GetActorLocation(), 10.f);
	Execute_SendDamageToHP(OverlappedActor, DollDamageConfig);

	DrawDebugSphere(GetWorld(), OverlappedActor->GetActorLocation() + FVector(0.f, 0.f, 5.f), 5.f, 32, FColor::Green, false, 1.f);
}

void ADoll_Fragile_Beam::OnSpawnBeam_Implementation(FVector StartLocation, FVector EndLocation, FRotator InBeamRotation)
{
	//FVector Left	{ -Beam_Direction.Y, Beam_Direction.X, 0.f};
	//FVector Right	{ Beam_Direction.Y, -Beam_Direction.X, 0.f};
	FVector Left = FVector::CrossProduct(Beam_Direction, FVector::UpVector).GetSafeNormal2D();

	//DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + Left * 2.f, 12.f, FColor::Blue, false, 3.f);
	//DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + Beam_Direction* 2.f, 12.f, FColor::Orange, false, 3.f);

	DrawDebugDirectionalArrow(GetWorld(), StartLocation + Left * BEAM_WIDTH,	EndLocation + Left * BEAM_WIDTH,	12.f, FColor::Green, false, 3.f);
	DrawDebugDirectionalArrow(GetWorld(), StartLocation,						EndLocation,						12.f, FColor::Orange, false, 3.f);
	DrawDebugDirectionalArrow(GetWorld(), StartLocation - Left * BEAM_WIDTH,	EndLocation - Left * BEAM_WIDTH,	12.f, FColor::Red, false, 3.f);
}


void ADoll_Fragile_Beam::BeamTraceOnce()
{
	if (BeamCapsuleComponent == nullptr) return;
	float CurrentTimeStamp = GetWorld()->GetTimeSeconds();
	auto Ticket_Iter = DollOverlapped.CreateIterator();
	for (; Ticket_Iter; ++Ticket_Iter)
	{
		DamagedTicket& Ticket = *Ticket_Iter;
		//Ticket.PassedTimeSec += Beam_DamageTick;
		//if (CurrentTimeStamp - Ticket.PassedTimeSec < Beam_DamageCoolDownPerDoll) break;
		if (CurrentTimeStamp - Ticket.PassedTimeSec >= Beam_DamageCoolDownPerDoll && Ticket.PlacedDollIdx >= 0)
		{
			DealtDoll_IdxContainer[Ticket.PlacedDollIdx] = 0;
			Ticket_Iter.RemoveCurrent();
		}
	}

	TSet<AActor*> OverlappingActors;
	//BeamCapsuleComponent->GetOverlappingComponents()
	BeamCapsuleComponent->GetOverlappingActors(OverlappingActors, DollClass);
	for (auto Iter : OverlappingActors)
	{
		if (Iter == this || Iter == nullptr || !Iter->Implements<UIGoDoll>() || IIGoDoll::Execute_GetDollFallFromBoard(Iter)) continue;
		int32 PlacedIdx = IIGoDoll::Execute_GetPlacedIndex(Iter);
		if (PlacedIdx < 0) continue;
		if (DealtDoll_IdxContainer[PlacedIdx]) continue;
		DealtDoll_IdxContainer[PlacedIdx] = 1;
		DollOverlapped.Emplace(PlacedIdx, CurrentTimeStamp);
		OnBeamOverlapped(Iter);

		UE_LOG(LogTemp, Log, TEXT("%s Beam Damage [%d] %s"), *GetFName().ToString(), PlacedIdx, *Iter->GetFName().ToString());
	}
	Beam_DealtTime += Beam_DamageTick;
	if (Beam_DealtTime >= Beam_Max_DealtSec)
	{
		Server_EndBeamDamage();
	}
	//TArray<AActor*> IgnoreActor;
	//IgnoreActor.Add(this);
	//TArray<FHitResult> HitResults;
	//UKismetSystemLibrary::SphereTraceMulti(
	//	GetWorld(), Beam_StartLocation, Beam_EndLocation,
	//	BEAM_WIDTH, TRACE_TYPE_QUERY_DOLL_BEAM,
	//	false, IgnoreActor, EDrawDebugTrace::ForDuration, HitResults, true);

	//int32 idx = 0;
	//for (auto HitResult : HitResults)
	//{
	//	if (HitResult.GetActor() == nullptr) continue;
	//	UE_LOG(LogTemp, Log, TEXT("[%d] Beam Trace : %s"), idx++, *HitResult.GetActor()->GetFName().ToString());
	//	OnBeamOverlapped(HitResult.GetActor());
	//}

	//Beam_DealtTime++;
	//if (Beam_DealtTime > Beam_Max_DealtTime) Beam_Flag = false;
}
