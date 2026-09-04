#include "GoSoccerGameState.h"
#include "Engine/Texture2D.h"
#include "Net/UnrealNetwork.h"
#include "GoSoccerPlayManager.h"
#include "Kismet/GameplayStatics.h"

//constexpr float DILATION_SCALE_HIT = 0.65f;
//constexpr float DILATION_SCALE_BROKEN = 0.25f;
constexpr float DILATION_MAX_TIME = 0.125f;
constexpr float DILATION_TIMER_RATE = 0.016667;
constexpr float DILATION_INTERP_EXP = 12.f;

void AGoSoccerGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGoSoccerGameState, SteamIDArr);
}

bool AGoSoccerGameState::PostSteamID_Implementation(UObject* PlayerControllerObject, int64 RawSteamID)
{
	if (!HasAuthority())
	{
		return false;
	}
	uint64 uRawSteamID = static_cast<uint64>(RawSteamID);
	// 재접속 / Load 재시도로 같은 ID 가 여러 번 올라올 수 있다.
	SteamIDArr.AddUnique(uRawSteamID);

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(
			TEXT("Steam ID Posted To GameState"));
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, LogString);

		LogString = FString::Printf(
			TEXT("===== Current Steam ID Posted GameState ====="));
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, LogString);
		uint32 idx = 0;
		for (const uint64& Iter : SteamIDArr)
		{
			LogString = FString::Printf(
				TEXT("%d = %d"), idx++, Iter
			);
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, LogString);
		}
	}
#endif

	if (HasAuthority())
	{
		OnRep_SteamIdArr();
	}
	return true;
}

bool AGoSoccerGameState::GetSteamAvatar_Implementation(FUniqueNetIdRepl& FindNetIdRef, UObject*& OutAvatarTexture2DObject)
{
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(
			TEXT("GetSteamAvatar : %s : Map Num : %d"),
			FindNetIdRef != nullptr ? TEXT("NetID Valid") : TEXT("NetID InValid"),
			Map_SteamAvatar.Num()
		);
		//GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, LogString);
	}
#endif
	if (FindNetIdRef.IsValid())
	{
		uint64 NetId_StamId = *((uint64*)FindNetIdRef->GetBytes());
		if (Map_SteamAvatar.Contains(NetId_StamId))
		{
			if (UTexture2D** FoundTexture = Map_SteamAvatar.Find(NetId_StamId))
			{
				OutAvatarTexture2DObject = *FoundTexture;
#if !UE_BUILD_SHIPPING
				if (GEngine)
				{
					FString LogString = FString::Printf(
						TEXT("Returning Texture : %s"),
						*(*FoundTexture)->GetFName().ToString());
					GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, LogString);
				}
#endif
				return true;
			}
		}
	}
	return false;
}

bool AGoSoccerGameState::RequestSteamAvatar_Implementation(int64 RawSteamID, const FOnSteamAvatarLoaded& OnAvatarLoaded)
{
	if (RawSteamID <= 0) return false;
	const uint64 uRawSteamID = static_cast<uint64>(RawSteamID);

	SteamIDArr.AddUnique(uRawSteamID);

	// 이미 받아둔 아바타가 있으면 바로 돌려준다.
	if (UTexture2D** FoundTexture = Map_SteamAvatar.Find(uRawSteamID))
	{
		if (*FoundTexture != nullptr)
		{
			OnAvatarLoaded.ExecuteIfBound(*FoundTexture);
			return true;
		}
	}

	// 로드가 끝나면 실행되도록 대기열에 넣는다.
	Map_PendingAvatarRequest.FindOrAdd(uRawSteamID).Emplace(OnAvatarLoaded);

	if (!SteamAPI_Init() || SteamFriends() == nullptr)
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(
				TEXT("RequestSteamAvatar : Steam API Load Failed"));
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, LogString);
		}
#endif
		return false;
	}

	CSteamID SteamIDIter{ uRawSteamID };
	int ImageHandle = SteamFriends()->GetMediumFriendAvatar(SteamIDIter);

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(
			TEXT("RequestSteamAvatar : %llu, ImageHandle : %d"), uRawSteamID, ImageHandle);
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, LogString);
	}
