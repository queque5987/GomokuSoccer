#pragma once

#include "CoreMinimal.h"
#include "Widget/SelectButtonWidget.h"
#include "SliderButtonWidget.generated.h"

class USlider;

UCLASS()
class GOSOCCER_API USliderButtonWidget : public USelectButtonWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> Slider;

	FOnCutomButtonValueChangedDelegate Delegate_OnValueChanged;
	virtual float GetSelectedValue() override;
	virtual void SetSelectedValue(float InValue) override;
	virtual FOnCutomButtonValueChangedDelegate* GetValueChangedDelegate() override { return &Delegate_OnValueChanged; };
	UFUNCTION(BlueprintCallable)
	void Execute_ValueChangedDelegate(float InChangedValue);
	UFUNCTION(BlueprintImplementableEvent)
	void OnSelectedValueChanged(float InValue);
};
