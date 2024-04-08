#pragma once

UENUM(BlueprintType)
enum class EShooterItemRarity : uint8
{
	EIR_Damaged UMETA(DisplayName = "Damaged"),
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_Uncommon UMETA(DisplayName = "Uncommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Legendary UMETA(DisplayName = "Legendary"),

	EIR_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EShooterItemType : uint8
{
	EIT_Ammo UMETA(DisplayName = "Ammo"),
	EIT_Weapon UMETA(DisplayName = "Weapon"),

	EIT_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EShooterWeaponType : uint8
{
	EWT_SubmachineGun UMETA(DisplayName ="SubmachineGun"),
	EWT_AssaultRifle UMETA(DisplayName ="AssaultRifle"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),

	EWT_MAX UMETA(DisplayName ="DefaultMax"),
};