#endif

	if (ImageHandle > 0)
	{
		UTexture2D* AvatarTexture = GetSteamAvatarAsTexture2D(ImageHandle);
		Map_SteamAvatar.Add(uRawSteamID, AvatarTexture);
		FlushPendingAvatarRequest(uRawSteamID, AvatarTexture);
	}
	// ImageHandle 이 0 이면 Steam 이 비동기로 받아오는 중 -> AvatarImageLoaded_t 콜백에서 처리된다.
	return true;
}

void AGoSoccerGameState::FlushPendingAvatarRequest(uint64 RawSteamID, UTexture2D* AvatarTexture)
{
	TArray<FOnSteamAvatarLoaded> PendingRequestArr;
	if (!Map_PendingAvatarRequest.RemoveAndCopyValue(RawSteamID, PendingRequestArr)) return;

	for (const FOnSteamAvatarLoaded& PendingRequestIter : PendingRequestArr)
	{
		PendingRequestIter.ExecuteIfBound(AvatarTexture);
	}
}

bool AGoSoccerGameState::SetGlobalTimeDilation_Implementation(float InDilationScale)
{
	if (HasAuthority())
	{
		if (CurrentDilationScale > InDilationScale)
		{
			DilationTimeLapse = 0.f;
			CurrentDilationScale = InDilationScale;
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_GlobalDilation);
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle_GlobalDilation, FTimerDelegate::CreateLambda([&, InDilationScale]
					{
						DilationTimeLapse += 1 / 60.f;
						CurrentDilationScale = FMath::InterpEaseIn(InDilationScale, 1.f, DilationTimeLapse / DILATION_MAX_TIME, DILATION_INTERP_EXP);
						if (DilationTimeLapse < DILATION_MAX_TIME)
						{
							UGameplayStatics::SetGlobalTimeDilation(GetWorld(), CurrentDilationScale);
						}
						else if (CurrentDilationScale >= 1.f || DilationTimeLapse >= DILATION_MAX_TIME)
						{
							UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
							GetWorld()->GetTimerManager().ClearTimer(TimerHandle_GlobalDilation);
						}
						//OnRep_CurrentDilationScale();
					}
				), 1 / 60.f, true
			);
		}
	}
	return true;
}

bool AGoSoccerGameState::Flicked_InvertCylinderCharge_Implementation(int32 InFlickCylinderInvert)
{
	auto PCIter = GetWorld()->GetPlayerControllerIterator();
	for (; PCIter; ++PCIter)
	{
		APlayerController* PC = PCIter->Get();
		if (PC == nullptr) continue;
		UGoSoccerPlayManager::Flicked_InvertCylinderCharge_SetInvertParam(PC, InFlickCylinderInvert);
	}
	return true;
}

bool AGoSoccerGameState::Flicked_Locking_Implementation(bool bLocked, UObject* CalledBy)
{
	auto PCIter = GetWorld()->GetPlayerControllerIterator();
	for (; PCIter; ++PCIter)
	{
		UObject* PC = PCIter->Get();
		if (PC == nullptr || CalledBy == PC) continue;
		UGoSoccerPlayManager::Flicked_Locking_SetBoolParam(PC, bLocked);
	}
	return true;
}

bool AGoSoccerGameState::Placed_NextDollType_Received_Implementation(const EDollType InNextDollType)
{
	auto PCIter = GetWorld()->GetPlayerControllerIterator();
	for (; PCIter; ++PCIter)
	{
		UObject* PC = PCIter->Get();
		if (PC == nullptr) continue;
		UGoSoccerPlayManager::Placed_NextDollType_Received_SetDollType(PC, InNextDollType);
	}
	return true;
}

void AGoSoccerGameState::OnRep_SteamIdArr()
{
#if UE_EDITOR
	UE_LOG(LogTemp, Log, TEXT("OnRep_SteamIdArr"));
#endif
	Client_OnRep_SteamIDArr();
}

void AGoSoccerGameState::Client_OnRep_SteamIDArr_Implementation()
{
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(
			TEXT("OnRep_SteamIdArr"));
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, LogString);
	}
#endif
	if (!SteamAPI_Init())
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			FString LogString = FString::Printf(
				TEXT("Steam API Load Failed"));
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, LogString);
		}
