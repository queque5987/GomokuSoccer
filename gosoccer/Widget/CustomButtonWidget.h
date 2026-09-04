// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "CustomButtonWidget.generated.h"

UCLASS()
class GOSOCCER_API UCustomButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> MainButton;

	UFUNCTION()
	FOnButtonReleasedEvent& GetOnReleased() { return MainButton->OnReleased; };
};
