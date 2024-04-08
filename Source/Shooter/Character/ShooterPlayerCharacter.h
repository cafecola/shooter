#pragma once

#include "ShooterCharacter.h"
#include "Items/ShooterAmmoType.h"
#include "ShooterPlayerCharacter.generated.h"

/** forward declarations */
class UShooterInputConfig;
class UInputMappingContext;
class USoundCue;
class UParticleSystem;
class UAnimMontage;
class AShooterItem;
class AShooterWeapon;
class AShooterAmmo;
class UShooterInventoryComponent;
struct FInputActionValue;

UENUM(BlueprintType)
enum class EShooterCombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_FireTimerInProgrress UMETA(DisplayName = "FireTimerInProgrress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_Equipping UMETA(DisplayName = "Equipping"),
	ECS_Stunned UMETA(DisplayName = "Stunned"),
	ECS_Dead UMETA(DisplayName = "Dead"),

	ECS_MAX UMETA(DisplayName = "DefaultMax")
};

USTRUCT(BlueprintType)
struct FShooterInterpLocation
{
	GENERATED_BODY()

	/** Scene component to use for it's location for interping */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SceneComponent;

	/** Number of items interping to/at this scene component location */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemCount;

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, CurrentSlotIndex, int32, NewSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHighlightIconDelegate, int32, SlotIndex, bool, bStartAnimation);

UCLASS()
class AShooterPlayerCharacter : public AShooterCharacter
{
	GENERATED_BODY()
public:
	AShooterPlayerCharacter();
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void StopJumping() override;
	virtual void Tick(float DeltaSeconds) override;

	// take combat damage
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	

protected:
	/** Called for forward/backword/left/right input */
	void Move(const FInputActionValue& InValue);

	void Look(const FInputActionValue& InValue);

	/**
	 * Called via input to look turn/up/down at a given rate.
	 * @param Rate	this is a normalized rate, i.e. 1.0 means 100% of desired rate
	 */
	void LookAtRate(const FInputActionValue& InValue);

	/** Called when the Fire Button is pressed */
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult);

	/** set bAiming to true or false with button press */
	void AimingButtonPressed();
	void AimingButtonReleased();

	void CameraInterpZoom(float DeltaSeconds);

	/** Set BaseTurnRate and BaseLookUpRate base on Aiming */
	void SetLookRates();

	void CalculateCrosshairSpread(float DeltaSeconds);

	void StartCrosshairBulletFire();

	UFUNCTION()
	void FinishCrosshairBulletFire();

	void FireButtonPressed();

	void FireButtonReleased();

	void StartFireTimer();

	UFUNCTION()
	void AutoFireReset();

	/** Line trace for items under the crosshairs */
	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);

	/** Trace for items if overlappedItemCount > 0 */
	void TraceForItems();

	/** Spawns a default weapon and equips it */
	AShooterWeapon* SpawnDefaultWeapon();

	/** Take a weapon and attaches it to the mesh */
	void EquipWeapon(AShooterWeapon* WeaponToEquip, bool bSwapping = false);

	/** Detach weapon and let it fall to the ground */
	void DropWeapon();

	void SelectButtonPressed();
	void SelectButtonReleased();

	/** Drops currently equipeed Weapon ad Equips TraceHitItem */
	void SwapWeapon(AShooterWeapon* WeaponToSwap);

	/** Initialize the Ammo Map with Ammo value */
	void InitializeAmmoMap();

	/** check to make sure our weapon has ammo */
	bool WeaponHasAmmo();

	/** fireWeapon functions */
	void PlayFireSound();
	void SendBullet();
	void PlayGunFireMontage();

	/** Bound to the R key and Gamepad Button Left */
	void ReloadButtonPressed();

	/** Handle reloading of the weapon */
	void ReloadWeapon();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	/** Checks to see if we have ammo of the EquippedWeapon's ammo type */
	bool CarryingAmmo();

	/** Called from Animation Blueprint with Grab Clip notify */
	UFUNCTION(BlueprintCallable)
	void GrabClip();

	/** Called from Animation Blueprint with Release Clip notify */
	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	void CrouchButtonPressed();

	virtual void Jump() override;

	/** Interps capsule half height when crouching/standing */
	void InterpCapsuleHalfHeight(float DeltaSeconds);

	void Aim();
	void StopAiming();

	void PickupAmmo(AShooterAmmo* Ammo);

	void InitializeInterpLocations();

	void ResetPickupSoundTimer();
	void ResetEquipSoundTimer();

	void FKeyPressed();
	void OneKeyPressed();
	void TwoKeyPressed();
	void ThreeKeyPressed();
	void FourKeyPressed();
	void FiveKeyPressed();

	void ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex);

	int32 GetEmptyInventorySlot();

	UFUNCTION(BlueprintCallable)
	EPhysicalSurface GetSurfaceType();

	UFUNCTION(BlueprintCallable)
	void EndStun();

	void Die();

	UFUNCTION(BlueprintCallable)
	void FinishDeath();

