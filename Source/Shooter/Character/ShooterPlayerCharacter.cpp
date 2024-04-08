#include "ShooterPlayerCharacter.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "DrawDebugHelpers.h"
#include "Input/ShooterInputConfig.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/ShooterItem.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Items/ShooterWeapon.h"
#include "Items/ShooterAmmo.h"
#include "Player/ShooterInventoryComponent.h"
#include "Player/ShooterPlayerState.h"
#include "Shooter.h"
#include "Interfaces/BulletHitInterface.h"
#include "Character/ShooterEnemy.h"
#include "AI/EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ShooterPlayerCharacter)

AShooterPlayerCharacter::AShooterPlayerCharacter() : 
	// Base rates for turning/looking up
	BaseTurnRate(45.f)
	,BaseLookupRate(45.f)
	// Turn rates for aiming/not aiming
	,HipTurnRate(90.f)
	,HipLookUpRate(90.f)
	,AimingTurnRate(20.f)
	,AimingLookUpRate(20.f)
	// Mouse look sensitivity scale factors
	,MouseHipTurnRate(1.0f)
	,MouseHipLookUpRate(1.0f)
	,MouseAimingTurnRate(0.6f)
	,MouseAimingLookUpRate(0.6f)
	// true whe aimg the weapon
	,bAming(false)
	// Camera field of view values
	,CameraDefaultFOV(0.f)	// set in BeginPlay
	,CameraZoomedFOV(25.f)
	,CameraCurrentFOV(0.f)
	,CameraTagetFOV(0.f)
	,ZoomInterpSpeed(20.f)
	// Crosshair spread factors
	,CrosshairSpredMultiplier(0.f)
	,CrosshairVelocityFactor(0.f)
	,CrosshairInAirFactor(0.f)
	,CrosshairAimFactor(0.f)
	,CrosshairShootingFactor(0.f)
	// Bullet fire timer variables
	, bFiringBullet(false)
	// Automatic fire variables
	, bFireButtonPressed(false)
	, bShouldFire(true)
	// Item trace variables
	,bShouldTraceForItems(false)
	// Camera interp location variables
	,CameraInterpDistance(250.f)
	,CameraInterpElevation(65.f)
	// Starting ammo amount
	,Starting9mmAmmo(85)
	,StartingARAmmo(120)
	// Combat variables
	,CombatState(EShooterCombatState::ECS_Unoccupied)
	,bCrouching(false)
	,BaseMovementSpeed(650.f)
	,CrouchMovementSpeed(300.f)
	,StandingCapsuleHalfHeight(88.f)
	,CrouchingCapsuleHalfHeight(44.f)
	,BaseGroundFriction(2.f)
	,CrouchingGroundFriction(100.f)
	,bAimingButtonPressed(false)
	// Pickup sound timer properties
	,bShouldPlayPickupSound(true)
	,bShouldPlayEquipSound(true)
	,PickupSoundResetTime(0.2f)
	,EquipSoundResetTime(0.2f)
	// Invetory Icon animation property
	,HighlightedSlot(-1)
	,Health(100.f)
	,MaxHealth(100.f)
	,StunChance(.25f)
	,DefaultCameraBoomSocketOffset(FVector::ZeroVector) // set in BeginPlay
	,CrouchingCameraBoomSocketOffset(FVector(0.f, 60.f, 45.f))
{
	// Create Hand Scene Component
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComponent"));

	WeaponInterpComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Component"));
	WeaponInterpComponent->SetupAttachment(GetFollowCamera());

	InterpComponent1 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 1"));
	InterpComponent1->SetupAttachment(GetFollowCamera());

	InterpComponent2 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 2"));
	InterpComponent2->SetupAttachment(GetFollowCamera());

	InterpComponent3 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 3"));
	InterpComponent3->SetupAttachment(GetFollowCamera());

	InterpComponent4 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 4"));
	InterpComponent4->SetupAttachment(GetFollowCamera());

	InterpComponent5 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 5"));
	InterpComponent5->SetupAttachment(GetFollowCamera());

	InterpComponent6 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 6"));
	InterpComponent6->SetupAttachment(GetFollowCamera());
}

void AShooterPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		// Add input mapping context
		UEnhancedInputLocalPlayerSubsystem* SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (::IsValid(SubSystem))
		{
			SubSystem->AddMappingContext(InputMappingContext, 0);
		}
	}

	// Default FOV
	if (::IsValid(FollowCamera))
	{
		CameraDefaultFOV = FollowCamera->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
		CameraTagetFOV = CameraDefaultFOV;
	}

	// Default CameraBoomSocketOffset
	if (::IsValid(CameraBoom))
	{
		DefaultCameraBoomSocketOffset = CameraBoom->SocketOffset;
	}

	// Spawn the default weapon and equip it
	EquipWeapon(SpawnDefaultWeapon());

	if (AShooterPlayerState* PS = Cast<AShooterPlayerState>(GetPlayerState()))
	{
		if (UShooterInventoryComponent* Inven = PS->GetInventory())
		{
			Inven->AddItem(EquippedWeapon);
			EquippedWeapon->SetSlotIndex(0);
		}
	}
	
	EquippedWeapon->DisableCustomDepth();
	EquippedWeapon->DisableGlowMaterial();
	EquippedWeapon->SetCharacter(this);

	InitializeAmmoMap();

	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

	// Create FShooterInterpLocation structs for each interp location. add to array
	InitializeInterpLocations();

}

void AShooterPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(InputConfig->MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(InputConfig->MouseLookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		EnhancedInputComponent->BindAction(InputConfig->KeyboardLookAction, ETriggerEvent::Triggered, this, &ThisClass::LookAtRate);
		EnhancedInputComponent->BindAction(InputConfig->JumpAction, ETriggerEvent::Started, this, &ThisClass::Jump);
		EnhancedInputComponent->BindAction(InputConfig->JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(InputConfig->FireAction, ETriggerEvent::Started, this, &ThisClass::FireButtonPressed);
		EnhancedInputComponent->BindAction(InputConfig->FireAction, ETriggerEvent::Completed, this, &ThisClass::FireButtonReleased);
		EnhancedInputComponent->BindAction(InputConfig->AimingAction, ETriggerEvent::Started, this, &ThisClass::AimingButtonPressed);
		EnhancedInputComponent->BindAction(InputConfig->AimingAction, ETriggerEvent::Completed, this, &ThisClass::AimingButtonReleased);
		EnhancedInputComponent->BindAction(InputConfig->SelectAction, ETriggerEvent::Started, this, &ThisClass::SelectButtonPressed);
		EnhancedInputComponent->BindAction(InputConfig->SelectAction, ETriggerEvent::Completed, this, &ThisClass::SelectButtonReleased);
		EnhancedInputComponent->BindAction(InputConfig->ReloadAction, ETriggerEvent::Started, this, &ThisClass::ReloadButtonPressed);
		EnhancedInputComponent->BindAction(InputConfig->CrouchAction, ETriggerEvent::Started, this, &ThisClass::CrouchButtonPressed);

		EnhancedInputComponent->BindAction(InputConfig->FKeyAction, ETriggerEvent::Started, this, &ThisClass::FKeyPressed);
		EnhancedInputComponent->BindAction(InputConfig->OneKeyAction, ETriggerEvent::Started, this, &ThisClass::OneKeyPressed);
		EnhancedInputComponent->BindAction(InputConfig->TwoKeyAction, ETriggerEvent::Started, this, &ThisClass::TwoKeyPressed);
		EnhancedInputComponent->BindAction(InputConfig->ThreeKeyAction, ETriggerEvent::Started, this, &ThisClass::ThreeKeyPressed);
		EnhancedInputComponent->BindAction(InputConfig->FourKeyAction, ETriggerEvent::Started, this, &ThisClass::FourKeyPressed);
		EnhancedInputComponent->BindAction(InputConfig->FiveKeyAction, ETriggerEvent::Started, this, &ThisClass::FiveKeyPressed);
	}
}

void AShooterPlayerCharacter::StopJumping()
{
	Super::StopJumping();
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("StopJumping!!!!!")));
}

void AShooterPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Handle interpolation for zoom when aiming
	CameraInterpZoom(DeltaSeconds);

	// Calculate crosshair spread multiplier
	CalculateCrosshairSpread(DeltaSeconds);

	// Check OverlappedItemCount, then trace for items
	TraceForItems();

	// Interpolate the capsule half height based on crouching/stading
	InterpCapsuleHalfHeight(DeltaSeconds);
}

float AShooterPlayerCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float FinalDamageAmount = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	UE_LOG(LogTemp, Warning, TEXT("TakeDamage!!!"));

	if (Health <= 0.f)
	{
		auto EnemyController = Cast<AEnemyController>(EventInstigator);
		if (EnemyController)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerDead!!!"));
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("PlayerDead"), true);
		}
		return FinalDamageAmount;
	}

	if (Health - FinalDamageAmount <= 0.f)
	{
		Health = 0.f;
		Die();

		auto EnemyController = Cast<AEnemyController>(EventInstigator);
		if (EnemyController)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerDead!!!"));
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("PlayerDead"), true);
		}
	}
	else
	{
		Health -= FinalDamageAmount;
	}

	return FinalDamageAmount;
}

void AShooterPlayerCharacter::Move(const FInputActionValue& InValue)
{
	FVector2D InputVector = InValue.Get<FVector2D>();

	check(GetController());

	const FRotator ControlRotation = GetController()->GetControlRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, InputVector.X);
	AddMovementInput(RightDirection, InputVector.Y);
}

void AShooterPlayerCharacter::Look(const FInputActionValue& InValue)
{
	FVector2D LookAxisVector = InValue.Get<FVector2D>();

	check(GetController());

	float TurnScaleFactor{};
	float LookUpScaleFactor{};
	if (bAming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
		LookUpScaleFactor = MouseHipLookUpRate;
	}

	AddControllerYawInput(LookAxisVector.X * TurnScaleFactor);
	AddControllerPitchInput(LookAxisVector.Y * LookUpScaleFactor);
}

void AShooterPlayerCharacter::LookAtRate(const FInputActionValue& InValue)
{
	FVector2D LookAxisVector = InValue.Get<FVector2D>();
	check(GetController());

	// Change look sensitivity base on aiming
	SetLookRates();

	// calculate delta for this frame the rate infomation
	AddControllerYawInput(LookAxisVector.X * BaseTurnRate * GetWorld()->GetDeltaSeconds());		// (deg/sec) * (sec/frame) = deg/frame
	AddControllerPitchInput(LookAxisVector.Y * BaseLookupRate * GetWorld()->GetDeltaSeconds());	// (deg/sec) * (sec/frame) = deg/frame
}

void AShooterPlayerCharacter::PlayFireSound()
{
	if (false == ::IsValid(EquippedWeapon)) return;

	// Play fire sound
	if (USoundCue* FireSound = EquippedWeapon->GetFireSound())
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void AShooterPlayerCharacter::SendBullet()
{
	if (false == ::IsValid(EquippedWeapon)) return;

	// send bullet
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if (::IsValid(BarrelSocket))
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());
		if (UParticleSystem* MuzzleFlash = EquippedWeapon->GetMuzzleFlash())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FHitResult BeamHitResult;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamHitResult);
		if (bBeamEnd)
		{
			// Does hit Actor implement BulletHitInterface?
			if (BeamHitResult.GetActor())
			{
				IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(BeamHitResult.GetActor());
				if (BulletHitInterface)
				{
					//BulletHitInterface->BulletHit_Implementation(BeamHitResult, this, GetController());
					BulletHitInterface->Execute_BulletHit(BeamHitResult.GetActor(), BeamHitResult, this, GetController());
				}
				else
				{
					// default ImpactParticles
					if (::IsValid(ImpactParticles))
					{
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamHitResult.Location);
					}
				}

				AShooterEnemy* HitEnemy = Cast<AShooterEnemy>(BeamHitResult.GetActor());
				if (HitEnemy)
				{
					int32 Damage{};
					bool bHeadShot = false;
					if (BeamHitResult.BoneName.ToString() == HitEnemy->GetHeadBone())
					{
						// Head shot
						Damage = EquippedWeapon->GetHeadShotDamage();
						bHeadShot = true;
					}
					else
					{
						// Body shot
						Damage = EquippedWeapon->GetDamage();
						bHeadShot = false;
					}
					UE_LOG(LogTemp, Warning, TEXT("Hit Component: %s"), *BeamHitResult.BoneName.ToString());
					UGameplayStatics::ApplyDamage(HitEnemy, Damage, GetController(), this, UDamageType::StaticClass());
					HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location, bHeadShot);
				}
			}		
		}

		if (::IsValid(BeamParticles))
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if (::IsValid(Beam))
			{
				Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location);
			}
		}
	}
}

