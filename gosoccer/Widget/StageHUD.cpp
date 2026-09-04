#include "Widget/StageHUD.h"
#include "Components/Image.h"
#include "Components/ListView.h"
#include "Components/TileView.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/EditableTextBox.h"
#include "Widget/Data/PlayerCardData.h"
#include "Widget/Data/TemporalMessageData.h"
#include "Widget/Data/ChattingBoxData.h"
#include "Misc/DateTime.h"

void UStageHUD::NativeConstruct()
{
	Super::NativeConstruct();
	if (Button_GameEnd_Regame) Button_GameEnd_Regame->OnReleased.AddDynamic(this, &UStageHUD::DEBUG_OnDebug);
	if (Button_GameEnd_OutSession)Button_GameEnd_OutSession->OnReleased.AddDynamic(this, &UStageHUD::DEBUG_OnDebug);

	
}

UObject* UStageHUD::CreateChattingBoxDataAsObject_Implementation(const FName& SendPlayerName, const FText& TextMessage, bool bPushToChat)
{
	UChattingBoxData* ChattBoxData = NewObject<UChattingBoxData>(GetGameInstance());

	if (ChattBoxData)
	{
		ChattBoxData->SetPlayerName(SendPlayerName);
		ChattBoxData->SetTextMessage(TextMessage);
		ChattBoxData->SetTimeStamp(FDateTime::Now());
		if (bPushToChat && ListView_ChattingBoxes)
		{
			ListView_ChattingBoxes->AddItem(ChattBoxData);
			ListView_ChattingBoxes->ScrollToBottom();
		}
	}
	return ChattBoxData;
}

void UStageHUD::DEBUG_OnDebug()
{
#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Log, TEXT("DEBUG_OnDebug"));
#endif
}

void UStageHUD::CallBack_SwitchFingerMode(uint8 SwitchMode)
{
	FingerMode = SwitchMode;

	switch (FingerMode)
	{
	case(0):
		FingerModePreview->SetBrushFromTexture(PutDollTexture);
		break;
	case(1):
		FingerModePreview->SetBrushFromTexture(FlickDollTexture);
		break;
	default:
		break;
	}
}

bool UStageHUD::AddPlayerCard(const FPlayerCard& AddPlayerCardStruct)
{
	UPlayerCardData* PlayerCardItemData = NewObject<UPlayerCardData>(GetOwningLocalPlayer());
	if (PlayerCardItemData == nullptr) return false;
	PlayerCardItemData->SetPlayerCard(AddPlayerCardStruct);
	if (ListView_PlayerCards == nullptr) return false;
	ListView_PlayerCards->AddItem(PlayerCardItemData);
	return true;
}

bool UStageHUD::AddTemporalMessage(const FString& ThrowMessage, ETemporalMessageType TemporalMessageType, float FloatTime)
{
	if (TileView_TemporalMessages == nullptr) return false;
	UTemporalMessageData* InData = NewObject<UTemporalMessageData>(GetOwningLocalPlayer());
	if (InData == nullptr) return false;

	if (TileView_TemporalMessages->GetListItems().Num() > 10)
	{
		TileView_TemporalMessages->ClearListItems();
	}
	InData->SetThrowMessage(ThrowMessage);
	InData->SetTemporalMessageType(TemporalMessageType);
	InData->SetFloatTime(FloatTime);
	TileView_TemporalMessages->AddItem(InData);
	if (TemporalMessage_ManualDestroyCounter < FloatTime)
	{
		TemporalMessage_ManualDestroyCounter = FloatTime * 1.1f;
	}
	CheckTemporalMessageIter();
	return TileView_TemporalMessages == nullptr;
}

bool UStageHUD::ResetCursorPlayerCards()
{
	if (ListView_PlayerCards == nullptr) return false;
	ListView_PlayerCards->ClearListItems();
	return true;
}

