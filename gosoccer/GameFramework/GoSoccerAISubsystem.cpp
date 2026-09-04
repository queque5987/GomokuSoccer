#include "GameFramework/GoSoccerAISubsystem.h"
#include "GameFramework/GameModeBase.h"
#include "GoSoccerPlayManager.h"

constexpr float GRID_MINIMUM_DISTANCE_THRESHOLD = 20.f;//6.5f;
constexpr float GRID_MAXIMUM_DISTANCE_THRESHOLD = 8.f;
constexpr float AI_STREAK_SCORE_WEIGHT = 12.f;
constexpr float PLAYER_STREAK_SCORE_WEIGHT = 10.f;
constexpr float DOLL_FLICK_THRESHOLD = 5.f;
constexpr int32 BOARD_GRID_SIZE = 19;
constexpr float BOARD_LOCATION_Z_AXIS = -180.f;

void UGoSoccerAISubsystem::Request_Calculate(
    UObject* WaitPlayer, const TArray<TWeakObjectPtr<AActor>>& PlacedDollArrRef,
    float InDistance_Threshold, float InRadian_Threshold,
    float InU, float InD, float InL, float InR,
    uint8 InAIDollColor, int8 AIStreakBonus
)
{
    if (WaitPlayer == nullptr) return;

    if (!CalculateCompleted.IsBound() || !CalculateCompleted.IsBoundToObject(this))
    {
        CalculateCompleted.Unbind();
        CalculateCompleted.BindUFunction(this, FName("Calculate_Completed"));
    }
    if (!OnDebugDoll.IsBound() || !OnDebugDoll.IsBoundToObject(this))
    {
        OnDebugDoll.Unbind();
        OnDebugDoll.BindLambda([&](const FStoneDebugData& DebugDat)->void
            {
                DrawDebugSphere(GetWorld(), DebugDat.Location, 5.f, 32, DebugDat.bOccupied ? FColor::Red : FColor::Green, false, 3.f);
            }
        );
    }
    double StartTime = FPlatformTime::Seconds();

    AWaitPlayer = WaitPlayer;
    //AIDollColor = InAIDollColor;
    //Distance_Threshold = InDistance_Threshold;
    //Radian_Threshold = InRadian_Threshold;
    //BoardStat.Empty();
    UF_Parent.Empty();
    //UF_Accessible.Empty();
    //UF_CloseEnemy.Empty();
    PlacedDollArr.Empty();

    FGoBoardConfig GoBoardConfig{ InL, InR, InU, InD, (InR - InL) / 18.f, InDistance_Threshold, InRadian_Threshold, InAIDollColor, AIStreakBonus };

    GoBoardConfig.Grid_Occupied.SetNum(BOARD_GRID_SIZE);
    for (int32 i = 0; i < BOARD_GRID_SIZE; ++i)
    {
        GoBoardConfig.Grid_Occupied[i].Init(FGridData{ PlacedDollArrRef.Num() }, BOARD_GRID_SIZE);
    }
    
    //TArray<TArray<FGridData>> Grid_Occupied;
    //Grid_Occupied.SetNum(BOARD_GRID_SIZE);
    //for (int32 i = 0; i < BOARD_GRID_SIZE; ++i)
    //{
    //    Grid_Occupied[i].Init(FGridData{ PlacedDollArrRef.Num() }, BOARD_GRID_SIZE);
    //}
    //TArray<FStoneData> BoardStat;

    //UGoSoccerPlayManager::AIPlay_ThrowLoadingScreen(WaitPlayer, true);

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
#if UE_BUILD_SHIPPING
	//Debug_GetPossibleSpawnDollLocation(true);
#endif
    double EndTime = FPlatformTime::Seconds();
    UE_LOG(LogTemp, Log, TEXT("Total Request_Calculate - Took : %.2f ms"), (EndTime - StartTime) * 1000.f);
}

