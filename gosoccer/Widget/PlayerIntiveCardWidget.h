#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Interface/IGoWidget.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "PlayerIntiveCardWidget.generated.h"

class UButton;
class UPlayerCardData;
class UTextBlock;
class UImage;

UCLASS()
class GOSOCCER_API UPlayerIntiveCardWidget : public UUserWidget, public IUserObjectListEntry, public IIGoWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_PlayerName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> Image_PlayerIcon;
	FOnSteamAvatarLoaded OnSteamAvatarLoaded;

private:
	UPROPERTY()
	FSteamFriendData SteamFirendData;

	//TWeakObjectPtr<UObject> MainWidget;
	//UObject* GetMainWidget();

	FOnWidgetSelected* DelegatePtr_OnWidgetSelected;
protected:
	virtual void NativeConstruct() override;

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	UFUNCTION()
	void OnInviteButtonReleased();

	UFUNCTION()
	void Callback_SteamAvatarLoaded(UObject* AvatarTexture2DObject);

public:
	virtual bool OnSelectedChanged_Implementation(bool bSelected) override;

	//UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	//void OnChangeSelected(bool bSelected);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetPlayerIcon(UObject* AvatarTexture2DObject);

	UFUNCTION(BlueprintCallable)
	bool GetSteamFriendData(FSteamFriendData& OutSteamFriendData) const;
};
