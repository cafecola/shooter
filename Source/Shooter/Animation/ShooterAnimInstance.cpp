#include "ShooterAnimInstance.h"
#include "Character/ShooterCharacter.h"
#include "Character/ShooterPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Items/ShooterWeapon.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ShooterAnimInstance)

UShooterAnimInstance::UShooterAnimInstance()
	:Speed(0.f)
	,bIsInAir(false)
	,bIsAcceleration(false)
	,MovementOffsetYaw(0.f)
	,LastMovementOffsetYaw(0.f)
	,bAiming(false)
	,TIPCharacterYaw(0.f)
	,TIPCharacterYawLastFrame(0.f)
	,RootYawOffset(0.f)
	,Pitch(0.f)
	,bReloading(false)
	,OffsetState(EShooterOffsetState::EOS_Hip)
	,CharacterRotation(FRotator(0.f))
	,CharacterRotationLastFrame(FRotator(0.f))
	,YawDelta(0.f)
	,RecoilWeight(1.f)
	, bTurningInPlace(false)
	,EquippedWeaponType(EShooterWeaponType::EWT_MAX)
	,bShouldUseFABRIK(false)
{
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (false == ::IsValid(ShooterCharacter))
	{
		ShooterCharacter = Cast<AShooterPlayerCharacter>(TryGetPawnOwner());
	}

	if (::IsValid(ShooterCharacter))
	{
		bCrouching = ShooterCharacter->GetCrouching();

		bReloading = ShooterCharacter->GetCombatState() == EShooterCombatState::ECS_Reloading;

		bEquipping = ShooterCharacter->GetCombatState() == EShooterCombatState::ECS_Equipping;

		bShouldUseFABRIK = ShooterCharacter->GetCombatState() == EShooterCombatState::ECS_Unoccupied || ShooterCharacter->GetCombatState() == EShooterCombatState::ECS_FireTimerInProgrress;

		FVector Velocity = ShooterCharacter->GetVelocity();
		Velocity.Z = 0;
		Speed = Velocity.Size();

		if (UCharacterMovementComponent* MovementComponent = ShooterCharacter->GetCharacterMovement())
		{
			bIsInAir = MovementComponent->IsFalling();

			if (MovementComponent->GetCurrentAcceleration().Size() > KINDA_SMALL_NUMBER)
			{
				bIsAcceleration = true;
			}
			else
			{
				bIsAcceleration = false;
			}

			FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
			FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
			MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

			if (ShooterCharacter->GetVelocity().Size() > 0.f)
			{
				LastMovementOffsetYaw = MovementOffsetYaw;
			}

			bAiming = ShooterCharacter->GetAiming();
			
			/*
			FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation : %f"), AimRotation.Yaw);
			FString MovementRotationMessage = FString::Printf(TEXT("Movement Rotation : %f"), MovementRotation.Yaw);
			FString MovementOffsetYawMessage = FString::Printf(TEXT("MovementOffsetYaw : %f"), MovementOffsetYaw);
			if (::IsValid(GEngine))
			{
				GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, MovementOffsetYawMessage);
			}*/
		}

		// if else : (bReloading = true, bIsInAir = true) EOS_Reloading
		// switch : (bReloading = true,  bIsInAir = true ) EOS_InAir
		if (bReloading)
		{
			OffsetState = EShooterOffsetState::EOS_Reloading;
		}
		else if (bIsInAir)
		{
			OffsetState = EShooterOffsetState::EOS_InAir;
		}
		else if (ShooterCharacter->GetAiming())
		{
			OffsetState = EShooterOffsetState::EOS_Aiming;
		}
		else
		{
			OffsetState = EShooterOffsetState::EOS_Hip;
		}

		// Check if ShooterPlayerCharacter has a valid EquippedWeapon
		if (AShooterWeapon* EquippedWeapon = ShooterCharacter->GetEquippedWeapon())
		{
			EquippedWeaponType = EquippedWeapon->GetWeaponType();
		}
	}
	TurnInPlace();
	Lean(DeltaTime);
}

void UShooterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ShooterCharacter = Cast<AShooterPlayerCharacter>(TryGetPawnOwner());

	if (::IsValid(ShooterCharacter))
	{
		RootYawOffset = 0.f;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		RotationCurveLastFrame = 0.f;
		RotationCurve = 0.f;
	}
}

void UShooterAnimInstance::NativeUninitializeAnimation()
{
	Super::NativeUninitializeAnimation();
}

void UShooterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

UE_DISABLE_OPTIMIZATION
void UShooterAnimInstance::TurnInPlace()
{
	if (false == ::IsValid(ShooterCharacter)) return;

	Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;

	if (Speed > 0.f || bIsInAir)
	{
		// Don't want to turn in place; Character is moving
		RootYawOffset = 0.f;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		RotationCurveLastFrame = 0.f;
		RotationCurve = 0.f;
	}
	else
	{
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		const float TIPYawDelta = TIPCharacterYaw - TIPCharacterYawLastFrame;

		// Root Yaw Offset, update and clamped to [-180, 180]
		// (== RootYawOffset -= YawDelta)
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);

		// 1.0 if turning, 0.0 if not
		const float Turning = GetCurveValue(TEXT("Turning"));
		if (Turning > 0)
		{
			bTurningInPlace = true;

			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));
			const float DeltaRotation = RotationCurve - RotationCurveLastFrame;

			// RootYawOffset > 0, -> Turning Left. RootYawOffset < 0, -> Turning Right.
			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

			const float ABSRootYawOffset = FMath::Abs(RootYawOffset);
			if (ABSRootYawOffset > 90.f)
			{
				const float YawExcess = ABSRootYawOffset - 90.f;
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
		else
		{
			bTurningInPlace = false;
		}
	}

	// Set the Recoil Weight
	if (bTurningInPlace)
	{
		if (bReloading || bEquipping)
		{
			RecoilWeight = 1.f;
		}
		else
		{
			RecoilWeight = 0.f;
		}

	}
	else // not turning in place
	{
		if (bCrouching)
		{
			if (bReloading || bEquipping)
			{
				RecoilWeight = 1.f;
			}
			else
			{
				RecoilWeight = 0.1f;
			}
		}
		else
		{
			if (bAiming || bReloading || bEquipping)
			{
				RecoilWeight = 1.f;
			}
			else
			{
				RecoilWeight = 0.5f;
			}
		}
	}
}
void UShooterAnimInstance::Lean(float DeltaSeconds)
{
	if (false == ::IsValid(ShooterCharacter)) return;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShooterCharacter->GetActorRotation();
	
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);

	const float Target = Delta.Yaw / DeltaSeconds;

	const float Interp = FMath::FInterpTo(YawDelta, Target, DeltaSeconds, 6.f);

	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);
}
UE_ENABLE_OPTIMIZATION