void UGoSoccerAISubsystem::Calculate_Completed(int32 FlickDollIdx, FVector FlickDirection, FVector PlaceLocation, UObject* NextPlayer)
{
    UE_LOG(
        LogTemp, Log,
        TEXT("===== Calculate_Completed =====")
    );
    UE_LOG(
        LogTemp, Log,
        TEXT("Calculate_Completed : FlickDollIdx : %d, FlickDirection : %s, PlaceLocation : %s"),
        FlickDollIdx, *FlickDirection.ToString(), *PlaceLocation.ToString()
    );
    //UGoSoccerPlayManager::AIPlay_ThrowLoadingScreen(NextPlayer, false);
    if (PlacedDollArr.IsValidIndex(FlickDollIdx))
    {
        UGoSoccerPlayManager::AIPlay_FlickDoll(GetWorld()->GetAuthGameMode(), NextPlayer, PlacedDollArr[FlickDollIdx].Get(), FlickDirection);
    }
    else 
    {
        UGoSoccerPlayManager::AIPlay_SpawnDollAtLocation(GetWorld()->GetAuthGameMode(), NextPlayer, PlaceLocation);
    }
}

void UGoSoccerAISubsystem::Initialize_GridArr(const TArray<TWeakObjectPtr<AActor>>& PlacedDollArrRef, FGoBoardConfig& GoBoardConfig)
{
    int32 temp_i = 0;
    for (const auto& Iter : PlacedDollArrRef)
    {
        if (!Iter.IsValid()) continue;
        AActor* IterDoll = Iter.Get();
        if (IterDoll == nullptr) continue;
        uint8 DollColor;
        if (!UGoSoccerPlayManager::GetDollColor(IterDoll, DollColor)) continue;
        int32 X;
        int32 Y;
        float StoneDist;
        FVector tempVector{ 0.f };
        bool bIsFallen = false;
        UGoSoccerPlayManager::GetDollFallFromBoard(IterDoll, bIsFallen);
        if (bIsFallen) continue;
        GetGridLocation(IterDoll->GetActorLocation(), X, Y, StoneDist, tempVector, GoBoardConfig);
        if (!IsInGrid(X, Y, GoBoardConfig)) continue;
        int32 E_idx = GoBoardConfig.BoardStat.Emplace(
            IterDoll->GetActorLocation(), DollColor,
            GoBoardConfig.BoardStat.Num(),
            X, Y, StoneDist, bIsFallen
        );
        TWeakObjectPtr<AActor> tempObjectPtr = IterDoll;
        PlacedDollArr.Emplace(tempObjectPtr);
        //Grid_Occupied[X][Y].UpdateCloseDollCounter(E_idx, true);
        GoBoardConfig.Grid_Occupied[X][Y].CloseDollContainer[E_idx] = true;
        GoBoardConfig.Grid_Occupied[X][Y].ContainDollColor |= (1ULL << DollColor);
        if (GoBoardConfig.Grid_Occupied[X][Y].ClosestStoneIdx < 0 ||
            (GoBoardConfig.Grid_Occupied[X][Y].ClosestStoneIdx >= 0 && GoBoardConfig.Grid_Occupied[X][Y].StoneDistance > StoneDist))
        {
            GoBoardConfig.Grid_Occupied[X][Y].ClosestStoneIdx = E_idx;
            GoBoardConfig.Grid_Occupied[X][Y].StoneDistance = StoneDist;
        }
        UE_LOG(LogTemp, Log, TEXT("PlacedDollArrRef Location [%d] : %s"), temp_i++, *IterDoll->GetActorLocation().ToString());
    }
}

void UGoSoccerAISubsystem::Union_Board(FGoBoardConfig& GoBoardConfig)
{
    UF_Visited.SetNum(1 + GoBoardConfig.BoardStat.Num() / 64);
    for (int32 i = 0; i < GoBoardConfig.BoardStat.Num(); ++i)
    {
        //FStoneData& Stone = GoBoardConfig.BoardStat[i];
        GoBoardConfig.UF_Accessible.Emplace(FStreakDollContainer{ GoBoardConfig.BoardStat.Num() / 64 + 1 });
        GoBoardConfig.UF_CloseEnemy.Emplace(FStreakDollContainer{ GoBoardConfig.BoardStat.Num() / 64 + 1 });
        //UF_Parent.Emplace(Stone.idx);
    }
    for (int32 i = 0; i < GoBoardConfig.BoardStat.Num(); ++i)
    {
        FStoneData& Stone = GoBoardConfig.BoardStat[i];
        UF_JoinDollIngoGroup(Stone, GoBoardConfig);
    }
}

