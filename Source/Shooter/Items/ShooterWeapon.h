#pragma once

#include "ShooterItem.h"
#include "ShooterAmmoType.h"
#include "ShooterItemType.h"
#include "ShooterWeapon.generated.h"

UCLASS()
class AShooterWeapon : public AShooterItem
{
	GENERATED_BODY()

public:
	AShooterWeapon();

	virtual void Tick(float DeltaSeconds) override;

protected:
	void StopFalling();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;

	void FinishMovingSlide();
	void UpdateSlideDisplacement();

private:
	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;

	/** Ammo count for this weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta =  (AllowPrivateAccess = "true"))
	int32 Ammo;

	/** Maximum ammo that our weapon can hold */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta =  (AllowPrivateAccess = "true"))
	int32 MagazineCapacity;

	/** the type of weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta =  (AllowPrivateAccess = "true"))
	EShooterWeaponType WeaponType;

	/** the type of ammo for this weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta =  (AllowPrivateAccess = "true"))
	EShooterAmmoType AmmoType;

	/** FName for the Reload Montage Section */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta =  (AllowPrivateAccess = "true"))
	FName ReloadMontageSction;

	/** True when moving the clip while reloading */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta =  (AllowPrivateAccess = "true"))
	bool bMovingClip;

	/** Name for the clip bone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName ClipBoneName;

	/** Data table for weapon properties */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataTable> WeaponDataTable;

	int32 PreviousMaterialIndex;

	/** Texture for the weapon crosshairs */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> CrosshairsMiddle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> CrosshairsLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> CrosshairsRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> CrosshairsBottom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> CrosshairsTop;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	float CrosshairsShootTimeDuration;

	/** The speed at which automatic fire happens */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	float AutoFireRate;

	/** Particle system spawned at the BarrelSocket */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> MuzzleFlash;

	/** Sound played when the weapon os fired */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundCue> FireSound;

	/** Name of the bone to hide on the weapon mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	FName BoneToHide;

	/** Amount that the slide is pushed back during pistol fire */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	float SlideDisplacement;

	/** Curve fot the slide displacement */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> SlideDisplacementCurve;

	/** Timer handle for updating slidedisplacement */
	FTimerHandle SlideTimer;

	/** Time for displacing the slide pistol fire */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	float SlideDisplacementTime;

	/** True when moving the pistol slide */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	bool bMovingSlide;

	/** Max distance for the slide on the pistol */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	float MaxSlideDisplacement;

	/** Max rotation for pistol recoil */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	float MaxRecoilRotation;

	/** Amount that the pistol will rotate during pistol fire */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	float RecoilRotation;

	/** True for auto gunfire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	bool bAutomatic;

	/** Amount of damage caused by a damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float Damage;

	/** Amount of damage whe a bullet hits the head */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float HeadShotDamage;


public:
	/** adds an impulse to the weapon */
	void ThrowWeapon();

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagizineCapacity() const { return MagazineCapacity; }

	/** Called from character class when firing weapon */
	void DecrementAmmo();

	FORCEINLINE EShooterWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EShooterAmmoType GetAmmoType() const { return AmmoType; }
	FORCEINLINE FName GetReloadMontageSection() const { return ReloadMontageSction; }
	FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }
	FORCEINLINE float GetAutoFireRate() const { return AutoFireRate; }
	FORCEINLINE UParticleSystem* GetMuzzleFlash() const { return MuzzleFlash; }
	FORCEINLINE USoundCue* GetFireSound() const { return FireSound; }
	FORCEINLINE float GetCrosshairsShootTimeDuration() const { return CrosshairsShootTimeDuration; }
	FORCEINLINE bool GetAutomatic() const { return bAutomatic; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }

	void StartSlideTimer();
	
	void ReloadAmmo(int32 Amount);

	FORCEINLINE void SetMovingClip(bool Move) { bMovingClip = Move; }

	bool ClipIsFull();


};