#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IGoBoard.generated.h"

UINTERFACE(MinimalAPI)
class UIGoBoard : public UInterface
{
	GENERATED_BODY()
};

class AActor;

class GOSOCCER_API IIGoBoard
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector GetPutLocation(FVector CursorLocation);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool PutDoll(FVector CursorLocation, uint8 DollColor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsDollFallen(UObject* CheckDollObject);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetDollStreakSpline(const TArray<FVector>& DollPosArr);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetStageCameraObject(UObject*& OutCameraObject);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetTranclucentDollLocation(FVector DollLocation);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetTranclucentDoll(UObject*& OutTranclucentDoll);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetPossibleCameraLocation(FVector& ToMoveLocation);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetBorderLine(float& U, float& D, float& L, float& R);
};
