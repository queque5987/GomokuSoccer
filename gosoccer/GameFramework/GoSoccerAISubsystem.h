#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GoSoccer_PCH.h"
#include "GoSoccerAISubsystem.generated.h"

class UTextureRenderTarget2D;

DECLARE_DELEGATE_FourParams(FCalculateCompleted, int32/*Flick DollIndex*/, FVector/*Flick Direction*/, FVector/*Place Location*/, UObject*/*Next Player*/)
DECLARE_DELEGATE_OneParam(FOnDebugDoll, const FStoneDebugData&/*bOccupied*/)

USTRUCT(BlueprintType)
struct FGoBoardConfig
{
	GENERATED_BODY()
	FGoBoardConfig() {};
	FGoBoardConfig(float InL, float InR, float InU, float InD, float InW)
		: L(InL), R(InR), U(InU), D(InD), W(InW) {};
	FGoBoardConfig(float InL, float InR, float InU, float InD, float InW,
		float InDistance_Threshold, float InRadian_Threshold, uint8 InAIDollColor, int8 InAIStreakBonus)
		: L(InL), R(InR), U(InU), D(InD), W(InW),
		Distance_Threshold(InDistance_Threshold),
		Radian_Threshold(InRadian_Threshold),
		AIDollColor(InAIDollColor), AIStreakBonus(InAIStreakBonus){};
public:
	float L;
	float R;
	float U;
	float D;
	float W;
	float Distance_Threshold;
	float Radian_Threshold;
	uint8 AIDollColor;
	int8 AIStreakBonus;

	TArray<FStreakDollContainer> UF_Accessible;
	TArray<FStreakDollContainer> UF_CloseEnemy;
	TArray<TArray<FGridData>> Grid_Occupied;
	TArray<FStoneData> BoardStat;
	TArray<FStoneDebugData> DebugDataArr;
};

//USTRUCT(BlueprintType)
//struct FStreakDollContainer
//{
//	GENERATED_BODY()
//	FStreakDollContainer() 
//	{
//		DollDataContainer.Reserve(1);
//		DollDataContainer.SetNum(1);
//	};
//	FStreakDollContainer(int32 SizeOfContainer)
//	{
//		DollDataContainer.Reserve(SizeOfContainer);
//		DollDataContainer.SetNum(SizeOfContainer);
//	};
//public:
//	struct FBitProxy 
//	{
//		uint64& Container;
//		uint64 Mask;
//		void operator=(bool b) 
//		{
//			if (b) Container |= Mask;
//			else   Container &= ~Mask;
//		}
//		operator bool() const { return (Container & Mask) != 0; }
//	};
//	FORCEINLINE FBitProxy operator[](int32 idx)
//	{
//		int32 C = idx / 64;
//		int32 i = idx % 64;
//		check(C < DollDataContainer.Num());
//		return FBitProxy{ DollDataContainer[C], 1ULL << i };
//	};
//	bool operator==(const FStreakDollContainer& Other) const
//	{
//		if (DollDataContainer.Num() != Other.DollDataContainer.Num()) return false;
//		return DollDataContainer == Other.DollDataContainer;
//	};
//	bool operator!=(const FStreakDollContainer& Other) const
//	{
//		return !(*this == Other);
//	};
//	void SetNum(int32 size)
//	{
//		DollDataContainer.Empty();
//		DollDataContainer.Reserve(size);
//		DollDataContainer.SetNum(size);
//	};
//	const TArray<uint64>& GetData() const { return DollDataContainer; };
//	int32 GetCount() const
//	{
//		int32 rtn = 0;
//		for (uint64 Iter : DollDataContainer) { rtn += FMath::CountBits(Iter); }
//		return rtn;
//	};
//	bool IsSubsetOf(const FStreakDollContainer& Other) const
//	{
//		for (int32 i = 0; i < DollDataContainer.Num(); i++)
//		{
//			if (i >= Other.DollDataContainer.Num()) return false;
//			else if (DollDataContainer[i] != (DollDataContainer[i] & Other.DollDataContainer[i]))
//			{
//				return false;
//			}
//		}
//		return true;
//	};
//	bool IsValidIndex(int32 idx)
//	{
//		int32 n = idx / 64;
//		return DollDataContainer.Num() > (n);
//	};
//	void Clear()
//	{
//		for (uint64& Iter : DollDataContainer)
//		{
//			Iter = 0;
//		}
//	}
//private:
//	TArray<uint64> DollDataContainer;
//};

