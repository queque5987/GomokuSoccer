#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/IGoBoard.h"
#include "GoBoard.generated.h"

class ADoll;
class ADollStreakSpline;
class ACameraActor;
class UBoxComponent;

UCLASS()
class GOSOCCER_API AGoBoard : public AActor, public IIGoBoard
{
	GENERATED_BODY()
	
public:	
	AGoBoard();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> CameraVolumeComponnet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ADollStreakSpline> DollStreakSpline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stat, meta = (AllowPrivateAccess = "true"))
	float BoardWidth;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stat, meta = (AllowPrivateAccess = "true"))
	float BoardHeight;
	// Side Gap Length Before Actual Fall
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stat, meta = (AllowPrivateAccess = "true"))
	float BoardFallenThreshold_X;
	// Side Gap Length Before Actual Fall
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stat, meta = (AllowPrivateAccess = "true"))
	float BoardFallenThreshold_Y;
	// Side Gap Length Before Actual Fall
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stat, meta = (AllowPrivateAccess = "true"))
	float BoardFallenThreshold_Z;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh> SM_GoBoard;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UClass> WhiteDoll;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UClass> BlackDoll;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UClass> TranslucentDoll;

	TObjectPtr<AActor> TranslucentDoll_Actor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ACameraActor> StageCamera;

	virtual void BeginPlay() override;

	void InitiateLocation();

	FVector TopRight;
	FVector BottomLeft;

	TArray<TArray<int32>> BoardState;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual FVector GetPutLocation_Implementation(FVector CursorLocation) override;
	int32 GetBoardState(int32 X, int32 Y) { return BoardState.IsValidIndex(X) && BoardState[X].IsValidIndex(Y) ? BoardState[X][Y] : -1; };
	virtual bool IsDollFallen_Implementation(UObject* CheckDollObject) override;
	virtual bool SetDollStreakSpline_Implementation(const TArray<FVector>& DollPosArr) override;
	virtual bool GetStageCameraObject_Implementation(UObject*& OutCameraObject) override;
	virtual bool SetTranclucentDollLocation_Implementation(FVector DollLocation) override;
	virtual bool GetTranclucentDoll_Implementation(UObject*& OutTranclucentDoll) override;
	virtual bool GetPossibleCameraLocation_Implementation(FVector& ToMoveLocation) override;
	virtual bool GetBorderLine_Implementation(float& U, float& D, float& L, float& R) override;
};