bool UGoSoccerAISubsystem::IsDollInRange(FVector A, FVector B, float Cite_Threshold)
{
    FVector Diff = A - B;
    return FMath::Abs(Diff.X) <= Cite_Threshold && FMath::Abs(Diff.Y) <= Cite_Threshold;
}

void UGoSoccerAISubsystem::UF_JoinDollIngoGroup(FStoneData& InStoneData, FGoBoardConfig& GoBoardConfig)
{
	FVector NewLoc = InStoneData.Location;
	for (FStoneData& IterData : GoBoardConfig.BoardStat)
	{
		FVector ToLoc = IterData.Location;
        if (IterData.idx == InStoneData.idx) continue;
		if (!IsDollInRange(NewLoc, ToLoc, GoBoardConfig.Distance_Threshold * 2.f)) continue;

		//bool IsSameOwner = InStoneData.bIsBlack == IterData.bIsBlack;
		bool IsSameOwner = InStoneData.DollColor== IterData.DollColor;
		//if (!IsSameOwner) continue;

        if (IsSameOwner
            && GoBoardConfig.UF_Accessible.IsValidIndex(InStoneData.idx)
            && GoBoardConfig.UF_Accessible.IsValidIndex(IterData.idx))
        {
            //UF_Union(InStoneData.idx, IterData.idx);
            GoBoardConfig.UF_Accessible[InStoneData.idx][IterData.idx] = true;
            GoBoardConfig.UF_Accessible[IterData.idx][InStoneData.idx] = true;
        }
        else if (!IsSameOwner
            && GoBoardConfig.UF_CloseEnemy.IsValidIndex(InStoneData.idx)
            && GoBoardConfig.UF_CloseEnemy.IsValidIndex(IterData.idx))
        {
            GoBoardConfig.UF_CloseEnemy[InStoneData.idx][IterData.idx] = true;
            GoBoardConfig.UF_CloseEnemy[IterData.idx][InStoneData.idx] = true;
        }
	}
}
//
//void UGoSoccerAISubsystem::UF_OutDollIngoGroup(FStoneData& OutStoneData)
//{
//}
//
void UGoSoccerAISubsystem::UF_Union(int32 A, int32 B)
{
	int32 Root_A = UF_Find(A);
	int32 Root_B = UF_Find(B);

	if (Root_A < Root_B)
	{
		UF_Parent[Root_B] = Root_A;
	}
	else if (Root_A > Root_B)
	{
		UF_Parent[Root_A] = Root_B;
	}
}

int32 UGoSoccerAISubsystem::UF_Find(int32 A)
{
	if (!UF_Parent.IsValidIndex(A)) return INDEX_NONE;
	if (UF_Parent[A] == A) return A;
	return UF_Parent[A] = UF_Find(UF_Parent[A]);
}

bool UGoSoccerAISubsystem::IsInGrid(int32 X, int32 Y, const FGoBoardConfig& GoBoardConfig)
{
    return GoBoardConfig.Grid_Occupied.IsValidIndex(X) && GoBoardConfig.Grid_Occupied[X].IsValidIndex(Y);
}