USTRUCT(BlueprintType)
struct FGridStreakTrailContainer
{
	GENERATED_BODY()
	FGridStreakTrailContainer()
	{
		StreakTrailContainer = FStreakDollContainer{ 1 };
		StreakDepth = 0;
		StreakRadian = 0.f;
	};
	FGridStreakTrailContainer(
		FStreakDollContainer& InStreakTrailContainer, uint8 InStreakDepth, float InRadian
	) : StreakTrailContainer(InStreakTrailContainer), StreakDepth(InStreakDepth), StreakRadian(InRadian) {};
	FGridStreakTrailContainer(int32 StreakTrainContainerSize, uint8 InStreakDepth, float InRadian) 
	{
		StreakTrailContainer = FStreakDollContainer{ StreakTrainContainerSize };
		StreakDepth = InStreakDepth;
		StreakRadian = InRadian;
	};
public:
	bool UpdateTipGridPos(int32 InX, int32 InY)
	{
		if (Tip0_Container.X < 0 || Tip0_Container.Y < 0)
		{
			Tip0_Container.X = InX;
			Tip0_Container.Y = InY;
			return true;
		}
		else if (Tip0_Container.X == InX && Tip0_Container.Y == InY)
		{
			return true;
		}
		if (Tip1_Container.X < 0 || Tip1_Container.Y < 0)
		{
			Tip1_Container.X = InX;
			Tip1_Container.Y = InY;
			return true;
		}
		else if (Tip1_Container.X == InX && Tip1_Container.Y == InY)
		{
			return true;
		}
		return false;
	}

	FStreakDollContainer StreakTrailContainer;
	uint8 StreakDepth;
	float StreakRadian;
	uint8 StreakDollColor;
	FIntVector2 Tip0_Container{-1, -1};
	FIntVector2 Tip1_Container{-1, -1};
};

USTRUCT(BlueprintType)
struct FGridData
{
	GENERATED_BODY()
	FGridData() {};
	FGridData(int32 BoardDollNumInit)
	{
		CloseDollContainer.SetNum(1 + BoardDollNumInit / 64);
	};
	FGridData(
		float InStoneDistance, float InScore, int32 BoardDollNumInit
	) : StoneDistance(InStoneDistance), Score(InScore) 
	{
		CloseDollContainer.SetNum(1 + BoardDollNumInit / 64);
	};
	FGridData(
		int32 Inidx, float InStoneDistance, float InScore, int32 BoardDollNumInit
	) : ClosestStoneIdx(Inidx), StoneDistance(InStoneDistance),
		Score(InScore) 
	{
		CloseDollContainer.SetNum(1 + BoardDollNumInit / 64);
	};
public:
	void UpdateCloseDollCounter(int32 idx, bool e)
	{
		if (CloseDollContainer.IsValidIndex(idx)) CloseDollContainer[idx] = e;
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Index Out Of Range"));
		}
	};

	//FStoneData* ClosestStoneData = nullptr;
	int32 ClosestStoneIdx = -1;
	float StoneDistance = INFINITY;
	float Score = 0.f;
	float DefenceScore = 0.f;
	TArray<FGridStreakTrailContainer> StreakTrailArr;
	// Deprecated
	//TArray<int32> StreakSizes;
	//TArray<FStreakDollContainer> StreakDollTraces;
	//TArray<float> StreakRadian;
	FStreakDollContainer CloseDollContainer;
	uint8 ContainDollColor = 0;
};

USTRUCT(BlueprintType)
struct FStoneDebugData
{
	GENERATED_BODY()
	FStoneDebugData() {};
	FStoneDebugData(
		FVector InLocation, bool InbOccupied
	) : Location(InLocation), bOccupied(InbOccupied) {};
public:
	FVector Location;
	bool bOccupied;
};

USTRUCT(BlueprintType)
struct FStoneData
{
	GENERATED_BODY()
	FStoneData() {};
	FStoneData(
		FVector InLocation, uint8 InDollColor,
		int32 Inidx, int32 GridPosX, int32 GridPosY,
		float GridDistance, bool InbIsFallen
	) : Location(InLocation), DollColor(InDollColor), idx(Inidx),
		ClosestGirdPos(GridPosX, GridPosY),
		Debug_DistFromGridCenter(GridDistance), bIsFallen(InbIsFallen) {};
public:
	FVector Location;
	uint8 DollColor;
	int32 idx;
	FVector2D ClosestGirdPos;
	float Debug_DistFromGridCenter;
	bool bIsFallen;
};

USTRUCT(BlueprintType)
struct FCandidate
{
	GENERATED_BODY()
	FCandidate() {};
	FCandidate(
		FVector2D InGridLocation, float InScore
	) : GridLocation(InGridLocation), Score(InScore) {};
	FCandidate(
		FVector2D InGridLocation, int32 InStoneIdx, float InScore
	) : GridLocation(InGridLocation), StoneIdx(InStoneIdx), Score(InScore) {};
public:
	FVector2D GridLocation;
	int32 StoneIdx = -1;
	float Score;
};

