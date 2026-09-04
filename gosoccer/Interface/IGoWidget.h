#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IGoWidget.generated.h"

UINTERFACE(MinimalAPI)
class UIGoWidget : public UInterface
{
	GENERATED_BODY()
};

class GOSOCCER_API IIGoWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnHoveredHoverableWidget(UUserWidget* HoveredWidget);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool OnSelectedChanged(bool bSelected);
};