//void UGoSoccerAISubsystem::Debug_GetPossibleSpawnDollLocation(bool bDebug)
//{
//    const int32 GridSize = 19;
//    const float Tolerance = 12.f;
//    const float BoardZ = -180.0f;
//
//    const float HalfExtent = (R - L) / 18.f / 8.f;
//
//    float StepX = (R - L) / (GridSize - 1);
//    float StepY = (D - U) / (GridSize - 1);
//
//    for (int32 X = 0; X < GridSize; ++X)
//    {
//        for (int32 Y = 0; Y < GridSize; ++Y)
//        {
//            FVector CenterLoc = FVector(L + X * StepX, U + Y * StepY, BoardZ);
//            int32 StoneIdx = Grid_Occupied[X][Y].ClosestStoneIdx;
//            bool bIsOccupied = BoardStat.IsValidIndex(StoneIdx);
//            
//            FVector P1 = CenterLoc + FVector(-HalfExtent, -HalfExtent, 0.f);
//            FVector P2 = CenterLoc + FVector(HalfExtent, -HalfExtent, 0.f);
//            FVector P3 = CenterLoc + FVector(HalfExtent, HalfExtent, 0.f);
//            FVector P4 = CenterLoc + FVector(-HalfExtent, HalfExtent, 0.f);
//
//            float Duration = 3.f;
//            bool bPersistentLines = false;
//            uint8 DepthPriority = 0;
//            float Thickness = 1.0f;
//            FString DistString;;
//            if (bIsOccupied) DistString += FString::Printf(TEXT("[ %d ]"), StoneIdx);
//            uint8 MaxDepth = 0;
//            for (const FGridStreakTrailContainer& Iter : Grid_Occupied[X][Y].StreakTrailArr)
//            {
//                if (Iter.StreakDepth > MaxDepth)
//                {
//                    MaxDepth = Iter.StreakDepth;
//                    if (MaxDepth > 3)
//                    {
//                        DistString += FString::Printf(TEXT("["));
//                        const FStreakDollContainer& ContIter = Iter.StreakTrailContainer;
//                        const TArray<uint64>& AccessiblesContainer = ContIter.GetData();
//                        int32 tempidx = 0;
//                        for (uint64 Accessibles : AccessiblesContainer)
//                        {
//                            while (Accessibles != 0)
//                            {
//                                int32 BitIdx = FMath::CountTrailingZeros64(Accessibles);
//                                int32 idx = tempidx * 64 + BitIdx;
//                                if (!(Accessibles & (1ULL << BitIdx))) continue;
//                                DistString += FString::Printf(TEXT("%d "), idx);
//                                Accessibles &= ~(1ULL << BitIdx);
//                            }
//                            tempidx++;
//                        }
//                        DistString += FString::Printf(TEXT("]"));
//                    }
//                }
//            }
//            DistString += FString::Printf(TEXT(" M : %d"), MaxDepth);
//            if (bDebug && Grid_Occupied[X][Y].Score > 0.f)
//            {
//                DistString += FString::Printf(TEXT(" %.1f"), Grid_Occupied[X][Y].Score);
//                DrawDebugString(GetWorld(), CenterLoc - FVector(0.f, 0.f, 0.f), DistString, nullptr, MaxDepth > 3 ? FColor::Magenta : FColor::Cyan, Duration, true);
//            }
//            if (bIsOccupied && Grid_Occupied[X][Y].StoneDistance > 7.5f) bIsOccupied = false;
//
//            if (bDebug)
//            {
//                //FColor LineColor = bIsOccupied ? FColor::Red : FColor::Green;
//                float s = Grid_Occupied[X][Y].Score;
//                FColor LineColor = bIsOccupied ? FColor::Red : FColor(255.f - s, 255.f, 255.f - s);
//                DrawDebugLine(GetWorld(), P1, P2, LineColor, bPersistentLines, Duration, DepthPriority, Thickness);
//                DrawDebugLine(GetWorld(), P2, P3, LineColor, bPersistentLines, Duration, DepthPriority, Thickness);
//                DrawDebugLine(GetWorld(), P3, P4, LineColor, bPersistentLines, Duration, DepthPriority, Thickness);
//                DrawDebugLine(GetWorld(), P4, P1, LineColor, bPersistentLines, Duration, DepthPriority, Thickness);
//                //float HeightMultiplier = FMath::Clamp(FMath::LogX(10.0f, s + 1.0f) / 3.7f, 0.0f, 0.99f);
//                float HeightMultiplier = FMath::Clamp(FMath::Pow(s / 5000.0f, 0.3f) * 0.99f, 0.0f, 0.99f);
//                float BarHeight = HeightMultiplier * 25.f;
//                FVector BottomPoint = CenterLoc;
//                FVector TopPoint = CenterLoc + FVector(0.f, 0.f, BarHeight);
//                DrawDebugLine(GetWorld(), BottomPoint, TopPoint, LineColor, bPersistentLines, Duration, DepthPriority, Thickness * 2.0f);
//            }
//        }
//    }
//}

