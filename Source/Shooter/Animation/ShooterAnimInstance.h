#pragma once

#include "Animation/AnimInstance.h"
#include "Items/ShooterItemType.h"
#include "ShooterAnimInstance.generated.h"

/** forward declarations */
class AShooterPlayerCharacter;

UENUM(BlueprintType)
enum class EShooterOffsetState : uint8
{
	EOS_Aiming UMETA(DisplayName = "Aiming"),
	EOS_Hip UMETA(DisplayName = "Hip"),
	EOS_Reloading UMETA(DisplayName = "Reloading"),
	EOS_InAir UMETA(DisplayName = "InAir"),

	EOS_MAX UMETA(DisplayName = "DefaultMax")
};


UCLASS()
class UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UShooterAnimInstance();

	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	virtual void NativeBeginPlay() override;
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUninitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	/** Handle turning in place variables */
	void TurnInPlace();

	/** Handle calculations for leaning while running */
	void Lean(float DeltaSeconds);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Movement, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<AShooterPlayerCharacter> ShooterCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw;

	/** offset yaw the frame before we stoped moving */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	/** Yaw of the Character this frame; Only updated when standing still and not in air */
	float TIPCharacterYaw;

	/** Yaw of Character the previous frame; Only updated when standing still and not in air */
	float TIPCharacterYawLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in Place", meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;

	/** Rotation curve value this frame */
	float RotationCurve;

	/** Rotation curve value last frame */
	float RotationCurveLastFrame;

	/** The pitch of the aim rotation, use for Aim Offset */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in Place", meta = (AllowPrivateAccess = "true"))
	float Pitch;

	/** True when reloading, use to prevent Aim Offset while reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in Place", meta = (AllowPrivateAccess = "true"))
	bool bReloading;

	/** Offset state; use to determine which Aim Offset to use */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in Place", meta = (AllowPrivateAccess = "true"))
	EShooterOffsetState OffsetState;

	/** Character Rotation this frame */
	FRotator CharacterRotation;

	/** Character Rotation Last frame */
	FRotator CharacterRotationLastFrame;

	/** Yaw delta used for leaning in the running blendspace */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Lean, meta = (AllowPrivateAccess = "true"))
	float YawDelta;

	/** True when crouching */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Crouching, meta = (AllowPrivateAccess = "true"))
	bool bCrouching;

	/** True when equipping */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Equipping, meta = (AllowPrivateAccess = "true"))
	bool bEquipping;

	/** Change the recoil weight based on turning in place and aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float RecoilWeight;

	/** True when turning in place */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bTurningInPlace;

	/** Weapon type for the cuttently equipped weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	EShooterWeaponType EquippedWeaponType;

	/** True when not reloading or equipping */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bShouldUseFABRIK;
};