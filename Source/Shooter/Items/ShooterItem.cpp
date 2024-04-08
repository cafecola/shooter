#include "ShooterItem.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/ShooterPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Curves/CurveVector.h"
#include "Data/ShooterDataTables.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ShooterItem)

AShooterItem::AShooterItem()
	:ItemName(FString("Default"))
	,ItemCount(0)
	,ItemRarity(EShooterItemRarity::EIR_Common)
	,ItemState(EShooterItemState::EIS_Pickup)
	// Item interp variables
	,ZCurveTime(0.7f)
	,ItemInterpStartLocation(FVector(0.f))
	,CameraTargetLocation(FVector(0.f))
	,bInterping(false)
	,ItemInterpX(0.f)
	,ItemInterpY(0.f)
	,InterpInitialYawOffset(0.f)
	,ItemType(EShooterItemType::EIT_MAX)
	,InterpLocationIndex(0)
	,bCanChangeCustomDepth(true)
	// Dynamic Material Parameter
// 	,GlowAmount(150.f)
// 	,FresnelExponent(3.f)
// 	,FresnelReflectFraction(4.f)
	, GlowAmount(3.f)
	, FresnelExponent(1.f)
	, FresnelReflectFraction(0.f)
	,PulseCurveTime(5.f)
	// Inventory Index
	,SlotIndex(0)
	,bCharacterInventoryFull(false)
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(GetRootComponent());
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(GetRootComponent());

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(GetRootComponent());
}

void AShooterItem::BeginPlay()
{
	Super::BeginPlay();

	// Hide pickup widget
	if (IsValid(PickupWidget))
	{
		PickupWidget->SetVisibility(false);
	}
	// Sets ActiveStars array based on Item Rarity
	SetActiveStars();

	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
	
	// Set item properties based on ItemState
	SetItemProperties(ItemState);

	// Set custom depth to disable
	InitializeCustomDepth();

	StartPulseTimer();
}

void AShooterItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (::IsValid(OtherActor))
	{
		if (AShooterPlayerCharacter* ShooterCharacter = Cast<AShooterPlayerCharacter>(OtherActor))
		{
			ShooterCharacter->IncrementOverlappedItemCount(1);
		}
	}
}

void AShooterItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (::IsValid(OtherActor))
	{
		if (AShooterPlayerCharacter* ShooterCharacter = Cast<AShooterPlayerCharacter>(OtherActor))
		{
			ShooterCharacter->IncrementOverlappedItemCount(-1);

			if (ItemState != EShooterItemState::EIS_EquipInterping)
			{
				ShooterCharacter->UnHighlightInventorySlot();
			}
		}
	}
}

void AShooterItem::SetActiveStars()
{
	// the 0 element isn't used
	for (int32 i = 0; i <= 5; i++)
	{
		ActiveStars.Add(false);
	}

	switch (ItemRarity)
	{
	case EShooterItemRarity::EIR_Damaged:
		ActiveStars[1] = true;
		break;
	case EShooterItemRarity::EIR_Common:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		break;
	case EShooterItemRarity::EIR_Uncommon:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		break;
	case EShooterItemRarity::EIR_Rare:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		break;
	case EShooterItemRarity::EIR_Legendary:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		ActiveStars[5] = true;
		break;
	}
}

void AShooterItem::SetItemProperties(EShooterItemState State)
{
	switch (State)
	{
	case EShooterItemState::EIS_Pickup:
		// Set mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		// Set Collision properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Block);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case EShooterItemState::EIS_EquipInterping:
		PickupWidget->SetVisibility(false);
		// Set mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Set Collision properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EShooterItemState::EIS_PickedUp:
		PickupWidget->SetVisibility(false);
		// Set mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(false);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Set Collision properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EShooterItemState::EIS_Equipped:
		PickupWidget->SetVisibility(false);
		// Set mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Set Collision properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EShooterItemState::EIS_Falling:
		// Set mesh properties
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetEnableGravity(true);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Set Collision properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	default:
		break;
	}
}

void AShooterItem::FinishInterping()
{
	bInterping = false;
	if (Character)
	{
		// Subtract 1 from the Item Count of the interp location struct
		Character->IncrementInterpLocationItemCount(InterpLocationIndex, -1);
		Character->GetPickupItem(this);

		Character->UnHighlightInventorySlot();
	}
	// Set scale back to normal
	SetActorScale3D(FVector(1.f));
	
	DisableGlowMaterial();
	bCanChangeCustomDepth = true;
	DisableCustomDepth();
}