void AShooterPlayerCharacter::PlayGunFireMontage()
{
	// play Hip Fire Montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (::IsValid(AnimInstance) && ::IsValid(HipFireMontage))
	{
		AnimInstance->Montage_Play(HipFireMontage);
		//AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterPlayerCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterPlayerCharacter::ReloadWeapon()
{
	if (CombatState != EShooterCombatState::ECS_Unoccupied) return;

	if (false == ::IsValid(EquippedWeapon)) return;

	// Do we have ammo of the correct type?
	if (CarryingAmmo() && !EquippedWeapon->ClipIsFull())
	{
		if (bAming)
		{
			StopAiming();
		}

		CombatState = EShooterCombatState::ECS_Reloading;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (::IsValid(AnimInstance) && ::IsValid(ReloadMontage))
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
		}
	}
}

void AShooterPlayerCharacter::FinishReloading()
{
	if (CombatState == EShooterCombatState::ECS_Stunned) return;
	if (CombatState == EShooterCombatState::ECS_Dead) return;

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("FinishReloading!!")));

	// Update Combat State
	CombatState = EShooterCombatState::ECS_Unoccupied;

	if (bAimingButtonPressed)
	{
		Aim();
	}

	if (false == ::IsValid(EquippedWeapon)) return;

	const auto AmmoType{ EquippedWeapon->GetAmmoType() };

	// Update the AmmoMap
	if (AmmoMap.Contains(AmmoType))
	{
		// Amount of ammo the Character is carrying if the EquippedWeapon type
		int32 CarraiedAmmo = AmmoMap[AmmoType];

		// Space left in the magazine of EquipedWeapon
		const int32 MagEmptySpace = EquippedWeapon->GetMagizineCapacity() - EquippedWeapon->GetAmmo();

		if (MagEmptySpace > CarraiedAmmo)
		{
			// Reload the magazine with all the ammo we are carrying
			EquippedWeapon->ReloadAmmo(CarraiedAmmo);
			CarraiedAmmo = 0;
			AmmoMap.Add(AmmoType, CarraiedAmmo);
		}
		else
		{
			// fill the magazine
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarraiedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarraiedAmmo);
		}
	}

}

void AShooterPlayerCharacter::FinishEquipping()
{
	if (CombatState == EShooterCombatState::ECS_Stunned) return;
	if (CombatState == EShooterCombatState::ECS_Dead) return;

	CombatState = EShooterCombatState::ECS_Unoccupied;
	if (bAimingButtonPressed)
	{
		Aim();
	}
}

bool AShooterPlayerCharacter::CarryingAmmo()
{
	if (false == ::IsValid(EquippedWeapon)) return false;

	auto AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}

	return false;
}

void AShooterPlayerCharacter::GrabClip()
{
	if (false == ::IsValid(EquippedWeapon)) return;
	if (false == ::IsValid(HandSceneComponent)) return;

	// Index for the clip bone on the Equipped Weapon
	int32 ClipBoneIndex = EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName());
	// Store the transform of the clip
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("Hand_L")));
	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquippedWeapon->SetMovingClip(true);

}

void AShooterPlayerCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);
}

void AShooterPlayerCharacter::CrouchButtonPressed()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;
	}
	if (bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
		GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;
		CameraBoom->SocketOffset = CrouchingCameraBoomSocketOffset;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
		CameraBoom->SocketOffset = DefaultCameraBoomSocketOffset;
	}
}

void AShooterPlayerCharacter::Jump()
{
	if (bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
		CameraBoom->SocketOffset = DefaultCameraBoomSocketOffset;
	}
	else
	{
		Super::Jump();
	}
}

void AShooterPlayerCharacter::InterpCapsuleHalfHeight(float DeltaSeconds)
{
	float TargetCapsuleHalfHeight{};
	if (bCrouching)
	{
		TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight;
	}
	else
	{
		TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;
	}
	const float InterpHalfHeight = FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHeight, DeltaSeconds, 20.f);

	const float DeltaCapsuleHalfHeight = InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector MeshOffset{ 0.f, 0.f, -DeltaCapsuleHalfHeight };
	GetMesh()->AddLocalOffset(MeshOffset);

	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

