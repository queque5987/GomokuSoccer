#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Misc/DateTime.h"
#include "ChattingBoxData.generated.h"

UCLASS(BlueprintType)
class GOSOCCER_API UChattingBoxData : public UObject
{
	GENERATED_BODY()
	
	UPROPERTY()
	FName PlayerName;
	UPROPERTY()
	FText TextMessage;
	UPROPERTY()
	FDateTime TimeStamp;
public:
	UFUNCTION(BlueprintCallable)
	void GetPlayerName(FName& OutPlayerName) { OutPlayerName = PlayerName; };
	UFUNCTION(BlueprintCallable)
	void GetTextMessage(FText& OutTextMessage) { OutTextMessage = TextMessage; };
	UFUNCTION(BlueprintCallable)
	void GetTimeStamp(FDateTime& OutDateTime) { OutDateTime = TimeStamp; };
	UFUNCTION(BlueprintCallable)
	void GetTimeStampAsString(FString& OutString) { OutString = TimeStamp.ToString(TEXT("%H : %M")); };

	void SetPlayerName(const FName& InPlayerName) { PlayerName = InPlayerName; };
	void SetTextMessage(const FText& InTextMessage) { TextMessage = InTextMessage; };
	void SetTimeStamp(const FDateTime InDateTime) { TimeStamp = InDateTime; };
};
