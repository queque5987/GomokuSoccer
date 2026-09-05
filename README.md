# Gomoku Soccer

![Unreal Engine](https://img.shields.io/badge/Unreal_Engine-5.2-black?logo=unrealengine)
![C++](https://img.shields.io/badge/C++-blue.svg?logo=c%2B%2B)
![Steamworks](https://img.shields.io/badge/Steamworks-OSS-171A21?logo=steam)

**Gomoku Soccer**
Steam 정식 출시를 목표로 개발 중인 1인 프로젝트입니다.

오목 축구는 오목과 알까기를 접목시킨 멀티플레이 기반 턴제/실시간 보드게임입니다.

---

# 사용한 기술

## **1. Steam OSS 기반 P2P 멀티플레이 구현**
* **Steamworks SDK** : 플레이어간 세션 생성, 검색, 참여 기능을 구현하였습니다.
* **RPC** : 각 돌들의 위치를 Replicate하고, 이펙트와 사운드 재생 그리고 스코어 업데이트 등의 기능을 구현하며 서버와 클라이언트를 동기화하였습니다.

## **2. 멀티스레딩 & 좌표 기반 Min-Max 알고리즘 구현**
* **기존 Min-Max 알고리즘 확장** : 기존 271개 비트를 사용하는 Min-Max 알고리즘을 확장하여 바둑판의 그리드 방식이 아닌 좌표계에서도 활용할 수 있는 알고리즘을 구현하였습니다.
* **멀티스레드** : 바둑돌의 위치를 구하는 기능이 백그라운드 스레드에서 동작하도록 구현하였습니다.
* **Calude 활용** : 1차원적으로 바둑판의 상태를 평가하고, 바둑돌을 소환하거나 튕길 돌과 방향 벡터를 산출하는 알고리즘을 기반으로 특정 depth까지 계산하는 알고리즘을 완성하였습니다.

## **3. 머티리얼 기반 UI 구현**
* **MID를 통한 UI 연출** : 다이나믹 머티리얼 인스턴스의 파라미터를 C++ 및 Blueprint에서 조정하여 UI 애니메이션을 구현하였습니다.
* **Material UI** : 버튼 및 HUD 등 모든 위젯을 머티리얼을 활용하여 구현하였습니다.

## **4. 확장성을 고려한 객체 지향적 구조**
* **컴파일 시간 최소화** : 인터페이스의 의존성을 정적인 클래스에 몰아두어 각 클래스 간 함수 호출을 자유롭게 하고, 순환 참조를 방지, 컴파일 시간을 단축시켰습니다.

---

## **1. Steam OSS 기반 P2P 멀티플레이 구현**
컴파일된 5.2 버전 엔진 사용
Steam SDK 1.53 사용
OnlineSubsystem 인터페이스를 사용하여 세션 생성, 검색, 참가, 초대 기능을 구현
Session관련 파라미터는 Enum과 구조체를 선언하여 관리
해당 구조체와 Enum을 통해 세션을 표시하는 위젯, 로비에서 세션 설정을 바꾸는 위젯 등에서 활용할 수 있도록 확장성 확보

```cpp
USTRUCT(BlueprintType)
struct FSessionCreateData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	bool IsSecret = false;
	UPROPERTY(BlueprintReadOnly)
	FString SessionName;

	UPROPERTY(BlueprintReadOnly)
	FString HostName;

	UPROPERTY(BlueprintReadOnly)
	EPlayGameRule PlayGameRule = EPlayGameRule::EPGR_Default;

	UPROPERTY(BlueprintReadOnly)
	EPlayGameMode PlayGameMode = EPlayGameMode::EPGM_Default;

	UPROPERTY(BlueprintReadOnly)
	EBoardPreset BoardPreset = EBoardPreset::EBP_Default;

	UPROPERTY(BlueprintReadOnly)
	EPlayTurnRule PlayTurnRule = EPlayTurnRule::EPTR_Default;

	UPROPERTY(BlueprintReadOnly)
	EPlayDollType PlayDollType = EPlayDollType::EPDT_Default;

	UPROPERTY(BlueprintReadOnly)
	EMatchRule MatchRule = EMatchRule::EMR_Default;

	UPROPERTY(BlueprintReadOnly)
	float TimeLimit = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float AngularThreshold = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float DistanceThreshold = 0.f;

	UPROPERTY(BlueprintReadOnly)
	bool bEnableItem = false;
};
```

## **2. 멀티스레딩 & 좌표 기반 Min-Max 알고리즘 구현**

기존 19*19개의 bit를 사용하는 오목 알고리즘을 좌표 기반의 오목 축구에 적용하기 위해 새 구조체를 선언하였습니다.

### FGridData 의 주요 파라미터 *(현재 그리드에 인접하는 바둑돌의 상태)*

<details>
<summary>FGridData</summary>
  
```cpp
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
```

</details>

* *TArray\<FGridStreakTrailContainer\>* (완성한 n목에 대한 데이터의 배열)


### FGridStreakTrailContainer의 주요 파라미터 *(완성한 n목에 대한 데이터)*

<details>
<summary>FGridStreakTrailContainer</summary>
  
```cpp
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
```

</details>

* *FIntVector2* (n목의 끝부분 좌표(*이하 *Tip*)를 grid형태로 저장)
* *FStreakDollContainer* (바둑돌의 index를 저장)
  * *TArray<uint64>* (각 바둑돌의 index를 동적 크기의 uint64를 통해 보유)

<details>
<summary>FStreakDollContainer</summary>

```cpp
USTRUCT(BlueprintType)
struct FStreakDollContainer
{
	GENERATED_BODY()
	FStreakDollContainer()
	{
		DollDataContainer.Reserve(1);
		DollDataContainer.SetNum(1);
	};
	FStreakDollContainer(int32 SizeOfContainer)
	{
		DollDataContainer.Reserve(SizeOfContainer);
		DollDataContainer.SetNum(SizeOfContainer);
	};
public:
	struct FBitProxy
	{
		uint64& Container;
		uint64 Mask;
		void operator=(bool b)
		{
			if (b) Container |= Mask;
			else   Container &= ~Mask;
		}
		operator bool() const { return (Container & Mask) != 0; }
	};
	FORCEINLINE FBitProxy operator[](int32 idx)
	{
		int32 C = idx / 64;
		int32 i = idx % 64;
		check(C < DollDataContainer.Num());
		return FBitProxy{ DollDataContainer[C], 1ULL << i };
	};
	bool operator==(const FStreakDollContainer& Other) const
	{
		if (DollDataContainer.Num() != Other.DollDataContainer.Num()) return false;
		return DollDataContainer == Other.DollDataContainer;
	};
	bool operator!=(const FStreakDollContainer& Other) const
	{
		return !(*this == Other);
	};
	void SetNum(int32 size)
	{
		DollDataContainer.Empty();
		DollDataContainer.Reserve(size);
		DollDataContainer.SetNum(size);
	};
	const TArray<uint64>& GetData() const { return DollDataContainer; };
	int32 GetCount() const
	{
		int32 rtn = 0;
		for (uint64 Iter : DollDataContainer) { rtn += FMath::CountBits(Iter); }
		return rtn;
	};
	bool IsSubsetOf(const FStreakDollContainer& Other) const
	{
		for (int32 i = 0; i < DollDataContainer.Num(); i++)
		{
			if (i >= Other.DollDataContainer.Num()) return false;
			else if (DollDataContainer[i] != (DollDataContainer[i] & Other.DollDataContainer[i]))
			{
				return false;
			}
		}
		return true;
	};
	bool IsValidIndex(int32 idx)
	{
		int32 n = idx / 64;
		return DollDataContainer.Num() > (n);
	};
	void Clear()
	{
		for (uint64& Iter : DollDataContainer)
		{
			Iter = 0;
		}
	}
private:
	TArray<uint64> DollDataContainer;
};
```
</details>

---

uint64 배열을 보유하고 해당 배열의 크기를 동적으로 조정할 수 있는 Container 구조체 FStreakDollContainer를 선언하였습니다.

바둑돌이 spawn된 순서대로 index를 부여하였고, 해당 index에 해당하는 비트를 container에서의 index로 사용하여 트리 구조 탐색 시 메모리 점유율을 크게 절약하였습니다.

FGridStreakTrailContainer는 FStreakDollContainer(n목에 해당하는 바둑돌의 index 모음)과 Tip_Container(좌우 끝부분에 해당하는 좌표)를 저장하는 구조체입니다.

각 19*19개의 그리드는 bit가 아닌 해당 그리드에 인접한 바둑돌과 거리를 저장하는 FGridData를 보유합니다.

---

### 1. 소환되어 있는 바둑돌의 배열을 순회하며 각 그리드에 FStreakDollContainer를 추가하였습니다.
* DFS 알고리즘을 통해 모든 돌들이 달성한 n목의 정보를 계산합니다.
* 바둑돌 순회 시, 거리(Distance Threshold)와 각도(Radian Threshold)를 고려합니다.
* 각 순회당 1회 Didtance Threshold를 *2하여 다음 Travel을 실시하여 중간이 비어있는 형태 또한 고려합니다.

<details>
<summary>바둑돌 순회하는 코드</summary>
  
```cpp
FGridStreakTrailContainer UGoSoccerAISubsystem::Travel_Accessibles_StreakContainer(
    int32 DollIdx, FStreakDollContainer& Visited,
    int32 Depth, const float TowardsRadian, const float InRadian_Threshold,
    const float InDistance_Threshold, FVector2D& JumpedGridPos, FGoBoardConfig& GoBoardConfig)
{
    auto GetRadianEstimatePhysicalLocation = [&](const FGridStreakTrailContainer& UpdateStreakContainer, const bool bReveresed, FVector2D& OutLocation, float& OutDistFromCenter, FVector& OutPhysicalLocation)
        -> bool
        {
            if (UpdateStreakContainer.StreakDepth <= 0 || !GoBoardConfig.BoardStat.IsValidIndex(DollIdx)) return false;
            FVector Direction{
                FMath::Cos(UpdateStreakContainer.StreakRadian),
                FMath::Sin(UpdateStreakContainer.StreakRadian),
                0.f
            };
            FVector Next_Location = GoBoardConfig.BoardStat[DollIdx].Location + (bReveresed ? -Direction : Direction) * InDistance_Threshold;
            int32 tX;
            int32 tY;
            GetGridLocation(Next_Location, tX, tY, OutDistFromCenter, OutPhysicalLocation, GoBoardConfig);
            OutLocation.X = tX;
            OutLocation.Y = tY;
            return IsInGrid(OutLocation.X, OutLocation.Y, GoBoardConfig);
        };
    auto GetRadianEstimateGridLocation = [&](const FGridStreakTrailContainer& UpdateStreakContainer, const bool bReveresed, FVector2D & OutLocation, float& OutDistFromCenter, FVector& OutEstimatedLocation)
        -> bool
    {
        if (UpdateStreakContainer.StreakDepth <= 0 || !GoBoardConfig.BoardStat.IsValidIndex(DollIdx)) return false;
        FVector Direction{
            FMath::Cos(UpdateStreakContainer.StreakRadian),
            FMath::Sin(UpdateStreakContainer.StreakRadian),
            0.f
        };
        FVector Next_Location = GoBoardConfig.BoardStat[DollIdx].Location + (bReveresed ? -Direction : Direction) * InDistance_Threshold * 0.85f;
        //FVector OnlyForDebugLocation;
        int32 tX;
        int32 tY;
        GetGridLocation(Next_Location, tX, tY, OutDistFromCenter, OutEstimatedLocation, GoBoardConfig);
        OutLocation.X = tX;
        OutLocation.Y = tY;
        return IsInGrid(OutLocation.X, OutLocation.Y, GoBoardConfig);
    };
    auto AddOrUpdateGridScore = [&](const FGridStreakTrailContainer& UpdateStreakContainer, const FVector2D& GirdLocation)
        -> bool
        {
            if (UpdateStreakContainer.StreakDepth <= 0 || !GoBoardConfig.BoardStat.IsValidIndex(DollIdx)) return false;
            if (!IsInGrid(GirdLocation.X, GirdLocation.Y, GoBoardConfig)) return false;
            bool bDoAdd = true;

            for (FGridStreakTrailContainer& Container_Iter : GoBoardConfig.Grid_Occupied[GirdLocation.X][GirdLocation.Y].StreakTrailArr)
            {
                if (Container_Iter.StreakTrailContainer.IsSubsetOf(UpdateStreakContainer.StreakTrailContainer))
                {   // One Of Olds Is Subset Of New One ---> Override Old One To New One
                    if (UpdateStreakContainer.StreakDepth >= Container_Iter.StreakDepth)
                    {
                        Container_Iter = UpdateStreakContainer;
                    }
                    bDoAdd = false;
                    break;
                }
                else if (UpdateStreakContainer.StreakTrailContainer.IsSubsetOf(Container_Iter.StreakTrailContainer))
                {   // New One Is Subset Of One Of Olds ---> if New One's Depth Is Less Than Old One, Ignore
                    if (Container_Iter.StreakDepth >= UpdateStreakContainer.StreakDepth)
                    {
                        bDoAdd = false;
                        break;
                    }
                }
            }

            if (bDoAdd) // Totally New Nodes
            {
                GoBoardConfig.Grid_Occupied[GirdLocation.X][GirdLocation.Y].StreakTrailArr.Add(UpdateStreakContainer);
            }
            return true;
        };

    const TArray<uint64>& AccessiblesContainer = GoBoardConfig.UF_Accessible[DollIdx].GetData();
    FGridStreakTrailContainer MaxDepthContainer{ Visited, StaticCast<uint8>(Depth), TowardsRadian };
    bool bVisitedAny = false;

    //if (Cal_BoardStat.IsValidIndex(DollIdx)) DrawDebugSphere(GetWorld(), Cal_BoardStat[DollIdx].Location + FVector(0.f, 0.f, 5.f), 5.f, 32, FColor::White, false, 2.5f);

    for (auto Iter = AccessiblesContainer.CreateConstIterator(); Iter; ++Iter)
    {
        uint64 Accessibles = *Iter;
        while (Accessibles != 0)
        {
            int32 BitIdx = FMath::CountTrailingZeros64(Accessibles);
            int32 idx = Iter.GetIndex() * 64 + BitIdx;
            bool bIsRootNode = Depth == 0;
            bool bIsYetJumped = JumpedGridPos.X < 0.f && JumpedGridPos.Y < 0.f;
            bool bJumpable = Depth < 3 && bIsYetJumped;
            if (!(Accessibles & (1ULL << BitIdx))) continue;
            Accessibles &= ~(1ULL << BitIdx);

            if (idx == DollIdx || Visited[idx] || !GoBoardConfig.BoardStat.IsValidIndex(idx))
            {
                continue;
            }
            FVector Direction = (GoBoardConfig.BoardStat[idx].Location - GoBoardConfig.BoardStat[DollIdx].Location).GetSafeNormal2D();
            float DestRadian = FMath::Atan2(Direction.Y, Direction.X);
            if (Depth > 0)
            {
                float Delta = FMath::FindDeltaAngleRadians(TowardsRadian, DestRadian);
                if (FMath::Abs(Delta) > InRadian_Threshold)
                {
                    continue;
                }
            }
            bool bInRange = IsDollInRange(GoBoardConfig.BoardStat[idx].Location, GoBoardConfig.BoardStat[DollIdx].Location, InDistance_Threshold);
            bool bInRange_DoubledScale = !bInRange && IsDollInRange(GoBoardConfig.BoardStat[idx].Location, GoBoardConfig.BoardStat[DollIdx].Location, InDistance_Threshold * 2.f);
            if (bInRange)
            { // Do Not Jump
                Visited[idx] = true;
                bVisitedAny = true;
                FGridStreakTrailContainer tempTrailCont = Travel_Accessibles_StreakContainer(
                    idx, Visited, Depth + 1,
                    Depth == 0 ? DestRadian : TowardsRadian,
                    InRadian_Threshold, InDistance_Threshold,
                    JumpedGridPos, GoBoardConfig
                );
                if (tempTrailCont.StreakDepth > MaxDepthContainer.StreakDepth) MaxDepthContainer = tempTrailCont;
                // Root Node Update Every Streks (These Block Is Always More Than 2 Streaks)
                if (GoBoardConfig.BoardStat[idx].DollColor != GoBoardConfig.AIDollColor && JumpedGridPos.X < 0.f && JumpedGridPos.Y < 0.f)
                {
                    int32 X;
                    int32 Y;
                    float DistFromCenter;
                    FVector CenterLocation;
                    GetGridLocation(GoBoardConfig.BoardStat[idx].Location, X, Y, DistFromCenter, CenterLocation, GoBoardConfig);
                    FVector2D GridPos;
                    GridPos.X = X;
                    GridPos.Y = Y;
                    //DrawDebugSphere(GetWorld(), CenterLocation + FVector(0.f, 0.f, 2.5f), 5.f, 32, FColor::Green, false, 3.f);
                    tempTrailCont.StreakDollColor = GoBoardConfig.BoardStat[idx].DollColor;
                    if (IsInGrid(GridPos.X, GridPos.Y, GoBoardConfig))
                    {
                        AddOrUpdateGridScore(tempTrailCont, GridPos);
                    }
                };
                if (bIsRootNode)
                {
                    //FVector2D Estimate_GridPos_R;
                    //float Estimate_DistFromCenter_R;
                    //if (GetRadianEstimateGridLocation(tempTrailCont, bIsRootNode, Estimate_GridPos_R, Estimate_DistFromCenter_R))
                    //{
                    //    if (JumpedGridPos.X < 0.f || JumpedGridPos.Y < 0.f) tempTrailCont.StreakDepth += 1;
                    //    else tempTrailCont.StreakDepth -= 1;
                    //    AddOrUpdateGridScore(tempTrailCont, Estimate_GridPos_R);
                    //    if (JumpedGridPos.X < 0.f || JumpedGridPos.Y < 0.f) tempTrailCont.StreakDepth -= 1;
                    //    else tempTrailCont.StreakDepth += 1;
                    //}
                }
                if (bIsYetJumped)
                {
                    JumpedGridPos.X = -1.f;
                    JumpedGridPos.Y = -1.f;
                }
                Visited[idx] = false;
            }
            else if (bJumpable && bInRange_DoubledScale)
            { // Do Jump When Jumpable
                FGridStreakTrailContainer JumpContatiner = FGridStreakTrailContainer{ Visited, StaticCast<uint8>(Depth + 1), bIsRootNode ? DestRadian : TowardsRadian };

                FVector Estimate_Location_J = (GoBoardConfig.BoardStat[DollIdx].Location + GoBoardConfig.BoardStat[idx].Location) / 2.f;
                float Estimate_DistFromCenter_J;
                int32 JumpedGridX;
                int32 JumpedGridY;
                FVector JumpedGridLoc;
                //DrawDebugSphere(GetWorld(), Estimate_Location_J + FVector(0.f, 0.f, 2.5f), 5.f, 32, FColor::Green, false, 3.f);
                GetGridLocation(Estimate_Location_J, JumpedGridX, JumpedGridY, Estimate_DistFromCenter_J, JumpedGridLoc, GoBoardConfig);
                FVector2D Estimate_GridPos_J;
                Estimate_GridPos_J.X = JumpedGridX;
                Estimate_GridPos_J.Y = JumpedGridY;
                if (!IsInGrid(JumpedGridX, JumpedGridY, GoBoardConfig))
                {
                    continue;
                }
                // Prev <-> Curr <-> (Jump_Estimated) <-> Dest
                bool bIsConnectable = true;
                FVector CJ_Direction = (Estimate_Location_J - GoBoardConfig.BoardStat[DollIdx].Location).GetSafeNormal2D();
                float CJ_Radian = FMath::Atan2(CJ_Direction.Y, CJ_Direction.X);

                FVector JD_Direction = (GoBoardConfig.BoardStat[idx].Location - Estimate_Location_J).GetSafeNormal2D();
                float JD_Radian = FMath::Atan2(JD_Direction.Y, JD_Direction.X);

                bool bCJ_DistTest = IsDollInRange(GoBoardConfig.BoardStat[DollIdx].Location, Estimate_Location_J, InDistance_Threshold);
                bool bJD_DistTest = IsDollInRange(Estimate_Location_J, GoBoardConfig.BoardStat[idx].Location, InDistance_Threshold);
                FGridStreakTrailContainer JumpedTrailCont{};
                if (!bCJ_DistTest || !bJD_DistTest)
                {
                    continue;
                }
                if (bIsRootNode)
                {
                    bool bRadTest = FMath::Abs(FMath::FindDeltaAngleRadians(CJ_Radian, JD_Radian)) <= InRadian_Threshold;
                    if (!bRadTest)
                    {
                        continue;
                    }
                    Visited[idx] = true;
                    bVisitedAny = true;
                    JumpedGridPos = Estimate_GridPos_J;
                    JumpedTrailCont = Travel_Accessibles_StreakContainer(
                        idx, Visited, Depth + 1,
                        CJ_Radian,
                        InRadian_Threshold, InDistance_Threshold,
                        JumpedGridPos, GoBoardConfig
                    );
                    JumpedGridPos.X = -1.f;
                    JumpedGridPos.Y = -1.f;
                    Visited[idx] = false;
                }
                else
                {
                    bool bJDRadTest = FMath::Abs(FMath::FindDeltaAngleRadians(TowardsRadian, JD_Radian)) <= InRadian_Threshold;
                    bool bCJRadTest = FMath::Abs(FMath::FindDeltaAngleRadians(TowardsRadian, CJ_Radian)) <= InRadian_Threshold;
                    if (!bJDRadTest || !bCJRadTest)
                    {
                        continue;
                    }
                    Visited[idx] = true;
                    bVisitedAny = true;
                    JumpedGridPos = Estimate_GridPos_J;
                    JumpedTrailCont = Travel_Accessibles_StreakContainer(
                        idx, Visited, Depth + 1,
                        TowardsRadian,
                        InRadian_Threshold, InDistance_Threshold,
                        JumpedGridPos, GoBoardConfig
                    );
                    JumpedGridPos.X = -1.f;
                    JumpedGridPos.Y = -1.f;
                    Visited[idx] = false;
                }
                if (JumpedTrailCont.StreakDepth > MaxDepthContainer.StreakDepth) MaxDepthContainer = JumpedTrailCont;
                JumpedTrailCont.StreakDepth += 1;
                JumpedTrailCont.StreakDollColor = GoBoardConfig.BoardStat[idx].DollColor;
                AddOrUpdateGridScore(JumpedTrailCont, Estimate_GridPos_J);
                // Root Node Update Every Streks (These Block Is Always More Than 2 Streaks)
                if (bIsRootNode)
                {
                    //JumpedTrailCont.StreakDepth -= 1;
                    //JumpedTrailCont.StreakDepth = FMath::Min((uint8)3, JumpedTrailCont.StreakDepth);
                    //FVector2D Estimate_GridPos_JR;
                    //float Estimate_DistFromCenter_JR;
                    //if (GetRadianEstimateGridLocation(JumpedTrailCont, bIsRootNode, Estimate_GridPos_JR, Estimate_DistFromCenter_JR))
                    //{
                    //    AddOrUpdateGridScore(JumpedTrailCont, Estimate_GridPos_JR);
                    //}
                }
            }
            /*
            if (tempTrailCont.StreakDepth == Depth + 1)
            ==> idx was leaf node Do In Travel(Idx) Not Here
            1. Assume : if Depth > 0, There's Only One Root Qualify TowardsRadian
            2. Assume : if There's More than One Root, Return Trail Container With Highest Depth
            --> Do Assume 2
            So Updated -> return TrainContainer
            if (Depth == 0)
            */
        }
    }
    // Leaf Node Update Whenever Depth > 0 (More Than 2 Streaks)
    bool bIsLeafNode = Depth > 0 && !bVisitedAny; // And Traveled Some Nodes
    if (bIsLeafNode)
    {
        FVector2D Estimate_GridPos;
        float Estimate_DistFromCenter;
        FVector Estimate_Location;
        if (GetRadianEstimateGridLocation(MaxDepthContainer, false, Estimate_GridPos, Estimate_DistFromCenter, Estimate_Location))
        {
            bool bJumped = JumpedGridPos.X >= 0.f && JumpedGridPos.Y >= 0.f;
            //if (bJumped) MaxDepthContainer.StreakDepth -= 1;
            if (!bJumped) MaxDepthContainer.StreakDepth += 1;
            MaxDepthContainer.StreakDollColor = GoBoardConfig.BoardStat[DollIdx].DollColor;
            MaxDepthContainer.UpdateTipGridPos(Estimate_GridPos.X, Estimate_GridPos.Y);
#if !UE_BUILD_SHIPPING
            
            //DrawDebugSphere(GetWorld(), Estimate_Location, 5.f, 32, Cal_BoardStat[DollIdx].DollColor == AIDollColor ? FColor::Cyan : FColor::Orange, false, 3.f);
#endif
            AddOrUpdateGridScore(MaxDepthContainer, Estimate_GridPos);
            if (!bJumped) MaxDepthContainer.StreakDepth -= 1;
            //else MaxDepthContainer.StreakDepth += 1;
        }
    }
    /*
    if MaxDepthContainer.Depth > 0
    if RootNode
    Calculate Placable Location
    Find Grid X, Y Coord
    Add TrailContainer
        -> Check Arr : If Iter Is Subset of NewCont or NewCont Is Subset of Iter
        -> If so, Del Iter/Add NewCont or Ignore
    else if LeafNode
    Do As Above But Revered Direction
    */
    return MaxDepthContainer;
}
```

</details>


<details>
<summary>순회 이후 데이터를 저장하는 코드</summary>
  
```cpp
void UGoSoccerAISubsystem::EvaluateBoard_WriteStreakData(FGoBoardConfig& GoBoardConfig)
{
    //// Assert Union Completed
    FStreakDollContainer VisitedRootDoll{ GoBoardConfig.BoardStat.Num() };
    FStreakDollContainer Visited{ GoBoardConfig.BoardStat.Num() / 64 + 1 };
    FStreakDollContainer VisitedTrace{ GoBoardConfig.BoardStat.Num() / 64 + 1 };
    for (int32 i = 0; i < GoBoardConfig.BoardStat.Num(); i++)
    {
        Visited[i] = true;
        FVector2D tempVector { -1.f, -1.f };
        int32 StartDepth = 0;
        bool AIBonus = GoBoardConfig.AIStreakBonus > 0 && GoBoardConfig.BoardStat[i].DollColor == GoBoardConfig.AIDollColor;
        bool PlayerBonus = GoBoardConfig.AIStreakBonus < 0 && GoBoardConfig.BoardStat[i].DollColor == (1ULL ^ GoBoardConfig.AIDollColor);
        if (AIBonus || PlayerBonus)
        {
            StartDepth += FMath::Abs(GoBoardConfig.AIStreakBonus);
        }
        FGridStreakTrailContainer MaxStreakTrailContainer = Travel_Accessibles_StreakContainer(
            i, Visited, StartDepth, 0.f, GoBoardConfig.Radian_Threshold, GoBoardConfig.Distance_Threshold, tempVector, GoBoardConfig
        );

        float NearbyScore = FMath::Pow(2.f, MaxStreakTrailContainer.StreakDepth);
        int32 X;
        int32 Y;
        float tempDist;
        FVector tempLoc;
        GetGridLocation(GoBoardConfig.BoardStat[i].Location, X, Y, tempDist, tempLoc, GoBoardConfig);
        for (int32 tX = X - 1; tX < X + 2; tX++)
        {
            for (int32 tY = Y - 1; tY < Y + 2; tY++)
            {
                if (!IsInGrid(tX, tY, GoBoardConfig)) continue;
                UE_LOG(LogTemp, Log, TEXT("NearbyScore : %f To [%d, %d]"), NearbyScore, tX, tY);
                GoBoardConfig.Grid_Occupied[tX][tY].Score += NearbyScore;
            }
        }
        Visited[i] = false;
    }
}
```

</details>

### 2. 바둑판의 Score 산정은 특정 그리드에 위치한 바둑돌이 아닌 특정 그리드를 지나는 n목의 데이터를 사용하였습니다.
* 모든 그리드(19*19)를 순회하며 FGridStreakTrailContainer 배열을 확인, 해당 그리드와 각 Tip에 점수를 더합니다.
* 점수를 더할 때 FStreakDollContainer::IsSubsetOf를 활용하여 동일하거나 이미 점수를 부여한 n목의 부분집합일 경우 넘어갑니다.

<details>
<summary>AI의 행동을 결정하는 코드</summary>

```cpp
void UGoSoccerAISubsystem::EvaluateBoard_GridStreakTrailsToScore(
    TArray<FCandidate>& PlacableLocations, TArray<FCandidate>& FlickableTargets, TArray<FCandidate>& UnrelatedGrids,
    float& TotalScore, uint8& Def_MaxStreak, FGoBoardConfig& GoBoardConfig, bool bDebug
)
{
    auto IsStreakTipOccupied = [&](FGridStreakTrailContainer& CheckStreakContainer, bool& bIsOccupied0, bool& bIsOccupied1)
        -> void 
        {
            bIsOccupied0 = true;
            bIsOccupied1 = true;
            if (IsInGrid(CheckStreakContainer.Tip0_Container.X, CheckStreakContainer.Tip0_Container.Y, GoBoardConfig)) // Grid Check
            {
                if (GoBoardConfig.Grid_Occupied[CheckStreakContainer.Tip0_Container.X][CheckStreakContainer.Tip0_Container.Y].ClosestStoneIdx < 0 // If Contain No Doll = false
                    || GoBoardConfig.Grid_Occupied[CheckStreakContainer.Tip0_Container.X][CheckStreakContainer.Tip0_Container.Y].StoneDistance > GRID_MINIMUM_DISTANCE_THRESHOLD) // Doll Distance Far = false
                {
                    bIsOccupied0 = false;
                }
            }
            if (IsInGrid(CheckStreakContainer.Tip1_Container.X, CheckStreakContainer.Tip1_Container.Y, GoBoardConfig))
            {
                if (GoBoardConfig.Grid_Occupied[CheckStreakContainer.Tip1_Container.X][CheckStreakContainer.Tip1_Container.Y].ClosestStoneIdx < 0
                    || GoBoardConfig.Grid_Occupied[CheckStreakContainer.Tip1_Container.X][CheckStreakContainer.Tip1_Container.Y].StoneDistance > GRID_MINIMUM_DISTANCE_THRESHOLD)
                {
                    bIsOccupied1 = false;
                }
            }
        };
    auto GetIdxArr = [&](FStreakDollContainer& ChcekContainer, uint8 InDollColor)
        -> TArray<int32>
        {
            TArray<int32> OutIdxArr;
            TArray<uint64> C = ChcekContainer.GetData();
            for (auto Iter = C.CreateConstIterator(); Iter; ++Iter)
            {
                uint64 Dolls = *Iter;
                while (Dolls != 0)
                {
                    int32 BitIdx = FMath::CountTrailingZeros64(Dolls);
                    int32 idx = Iter.GetIndex() * 64 + BitIdx;
                    if (!(Dolls & (1ULL << BitIdx))) continue;

                    if (GoBoardConfig.BoardStat.IsValidIndex(idx)
                        && (InDollColor < 0 || GoBoardConfig.BoardStat[idx].DollColor == InDollColor))
                    {
                        OutIdxArr.Emplace(idx);
                    }
                    Dolls &= ~(1ULL << BitIdx);
                }
            }
            return OutIdxArr;
        };
    const int32 GridSize = 19;
    Def_MaxStreak = 0;
    for (int32 X = 0; X < GridSize; ++X)
    {
        FString RowLogString = TEXT("");
        for (int32 Y = 0; Y < GridSize; ++Y)
        {
            if (!IsInGrid(X, Y, GoBoardConfig)) continue;
            // Grid Param**
            uint8 DollIdx = GoBoardConfig.Grid_Occupied[X][Y].ClosestStoneIdx;
            //Grid_Occupied[X][Y].
            bool bIsOccupied = DollIdx >= 0 && GoBoardConfig.Grid_Occupied[X][Y].StoneDistance <= GRID_MINIMUM_DISTANCE_THRESHOLD;
#if UE_EDITOR
            float StepX = (GoBoardConfig.R - GoBoardConfig.L) / (BOARD_GRID_SIZE - 1);
            float StepY = (GoBoardConfig.D - GoBoardConfig.U) / (BOARD_GRID_SIZE - 1);
            FVector DrawDebugCenterLoc = FVector(
                GoBoardConfig.L + X * StepX,
                GoBoardConfig.U + Y * StepY,
                BOARD_LOCATION_Z_AXIS + 5.f
            );
            if (bIsOccupied)
            {
                GoBoardConfig.DebugDataArr.Emplace(DrawDebugCenterLoc, bIsOccupied);
            }
#endif

            bool bHasDoll = DollIdx >= 0;
            bool bContainAIDoll = (1ULL << GoBoardConfig.AIDollColor) & GoBoardConfig.Grid_Occupied[X][Y].ContainDollColor;
        // To Flick Doll Params
            float LocationValueForAI = 0.f;
            TArray<int32> AIDollArr = GetIdxArr(GoBoardConfig.Grid_Occupied[X][Y].CloseDollContainer, GoBoardConfig.AIDollColor);
            TArray<float> FlickableScore;
            FlickableScore.Init(0.f, AIDollArr.Num());
        // Flicked Doll Params
            float LocationValueForPlayer = 0.f;
            TArray<int32> PlayerDollArr = GetIdxArr(GoBoardConfig.Grid_Occupied[X][Y].CloseDollContainer, (GoBoardConfig.AIDollColor + 1) % 2);
            TArray<float> DefenceScore;
            DefenceScore.Init(0.f, PlayerDollArr.Num());

            bool bContainPlayerDoll = (1ULL << ((GoBoardConfig.AIDollColor + 1) % 2)) & GoBoardConfig.Grid_Occupied[X][Y].ContainDollColor;
            uint8 CurrPosDollColor = GoBoardConfig.BoardStat.IsValidIndex(DollIdx) ? GoBoardConfig.BoardStat[DollIdx].DollColor : -1;
            uint8 tempDef_MaxStreak = 0;
            uint8 Streak3OpenPlaceCount = 0;
            uint8 AccessibleCount = GoBoardConfig.UF_Accessible.IsValidIndex(DollIdx) ? GoBoardConfig.UF_Accessible[DollIdx].GetCount() : 0;
            uint8 EnemyNearbyCount = GoBoardConfig.UF_CloseEnemy.IsValidIndex(DollIdx) ? GoBoardConfig.UF_CloseEnemy[DollIdx].GetCount() : 0;
            uint8 CanBePlaced_MaxStreak = 0;
            bool bEndure = false;
            bool bSuperDanger = false;
            // Grid Param End**
            for (FGridStreakTrailContainer& Iter : GoBoardConfig.Grid_Occupied[X][Y].StreakTrailArr)
            {
                bool bIsTipOccupied_0 = true;
                bool bIsTipOccupied_1 = true;
                IsStreakTipOccupied(Iter, bIsTipOccupied_0, bIsTipOccupied_1);
                float StreakValue = FMath::Pow(
                    Iter.StreakDollColor == GoBoardConfig.AIDollColor ? AI_STREAK_SCORE_WEIGHT : PLAYER_STREAK_SCORE_WEIGHT,
                    FMath::Min((uint8)4, Iter.StreakDepth));
                float TipWeight = bIsTipOccupied_0 && bIsTipOccupied_1 ? 0.75f : (!bIsTipOccupied_0 && !bIsTipOccupied_1 ? 2.5f : 1.2f);
                float ReverseTipWeight = !bIsTipOccupied_0 && !bIsTipOccupied_1 ? 0.95f : (bIsTipOccupied_0 && bIsTipOccupied_1 ? 2.5f : 1.4f);
                if (bContainAIDoll)
                { // Flick Doll In This Grid ?
                    if (Iter.StreakDollColor == GoBoardConfig.AIDollColor)
                    {
                        for (int32 i = 0; i < AIDollArr.Num(); ++i) // Doll Indivisual Streak Score
                        {
                            int32 D_Idx = AIDollArr[i];
                            if (!Iter.StreakTrailContainer.IsValidIndex(D_Idx)) continue;
                            if (!Iter.StreakTrailContainer[D_Idx]) continue;
                            if (!FlickableScore.IsValidIndex(i)) continue;
                            FlickableScore[i] += StreakValue * TipWeight;
                        }
                    }
                    else // AI Wouldn't Want To Leave This Grid Score
                    {
                        LocationValueForAI += StreakValue * TipWeight;
                    }
                }
                if (bContainPlayerDoll)
                { // Flick To This Grid ?
                    if (Iter.StreakRadian == GoBoardConfig.AIDollColor) // Steal This Grid
                    {
                        LocationValueForPlayer += StreakValue * TipWeight;
                        if (!bIsTipOccupied_0 && !bIsTipOccupied_1) // Open Both Tip Side
                        {
                            LocationValueForPlayer *= 15.f;
                        }
                        else if (!bIsTipOccupied_0 || !bIsTipOccupied_1) // Least On Tip Siee Open
                        {
                            LocationValueForPlayer *= 1.5f;
                        }
                    }
                    else
                    {
                        for (int32 i = 0; i < PlayerDollArr.Num(); ++i) // Doll Indivisual Streak Score
                        {
                            int32 D_Idx = PlayerDollArr[i];
                            if (!Iter.StreakTrailContainer.IsValidIndex(D_Idx)) continue;
                            if (!Iter.StreakTrailContainer[D_Idx]) continue;
                            if (!DefenceScore.IsValidIndex(i)) continue;
                            DefenceScore[i] += StreakValue * TipWeight * (!bIsTipOccupied_0 && !bIsTipOccupied_1 ? 15.f : 1.f);
                        }
                    }
                }
                if (!bIsOccupied) // This Grid Is Tip or Jumped
                { // Priority Zone - Not Occupied -> Place Here
                    GoBoardConfig.Grid_Occupied[X][Y].Score += StreakValue * ReverseTipWeight; // If Both Tip Is Open -> Flick Is Better
                }
            }
            if (!bIsOccupied)
            { // Placable Location Push
                TotalScore += GoBoardConfig.Grid_Occupied[X][Y].Score;
                FCandidate tempPlacable{ FVector2D(X, Y), GoBoardConfig.Grid_Occupied[X][Y].Score };
                PlacableLocations.HeapPush(
                    tempPlacable,
                    [](const FCandidate& A, const FCandidate& B) {
                        return A.Score > B.Score;
                    }
                );
            }
            if (AIDollArr.Num() > 0)
            { //Flick This Doll
                for (int32 i = 0; i < AIDollArr.Num(); ++i)
                {
                    //if (!FlickableScore.IsValidIndex(idx)) continue;
                    FCandidate tempCandidate{ FVector2D(X, Y), AIDollArr[i], LocationValueForAI + FlickableScore[i]};
                    UnrelatedGrids.HeapPush(
                        tempCandidate,
                        [](const FCandidate& A, const FCandidate& B) { // Flick Smaller Score
                            return A.Score < B.Score;
                        }
                    );
                }
            }
            if (PlayerDollArr.Num() > 0)
            { // Flick To This Doll
                //for (int32 idx : PlayerDollArr)
                for (int32 i = 0; i < PlayerDollArr.Num(); ++i)
                {
                    //if (!DefenceScore.IsValidIndex(idx)) continue;
                    FCandidate tempCandidate{ FVector2D(X, Y), PlayerDollArr[i], LocationValueForPlayer + DefenceScore[i]};
                    FlickableTargets.HeapPush(
                        tempCandidate,
                        [](const FCandidate& A, const FCandidate& B) { // Attack Most Score
                            return A.Score > B.Score;
                        }
                    );
                }
            }
            RowLogString += FString::Printf(TEXT("[%4.1f] "), GoBoardConfig.Grid_Occupied[X][Y].Score);
        }
        UE_LOG(LogTemp, Log, TEXT("%s"), *RowLogString);
    }
}
```

</details>


### 3. 모든 Grid에 대해 Score를 산정한 이후 다음의 분기에 따라 행동을 결정하였습니다.
* 방어가 가능한 수준 (FStreakDollContainer::GetCount가 4 미만이거나 각 Tip중 하나가 점유중 일 때) 돌을 소환합니다.
* 특정 확률(소환이 필요하더라도) 또는 방어가 불가능한 수준일 경우 Score가 낮은 돌을 튕겨 제일 위험한 수준의 돌을 향해 튕깁니다.
* 해당 여부는 각각의 Score를 Heap 배열에 Push하여 정렬한 상태로 보관하였습니다.

<details>
<summary>AI의 행동을 결정하는 코드</summary>

```cpp
    EvaluateBoard_GridStreakTrailsToScore(PlacableLocations, FlickableTargets, UnrelatedGrids, TotalScore, Def_MaxStreak, GoBoardConfig, false);

    // RNG Starts ---------------------------------------------------------------

    float Place_TopScore = (PlacableLocations.Num() > 0) ? PlacableLocations.HeapTop().Score : TNumericLimits<float>::Min();
    float Flick_TopScore = (FlickableTargets.Num() > 0) ? FlickableTargets.HeapTop().Score : TNumericLimits<float>::Min();

    // Flick Debug
    Flick_TopScore *= FMath::FRandRange(1.f, 10.f);

    UE_LOG(LogTemp, Log, TEXT("DollCounter_AI : %d"), DollCounter_AI);
    UE_LOG(LogTemp, Log, TEXT("Place_TopScore : %f"), Place_TopScore);
    UE_LOG(LogTemp, Log, TEXT("Flick_TopScore : %f"), Flick_TopScore);
    if (Place_TopScore < Flick_TopScore && UnrelatedGrids.Num() > 0 && FlickableTargets.Num() > 0)
    {
        FCandidate& FlickTarget = FlickableTargets.HeapTop();
        int32 FlickTargetIdx = FlickTarget.StoneIdx;
        FVector2D FlickTargetGridPos = FlickTarget.GridLocation;
        if (GoBoardConfig.BoardStat.IsValidIndex(FlickTargetIdx) && IsInGrid(FlickTargetGridPos.X, FlickTargetGridPos.Y, GoBoardConfig))
        {
            int32 Min_Doll_Count = TNumericLimits<int32>::Max();
            int32 Min_Doll_Idx = -1;
            float Min_Doll_Distance = TNumericLimits<float>::Max();
            FVector FlickTargetLocation = GoBoardConfig.BoardStat[FlickTargetIdx].Location;
            FStreakDollContainer VisitedDoll{ 1 + GoBoardConfig.BoardStat.Num() / 64 };
            while (UnrelatedGrids.Num() > 0)
            {
                FCandidate RootCandidate;
                UnrelatedGrids.HeapPop(
                    RootCandidate,
                    [](const FCandidate& A, const FCandidate& B) {
                        return A.Score < B.Score;
                    });
                int32 FlickThisDollIdx = RootCandidate.StoneIdx;
                if (FlickTargetIdx == FlickThisDollIdx) continue;
                FVector2D FlickThisDollGridPos = RootCandidate.GridLocation;
                if (!GoBoardConfig.BoardStat.IsValidIndex(FlickThisDollIdx) || !IsInGrid(FlickThisDollGridPos.X, FlickThisDollGridPos.Y, GoBoardConfig)) continue;
                if (GoBoardConfig.BoardStat[FlickThisDollIdx].DollColor != GoBoardConfig.AIDollColor) continue;
                FVector FlickThisDollLocation = GoBoardConfig.BoardStat[FlickThisDollIdx].Location;
                int32 Doll_In_Way_Count = 0;
                float Flick_Distance = FVector::Dist2D(FlickTargetLocation, FlickThisDollLocation);
                VisitedDoll.Clear();
                //CountDollInWay_Approximate(FlickThisDollGridPos, FlickTargetGridPos, Doll_In_Way_Count);
                CountDollInWay(FlickThisDollGridPos, FlickTargetGridPos, FlickThisDollLocation, FlickTargetLocation, Doll_In_Way_Count, VisitedDoll);
                if ((Min_Doll_Count == Doll_In_Way_Count && Min_Doll_Distance > Flick_Distance)
                    || Min_Doll_Count > Doll_In_Way_Count)
                {
                    //if (PlacedDollArr.IsValidIndex(FlickThisDollIdx))
                    //{
                        Min_Doll_Count = Doll_In_Way_Count;
                        Min_Doll_Idx = FlickThisDollIdx;
                        Min_Doll_Distance = Flick_Distance;
                    //}
                }
            }
            if (FlickTargetIdx != Min_Doll_Idx && GoBoardConfig.BoardStat.IsValidIndex(Min_Doll_Idx))
            {
                FVector Flick_Direction = FlickTargetLocation - GoBoardConfig.BoardStat[Min_Doll_Idx].Location;
                float Flick_Distance = Flick_Direction.Size2D();
                //Flick_Direction = Flick_Direction.GetSafeNormal2D();
                UE_LOG(LogTemp, Log, TEXT("Flick_Distance : %f"), Flick_Distance);
                float PowerRNG = FMath::Abs(100.f - Flick_Distance);
                PowerRNG = FMath::FRandRange(3.f, FMath::Min(100.f, PowerRNG));
                // Flick This Doll To Flick_Direction
                FlickDirection = Flick_Direction * PowerRNG;
                FlickDollIndex = Min_Doll_Idx;
                //UGoSoccerPlayManager::AIPlay_FlickDoll(GetWorld()->GetAuthGameMode(), AWaitPlayer, PlacedDollArr[Min_Doll_Idx].Get(), Flick_Direction * PowerRNG);
                //return;
            }
        }
    }

    float ScoreStack = 0.f;
    if (PlacableLocations.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Available Place Location ! ! !"));
        // TODO Play AnyWhere
    }
    while (PlacableLocations.Num() > 0)
    {
        FCandidate RootPlacable;
        PlacableLocations.HeapPop(
            RootPlacable,
            [](const FCandidate& A, const FCandidate& B) {
                return A.Score > B.Score;
            });
        ScoreStack += RootPlacable.Score;
        if (FMath::FRandRange(0.f, TotalScore * 0.5f) <= ScoreStack)
        {
            //const int32 GridSize = 19;
            //const float BoardZ = -180.0f;
            float StepX = (GoBoardConfig.R - GoBoardConfig.L) / (BOARD_GRID_SIZE - 1);
            float StepY = (GoBoardConfig.D - GoBoardConfig.U) / (BOARD_GRID_SIZE - 1);
            FVector CenterLoc = FVector(
                GoBoardConfig.L + RootPlacable.GridLocation.X * StepX,
                GoBoardConfig.U + RootPlacable.GridLocation.Y * StepY,
                BOARD_LOCATION_Z_AXIS + 5.f
            );
            //DrawDebugSphere(
            //    GetWorld(), CenterLoc, 2.f, 32, FColor::Green, false, 2.5f
            //);
            PlaceLocation = CenterLoc;
            break;
            //bool bSpawned = UGoSoccerPlayManager::AIPlay_SpawnDollAtLocation(GetWorld()->GetAuthGameMode(), AWaitPlayer, CenterLoc);
            //if (bSpawned)
            //{
            //    break;
            //}
            //else
            //{
            //    continue;
            //}
        }
    }
```

</details>

### 4. 알고리즘을 구현한 뒤, 멀티스레드로 수정하였습니다.
* 사용한 함수를 static 함수로 수정하여 Background thread에서 실행하였습니다.
* AI의 행동(소환 또는 튕기기)는 Delegate를 통해 전달하였습니다.
<details>
<summary>Background 스레드에서 함수를 호출하는 코드</summary>

```cpp
    Initialize_GridArr(PlacedDollArrRef, GoBoardConfig);
    double InitGrid_Time = FPlatformTime::Seconds();
    UE_LOG(LogTemp, Log, TEXT("1/4 - Initialize_GridArr - Took : %.2f ms"), (InitGrid_Time - StartTime) * 1000.f);

    Union_Board(GoBoardConfig);
    double UnionBoard_Time = FPlatformTime::Seconds();
    UE_LOG(LogTemp, Log, TEXT("2/4 - Union_Board - Took : %.2f ms"), (UnionBoard_Time - InitGrid_Time) * 1000.f);

    AsyncTask(
        ENamedThreads::AnyBackgroundThreadNormalTask,
        [GoBoardContext = MoveTemp(GoBoardConfig), WaitPlayer, InCalculateCompleted = CalculateCompleted, InOnDebugDoll = OnDebugDoll]() mutable
        {
            EvaluateBoard_WriteStreakData(GoBoardContext);
            //double EvaluageBoard_Time = FPlatformTime::Seconds();
            //UE_LOG(LogTemp, Log, TEXT("3/4 - EvaluateBoard - Took : %.2f ms"), (EvaluageBoard_Time - UnionBoard_Time) * 1000.f);
            int32 FlickDollIdx = -1;
            FVector FlickDirection;
            FVector PlaceLocation;

            //FPlatformProcess::Sleep(5.f);
            Notify_PlaceLocation_Calculated(FlickDollIdx, FlickDirection, PlaceLocation, GoBoardContext);
            //double AIPlayCalculate_Time = FPlatformTime::Seconds();
            //UE_LOG(LogTemp, Log, TEXT("4/4 - AIPlay_Calculated - Took : %.2f ms"), (AIPlayCalculate_Time - EvaluageBoard_Time) * 1000.f);
            
#if UE_EDITOR
            AsyncTask(ENamedThreads::GameThread, [GoBoardContext, InOnDebugDoll]()
                {
                    for (const auto& Iter : GoBoardContext.DebugDataArr)
                    {
                        InOnDebugDoll.ExecuteIfBound(Iter);
                    }
                }
            );
#endif
            AsyncTask(ENamedThreads::GameThread, [FlickDollIdx, FlickDirection, PlaceLocation, WaitPlayer, InCalculateCompleted]()
                {
                    InCalculateCompleted.ExecuteIfBound(FlickDollIdx, FlickDirection, PlaceLocation, WaitPlayer);

                }
            );
        }
    );
    double AIPlayCalculate_Time = FPlatformTime::Seconds();
    UE_LOG(LogTemp, Log, TEXT("4/4 - BackGround Threads - Took : %.2f ms"), (AIPlayCalculate_Time - UnionBoard_Time) * 1000.f);
#if !UE_BUILD_SHIPPING
	//Debug_GetPossibleSpawnDollLocation(true);
#endif
    double EndTime = FPlatformTime::Seconds();
    UE_LOG(LogTemp, Log, TEXT("Total Request_Calculate - Took : %.2f ms"), (EndTime - StartTime) * 1000.f);
```
    
</details>

### 5. 구현 및 디버깅 이후 Claude를 활용하여 Min/Max 알고리즘을 구현하였습니다.
* 기존 수기로 작성한 코드는 Depth가 1일 경우에 행동을 결정하는 알고리즘으로 한정적이었습니다.
* 기존 코드를 활용하여 2이상의 Depth를 탐색하는 코드로 확장하도록 하였습니다.
<details>
<summary>사용된 프롬프트</summary>
  
  ---

# 1. 코드 파악 / 기능별 정리

## 프롬프트
  GoSoccerAISubSytem 훑어보고 기능별로 정리해줘, 목표는 Min Max 알고리즘을
  적용시켜서 특정 depth이상 탐색하고 싶은데 정리먼저 하게

### 결과
  - 파이프라인 정리 (AITurnStart -> Request_Calculate -> Initialize_GridArr
    -> Union_Board -> [BG] EvaluateBoard_WriteStreakData
    -> Notify_PlaceLocation_Calculated -> Calculate_Completed)
  - 자료구조 정리 (FStreakDollContainer, FStoneData, FGridData,
    FGridStreakTrailContainer, FCandidate, FGoBoardConfig)
  - 기능별 정리 (보드 스캔 / 인접 그래프 / 스트릭 DFS / 점수화 / 최종 결정 /
    좌표 변환 / 디버그)
  - Min-Max 도입 관점 진단
      (1) Apply/Undo 없음
      (2) 평가가 누적형이라 재진입 불가
      (3) 스칼라 평가값 없음
      (4) RNG가 결정 로직에 섞여 있음
      (5) 분기 인수 과다 (Place 361, Flick 연속값)
      (6) 성능 예산
      (7) 죽은 코드 (UF_Parent/UF_Union/UF_Find, Travel_Accessibles 등)
      (8) 잠재 버그 2건 (uint8 DollIdx, StreakRadian == AIDollColor)

# 2. 상위 N개 후보 순회 + depth 탐색 구현

## 프롬프트
  19*19를 모두 탐색하는 것부터 수정하고 싶어 최초 실행 시 각 그리드에 점수를
  매기고 상위 n개에 대해서만 순회(순회 중 depth탐사 시 점수 수정, depth 탐사
  완료 시 점수 롤백)하는 방식에 대해 평가해보고 수정해줘

### 평가 결론
  - 상위 N개 순회 = move ordering + beam 가지치기, 타당함
  - "점수 수정 -> 롤백"은 누적형 Score/StreakTrailArr 구조에서는 stale 항목
    추적이 어려워 위험 -> 상태를 둘로 분리
      * 점유 상태 (BoardStat / 그리드 점유 / UF 비트셋) : ApplyPlace / UndoPlace 로 증분 롤백
      * 평가 스크래치 (Score / StreakTrailArr)        : 노드마다 Reset 후 재계산

### 구현 내용
#### 헤더
  - FGoBoardConfig : DollCapacity, bSearchMode 추가
  - FPlaceUndoRecord 구조체 추가
  - UF_JoinDollIngoGroup -> static
  - 신규 static 함수 선언 :
      GridToWorldLocation, ApplyPlace, UndoPlace, ResetEvaluation,
      GetStreakTipOccupied, GetMaxContiguousStreak,
      EvaluateBoard_ForSearch, Search_Place, Search_SelectPlace
#### cpp
  - 상수 : AI_SEARCH_TOP_N(8), AI_SEARCH_DEPTH(2), AI_SEARCH_MAX_ADDED_DOLLS,
           AI_WIN_STREAK(5), AI_WIN_SCORE
  - Request_Calculate / Union_Board : 비트셋 사이징을 DollCapacity 기준으로
  - EvaluateBoard_WriteStreakData : 탐색 중(bSearchMode) 로그 억제
  - EvaluateBoard_GridStreakTrailsToScore : IsStreakTipOccupied 람다가
    static GetStreakTipOccupied 를 호출하도록 변경
  - Notify_PlaceLocation_Calculated : Flick 미결정 && AI_SEARCH_DEPTH > 0 이면
    Search_SelectPlace 로 Place 결정, 아니면 기존 룰렛 유지
  - 파일 끝에 Min-Max 탐색 함수 일괄 추가
  - 누락 include 추가 (Engine/World.h, DrawDebugHelpers.h, Async/Async.h)

### 검증
  - UBT -SingleFile 로 GoSoccerAISubsystem.cpp 단독 컴파일 : 에러 0 / 경고 0
  - 전체 에디터 빌드는 에디터 실행 중(.pdb 잠금)으로 미수행 -> Live Coding 으로 확인 필요

### 확인 필요 가정
  - 승리 판정(AI_WIN_STREAK)에 AIStreakBonus 미포함 (5or5 규칙 확인 필요)
  - 정사각 보드 가정 ((R-L)/18 == (U-D)/18) - 기존 코드와 동일

  ---

</details>

-----


## **3. 머티리얼 기반 UI 구현**

<img width="474" height="227" alt="image" src="https://github.com/user-attachments/assets/b3f1569a-e9e8-4939-99d7-da19ed633429" />

<img width="1040" height="577" alt="image" src="https://github.com/user-attachments/assets/f99f7c2a-5e1f-4976-ae18-a0bad8ba4be6" />

메인 위젯의 플로우는 Enum을 활용하여 c++ 스크립트에서 변경, 블루프린트에서 애니메이션을 재생하는 방식으로 구현하였습니다.

Check_*UI*와 Out_*UI* 파라미터를 통해 각 패널의 체크 여부와, 목표 Visibility를 전달하는 방식을 사용하였습니다. (예: Setting Panel로 진입 시 다른 패널은 체크할 필요 없이 Setting Panel의 Visiblity만 체크하면 됨)

c++ 클래스를 상속한 블루프린트 위젯에서 Material Instance Dynamic(MID)을 생성, 지정하였습니다.

위젯의 애니메이션은 Blueprint Implementable Event 매크로를 적용한 함수를 호출하여 MID의 파라미터를 조정하는 방식으로 구현하였습니다.

## **4. 확장성을 고려한 객체 지향적 구조**

static 클래스 GoSoccerPlayManager를 구현하여 각 클래스별 함수 호출을 자유롭게 하였습니다.

각 클래스는 인터페이스에 BlueprintNativeEvent 매크로를 추가하여 Execute_foo 형태로 호출이 가능하도록 구현하였습니다.

GoSoccerPlayManager는 특정 객체의 함수 호출 시, 해당 객체를 UObject로 캐스팅하여 전달받고, 인터페이스에 선언되어 있는 함수를 Execute_foo(UOBject* O) 형식으로 호출합니다.

이전 프로젝트에서 각 클래스별 의존성과 프로젝트가 커짐에 따라 컴파일 시간이 증가함(특정 클래스를 include하는 타 클래스까지 컴파일을 진행함)을 확인하고 해당 구조를 선택하였습니다.
