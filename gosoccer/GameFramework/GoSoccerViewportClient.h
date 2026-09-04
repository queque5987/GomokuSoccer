#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "GoSoccerViewportClient.generated.h"

UCLASS()
class GOSOCCER_API UGoSoccerViewportClient : public UGameViewportClient
{
	GENERATED_BODY()
public:
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
};