private:
	/** Base turn rate, in deg/sec. Other scaling my affect final turn rate */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	/** base look up/down rate, in deg/sec. Other scaling my affect final turn rate */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookupRate;

	/** Turn rate while not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Camera, meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;

	/** Look up rate whe not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Camera, meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	/** Turn rate when aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Camera, meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;

	/** Look up rate when aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Camera, meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	/** scale factor for mouse look sensitivity, Turn rate when not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;

	/** scale factor for mouse look sensitivity, Look up rate when not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	/** scale factor for mouse look sensitivity, Turn rate when aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;

	/** scale factor for mouse look sensitivity, Look up rate when aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;

	bool bFiringBullet;
	FTimerHandle CrosshairShootTimer;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UShooterInputConfig> InputConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> InputMappingContext;

	/** Particles spawned upon bullet impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> ImpactParticles;

	/** Smoke trail for bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> BeamParticles;

	/** Montage for firing the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> HipFireMontage;

	/** true when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Combat, meta = (AllowPrivateAccess = "true"))
	bool bAming;

	/** Default camera field of view value */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Combat, meta = (AllowPrivateAccess = "true"))
	float CameraDefaultFOV;

	/** Field of view value for when zoomed in */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Combat, meta = (AllowPrivateAccess = "true"))
	float CameraZoomedFOV;

	/** Current field of view this frame */
	float CameraCurrentFOV;

	/** Target Field of view */
	float CameraTagetFOV;

	/** Interp speed for zooming whe aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;

	/** Determines the spred of the crosshairs */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpredMultiplier;

	/** Velocity component for crosshairs spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	/** In air component for crosshairs spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	/** Aim component for crosshairs spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	/** Shooting component for crosshairs spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	/** Left mouse button or right console tirgger pressed */
	bool bFireButtonPressed;

	/** True when we can fire. Flase when waiting for the timer */
	bool bShouldFire;

	/** Sets a timer between gunshot */
	FTimerHandle AutoFireTimer;

	/** True if we should trace every frame for items */
	bool bShouldTraceForItems;

	/** Number of overlapped items */
	int8 OverlappedItemCount;

	/** The AShooterItem we hit last frame */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AShooterItem> TraceHitItemLastFrame;

	/** Currently equipped weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AShooterWeapon> EquippedWeapon;

	/** Set this in Blueprints for the default weapon class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AShooterWeapon> DefaultWeaponClass;

	/** The item currently hit by our trace in TraceForItems (could be null) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AShooterItem> TraceHitItem;

	/** Distance outward from the camera for the interp destination */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpDistance;

	/** Distance upward from the camera for the interp destination */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpElevation;

	/** Map to keep track of ammo of the different ammo types */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	TMap<EShooterAmmoType, int32> AmmoMap;

	/** Starting amount 9mm ammo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 Starting9mmAmmo;

	/** Starting amount AR ammo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmo;

	/** Combat State, can only fire or reload if Unoccupied */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	EShooterCombatState CombatState;

	/** Montage for reload animations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> ReloadMontage;

	/** Montage for Equip animations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> EquipMontage;

	/** Transform of the clip when we first grab the clip during reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FTransform ClipTransform;

	/** Scene component to attach to the Character's hand during reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> HandSceneComponent;

	/** True when crouching */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bCrouching;

	/** Regular movement speed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseMovementSpeed;

	/** Crouch movement speed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchMovementSpeed;

	/** Current half height of the capsule */
	float CurrentCapsuleHalfHeight;

	/** Half height of the capsule when not crouching */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float StandingCapsuleHalfHeight;

	/** Half height of the capsule when crouching */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingCapsuleHalfHeight;

	/** Ground friction while not crouching */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseGroundFriction;

	/** Ground friction while crouching */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingGroundFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FVector DefaultCameraBoomSocketOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FVector CrouchingCameraBoomSocketOffset;

	/** Used for knowing when the aiming button is pressed */
	bool bAimingButtonPressed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> WeaponInterpComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> InterpComponent1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> InterpComponent2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> InterpComponent3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> InterpComponent4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> InterpComponent5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> InterpComponent6;

	/** Array of interp location structs */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FShooterInterpLocation> InterpLocations;

	FTimerHandle PickupSoundTimer;
	FTimerHandle EquipSoundTimer;

	bool bShouldPlayPickupSound;
	bool bShouldPlayEquipSound;

	/** Time to wait before we can play another pickup sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float PickupSoundResetTime;

	/** Time to wait before we can play another equip sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float EquipSoundResetTime;

	/** Delegate for sending infomation to InventoryBar when equipping */
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FEquipItemDelegate EquipItemDelegate;

	/** Delegate for sending infomation for playing the icon animation */
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FHighlightIconDelegate HighlightIconDelegate;

	/** The index for the currently highlighted slot */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	int32 HighlightedSlot;

	/** Character Health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Health;

	/** Character maximum Health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	/** Sound make when Character gets hit by a melee attack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundCue> MeleeImpactSound;

	/** Blood splatter particles for melee hit */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> BloodParticles;

	/** Hit react anim montage; for whe character is stunned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> HitReactMontage;

	/** Chance of being stunned when hit by an enemy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float StunChance;

	/** montage for character death */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> DeathMontage;

