#include "ShooterAmmo.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/ShooterPlayerCharacter.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ShooterAmmo)

AShooterAmmo::AShooterAmmo()
{
	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
	SetRootComponent(AmmoMesh);

	GetCollisionBox()->SetupAttachment(GetRootComponent());
	GetPickupWidget()->SetupAttachment(GetRootComponent());
	GetAreaSphere()->SetupAttachment(GetRootComponent());

	AmmoCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AmmoCollisionSphere"));
	AmmoCollisionSphere->SetupAttachment(GetRootComponent());
	AmmoCollisionSphere->SetSphereRadius(50.f);
}

void AShooterAmmo::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AShooterAmmo::BeginPlay()
{
	Super::BeginPlay();

	AmmoCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnAmmoSphereOverlap);
}

void AShooterAmmo::SetItemProperties(EShooterItemState State)
{
	Super::SetItemProperties(State);

	switch (State)
	{
	case EShooterItemState::EIS_Pickup:
		// Set mesh properties
		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetEnableGravity(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		break;
	case EShooterItemState::EIS_EquipInterping:
		// Set mesh properties
		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetEnableGravity(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		break;
	case EShooterItemState::EIS_PickedUp:
		break;
	case EShooterItemState::EIS_Equipped:
		// Set mesh properties
		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetEnableGravity(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		break;
	case EShooterItemState::EIS_Falling:
		// Set mesh properties
		AmmoMesh->SetSimulatePhysics(true);
		AmmoMesh->SetEnableGravity(true);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		
		break;
	default:
		break;
	}

}

void AShooterAmmo::OnAmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (::IsValid(OtherActor))
	{
		if (auto OverlappedCharacter = Cast<AShooterPlayerCharacter>(OtherActor))
		{
			StartItemCurve(OverlappedCharacter);
			AmmoCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AShooterAmmo::EnableCustomDepth()
{
	if (bCanChangeCustomDepth)
	{
		AmmoMesh->SetRenderCustomDepth(true);
	}
}

void AShooterAmmo::DisableCustomDepth()
{
	if (bCanChangeCustomDepth)
	{
		AmmoMesh->SetRenderCustomDepth(false);
	}
}

