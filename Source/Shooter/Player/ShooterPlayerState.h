#pragma once

#include "GameFramework/PlayerState.h"
#include "ShooterPlayerState.generated.h"

/** forward declarations */
class UShooterStatComponent;
class UShooterInventoryComponent;
class AShooterItem;

UCLASS()
class AShooterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AShooterPlayerState();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UShooterInventoryComponent> InventoryComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Stat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UShooterStatComponent> StatComponent;

public:
	FORCEINLINE UShooterInventoryComponent* GetInventory() const { return InventoryComponent; }

};