public:
	FORCEINLINE bool GetAiming() const { return bAming; }

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;

	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

	/** Add/subtracts to/from OverlappedItemCount and update bShouldTraceItems */
	void IncrementOverlappedItemCount(int8 Amount);

	// No longer needed; AShooterItem has GetInterpLocation
	//FVector GetCameraInterpLocation();

	void GetPickupItem(AShooterItem* Item);

	FORCEINLINE EShooterCombatState GetCombatState() const { return CombatState; }

	FORCEINLINE bool GetCrouching() const { return bCrouching; }

	FShooterInterpLocation GetInterpLocation(int32 Index);

	/** Return the index in InterpLocations array with the lowest ItemCount */
	int32 GetInterpLocationIndex();

	void IncrementInterpLocationItemCount(int32 Index, int32 Amount);

	FORCEINLINE bool ShouldPlayPickupSound() const { return bShouldPlayPickupSound; }
	FORCEINLINE bool ShouldPlayEquipSound() const { return bShouldPlayEquipSound; }

	void StartPickupSoundTimer();
	void StartEquipSoundTimer();

	UShooterInventoryComponent* GetInventory();

	void HighlightInventorySlot();
	void UnHighlightInventorySlot();
	
	FORCEINLINE AShooterWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	FORCEINLINE USoundCue* GetMeleeImpactSound() const { return MeleeImpactSound; }
	FORCEINLINE UParticleSystem* GetBloodParticles() const { return BloodParticles; }

	void Stun();
	FORCEINLINE float GetStunChance() const { return StunChance; }
};