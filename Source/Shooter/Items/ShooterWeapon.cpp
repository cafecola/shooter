#include "ShooterWeapon.h"
#include "Data/ShooterDataTables.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ShooterWeapon)

AShooterWeapon::AShooterWeapon()
	:ThrowWeaponTime(0.7f)
	,bFalling(false)
	,Ammo(30)
	,MagazineCapacity(30)
	,WeaponType(EShooterWeaponType::EWT_SubmachineGun)
	,AmmoType(EShooterAmmoType::EAT_9mm)
	,ReloadMontageSction(FName(TEXT("Reload_SMG")))
	,ClipBoneName(TEXT("smg_clip"))
	,SlideDisplacement(0.f)
	,SlideDisplacementTime(0.2f)
	,bMovingSlide(false)
	,MaxSlideDisplacement(4.f)
	,MaxRecoilRotation(20.f)
	,bAutomatic(true)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AShooterWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Keep the Weapon upright
	if (GetItemState() == EShooterItemState::EIS_Falling && bFalling)
	{
		const FRotator MeshRotation(0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f);
		GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}

	UpdateSlideDisplacement();
}

void AShooterWeapon::ThrowWeapon()
{
	FRotator MeshRotation(0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f);
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector MeshForward(GetItemMesh()->GetForwardVector());
	const FVector MeshRight(GetItemMesh()->GetRightVector());
	// Direction in which we throw the weapon
	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);
	
	//float RandomRotation = FMath::FRandRange(0.f, 30.f);
	float RandomRotation = 30.f;
	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
	ImpulseDirection *= 3'000.f;
	GetItemMesh()->AddImpulse(ImpulseDirection);

	bFalling = true;
	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &ThisClass::StopFalling, ThrowWeaponTime);

	EnableGlowMaterial();
}

void AShooterWeapon::DecrementAmmo()
{
	if (Ammo - 1 <= 0)
	{
		Ammo = 0;
	}
	else
	{
		--Ammo;
	}
}

void AShooterWeapon::StartSlideTimer()
{
	bMovingSlide = true;
	GetWorldTimerManager().SetTimer(SlideTimer, this, &ThisClass::FinishMovingSlide, SlideDisplacementTime);
}

void AShooterWeapon::ReloadAmmo(int32 Amount)
{
	checkf(Ammo + Amount <= MagazineCapacity, TEXT("Attempted to reload with more than magagize capacity"))
	Ammo += Amount;
}

bool AShooterWeapon::ClipIsFull()
{
	return Ammo >= MagazineCapacity;
}

void AShooterWeapon::StopFalling()
{
	bFalling = false;
	SetItemState(EShooterItemState::EIS_Pickup);
	StartPulseTimer();
}

UE_DISABLE_OPTIMIZATION
void AShooterWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const FString WeaponTablePath{ TEXT("/Script/Engine.DataTable'/Game/_Game/DataTable/DT_Weapon.DT_Weapon'")};
	UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));

	if (WeaponTableObject)
	{
		FWeaponDataTableRow* WeaponDataRow = nullptr;
		switch (WeaponType)
		{
		case EShooterWeaponType::EWT_SubmachineGun:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTableRow>(FName("SubmachineGun"), TEXT(""));
			break;
		case EShooterWeaponType::EWT_AssaultRifle:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTableRow>(FName("AssaultRifle"), TEXT(""));
			break;
		case EShooterWeaponType::EWT_Pistol:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTableRow>(FName("Pistol"), TEXT(""));
			break;
		}
		
		if (WeaponDataRow)
		{
			AmmoType = WeaponDataRow->AmmoType;
			Ammo = WeaponDataRow->WeaponAmmo;
			MagazineCapacity = WeaponDataRow->MagazingCapacity;
			SetPickupSound(WeaponDataRow->PickupSound);
			SetEquipSound(WeaponDataRow->EquipSound);
			GetItemMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);
			SetItemName(WeaponDataRow->ItemName);
			SetIconItem(WeaponDataRow->InventoryIcon);
			SetIconAmmo(WeaponDataRow->AmmoIcon);

			SetMaterialInstance(WeaponDataRow->MaterialInstance);

			PreviousMaterialIndex = GetMaterialIndex();
			GetItemMesh()->SetMaterial(PreviousMaterialIndex, nullptr);

			SetMaterialIndex(WeaponDataRow->MaterialIndex);

			ClipBoneName = WeaponDataRow->ClipBoneName;
			ReloadMontageSction = WeaponDataRow->ReloadMontageSection;
			GetItemMesh()->SetAnimInstanceClass(WeaponDataRow->AnimBP);

			CrosshairsMiddle = WeaponDataRow->CrosshairsMiddle;
			CrosshairsLeft = WeaponDataRow->CrosshairsLeft;
			CrosshairsRight = WeaponDataRow->CrosshairsRight;
			CrosshairsBottom = WeaponDataRow->CrosshairsBottom;
			CrosshairsTop = WeaponDataRow->CrosshairsTop;
			CrosshairsShootTimeDuration = WeaponDataRow->CrosshairsShootTimeDuration;

			AutoFireRate = WeaponDataRow->AutoFireRate;
			MuzzleFlash = WeaponDataRow->MuzzleFlash;
			FireSound = WeaponDataRow->FireSound;

			BoneToHide = WeaponDataRow->BoneToHide;

			bAutomatic = WeaponDataRow->bAutomatic;

			Damage = WeaponDataRow->Damage;
			HeadShotDamage = WeaponDataRow->HeadShotDamage;
		}

		if (::IsValid(GetMaterialInstance()))
		{
			SetDynamicMaterialInstance(UMaterialInstanceDynamic::Create(GetMaterialInstance(), this));
			GetDynamicMaterialInstance()->SetVectorParameterValue(TEXT("FresnelColor"), GetGlowColor());
			GetItemMesh()->SetMaterial(GetMaterialIndex(), GetDynamicMaterialInstance());

			EnableGlowMaterial();
		}
	}
}
void AShooterWeapon::BeginPlay()
{
	Super::BeginPlay();
	if (false == ::IsValid(GetItemMesh())) return;
	
	if (BoneToHide != FName("") && BoneToHide != FName("None"))
	{
		GetItemMesh()->HideBoneByName(BoneToHide, EPhysBodyOp::PBO_None);
	}
}
void AShooterWeapon::FinishMovingSlide()
{
	bMovingSlide = false;
}
void AShooterWeapon::UpdateSlideDisplacement()
{
	if (::IsValid(SlideDisplacementCurve) && bMovingSlide)
	{
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(SlideTimer);
		const float CurveValue = SlideDisplacementCurve->GetFloatValue(ElapsedTime);
		SlideDisplacement = CurveValue * MaxSlideDisplacement;
		RecoilRotation = CurveValue * MaxRecoilRotation;
	}
}
UE_ENABLE_OPTIMIZATION