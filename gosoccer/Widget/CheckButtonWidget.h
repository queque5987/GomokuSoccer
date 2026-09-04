#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CheckButtonWidget.generated.h"

class UCheckBox;
class UButton;

UCLASS()
class GOSOCCER_API UCheckButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckBox> CheckBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_BG;

	TObjectPtr<UCheckBox> GetCheckBox() { return CheckBox; };
	TObjectPtr<UButton> GetButton_BG() { return Button_BG; };
};
