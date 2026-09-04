#include "Actor/GoBoard.h"
#include "Doll.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Actor/DollStreakSpline.h"
#include "Camera/CameraActor.h"
#include "Components/BoxComponent.h"

AGoBoard::AGoBoard()
{
	PrimaryActorTick.bCanEverTick = true;
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	CameraVolumeComponnet = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));

	SetRootComponent(SceneComponent);
	if (StaticMeshComponent != nullptr)
	{
		StaticMeshComponent->SetStaticMesh(SM_GoBoard);
		StaticMeshComponent->SetupAttachment(GetRootComponent());
	}
	if (CameraVolumeComponnet != nullptr)
	{
		CameraVolumeComponnet->SetupAttachment(GetRootComponent());
	}
}

void AGoBoard::BeginPlay()
{
	Super::BeginPlay();
	InitiateLocation();
	StaticMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_EngineTraceChannel1, ECollisionResponse::ECR_Block);
}

void AGoBoard::InitiateLocation()
{
	FVector PlacedLocation = GetActorLocation();
	
	BottomLeft = PlacedLocation - FVector(BoardHeight / 2.f, BoardWidth / 2.f, 0.f);
	TopRight = PlacedLocation + FVector(BoardHeight / 2.f, BoardWidth / 2.f, 0.f);
}

void AGoBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
#if !UE_BUILD_SHIPPING
	DrawDebugLine(GetWorld(), BottomLeft - FVector(BoardFallenThreshold_X, BoardFallenThreshold_Y, 0.f), BottomLeft - FVector(BoardFallenThreshold_X, BoardFallenThreshold_Y, 0.f) + FVector(BoardHeight + BoardFallenThreshold_X * 2.f, 0.f, 0.f), FColor::Green);
	DrawDebugLine(GetWorld(), BottomLeft - FVector(BoardFallenThreshold_X, BoardFallenThreshold_Y, 0.f), BottomLeft - FVector(BoardFallenThreshold_X, BoardFallenThreshold_Y, 0.f) + FVector(0.f, BoardWidth + BoardFallenThreshold_Y * 2.f, 0.f), FColor::Green);
	DrawDebugLine(GetWorld(), TopRight + FVector(BoardFallenThreshold_X, BoardFallenThreshold_Y, 0.f), TopRight + FVector(BoardFallenThreshold_X, BoardFallenThreshold_Y, 0.f) - FVector(BoardHeight + BoardFallenThreshold_X * 2.f, 0.f, 0.f), FColor::Green);
	DrawDebugLine(GetWorld(), TopRight + FVector(BoardFallenThreshold_X, BoardFallenThreshold_Y, 0.f), TopRight + FVector(BoardFallenThreshold_X, BoardFallenThreshold_Y, 0.f) - FVector(0.f, BoardWidth + BoardFallenThreshold_Y * 2.f, 0.f), FColor::Green);
#endif
}

FVector AGoBoard::GetPutLocation_Implementation(FVector CursorLocation)
{
	FVector tempVector = CursorLocation - BottomLeft;
	float w = BoardWidth / 18.f;
	float _x = FMath::Max(tempVector.X, 0.f);
	float _dx = FMath::Floor(_x / w);
	float _px = FMath::Fmod(_x, w);
	if (_px >= w / 2.f)
	{
		_dx++;
		_px -= w;
	}
	_px = FMath::Clamp(_px, -w / 8.f, w / 8.f);

	float _y = FMath::Max(tempVector.Y, 0.f);
	float _dy = FMath::Floor(_y / w);
	float _py = FMath::Fmod(_y, w);
	if (_py >= w / 2.f)
	{
		_dy++;
		_py -= w;
	}
	_py = FMath::Clamp(_py, -w / 8.f, w / 8.f);

	return FVector(
		BottomLeft.X + (_dx + _px / w) * w,
		BottomLeft.Y + (_dy + _py / w) * w,
		CursorLocation.Z
	);
}