void UGoSoccerAISubsystem::GetGridLocation(FVector ConvertLocation, int32& Out_X, int32& Out_Y, float& DistFromCenter, FVector& CenterLocation, const FGoBoardConfig& GoBoardConfig)
{
    Out_X = FMath::RoundToInt((ConvertLocation.X - GoBoardConfig.L) / GoBoardConfig.W);
    Out_Y = FMath::RoundToInt((ConvertLocation.Y - GoBoardConfig.D) / GoBoardConfig.W);
    CenterLocation = FVector(GoBoardConfig.L + Out_X * GoBoardConfig.W, GoBoardConfig.D + Out_Y * GoBoardConfig.W, ConvertLocation.Z);
    Out_Y = 18 - Out_Y;
    DistFromCenter = FVector::Dist2D(ConvertLocation, CenterLocation);
}

void UGoSoccerAISubsystem::Notify_PlaceLocation_Calculated(int32& FlickDollIndex, FVector& FlickDirection, FVector& PlaceLocation, FGoBoardConfig& GoBoardConfig)
{
    /*auto CountDollInWay_Approximate = [&](FVector2D StartGridPos, FVector2D DestGridPos, int32& Out_DollInWayCount)
        -> bool 
        {
            Out_DollInWayCount = 0;
            if (!IsInGrid(StartGridPos.X, StartGridPos.Y)) return false;
            if (!IsInGrid(DestGridPos.X, DestGridPos.Y)) return false;
            if (StartGridPos == DestGridPos) return false;

            FVector2D Direction = DestGridPos - StartGridPos;
            int32 StepX = Direction.X > 0 ? 1 : (Direction.X < 0 ? -1 : 0);
            int32 StepY = Direction.Y > 0 ? 1 : (Direction.Y < 0 ? -1 : 0);

            float DeltaX = StepX != 0 ? FMath::Abs(W / Direction.X) : TNumericLimits<float>::Max();
            float DeltaY = StepY != 0 ? FMath::Abs(W / Direction.Y) : TNumericLimits<float>::Max();

            float tX = StepX != 0 ? FMath::Abs((W / 2.f) / Direction.X) : TNumericLimits<float>::Max();
            float tY = StepY != 0 ? FMath::Abs((W / 2.f) / Direction.Y) : TNumericLimits<float>::Max();

            FIntPoint CurrPos{ (int32)StartGridPos.X, (int32)StartGridPos.Y };

            int32 MaxStep = FMath::Abs(DestGridPos.X - StartGridPos.X) + FMath::Abs(DestGridPos.Y - StartGridPos.Y);

            for (int32 i = 0; i < MaxStep; ++i)
            {
                if (tX < tY)
                {
                    tX += DeltaX;
                    CurrPos.X += StepX;
                }
                else
                {
                    tY += DeltaY;
                    CurrPos.Y += StepY;
                }
                
                if (!IsInGrid(CurrPos.X, CurrPos.Y)) break;
                Out_DollInWayCount += Grid_Occupied[CurrPos.X][CurrPos.Y].CloseDollContainer.GetCount();

                if (CurrPos.X == DestGridPos.X && CurrPos.Y == DestGridPos.Y) break;
            }
            
            return true;
        };*/
    auto CountDollInWay = [&](FVector2D StartGridPos, FVector2D DestGridPos, FVector StartDollLocation, FVector DestDollLocation, int32& Out_Count, FStreakDollContainer& VisitedDoll)
        -> void
        {
            Out_Count = 0;

            if (!IsInGrid(StartGridPos.X, StartGridPos.Y, GoBoardConfig)) return;
            if (!IsInGrid(DestGridPos.X, DestGridPos.Y, GoBoardConfig)) return;
            if (StartGridPos == DestGridPos) return;

            FVector2D Direction = DestGridPos - StartGridPos;
            int32 StepX = Direction.X > 0 ? 1 : (Direction.X < 0 ? -1 : 0);
            int32 StepY = Direction.Y > 0 ? 1 : (Direction.Y < 0 ? -1 : 0);

            float DeltaX = StepX != 0 ? FMath::Abs(GoBoardConfig.W / Direction.X) : TNumericLimits<float>::Max();
            float DeltaY = StepY != 0 ? FMath::Abs(GoBoardConfig.W / Direction.Y) : TNumericLimits<float>::Max();

            float tX = StepX != 0 ? FMath::Abs((GoBoardConfig.W / 2.f) / Direction.X) : TNumericLimits<float>::Max();
            float tY = StepY != 0 ? FMath::Abs((GoBoardConfig.W / 2.f) / Direction.Y) : TNumericLimits<float>::Max();

            FIntPoint CurrPos{ (int32)StartGridPos.X, (int32)StartGridPos.Y };
            int32 MaxStep = FMath::Abs(DestGridPos.X - StartGridPos.X) + FMath::Abs(DestGridPos.Y - StartGridPos.Y);

            for (int32 i = 0; i < MaxStep; ++i)
            {
                if (tX < tY)
                {
                    tX += DeltaX;
                    CurrPos.X += StepX;
                }
                else
                {
                    tY += DeltaY;
                    CurrPos.Y += StepY;
                }

                if (!IsInGrid(CurrPos.X, CurrPos.Y, GoBoardConfig)) break;
                if (GoBoardConfig.Grid_Occupied[CurrPos.X][CurrPos.Y].CloseDollContainer.GetCount() == 0)
                {
                    if (CurrPos.X == DestGridPos.X && CurrPos.Y == DestGridPos.Y) break;
                    continue;
                }

                const TArray<uint64>& DollChunks = GoBoardConfig.Grid_Occupied[CurrPos.X][CurrPos.Y].CloseDollContainer.GetData();
                for (auto Iter = DollChunks.CreateConstIterator(); Iter; ++Iter)
                {
                    uint64 Dolls = *Iter;
                    while (Dolls != 0)
                    {
                        int32 BitIdx = FMath::CountTrailingZeros64(Dolls);
                        int32 StoneIdx = Iter.GetIndex() * 64 + BitIdx;
                        if (!(Dolls & 1ULL << BitIdx)) continue;
                        Dolls &= ~(1ULL << BitIdx);

                        if (!VisitedDoll.IsValidIndex(StoneIdx) || VisitedDoll[StoneIdx]) continue;
                        VisitedDoll[StoneIdx] = true;

                        if (!GoBoardConfig.BoardStat.IsValidIndex(StoneIdx)) continue;
                        FVector TargetDollLoc = GoBoardConfig.BoardStat[StoneIdx].Location;
                        float DistanceToPath = FMath::PointDistToSegment(TargetDollLoc, StartDollLocation, DestDollLocation);
                        if (DistanceToPath < DOLL_FLICK_THRESHOLD)
                        {
                            Out_Count++;
                        }
                    }
                }

                if (CurrPos.X == DestGridPos.X && CurrPos.Y == DestGridPos.Y) break;
            }
        };
    FlickDollIndex = -1;
    TArray<FCandidate> PlacableLocations;
    TArray<FCandidate> FlickableTargets;
    TArray<FCandidate> UnrelatedGrids;
    //TArray<uint8> UF_Depths;
    //UF_Depths.SetNum(Cal_BoardStat.Num());
    uint8 DollCounter_AI = 0;
    uint8 DollCounter_Player = 0;
    for (int32 i = 0; i < GoBoardConfig.BoardStat.Num(); i++)
    {
        //UF_Depths[UF_Find(i)]++;
        if (GoBoardConfig.BoardStat[i].DollColor == GoBoardConfig.AIDollColor) // TODO Check Fallen
        {
            DollCounter_AI++;
        }
        else
        {
            DollCounter_Player++;
        }
    }
    PlacableLocations.Reserve(0);
    FlickableTargets.Reserve(0);
    float TotalScore = 0.f;
    uint8 Def_MaxStreak = 0;
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

    //bool SuperDangerShouldFlickSpotNow = FlickableTargets.Num() > 0
    //    && Def_MaxStreak > 3 // More Than Four Streak Is Surrounded With Doll ???
    //    && (PlacableLocations.Num() <= 0 || PlacableLocations.HeapTop().Score < 10000.f); // Cant Win With Done Doll
    //if (DollCounter_AI > 3 && Def_MaxStreak > 3) // Danger Attack First TODO Tip Test
    //{
    //    FVector2D DestGridPos = FlickableTargets.HeapTop().GridLocation;
    //    int32 GX = DestGridPos.X;
    //    int32 GY = DestGridPos.Y;
    //    if (IsInGrid(GX, GY) && UF_CloseEnemy.IsValidIndex(Grid_Occupied[GX][GY].ClosestStoneIdx))
    //    {
    //        int32 Min_Traced_Idx = -1;
    //        int32 Min_DollCount = TNumericLimits<int32>::Max();
    //        
    //        const TArray<uint64>& CloseDollContainer = UF_CloseEnemy[Grid_Occupied[GX][GY].ClosestStoneIdx].GetData();
    //        for (auto Iter = CloseDollContainer.CreateConstIterator(); Iter; ++Iter)
    //        {
    //            uint64 Closes = *Iter;
    //            while (Closes != 0)
    //            {
    //                int32 BitIdx = FMath::CountTrailingZeros64(Closes);
    //                int32 idx = Iter.GetIndex() * 64 + BitIdx;
    //                if (!(Closes & (1ULL << BitIdx))) continue;

    //                int32 DollCount = 0;
    //                if (Cal_BoardStat.IsValidIndex(idx)
    //                    && CountDollsInWay(Cal_BoardStat[idx].ClosestGirdPos, DestGridPos, DollCount))
    //                {
    //                    if (Min_DollCount > DollCount)
    //                    {
    //                        Min_DollCount = DollCount;
    //                        Min_Traced_Idx = idx;
    //                    }
    //                    if (Min_DollCount == 0) break;
    //                }
    //                Closes &= ~(1ULL << BitIdx);
    //            }
    //        }
    //        //CountDollsInWay
    //        // Do Flick
    //        if (Cal_BoardStat.IsValidIndex(Grid_Occupied[GX][GY].ClosestStoneIdx)
    //            && Cal_BoardStat.IsValidIndex(Min_Traced_Idx))
    //        {
    //            FVector FlickDirection = Cal_BoardStat[Grid_Occupied[GX][GY].ClosestStoneIdx].Location - Cal_BoardStat[Min_Traced_Idx].Location;
    //            DrawDebugDirectionalArrow(GetWorld(),
    //                Cal_BoardStat[Min_Traced_Idx].Location,
    //                Cal_BoardStat[Grid_Occupied[GX][GY].ClosestStoneIdx].Location,
    //                5.f, FColor::Green, false, 5.f
    //            );

    //        }
    //    }
    //}
    //else if (Def_MaxStreak > 2)
    //{
    //    if (PlacableLocations.Num() > 0 && PlacableLocations[0].Score < 1000.f) // Cant Make 4
    //    {
    //        // Do Flick Randomly
    //    }
    //}

}

