#include "Widget/PlayerIntiveCardWidget.h"
#include "Components/Button.h"
#include "Widget/Data/PlayerCardData.h"
#include "GameFramework/GoSoccerGameInstance.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "GoSoccerPlayManager.h"

//UObject* UPlayerIntiveCardWidget::GetMainWidget()
//{
//	if (MainWidget.IsValid() && MainWidget.Get() != nullptr)
//	{
//		return MainWidget;
//	}
//	UObject* tempMainWidget;
//	UGoSoccerPlayManager::GetMainWidgetAsObject(GetOwningPlayer(), tempMainWidget);
//	MainWidget = tempMainWidget;
//	return MainWidget.IsValid() ? MainWidget.Get() : nullptr;
//}

void UPlayerIntiveCardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button)
	{
		Button->OnReleased.RemoveDynamic(this, &UPlayerIntiveCardWidget::OnInviteButtonReleased);
		Button->OnReleased.AddDynamic(this, &UPlayerIntiveCardWidget::OnInviteButtonReleased);
	}
}

void UPlayerIntiveCardWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	auto* PlayerCardData = Cast<UPlayerCardData>(ListItemObject);
	if (PlayerCardData == nullptr)
	{
#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Warning, TEXT("PlayerIntiveCardWidget : ListItemObject is not a UPlayerCardData"));
#endif
		return;
	}
	PlayerCardData->GetSteamFriendData(SteamFirendData);
	DelegatePtr_OnWidgetSelected = PlayerCardData->GetOnWidgetSelected();
	if (Text_PlayerName)
	{
		Text_PlayerName->SetText(FText::FromString(SteamFirendData.PersonaName));
	}

	if (SteamFirendData.SteamID <= 0) return;
	if (GetWorld() == nullptr) return;

	OnSteamAvatarLoaded.BindUFunction(this, TEXT("Callback_SteamAvatarLoaded"));
	UGoSoccerPlayManager::RequestSteamAvatar(GetWorld()->GetGameState(), SteamFirendData.SteamID, OnSteamAvatarLoaded);

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("RequestSteamAvatar : %s"), *SteamFirendData.PersonaName);
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, LogString);
	}
#endif

	//if (OnWidgetSelected.IsBound()) OnWidgetSelected.Unbind();
	//UObject* tempMainWidget = nullptr;
	//UGoSoccerPlayManager::GetMainWidgetAsObject(GetOwningPlayer(), tempMainWidget);
	//OnWidgetSelected.BindUFunction(tempMainWidget, TEXT("OnInviteItemSelectionChanged"));
}

void UPlayerIntiveCardWidget::Callback_SteamAvatarLoaded(UObject* AvatarTexture2DObject)
{
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		FString LogString = FString::Printf(TEXT("Callback_SteamAvatarLoaded : %s"), *Text_PlayerName->GetText().ToString());
		GEngine->AddOnScreenDebugMessage(-1, 3.f, AvatarTexture2DObject != nullptr ? FColor::Green : FColor::Red, LogString);
	}
#endif
	if (AvatarTexture2DObject == nullptr) return;
	SetPlayerIcon(AvatarTexture2DObject);
}

bool UPlayerIntiveCardWidget::OnSelectedChanged_Implementation(bool bSelected)
{
	//OnChangeSelected(bSelected);
	return true;
}

bool UPlayerIntiveCardWidget::GetSteamFriendData(FSteamFriendData& OutSteamFriendData) const
{
	if (SteamFirendData.SteamID <= 0) return false;
	OutSteamFriendData = SteamFirendData;
	return true;
}

void UPlayerIntiveCardWidget::OnInviteButtonReleased()
{
	if (DelegatePtr_OnWidgetSelected != nullptr)
	{
		DelegatePtr_OnWidgetSelected->ExecuteIfBound(this, GetListItem<UObject>());
	}

	//if (!OnWidgetSelected.IsBound())
	//{
	//	UObject* tempMainWidget = nullptr;
	//	UGoSoccerPlayManager::GetMainWidgetAsObject(GetOwningPlayer(), tempMainWidget);
	//	OnWidgetSelected.BindUFunction(tempMainWidget, TEXT("OnInviteItemSelectionChanged"));
	//}

	//OnWidgetSelected.ExecuteIfBound(this);

//	FSteamFriendData SteamFriendData;
//	if (!GetSteamFriendData(SteamFriendData)) return;
//	if (SteamFriendData.SteamID <= 0) return;
//
//	const bool bResult = UGoSoccerPlayManager::Steam_InviteFriend(GetGameInstance(), SteamFriendData.SteamID);
//
//#if !UE_BUILD_SHIPPING
//	if (GEngine)
//	{
//		FString LogString = FString::Printf(TEXT("Invite %s (%lld) - %s"),
//			*SteamFriendData.PersonaName,
//			SteamFriendData.SteamID,
//			bResult ? TEXT("Success") : TEXT("Failed")
//		);
//		GEngine->AddOnScreenDebugMessage(-1, 3.f, bResult ? FColor::Green : FColor::Red, LogString);
//	}
//#endif
}
