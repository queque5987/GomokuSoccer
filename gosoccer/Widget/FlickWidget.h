#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FlickWidget.generated.h"

class UProgressBar;
class UImage;

UCLASS()
class GOSOCCER_API UFlickWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Texture, meta = (BindWidget))
	TObjectPtr<UProgressBar> ChargeDirectionBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Texture, meta = (BindWidget))
	TObjectPtr<UImage> RoundBar;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Callback_AimFlicker(FVector AimDirection);

	virtual void Callback_AimFlicker_Implementation(FVector AimDirection);
};