//bool UStageHUD::RefreshPlayerCardListView(const TArray<FPlayerCard>& NewPlayerCardArr)
//{
//	if (ListView_PlayerCards == nullptr) return false;
//	ListView_PlayerCards->ClearListItems();
//	UE_LOG(LogTemp, Log, TEXT("RefreshPlayerCardListView - %s : %s : %d"),
//		*GetOwningPlayer()->GetFName().ToString(),
//		(GetWorld()->GetNetMode() == ENetMode::NM_ListenServer ? TEXT("Listen Server") : TEXT("Client")),
//		NewPlayerCardArr.Num()
//	);
//	for (const auto& Iter : NewPlayerCardArr)
//	{
//		UE_LOG(LogTemp, Log, TEXT("UStageHUD : RefreshPlayerCardListView : %s"), (Iter.OwningPlayerControllerObject != nullptr ? *Iter.OwningPlayerControllerObject->GetFName().ToString() : TEXT("Nullptr")));
//		UPlayerCardData* PlayerCardItemData = NewObject<UPlayerCardData>(GetOwningLocalPlayer());
//		if (PlayerCardItemData != nullptr)
//		{
//			PlayerCardItemData->SetPlayerCard(Iter);
//			ListView_PlayerCards->AddItem(PlayerCardItemData);
//		}
//		if (GEngine)
//		{
//			FString LogString = FString::Printf(TEXT("RefreshPlayerCardListView Iter : %s"), Iter.OwningPlayerControllerObject != nullptr ? *Iter.OwningPlayerControllerObject->GetFName().ToString() : *Iter.PlayerName.ToString());
//			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, LogString);
//		}
//	}
//	return true;
//}
//
//bool UStageHUD::RefreshPlayerCardListView(const FPlayerCard& NewPlayerCard)
//{
//	if (ListView_PlayerCards == nullptr) return false;
//	for (const auto& Iter : ListView_PlayerCards->GetListItems())
//	{
//		UPlayerCardData* tempPlayerCardData = Cast<UPlayerCardData>(Iter);
//		if (tempPlayerCardData == nullptr) continue;
//		FPlayerCard SneakPickCard;
//		tempPlayerCardData->GetPlayerCard(SneakPickCard);
//		if (SneakPickCard.OwningPlayerControllerObject != nullptr && SneakPickCard.OwningPlayerControllerObject == NewPlayerCard.OwningPlayerControllerObject)
//		{
//			tempPlayerCardData->SetPlayerCard(NewPlayerCard);
//			ListView_PlayerCards->RequestRefresh();
//			return true;
//		}
//	}
//	return false;
//}

void UStageHUD::CheckTemporalMessageIter()
{
	float DeltaTime = 1.f;
	if (!GetTimerManager_HUD() || !TileView_TemporalMessages) return;
	TemporalMessage_ManualDestroyCounter -= DeltaTime;
	if (TemporalMessage_ManualDestroyCounter > 0.f)
	{
		GetTimerManager_HUD()->SetTimer(TemporalMessageTimerHandle, this, &UStageHUD::CheckTemporalMessageIter, DeltaTime, false);
	}
	else
	{
		TileView_TemporalMessages->ClearListItems();
		GetTimerManager_HUD()->ClearTimer(TemporalMessageTimerHandle);
	}

//	const TArray<UObject*>& ItemArr = TileView_TemporalMessages->GetListItems();
//	TArray<UObject*> DelArr;
//	for (auto* Iter : ItemArr)
//	{
//		UTemporalMessageData* Data_Iter = Cast<UTemporalMessageData>(Iter);
//		if (Data_Iter == nullptr) continue;
//		float OutFloat = 1.f;
//		Data_Iter->GetFloatTime(OutFloat);
//		if (OutFloat > 0.f) continue;
//		DelArr.Emplace(Data_Iter);
//	}
//	if (DelArr.Num() == ItemArr.Num())
//	{
//		for (auto* Iter : DelArr)
//		{
//			if (Iter == nullptr) continue;
//			TileView_TemporalMessages->RemoveItem(Iter);
//		}
//	}
//#if !UE_BUILD_SHIPPING
//	UE_LOG(LogTemp, Log, TEXT("CheckTemporalMessageIter : Left Widgets - %d"), ItemArr.Num());
//#endif
//	if (ItemArr.Num() > 0)
//	{
//		if (TemporalMessage_ManualDestroyCounter < 0)
//		{
//			TileView_TemporalMessages->ClearListItems();
//		}
//		TemporalMessage_ManualDestroyCounter -= 1;
//		GetTimerManager_HUD()->SetTimer(TemporalMessageTimerHandle, this, &UStageHUD::CheckTemporalMessageIter, DeltaTime, false);
//	}
}

FTimerManager* UStageHUD::GetTimerManager_HUD()
{
	if (TimerManager != nullptr) return TimerManager;
	if (GetWorld() == nullptr) return nullptr;
	TimerManager = &(GetWorld()->GetTimerManager());
	return TimerManager;
}