UCLASS()
class GOSOCCER_API UGoSoccerAISubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	UPROPERTY()
	UObject* AWaitPlayer;

	FOnDebugDoll OnDebugDoll;
	FCalculateCompleted CalculateCompleted;
	//UPROPERTY()
	//TArray<FStoneData> BoardStat;

	// TODO Make Into Some Struct (Able To Undo / Iterate Recently Updated Indexes, and Clear Indexes)
	//TArray<TArray<FGridData>> Grid_Occupied;

	UPROPERTY()
	TArray<int32> UF_Parent;
	
	//UPROPERTY()
	//TArray<FStreakDollContainer> UF_Accessible;
	//UPROPERTY()
	//TArray<FStreakDollContainer> UF_CloseEnemy;
	
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> PlacedDollArr;
	//UPROPERTY()
	//TArray<bool> UF_Visited;
	UPROPERTY()
	FStreakDollContainer UF_Visited;

	//UPROPERTY()
	//float Distance_Threshold;
	//UPROPERTY()
	//float Radian_Threshold;

	//UPROPERTY()
	//float U;
	//UPROPERTY()
	//float D;
	//UPROPERTY()
	//float L;
	//UPROPERTY()
	//float R;
	//UPROPERTY()
	//float W;
	//UPROPERTY()
	//uint8 AIDollColor;

public:
	void Request_Calculate(
		UObject* WaitPlayer,
		const TArray<TWeakObjectPtr<AActor>>& PlacedDollArrRef,
		float InDistance_Threshold, float InRadian_Threshold,
		float InU, float InD, float InL, float InR, uint8 InAIDollColor, int8 AIStreakBonus);

	UFUNCTION()
	void Calculate_Completed(int32 FlickDollIdx, FVector FlickDirection, FVector PlaceLocation, UObject* NextPlayer);
private:

	void Initialize_GridArr(const TArray<TWeakObjectPtr<AActor>>& PlacedDollArrRef, FGoBoardConfig& GoBoardConfig);
	void Union_Board(FGoBoardConfig& GoBoardConfig);

	static bool IsDollInRange(FVector A, FVector B, float Cite_Threshold);
	static bool IsInGrid(int32 X, int32 Y, const FGoBoardConfig& GoBoardConfig);
	void UF_JoinDollIngoGroup(FStoneData& InStoneData, FGoBoardConfig& GoBoardConfig);
	void UF_Union(int32 A, int32 B);
	int32 UF_Find(int32 A);

	void Debug_GetPossibleSpawnDollLocation(bool bDebug = false);
	static void GetGridLocation(FVector ConvertLocation, int32& Out_X, int32& Out_Y, float& DistFromCenter, FVector& CenterLocation, const FGoBoardConfig& GoBoardConfig);

	//static void EvaluateBoard_WriteStreakData(TArray<TArray<FGridData>>& Cal_Grid_Occupied, const TArray<FStoneData>& Cal_BoardStat, const float InRadian_Threshold, const float InDistance_Threshold, const FGoBoardConfig& GoBoardConfig);
	static void EvaluateBoard_WriteStreakData(FGoBoardConfig& GoBoardConfig);
	// Deprecated
	int32 Travel_Accessibles(int32 DollIdx, FStreakDollContainer& Visited, int32 Depth, const float TowardsRadian, const float InRadian_Threshold, FStreakDollContainer& VisitedTrace);

	//FGridStreakTrailContainer Travel_Accessibles_StreakContainer(
	//	int32 DollIdx, TArray<TArray<FGridData>>& Cal_Grid_Occupied,
	//	const TArray<FStoneData>& Cal_BoardStat,
	//	FStreakDollContainer& Visited, int32 Depth,
	//	const float TowardsRadian, const float InRadian_Threshold,
	//	const float InDistance_Threshold, FVector2D& JumpedGridPos,
	//	const FGoBoardConfig& GoBoardConfig
	//);
	static FGridStreakTrailContainer Travel_Accessibles_StreakContainer(
		int32 DollIdx, FStreakDollContainer& Visited,
		int32 Depth, const float TowardsRadian, const float InRadian_Threshold,
		const float InDistance_Threshold, FVector2D& JumpedGridPos, FGoBoardConfig& GoBoardConfig
	);
	//static void EvaluateBoard_GridStreakTrailsToScore(
	//	const TArray<FStoneData>& Cal_BoardStat, TArray<TArray<FGridData>>& Cal_Grid_Occupied, 
	//	//const TArray<uint8>& UF_Depths,
	//	TArray<FCandidate>& PlacableLocations,
	//	TArray<FCandidate>& FlickableTargets,
	//	TArray<FCandidate>& UnrelatedGrids,
	//	float& TotalScore, uint8& Def_MaxStreak, bool bDebug = false
	//);
	static void EvaluateBoard_GridStreakTrailsToScore(
		TArray<FCandidate>& PlacableLocations,
		TArray<FCandidate>& FlickableTargets,
		TArray<FCandidate>& UnrelatedGrids,
		float& TotalScore, uint8& Def_MaxStreak,
		FGoBoardConfig& GoBoardConfig, bool bDebug = false
	);
	//static void Notify_PlaceLocation_Calculated(
	//	TArray<TArray<FGridData>>& Cal_Grid_Occupied,
	//	const TArray<FStoneData>& Cal_BoardStat,
	//	int32& FlickDollIndex, FVector& FlickDirection,
	//	FVector& PlaceLocation
	//);
	static void Notify_PlaceLocation_Calculated(
		int32& FlickDollIndex, FVector& FlickDirection,
		FVector& PlaceLocation, FGoBoardConfig& GoBoardConfig
	);
};