//bool AGoBoard::PutDoll_Implementation(FVector CursorLocation, uint8 DollColor)
//{
//	//if (DollColor < 1)
//	//{
//	//	GetWorld()->SpawnActor<AActor>(BlackDoll, CursorLocation, FRotator::ZeroRotator);
//	//}
//	//else
//	//{
//	//	GetWorld()->SpawnActor<AActor>(WhiteDoll, CursorLocation, FRotator::ZeroRotator);
//	//}
//	//Server_PutDoll(CursorLocation, DollColor);
//	return true;
//}

bool AGoBoard::IsDollFallen_Implementation(UObject* CheckDollObject)
{
	AActor* DollObject = Cast<AActor>(CheckDollObject);
	if (DollObject == nullptr) return false;
	FVector DollLocation = DollObject->GetActorLocation();
	bool bFallen = false;
	if (BottomLeft.X - BoardFallenThreshold_X > DollLocation.X || TopRight.X + BoardFallenThreshold_X < DollLocation.X ||
		BottomLeft.Y - BoardFallenThreshold_Y > DollLocation.Y || TopRight.Y + BoardFallenThreshold_Y < DollLocation.Y ||
		BottomLeft.Z - BoardFallenThreshold_Z > DollLocation.Z || TopRight.Z + BoardFallenThreshold_Z * 3 < DollLocation.Z)
	{
		bFallen = true;
	}
#if !UE_BUILD_SHIPPING
	DrawDebugLine(GetWorld(), DollLocation, DollLocation + FVector(0.f, 0.f, 10.f), bFallen ? FColor::Red : FColor::Green, false, 3.f);
#endif
	return bFallen;
}

bool AGoBoard::SetDollStreakSpline_Implementation(const TArray<FVector>& DollPosArr)
{
	if (DollStreakSpline == nullptr) return false;
	ADollStreakSpline* TempSpline = GetWorld()->SpawnActor<ADollStreakSpline>(DollStreakSpline);
	if (TempSpline == nullptr) return false;
	return TempSpline->SetSplinePoints(DollPosArr);
}

bool AGoBoard::GetStageCameraObject_Implementation(UObject*& OutCameraObject)
{
	if (StageCamera == nullptr) return false;
	OutCameraObject = StageCamera;
	return true;
}

bool AGoBoard::SetTranclucentDollLocation_Implementation(FVector DollLocation)
{
	if (TranslucentDoll_Actor == nullptr)
	{
		TranslucentDoll_Actor = GetWorld()->SpawnActor<AActor>(TranslucentDoll, DollLocation, FRotator::ZeroRotator);
	}
	else if (TranslucentDoll_Actor != nullptr)
	{
		TranslucentDoll_Actor->SetActorLocation(DollLocation);
	}
	return TranslucentDoll_Actor != nullptr;
}

bool AGoBoard::GetTranclucentDoll_Implementation(UObject*& OutTranclucentDoll)
{
	OutTranclucentDoll = TranslucentDoll_Actor;
	return TranslucentDoll_Actor != nullptr;
}

bool AGoBoard::GetPossibleCameraLocation_Implementation(FVector& ToMoveLocation)
{
	if (!CameraVolumeComponnet)
	{
		return false;
	}
	FVector BoxOrigin = CameraVolumeComponnet->GetComponentLocation();
	FVector BoxExtent = CameraVolumeComponnet->GetScaledBoxExtent();

	FVector MinBound = BoxOrigin - BoxExtent;
	FVector MaxBound = BoxOrigin + BoxExtent;

	ToMoveLocation.X = FMath::Clamp(ToMoveLocation.X, MinBound.X, MaxBound.X);
	ToMoveLocation.Y = FMath::Clamp(ToMoveLocation.Y, MinBound.Y, MaxBound.Y);
	ToMoveLocation.Z = FMath::Clamp(ToMoveLocation.Z, MinBound.Z, MaxBound.Z);

	return true;
}

bool AGoBoard::GetBorderLine_Implementation(float& U, float& D, float& L, float& R)
{
	L = BottomLeft.X;// - BoardFallenThreshold_X;
	R = TopRight.X;// +BoardFallenThreshold_X;
	D = BottomLeft.Y;// -BoardFallenThreshold_Y;
	U = TopRight.Y;// +BoardFallenThreshold_Y;
	return true;
}

