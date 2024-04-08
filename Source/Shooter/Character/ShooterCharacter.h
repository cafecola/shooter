#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

/** forward declarations */
class USpringArmComponent;
class UCameraComponent;


UCLASS()
class AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AShooterCharacter();

	virtual void BeginPlay() override;
	
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	TObjectPtr<UCameraComponent> FollowCamera;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};