void AShooterItem::ItemInterp(float DeltaTime)
{
	if (!bInterping) return;

	if (Character && ItemZCurve)
	{
		// Elapsed time since we started ItemInterpTimer
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
		// Get curve value correspondig to ElapsedTime
		const float CurveValue = ItemZCurve->GetFloatValue(ElapsedTime);
		// Get the item's initial location when the curve started
		FVector ItemLocation = ItemInterpStartLocation;
		// Get location in front of the camera
		//const FVector CameraInterpLocation(Character->GetCameraInterpLocation());
		const FVector CameraInterpLocation(GetInterpLocation());

		// Vector from Item to Camera Interp location, X and Y are zeroed out
		const FVector ItemToCamera(FVector(0.f, 0.f, (CameraInterpLocation - ItemLocation).Z));
		// Scale factor to multiply with CurveValue
		const float DeltaZ = ItemToCamera.Size();

		const FVector CurrentLocation = GetActorLocation();
		// Interpolated X value
		const float InterpXValue = FMath::FInterpTo(CurrentLocation.X, CameraInterpLocation.X, DeltaTime, 30.f);
		// Interpolated Y value
		const float InterpYValue = FMath::FInterpTo(CurrentLocation.Y, CameraInterpLocation.Y, DeltaTime, 30.f);

		// Set X and Y of ItemLocation to Interped values
		ItemLocation.X = InterpXValue;
		ItemLocation.Y = InterpYValue;

		// Adding curve value to the Z component of the Initial Location (scaled by DeltaZ)
		ItemLocation.Z += CurveValue * DeltaZ;
		SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);

		// Camera rotation this frame
		const FRotator CameraRotation = Character->GetFollowCamera()->GetComponentRotation();
		// Camera rotation + inital Yaw Offset
		FRotator ItemRotation{ 0.f, CameraRotation.Yaw + InterpInitialYawOffset, 0.f };
		SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);

		if (ItemScaleCurve)
		{
			const float ScaleCurveValue = ItemScaleCurve->GetFloatValue(ElapsedTime);
			SetActorScale3D(FVector(ScaleCurveValue, ScaleCurveValue, ScaleCurveValue));
		}
	}
}

FVector AShooterItem::GetInterpLocation()
{
	if (false == ::IsValid(Character)) return FVector(0.f);

	switch (ItemType)
	{
	case EShooterItemType::EIT_Ammo:
		return Character->GetInterpLocation(InterpLocationIndex).SceneComponent->GetComponentLocation();
		break;
	case EShooterItemType::EIT_Weapon:
		return Character->GetInterpLocation(0).SceneComponent->GetComponentLocation();
		break;
	}

	return FVector();
}

void AShooterItem::PlayPickupSound(bool bForcePlaySound)
{
	if (::IsValid(Character))
	{
		if (bForcePlaySound)
		{
			if (PickupSound)
			{
				UGameplayStatics::PlaySound2D(this, PickupSound);
			}
		}
		else if (Character->ShouldPlayPickupSound())
		{
			Character->StartPickupSoundTimer();
			if (PickupSound)
			{
				UGameplayStatics::PlaySound2D(this, PickupSound);
			}
		}
	}
}

void AShooterItem::EnableCustomDepth()
{
	if (bCanChangeCustomDepth)
	{
		ItemMesh->SetRenderCustomDepth(true);
	}
}

void AShooterItem::DisableCustomDepth()
{
	if (bCanChangeCustomDepth)
	{
		ItemMesh->SetRenderCustomDepth(false);
	}
}

void AShooterItem::EnableGlowMaterial()
{
	if (::IsValid(DynamicMaterialInstance))
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 0);
	}
}

void AShooterItem::DisableGlowMaterial()
{
	if (::IsValid(DynamicMaterialInstance))
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 1);
	}
}

void AShooterItem::ResetPulseTimer()
{
	StartPulseTimer();
}

void AShooterItem::StartPulseTimer()
{
	if (ItemState == EShooterItemState::EIS_Pickup)
	{
		GetWorldTimerManager().SetTimer(PulseTimer, this, &ThisClass::ResetPulseTimer, PulseCurveTime);
	}
}

void AShooterItem::InitializeCustomDepth()
{
	DisableCustomDepth();
}

