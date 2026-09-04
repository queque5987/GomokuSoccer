#pragma once

#include "CoreMinimal.h"
#include "Actor/Doll_Fragile.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "Doll_Fragile_Beam.generated.h"

class UCapsuleComponent;
UCLASS()
class GOSOCCER_API ADoll_Fragile_Beam : public ADoll_Fragile
{
	GENERATED_BODY()

public:
	ADoll_Fragile_Beam();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UCapsuleComponent> BeamCapsuleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Beam_Flag = false;
	UPROPERTY()
	FVector Beam_StartLocation;
	UPROPERTY()
	FVector Beam_EndLocation;
	UPROPERTY()
	FVector Beam_Direction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Beam_DamageTick = 0.0167;

	bool Beam_DamageStartFlag = false;
	bool Beam_DamageEndFlag = false;
	//bool bEverFlicked = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BEAM_LENGTH = 1500.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BEAM_WIDTH = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Beam_ClosingDeltaTime = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Beam_DamageStartDeltaTime = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Beam_Max_DealtSec = 2.f;

	UPROPERTY()
	float Beam_DealtTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Beam_DamagePerTick;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Beam_DamageCoolDownPerDoll = 0.125f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Beam_DamageKnockbackAmount = 100.f;

	virtual bool FlickDoll_Implementation(FVector FlickDirection) override;
	virtual bool IsDollConsideredStationary_Implementation() override;

	UFUNCTION()
	void BeamTraceOnce();

	UFUNCTION(BlueprintNativeEvent)
	void OnSpawnBeam(FVector StartLocation, FVector EndLocation, FRotator InBeamRotation);

	UFUNCTION(BlueprintCallable)
	bool StartBeamDamage();
	UFUNCTION(BlueprintCallable)
	bool EndBeamDamage();
	UFUNCTION(BlueprintCallable)
	bool DisableBeam();

	UFUNCTION(Server, Reliable)
	void Server_StartBeamDamage();

	UFUNCTION(Server, Reliable)
	void Server_EndBeamDamage();

	UFUNCTION(Server, Reliable)
	void Server_OnSpawnBeam(FVector StartLocation, FVector EndLocation, FRotator InBeamRotation);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_OnSpawnBeam(FVector StartLocation, FVector EndLocation, FRotator InBeamRotation);

protected:
	struct DamagedTicket
	{
		DamagedTicket() 
		{
			PlacedDollIdx = -1; 
			PassedTimeSec = 0.f;
		};
		DamagedTicket(int32 InIdx) : PlacedDollIdx(InIdx)
		{
			PassedTimeSec = 0.f;
		}
		DamagedTicket(int32 InIdx, float InTime) : PlacedDollIdx(InIdx), PassedTimeSec(InTime)
		{
			PassedTimeSec = 0.f;
		}
		int32 PlacedDollIdx;
		float PassedTimeSec;
	};

	FTimerHandle TimerHandle_BeamSendDamage;

	UPROPERTY()
	FStreakDollContainer DealtDoll_IdxContainer;

	TArray<DamagedTicket> DollOverlapped;
	TSubclassOf<ADoll> DollClass;

	void OnBeamOverlapped(AActor* OverlappedActor);
};
