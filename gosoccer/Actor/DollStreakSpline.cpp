#include "Actor/DollStreakSpline.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

ADollStreakSpline::ADollStreakSpline()
{
	PrimaryActorTick.bCanEverTick = true;

	//SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	
	//SplineMeshComponents.Reserve(4);

	SplineMeshComponent = CreateDefaultSubobject<USplineMeshComponent>(TEXT("SplineMeshComponent"));
	if (SplineMeshComponent)
	{
		SetRootComponent(SplineMeshComponent);
		SplineMeshComponent->SetMobility(EComponentMobility::Movable);
	}

	//for (int8 i = 0; i < 4; i++)
	//{
	//	FName ComponentName = FName(FString::Printf(TEXT("SplineMeshComponent_%d"), i));
	//	auto* SplineMeshComponent = CreateDefaultSubobject<USplineMeshComponent>(ComponentName);
	//	if (SplineMeshComponent)
	//	{
	//		if (i == 0) 
	//		{
	//			SetRootComponent(SplineMeshComponent);
	//		}
	//		else 
	//		{
	//			SplineMeshComponent->SetupAttachment(RootComponent);
	//		}
	//		SplineMeshComponent->SetMobility(EComponentMobility::Movable);
	//		SplineMeshComponents.Emplace(SplineMeshComponent);
	//	}
	//}

	//if (SplineComponent)
	//{
	//	SplineComponent->SetupAttachment(RootComponent);
	//	for (int8 i = 0; i < 3; i++)
	//	{
	//		SplineComponent->AddSplinePoint(FVector::ZeroVector, ESplineCoordinateSpace::Local);
	//	}
	//}
}

void ADollStreakSpline::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADollStreakSpline::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (SplineComponent == nullptr || !bDebugFlag) return;

	fDebugFloat += DeltaTime;
	if (fDebugFloat >= 1.f) fDebugFloat -= 1.f;

	DrawDebugSphere(
		GetWorld(),
		SplineComponent->GetLocationAtTime(fDebugFloat, ESplineCoordinateSpace::World),
		2.f, 32, FColor::Green, false, 0.75f
	);
}

bool ADollStreakSpline::SetSplinePoints(const TArray<FVector>& WorldPosArr)
{
	SplineComponent->SetSplineWorldPoints(WorldPosArr);
	if (SplineMeshComponent == nullptr || WorldPosArr.Num() < 1) return false;
	SplineMeshComponent->SetWorldLocation(WorldPosArr[0] + FVector(0.f, 0.f, 5.f));
	SplineMeshComponent->SetStartAndEnd(
		FVector::ZeroVector, 
		FVector(1.f), 
		WorldPosArr[WorldPosArr.Num() - 1] - WorldPosArr[0],
		FVector(1.f)
	);
	bDebugFlag = true;
	//int32 PosNum = WorldPosArr.Num();
	//for (int32 i = 0; i < WorldPosArr.Num() - 1; i++)
	//{
	//	FVector CurrentPos = WorldPosArr[i];
	//	FVector NextPos = WorldPosArr[i + 1];

	//	FVector Direction_0 = (NextPos - CurrentPos).GetSafeNormal2D();
	//	FVector Direction_1 = (CurrentPos - NextPos).GetSafeNormal2D();		

	//	FVector SideDirection_0 = FVector::CrossProduct(Direction_0, FVector(0.f, 0.f, 1.f));
	//	FVector SideDirection_1 = FVector::CrossProduct(Direction_1, FVector(0.f, 0.f, 1.f));

	//	FVector SidePos_0 = CurrentPos + SideDirection_0 * 2.f + FVector(0.f, 0.f, 3.f);
	//	FVector SidePos_1 = CurrentPos + SideDirection_1 * 2.f + FVector(0.f, 0.f, 3.f);

	//	FVector SidePos_2 = NextPos + SideDirection_0 * 2.f + FVector(0.f, 0.f, 3.f);
	//	FVector SidePos_3 = NextPos + SideDirection_1 * 2.f + FVector(0.f, 0.f, 3.f);

	//	//DrawDebugSphere(GetWorld(), SidePos_0, 1.f, 32, FColor::Yellow, false, 3.f);
	//	DrawDebugLine(GetWorld(), SidePos_0, SidePos_2, FColor::Yellow, false, 3.f);
	//	//DrawDebugSphere(GetWorld(), SidePos_1, 1.f, 32, FColor::Blue, false, 3.f);
	//	DrawDebugLine(GetWorld(), SidePos_1, SidePos_3, FColor::Blue, false, 3.f);
	//}

	//for (int32 i = 0; i < 4; i++)
	//{
	//	if (!WorldPosArr.IsValidIndex(i) || !WorldPosArr.IsValidIndex(i + 1)) continue;
	//	if (!SplineMeshComponents.IsValidIndex(i)) continue;
	//	auto& SplineMeshComponent = SplineMeshComponents[i];
	//	if (SplineMeshComponent == nullptr) continue;
	//	//UE_LOG(LogTemp, Log, TEXT("Doll Pos %d : %s - %s"), i, *WorldPosArr[i].ToString(), *WorldPosArr[i + 1].ToString())
	//	SplineMeshComponent->SetWorldLocation(WorldPosArr[i] + FVector(0.f, 0.f, 3.f));
	//	//SplineMeshComponent->SetStartAndEnd(FVector::ZeroVector, FVector(1.f), WorldPosArr[i + 1] - WorldPosArr[i], FVector(1.f));
	//	SplineMeshComponent->SetStartAndEnd(FVector::ZeroVector, FVector(1.f), WorldPosArr[WorldPosArr.Num() - 1] - WorldPosArr[i], FVector(1.f));
	//	break;
	//}
	return true;
}

