// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "TemporalMessageData.generated.h"

UCLASS(BlueprintType)
class GOSOCCER_API UTemporalMessageData : public UObject
{
	GENERATED_BODY()
	UPROPERTY()
	FString ThrowMessage;
	UPROPERTY()
	ETemporalMessageType TemporalMessageType;
	UPROPERTY()
	float FloatTime = 3.f;

public:
	UFUNCTION(BlueprintCallable)
	void GetThrowMessage(FString& OutString) const { OutString = ThrowMessage; };
	UFUNCTION(BlueprintCallable)
	void GetTemporalMessageType(ETemporalMessageType& OutTemporalMessageType) const { OutTemporalMessageType = TemporalMessageType; };
	UFUNCTION(BlueprintCallable)
	void GetFloatTime(float& OutFloatTime) const { OutFloatTime = FloatTime; };

	UFUNCTION(BlueprintCallable)
	void SetThrowMessage(const FString& InString) { ThrowMessage = InString; };
	UFUNCTION(BlueprintCallable)
	void SetTemporalMessageType(const ETemporalMessageType& InTemporalMessageType) { TemporalMessageType = InTemporalMessageType; };
	UFUNCTION(BlueprintCallable)
	void SetFloatTime(const float InFloatTime) { FloatTime = InFloatTime; };


};
