#include "ShooterPlayerState.h"
#include "ShooterInventoryComponent.h"
#include "ShooterStatComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ShooterPlayerState)

AShooterPlayerState::AShooterPlayerState()
{
	InventoryComponent = CreateDefaultSubobject<UShooterInventoryComponent>(TEXT("InventoryComponent"));
	StatComponent = CreateDefaultSubobject<UShooterStatComponent>(TEXT("StatComponent"));
}

