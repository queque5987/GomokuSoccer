#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "SelectButtonWidget.generated.h"

UCLASS()
class GOSOCCER_API USelectButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	int32 SelectedIndex;

	FOnCutomButtonValueChangedDelegate Delegate_OnIndexChanged;

	UFUNCTION(BlueprintImplementableEvent)
	void ChangeSelectedIndex(int32 InSelectedIndex);

	int32 GetSelectedIndex() { return SelectedIndex; };
	virtual FOnCutomButtonValueChangedDelegate* GetValueChangedDelegate() { return &Delegate_OnIndexChanged; };
	UFUNCTION(BlueprintCallable)
	void Execute_IndexChangedDelegate();
	/*Slider Only*/
	virtual float GetSelectedValue() { return 0.f; };
	/*Slider Only*/
	virtual void SetSelectedValue(float InValue) {};
};
