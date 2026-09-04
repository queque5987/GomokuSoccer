#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IGoDoll.generated.h"

UINTERFACE(MinimalAPI)
class UIGoDoll : public UInterface
{
	GENERATED_BODY()
};

class GOSOCCER_API IIGoDoll
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetPlacedIndex(int32 InPlacedIndex);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetPlacedIndex();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetDollCustomStencil(uint8 StencilValue);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool FlickDoll(FVector FlickDirection);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetWinDollCircularEffect(bool bOnEffect);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetDollCircularEffect_Progressive(float CurrentTimeSec, float MaxTimeSec);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetDollcolor(uint8& OutDollColor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetDollFallFromBoard(bool e, bool bPlaySoundFx);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetDollFallFromBoard();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool Transpalent_GetIsPuttable();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool OnBoardGetStationary();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	float GetFlickedDirectionSize();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector GetFlickedDirection();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetCurrHP(float& OutHP);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetDollStencil_Scoring();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	//void SendDamageToHP(float DamageAmount, FVector HitDirection, FVector InHitLocation, FVector InImpulseLocation, float KnockBackAmount = 0.f);
	void SendDamageToHP(const FDollDamageConfig& DollDamageConfig);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsDollConsideredStationary();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Simple_AddImpulse(FVector Direction, float Power);
};