//void UGoSoccerAISubsystem::EvaluateBoard_WriteStreakData(TArray<TArray<FGridData>>& Cal_Grid_Occupied, const TArray<FStoneData>& Cal_BoardStat, const float InRadian_Threshold, const float InDistance_Threshold, const FGoBoardConfig& GoBoardConfig)
void UGoSoccerAISubsystem::EvaluateBoard_WriteStreakData(FGoBoardConfig& GoBoardConfig)
{
    //// Assert Union Completed
    FStreakDollContainer VisitedRootDoll{ GoBoardConfig.BoardStat.Num() };
//#if !UE_BUILD_SHIPPING
//    for (int32 i = 0; i < Cal_BoardStat.Num(); i++)
//    {
//        FString BitString = TEXT("");
//        for (int32 j = 0; j < Cal_BoardStat.Num(); j++)
//        {
//            bool bIsAccessible = GoBoardConfig.UF_Accessible[i][j];
//
//            BitString += bIsAccessible ? TEXT("1") : TEXT("0");
//            if (bIsAccessible)
//            {
//#if UE_BUILD_SHIPPING
//                DrawDebugLine(
//                    GetWorld(),
//                    Cal_BoardStat[i].Location + FVector(0.f, 0.f, 5.f),
//                    Cal_BoardStat[j].Location + FVector(0.f, 0.f, 5.f),
//                    FColor::Green, false, 5.f
//                );
//#endif
//            }
//        }
//        UE_LOG(LogTemp, Log, TEXT("Row %d: %s"), i, *BitString);
//    }
//#endif
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

//void UGoSoccerAISubsystem::EvaluateBoard_GridStreakTrailsToScore(
//    const TArray<FStoneData>& Cal_BoardStat, TArray<TArray<FGridData>>& Cal_Grid_Occupied, //const TArray<uint8>& UF_Depths,
//    TArray<FCandidate>& PlacableLocations, TArray<FCandidate>& FlickableTargets, TArray<FCandidate>& UnrelatedGrids,
//    float& TotalScore, uint8& Def_MaxStreak, bool bDebug
//)
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
                //if (bDebug)
                //{
                //    FString DebugString;
                //    DebugString += FString::Printf(TEXT("Trail : ["));
                //    const FStreakDollContainer& ContIter = Iter.StreakTrailContainer;
                //    const TArray<uint64>& AccessiblesContainer = ContIter.GetData();
                //    int32 tempidx = 0;
                //    for (uint64 Accessibles : AccessiblesContainer)
                //    {
                //        while (Accessibles != 0)
                //        {
                //            int32 BitIdx = FMath::CountTrailingZeros64(Accessibles);
                //            int32 idx = tempidx * 64 + BitIdx;
                //            if (!(Accessibles & (1ULL << BitIdx))) continue;
                //            DebugString += FString::Printf(TEXT("%d "), idx);
                //            Accessibles &= ~(1ULL << BitIdx);
                //        }
                //        tempidx++;
                //    }
                //    DebugString += FString::Printf(TEXT("]"));
                //    //UE_LOG(
                //    //    LogTemp, Log,
                //    //    TEXT("%d : %s, score : %f"),
                //    //    Grid_Occupied[X][Y].ClosestStoneIdx, *DebugString, Grid_Occupied[X][Y].Score
                //    //);
                //    UE_LOG(
                //        LogTemp, Log,
                //        TEXT("%d : %s, Def.S : %f"),
                //        Grid_Occupied[X][Y].ClosestStoneIdx, *DebugString, Grid_Occupied[X][Y].DefenceScore
                //    );
                //}
            }
            //Def_MaxStreak = FMath::Max(tempDef_MaxStreak, Def_MaxStreak);
            //if (Streak3OpenPlaceCount > 1) // Two 3++ Streak With 0 ~ 1 Blocked ! ! ! Must Be Flicked Unless Winable
            //{
            //    Grid_Occupied[X][Y].DefenceScore += FMath::Pow(PLAYER_STREAK_SCORE_WEIGHT * 1.3f, Def_MaxStreak) * (AccessibleCount / 4.f) * Streak3OpenPlaceCount;
            //}

            //if (tempDef_MaxStreak > 2 && Grid_Occupied[X][Y].DefenceScore > 0.f)  // Both Estimate Tip Location Is Unabled To Place && Should Be More Than 3 Doll
            //{
            //    FCandidate tempFlickableTarget{ FVector2D(X, Y), Grid_Occupied[X][Y].DefenceScore };
            //    FlickableTargets.HeapPush(
            //        tempFlickableTarget,
            //        [](const FCandidate& A, const FCandidate& B) {
            //            return A.Score > B.Score;
            //        }
            //    );
            //}
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

//FGridStreakTrailContainer UGoSoccerAISubsystem::Travel_Accessibles_StreakContainer(
//    int32 DollIdx, TArray<TArray<FGridData>>& Cal_Grid_Occupied,
//    const TArray<FStoneData>& Cal_BoardStat, FStreakDollContainer& Visited,
//    int32 Depth, const float TowardsRadian, const float InRadian_Threshold,
//    const float InDistance_Threshold, FVector2D& JumpedGridPos, const FGoBoardConfig& GoBoardConfig)
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
