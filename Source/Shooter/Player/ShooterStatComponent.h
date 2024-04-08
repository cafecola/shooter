#pragma once

#include "CoreMinimal.h"
#include "UObject/UObjectGlobals.h"
#include "Components/ActorComponent.h"
#include "ShooterStatComponent.generated.h"

UCLASS()
class UShooterStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShooterStatComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};