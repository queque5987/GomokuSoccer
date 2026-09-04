#include "GameFramework/GoSoccerViewportClient.h"
#include "Interface/IGoSoccerPlayerController.h"

bool UGoSoccerViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	bool rtn = Super::InputKey(EventArgs);

	return rtn;
}