UE_DISABLE_OPTIMIZATION
void AShooterPlayerCharacter::FireWeapon()
{
	if (false == ::IsValid(EquippedWeapon)) return;

	if (CombatState != EShooterCombatState::ECS_Unoccupied) return;

	if (WeaponHasAmmo())
	{
		PlayFireSound();
		SendBullet();
		PlayGunFireMontage();

		// Subtract 1 from the weapon's Ammo
		EquippedWeapon->DecrementAmmo();

		// Start bullet fire timer for crosshairs
		StartCrosshairBulletFire();

		StartFireTimer();

		if (EquippedWeapon->GetWeaponType() == EShooterWeaponType::EWT_Pistol)
		{
			// Start moving slide timer
			EquippedWeapon->StartSlideTimer();
		}
	}
}
UE_ENABLE_OPTIMIZATION

bool AShooterPlayerCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult)
{	
	FVector OutBeamLocation;

	// check for crosshair trace hit
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);
	if (bCrosshairHit)
	{
		// Tentative beam location - still need to trace from gun
		OutBeamLocation = CrosshairHitResult.Location;
	}
	else // no crosshair trace hit
	{
		// OutBeamLocation is the End Location for the line trace
	}

	const FVector WeaponTraceStart(MuzzleSocketLocation);
	const FVector StartToEnd(OutBeamLocation - MuzzleSocketLocation);
	const FVector WeaponTraceEnd(MuzzleSocketLocation + StartToEnd * 1.25f);
	GetWorld()->LineTraceSingleByChannel(OutHitResult, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
	if (!OutHitResult.bBlockingHit)
	{
		OutHitResult.Location = OutBeamLocation;
		return false;
	}

	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.f);
	//DrawDebugPoint(GetWorld(), OutHitResult.Location, 5.f, FColor::Red, false, 2.f);

	return true;
}

void AShooterPlayerCharacter::Aim()
{
	bAming = true;
	CameraTagetFOV = CameraZoomedFOV;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AShooterPlayerCharacter::StopAiming()
{
	bAming = false;
	CameraTagetFOV = CameraDefaultFOV;
	if (!bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
}

void AShooterPlayerCharacter::PickupAmmo(AShooterAmmo* Ammo)
{
	if (Ammo->GetItemCount() > 0)
	{
		if (AmmoMap.Find(Ammo->GetAmmoType()))
		{
			int32 AmmoCount = AmmoMap[Ammo->GetAmmoType()];
			AmmoCount += Ammo->GetItemCount();
			AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
		}
		else
		{
			AmmoMap.Add(Ammo->GetAmmoType(), Ammo->GetItemCount());
		}

		if (EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
		{
			if (EquippedWeapon->GetAmmo() == 0)
			{
				ReloadWeapon();
			}
		}
	}

	Ammo->Destroy();
}

void AShooterPlayerCharacter::InitializeInterpLocations()
{
	FShooterInterpLocation WeaponLocation{ WeaponInterpComponent, 0 };
	InterpLocations.Add(WeaponLocation);

	InterpLocations.Add(FShooterInterpLocation{ InterpComponent1, 0 });
	InterpLocations.Add(FShooterInterpLocation{ InterpComponent2, 0 });
	InterpLocations.Add(FShooterInterpLocation{ InterpComponent3, 0 });
	InterpLocations.Add(FShooterInterpLocation{ InterpComponent4, 0 });
	InterpLocations.Add(FShooterInterpLocation{ InterpComponent5, 0 });
	InterpLocations.Add(FShooterInterpLocation{ InterpComponent6, 0 });
}

void AShooterPlayerCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickupSound = true;
}

void AShooterPlayerCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}

void AShooterPlayerCharacter::FKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 0) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
}

void AShooterPlayerCharacter::OneKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 1) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);
}

void AShooterPlayerCharacter::TwoKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 2) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
}

void AShooterPlayerCharacter::ThreeKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 3) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);
}

void AShooterPlayerCharacter::FourKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 4) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);
}

void AShooterPlayerCharacter::FiveKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 5) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);
}

