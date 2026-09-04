#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DollStreakSpline.generated.h"

class USplineComponent;
class USplineMeshComponent;

UCLASS()
class GOSOCCER_API ADollStreakSpline : public AActor
{
	GENERATED_BODY()
	
public:	
	ADollStreakSpline();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USplineComponent> SplineComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USplineMeshComponent> SplineMeshComponent;

	virtual void BeginPlay() override;
	float fDebugFloat = 0.f;
	bool bDebugFlag = false;
public:	
	virtual void Tick(float DeltaTime) override;

	bool SetSplinePoints(const TArray<FVector>& WorldPosArr);
};
