#include "ShooterPlayerController.h"
#include "Blueprint/UserWidget.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ShooterPlayerController)

AShooterPlayerController::AShooterPlayerController()
{
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HudOverlayClass)
	{
		HudOverlay = CreateWidget<UUserWidget>(this, HudOverlayClass);
		if (HudOverlay)
		{
			HudOverlay->AddToViewport();
			HudOverlay->SetVisibility(ESlateVisibility::Visible);

		}
	}
	FInputModeGameOnly GameOnlyInputMode;
	SetInputMode(GameOnlyInputMode);
}