void AShooterItem::OnConstruction(const FTransform& Transform)
{
	// Load the data in the Item Rarity Data Table
	FString RarityTablePath(TEXT("/Script/Engine.DataTable'/Game/_Game/DataTable/DT_ItemRarity.DT_ItemRarity'"));
	UDataTable* RarityTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), this, *RarityTablePath));
	if (RarityTableObject)
	{
		FItemRarityTableRow* RarityRow = nullptr;
		switch (ItemRarity)
		{
		case EShooterItemRarity::EIR_Damaged:
			RarityRow = RarityTableObject->FindRow<FItemRarityTableRow>(FName("Damaged"), TEXT(""));
			break;
		case EShooterItemRarity::EIR_Common:
			RarityRow = RarityTableObject->FindRow<FItemRarityTableRow>(FName("Common"), TEXT(""));
			break;
		case EShooterItemRarity::EIR_Uncommon:
			RarityRow = RarityTableObject->FindRow<FItemRarityTableRow>(FName("Uncommon"), TEXT(""));
			break;
		case EShooterItemRarity::EIR_Rare:
			RarityRow = RarityTableObject->FindRow<FItemRarityTableRow>(FName("Rare"), TEXT(""));
			break;
		case EShooterItemRarity::EIR_Legendary:
			RarityRow = RarityTableObject->FindRow<FItemRarityTableRow>(FName("Legendary"), TEXT(""));
			break;
		}

		if (RarityRow)
		{
			GlowColor = RarityRow->GlowColor;
			LightColor = RarityRow->LightColor;
			DarkColor = RarityRow->DarkColor;
			NumberOfStars = RarityRow->NumberOfStars;
			IconBackground = RarityRow->IconBackground;
			if (GetItemMesh())
			{
				GetItemMesh()->SetCustomDepthStencilValue(RarityRow->CustomDepthStencil);
			}
		}
	}

	if (::IsValid(MaterialInstance))
	{
		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);
		DynamicMaterialInstance->SetVectorParameterValue(TEXT("FresnelColor"), GlowColor);
		ItemMesh->SetMaterial(MaterialIndex, DynamicMaterialInstance);

		EnableGlowMaterial();
	}
}

UE_DISABLE_OPTIMIZATION
void AShooterItem::UpdatePulse()
{
	float ElapsedTime{};
	FVector CurveValue{};

	switch (ItemState)
	{
	case EShooterItemState::EIS_Pickup:
		if (PulseCurve)
		{
			ElapsedTime = GetWorldTimerManager().GetTimerElapsed(PulseTimer);
			CurveValue = PulseCurve->GetVectorValue(ElapsedTime);
		}
		break;
	case EShooterItemState::EIS_EquipInterping:
		if (InterpPulseCurve)
		{
			ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
			CurveValue = InterpPulseCurve->GetVectorValue(ElapsedTime);
		}
		break;
	}

	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowAmount"), CurveValue.X * GlowAmount);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("FresnelExponent"), CurveValue.Y * FresnelExponent);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("FresnelReflectFraction"), CurveValue.Z * FresnelReflectFraction);
	}
}
UE_ENABLE_OPTIMIZATION

void AShooterItem::PlayEquipSound(bool bForcePlaySound)
{
	if (::IsValid(Character))
	{
		if (bForcePlaySound)
		{
			if (EquipSound)
			{
				UGameplayStatics::PlaySound2D(this, EquipSound);
			}
		}
		else if (Character->ShouldPlayEquipSound())
		{
			Character->StartEquipSoundTimer();
			if (EquipSound)
			{
				UGameplayStatics::PlaySound2D(this, EquipSound);
			}
		}
	}
}

void AShooterItem::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Handle Item Interping when in the EquipInterping state
	ItemInterp(DeltaSeconds);

	// Get curve values from PulseCurve and set dynamic material parameters
	UpdatePulse();
}

void AShooterItem::SetItemState(EShooterItemState State)
{
	ItemState = State;
	SetItemProperties(ItemState);
}

void AShooterItem::StartItemCurve(AShooterPlayerCharacter* InCharacter, bool bForcePlaySound)
{
	// Store a handle to the character
	Character = InCharacter;

	// Get array index in InterpLocations with the lowest item count
	InterpLocationIndex = Character->GetInterpLocationIndex();
	// Add 1 to the Item Count for this interp location struct
	Character->IncrementInterpLocationItemCount(InterpLocationIndex, 1);

	PlayPickupSound(bForcePlaySound);

	// Store initial location of the Item
	ItemInterpStartLocation = GetActorLocation();
	bInterping = true;
	SetItemState(EShooterItemState::EIS_EquipInterping);
	GetWorldTimerManager().ClearTimer(PulseTimer);

	GetWorldTimerManager().SetTimer(ItemInterpTimer, this, &ThisClass::FinishInterping, ZCurveTime);

	// Get initial Yaw of the Camera
	const float CameraRotationYaw = Character->GetFollowCamera()->GetComponentRotation().Yaw;
	// Get initial Yaw of the Item
	const float ItemRotationYaw = GetActorRotation().Yaw;
	// initial Yaw offset between Camera and Item
	InterpInitialYawOffset = ItemRotationYaw - CameraRotationYaw;

	bCanChangeCustomDepth = false;
}