void AShooterPlayerCharacter::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
	if (UShooterInventoryComponent* Inven = GetInventory())
	{
		const bool bCanExchangeItems = (CurrentItemIndex != NewItemIndex) && 
			(NewItemIndex < Inven->Size()) && 
			(CombatState == EShooterCombatState::ECS_Unoccupied || CombatState == EShooterCombatState::ECS_Equipping);
		
		if (bCanExchangeItems)
		{
			if (bAming)
			{
				StopAiming();
			}
			auto OldEquipWeapon = EquippedWeapon;
			auto NewWeapon = Cast<AShooterWeapon>(Inven->GetItem(NewItemIndex));
			EquipWeapon(NewWeapon);

			OldEquipWeapon->SetItemState(EShooterItemState::EIS_PickedUp);
			NewWeapon->SetItemState(EShooterItemState::EIS_Equipped);

			CombatState = EShooterCombatState::ECS_Equipping;
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance && EquipMontage)
			{
				AnimInstance->Montage_Play(EquipMontage, 1.0f);
				//AnimInstance->Montage_JumpToSection(FName("Equip"));
			}
			NewWeapon->PlayEquipSound(true);
		}
	}
}

int32 AShooterPlayerCharacter::GetEmptyInventorySlot()
{
	if (UShooterInventoryComponent* Inven = GetInventory())
	{
		return Inven->GetEmptySlot();
	}
	return -1;
}

EPhysicalSurface AShooterPlayerCharacter::GetSurfaceType()
{
	FHitResult HitResult;
	const FVector Start = GetActorLocation();
	const FVector End = Start + FVector(0.f, 0.f, -400.f);
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;

	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);

	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
}

void AShooterPlayerCharacter::EndStun()
{
	CombatState = EShooterCombatState::ECS_Unoccupied;

	if (bAimingButtonPressed)
	{
		Aim();
	}
}

void AShooterPlayerCharacter::Die()
{
	CombatState = EShooterCombatState::ECS_Dead;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		DisableInput(PC);
	}
}

void AShooterPlayerCharacter::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
}

void AShooterPlayerCharacter::HighlightInventorySlot()
{
	const int32 EmptySlot = GetEmptyInventorySlot();
	if (HighlightIconDelegate.IsBound())
	{
		HighlightIconDelegate.Broadcast(EmptySlot, true);
	}
	HighlightedSlot = EmptySlot;
}

UE_DISABLE_OPTIMIZATION
void AShooterPlayerCharacter::UnHighlightInventorySlot()
{
	if (HighlightIconDelegate.IsBound())
	{
		HighlightIconDelegate.Broadcast(HighlightedSlot, false);
		HighlightedSlot = -1;
	}
}
UE_ENABLE_OPTIMIZATION

int32 AShooterPlayerCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;

	for (int32 i = 1; i < InterpLocations.Num(); i++)
	{
		if (InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
		}
	}

	return LowestIndex;
}

void AShooterPlayerCharacter::IncrementInterpLocationItemCount(int32 Index, int32 Amount)
{
	if (Amount < -1 || Amount > 1) return;

	if (InterpLocations.Num() > Index)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
}

void AShooterPlayerCharacter::AimingButtonPressed()
{
	bAimingButtonPressed = true;
	if (CombatState != EShooterCombatState::ECS_Reloading && CombatState != EShooterCombatState::ECS_Equipping && CombatState != EShooterCombatState::ECS_Stunned)
	{
		Aim();
	}
}

void AShooterPlayerCharacter::AimingButtonReleased()
{
	bAimingButtonPressed = false;
	StopAiming();
}

void AShooterPlayerCharacter::CameraInterpZoom(float DeltaSeconds)
{
	if (KINDA_SMALL_NUMBER < abs(CameraTagetFOV - CameraCurrentFOV))
	{
		// Interpolate to zoom in/out FOV		
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraTagetFOV, DeltaSeconds, ZoomInterpSpeed);
		GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
	}
}

void AShooterPlayerCharacter::SetLookRates()
{
	if (bAming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookupRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookupRate = HipLookUpRate;
	}
}

void AShooterPlayerCharacter::CalculateCrosshairSpread(float DeltaSeconds)
{
	FVector2D WalkSpeedRange{ 0.f, 600.f };
	FVector2D VelocityMultiplierRange{ 0.f, 1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0;

	// Calculate crosshair velocity factor
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	// Calculate crosshair in air factor
	if (GetCharacterMovement()->IsFalling()) // is in air?
	{
		// Spread the crosshair slowly while in air
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaSeconds, 2.25f);
	}
	else // Character in on the ground
	{
		// Shrink the crosshairs rapidly while on the ground
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaSeconds, 30.f);
	}

	// Calculate crosshair aim facter
	if (bAming) // Are we aiming?
	{
		// Shrink crosshair a small amount very quickly
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.6f, DeltaSeconds, 30.f);
	}
	else // Not aiming
	{
		// Spread crosshairs back to normal very quckly
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaSeconds, 30.f);
	}

	// True 0.05 second after firing
	if (bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaSeconds, 60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaSeconds, 60.f);
	}

	CrosshairSpredMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

