#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "IngameCursor.generated.h"

UCLASS()
class GOSOCCER_API UIngameCursor : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetCoolDown(float fPercent);
	UFUNCTION(BlueprintImplementableEvent)
	void SetCoolDown_R(float fPercent);
	UFUNCTION(BlueprintImplementableEvent)
	void LMBPressed();
	UFUNCTION(BlueprintImplementableEvent)
	void LMBReleased();
	UFUNCTION(BlueprintImplementableEvent)
	void RMBPressed();
	UFUNCTION(BlueprintImplementableEvent)
	void RMBReleased();
};
