#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

/** forward declarations */
class UUserWidget;

UCLASS()
class AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AShooterPlayerController();

protected:
	virtual void BeginPlay() override;

private:
	/** Reference to the Overall HUD Overlay Blueprint class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> HudOverlayClass;

	/** Variable to hold the HUD Overlay Widget after creating it */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UUserWidget> HudOverlay;
};