void AShooterPlayerCharacter::StartCrosshairBulletFire()
{
	if (false == ::IsValid(EquippedWeapon)) return;
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &ThisClass::FinishCrosshairBulletFire, EquippedWeapon->GetCrosshairsShootTimeDuration());
}

void AShooterPlayerCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterPlayerCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	FireWeapon();
}

void AShooterPlayerCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterPlayerCharacter::StartFireTimer()
{
	if (false == ::IsValid(EquippedWeapon)) return;

	CombatState = EShooterCombatState::ECS_FireTimerInProgrress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &ThisClass::AutoFireReset, EquippedWeapon->GetAutoFireRate());
}

void AShooterPlayerCharacter::AutoFireReset()
{
	if (CombatState == EShooterCombatState::ECS_Stunned) return;
	if (CombatState == EShooterCombatState::ECS_Dead) return;

	CombatState = EShooterCombatState::ECS_Unoccupied;

	if (false == ::IsValid(EquippedWeapon)) return;

	if (WeaponHasAmmo())
	{
		if (bFireButtonPressed && EquippedWeapon->GetAutomatic())
		{
			FireWeapon();
		}
	}
	else
	{
		// Reload Weapon
		ReloadWeapon();
	}
}

bool AShooterPlayerCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	FVector2D ViewportSize;
	if (::IsValid(GEngine) && ::IsValid(GEngine->GameViewport))
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocaton(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocaton, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		const FVector Start(CrosshairWorldPosition);
		const FVector End(Start + CrosshairWorldDirection * 50'000.f);
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);
		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}

	return false;
}

void AShooterPlayerCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);
		if (ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AShooterItem>(ItemTraceResult.GetActor());

			const auto TraceHitWeapon = Cast<AShooterWeapon>(TraceHitItem);
			if (::IsValid(TraceHitWeapon))
			{
				if (HighlightedSlot == -1)
				{
					// Not currently highlighting a slot; hilight one
					HighlightInventorySlot();
				}
			}
			else
			{
				// Is a slot being highlight?
				if (HighlightedSlot != -1)
				{
					UnHighlightInventorySlot();
				}
			}

			if (::IsValid(TraceHitItem) && TraceHitItem->GetItemState() == EShooterItemState::EIS_EquipInterping)
			{
				TraceHitItem = nullptr;
			}

			if (::IsValid(TraceHitItem))
			{
				if (UWidgetComponent* PickupWidget = TraceHitItem->GetPickupWidget())
				{
					// Show Items's pickup Widget
					PickupWidget->SetVisibility(true);
					TraceHitItem->EnableCustomDepth();

					if (auto Inven = GetInventory())
					{
						if (!Inven->HasEmptySlot())
						{
							// Inventory is full
							TraceHitItem->SetCharacterInventoryFull(true);
						}
						else
						{
							TraceHitItem->SetCharacterInventoryFull(false);
						}
					}
				}
			}

			// We hit an AShooterItem last frame
			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)
				{
					// We ar hitting a different AShooterItem this frame from last frame
					// Or AShooterItem is null
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
					TraceHitItemLastFrame->DisableCustomDepth();
				}
			}

			// Store a reference to HitItem for next frame
			TraceHitItemLastFrame = TraceHitItem;
		}
	}
	else if (TraceHitItemLastFrame)
	{
		// No longer overlapping any items,
		// Item lat frame should not show widget
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		TraceHitItemLastFrame->DisableCustomDepth();
	}
}

AShooterWeapon* AShooterPlayerCharacter::SpawnDefaultWeapon()
{
	// Check the TsubclassOf variable
	if (::IsValid(DefaultWeaponClass))
	{
		// Spawn the weapon
		return GetWorld()->SpawnActor<AShooterWeapon>(DefaultWeaponClass);
	}
	return nullptr;
}