#endif
		return;
	}

	for (const uint64 RawSteamIDIter : SteamIDArr)
	{
		UTexture2D** SteamAvatar = Map_SteamAvatar.Find(RawSteamIDIter);
		if (SteamAvatar != nullptr) continue;

		CSteamID SteamIDIter{ RawSteamIDIter };
		if (SteamFriends())
		{
			int ImageHandle = SteamFriends()->GetMediumFriendAvatar(SteamIDIter);

#if !UE_BUILD_SHIPPING
			if (GEngine)
			{
				FString LogString = FString::Printf(
					TEXT("Avatar Requested"));
				GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, LogString);
			}
#endif

			if (ImageHandle > 0)
			{
				Map_SteamAvatar.Add(RawSteamIDIter, GetSteamAvatarAsTexture2D(ImageHandle));

#if !UE_BUILD_SHIPPING
				if (GEngine)
				{
					FString LogString = FString::Printf(
						TEXT("Avatar Immediate Update %d"), Map_SteamAvatar.Num());
					GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Blue, LogString);
				}
#endif
			}

		}
		else
		{
#if !UE_BUILD_SHIPPING
			if (GEngine)
			{
				FString LogString = FString::Printf(
					TEXT("SteamFriends Load FAiled"));
				GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, LogString);
			}
#endif
		}
	}
}

void AGoSoccerGameState::OnAvatarImageLoaded(AvatarImageLoaded_t* CallbackPtr)
{
	uint64 LoadedRawSteamID = CallbackPtr->m_steamID.ConvertToUint64();
	int LoadedImageHandle = CallbackPtr->m_iImage;
	if (LoadedImageHandle > 0)
	{
		auto* tempTexture = GetSteamAvatarAsTexture2D(LoadedImageHandle);
		Map_SteamAvatar.Add(LoadedRawSteamID, tempTexture);
		FlushPendingAvatarRequest(LoadedRawSteamID, tempTexture);
	}

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(
			TEXT("AvatarImageLoaded_t Callback, ImageHandle : %d"),
			LoadedImageHandle
		);
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Blue, LogString);
	}
#endif
}

UTexture2D* AGoSoccerGameState::GetSteamAvatarAsTexture2D(int ImageHandle)
{
	uint32 Width, Height;
	if (!SteamUtils()->GetImageSize(ImageHandle, &Width, &Height)) return nullptr;

	TArray<uint8> RawData;
	RawData.AddUninitialized(Width * Height * 4);
	if (SteamUtils()->GetImageRGBA(ImageHandle, RawData.GetData(), RawData.Num()))
	{
		for (int32 i = 0; i < RawData.Num(); i += 4)
		{
			uint8 TempR = RawData[i];
			RawData[i] = RawData[i + 2];
			RawData[i + 2] = TempR;
		}
		UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height);
		if (Texture)
		{
			void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());
			Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
			Texture->UpdateResource();
#if !UE_BUILD_SHIPPING
			if (GEngine)
			{
				FString LogString = FString::Printf(
					TEXT("Avatar Load Succeded, Texture : %s"),
					Texture != nullptr? TEXT("ValidPtr") : TEXT("Nullptr")
				);
				GEngine->AddOnScreenDebugMessage(-1, 6.f, Texture != nullptr ? FColor::Green : FColor::Red, LogString);
			}
#endif
			if (Texture != nullptr)
			{
				if (GetWorld())
				{
#if !UE_BUILD_SHIPPING
					if (GEngine)
					{
						FString LogString = FString::Printf(
							TEXT("Refreshing Player Card : %s"),
							*Texture->GetFName().ToString()
						);
						GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, LogString);
					}
#endif
					APlayerController* tempPC = GetWorld()->GetFirstPlayerController();
					UGoSoccerPlayManager::RefreshPlayerCardListView(tempPC);
				}
				else
				{
#if !UE_BUILD_SHIPPING
					if (GEngine)
					{
						FString LogString = FString::Printf(
							TEXT("World Not Found")
						);
						GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, LogString);
					}
#endif
				}
			}
			return Texture;
		}
	}
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(
			TEXT("Avatar Load Failed"));
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, LogString);
	}
#endif
	return nullptr;
}