void AShooterPlayerCharacter::EquipWeapon(AShooterWeapon* WeaponToEquip, bool bSwapping)
{
	if (WeaponToEquip)
	{
		//Get the Hand Socket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			// Attach the Weapon to the hand socket RightHandSocket
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}

		if (EquippedWeapon == nullptr)
		{
			// -1 == no EquippedWeapon yet. No need to reverse the icon animation
			if (EquipItemDelegate.IsBound())
			{
				EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
			}
		}
		else if (!bSwapping)
		{
			if (EquipItemDelegate.IsBound())
			{
				EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
			}
		}

		// Set EquippedWeapon to the newly spawned weapon.
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EShooterItemState::EIS_Equipped);
	}
}

void AShooterPlayerCharacter::DropWeapon()
{
	if (EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);

		EquippedWeapon->SetItemState(EShooterItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AShooterPlayerCharacter::SelectButtonPressed()
{
	if (CombatState != EShooterCombatState::ECS_Unoccupied) return;
	if (TraceHitItem)
	{
		TraceHitItem->StartItemCurve(this, true);
		TraceHitItem = nullptr;
	}
}

void AShooterPlayerCharacter::SelectButtonReleased()
{
}

void AShooterPlayerCharacter::SwapWeapon(AShooterWeapon* WeaponToSwap)
{
	if (UShooterInventoryComponent* Inven = GetInventory())
	{
		if (Inven->Size() - 1 >= EquippedWeapon->GetSlotIndex())
		{
			Inven->SetItem(EquippedWeapon->GetSlotIndex(), WeaponToSwap);
			WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
		}
	}

	DropWeapon();
	EquipWeapon(WeaponToSwap, true);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

void AShooterPlayerCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EShooterAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EShooterAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterPlayerCharacter::WeaponHasAmmo()
{
	if (false == ::IsValid(EquippedWeapon)) return false;

	return EquippedWeapon->GetAmmo() > 0;
}

float AShooterPlayerCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpredMultiplier;
}

void AShooterPlayerCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

//FVector AShooterPlayerCharacter::GetCameraInterpLocation()
//{
//	const FVector CameraWorldLocation(FollowCamera->GetComponentLocation());
//	const FVector CameraForward(FollowCamera->GetForwardVector());
//	// Desired = CameraWorldLocation + CameraForward * A + Up * B
//	return CameraWorldLocation + CameraForward * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);
//}

void AShooterPlayerCharacter::GetPickupItem(AShooterItem* Item)
{
	if (false == ::IsValid(Item)) return;

	if (AShooterPlayerState* PS = Cast<AShooterPlayerState>(GetPlayerState()))
	{
		if (UShooterInventoryComponent* Inven = PS->GetInventory())
		{
			Item->PlayEquipSound();

			auto Weapon = Cast<AShooterWeapon>(Item);
			if (Weapon)
			{
				if (Inven->HasEmptySlot())
				{
					Weapon->SetSlotIndex(Inven->Size());
					Inven->AddItem(Weapon);
					Weapon->SetItemState(EShooterItemState::EIS_PickedUp);
					TraceHitItem = nullptr;
					TraceHitItemLastFrame = nullptr;
				}
				else // Inventory is full! Swap with EquippedWeapon
				{
					SwapWeapon(Weapon);
				}
			}

			auto Ammo = Cast<AShooterAmmo>(Item);
			if (Ammo)
			{
				PickupAmmo(Ammo);
			}
		}
	}
}

FShooterInterpLocation AShooterPlayerCharacter::GetInterpLocation(int32 Index)
{
	if (Index < InterpLocations.Num())
	{
		return InterpLocations[Index];
	}
	return FShooterInterpLocation();
}

void AShooterPlayerCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;
	GetWorldTimerManager().SetTimer(PickupSoundTimer, this, &ThisClass::ResetPickupSoundTimer, PickupSoundResetTime);
}

void AShooterPlayerCharacter::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;
	GetWorldTimerManager().SetTimer(EquipSoundTimer, this, &ThisClass::ResetEquipSoundTimer, EquipSoundResetTime);
}

UShooterInventoryComponent* AShooterPlayerCharacter::GetInventory()
{
	if (AShooterPlayerState* PS = Cast<AShooterPlayerState>(GetPlayerState()))
	{
		if (UShooterInventoryComponent* Inventory = PS->GetInventory())
		{
			return Inventory;
		}
	}
	return nullptr;
}

void AShooterPlayerCharacter::Stun()
{
	if (Health <= 0.f) return;

	CombatState = EShooterCombatState::ECS_Stunned;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
	}
}
