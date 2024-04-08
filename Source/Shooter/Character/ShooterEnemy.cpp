#include "ShooterEnemy.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "AI/EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Character/ShooterPlayerCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ShooterEnemy)

AShooterEnemy::AShooterEnemy() :
	Health(100.f),
	MaxHealth(100.f),
	HealthBarDisplayTime(4.f),
	bCanHitReact(true),
	HitReactTimeMin(.5f),
	HitReactTimeMax(3.f),
	HitNumberDestroyTime(1.5f),
	bStunned(false),
	StunChance(.5f),
	AttackLFast(TEXT("AttackLFast")),
	AttackRFast(TEXT("AttackRFast")),
	AttackL(TEXT("AttackL")),
	AttackR(TEXT("AttackR")),
	BaseDamage(20.f),
	LeftWeaponFxSocket(TEXT("FX_Trail_L_01")),
	RightWeaponFxSocket(TEXT("FX_Trail_R_01")),
	bCanAttack(true),
	AttackWaitTime(1.f),
	bDying(false),
	DeathTime(4.f)
{
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());

	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRangeSphere"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());

	// Construct left and right weapon collision box
	LeftWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWeaponBox"));
	LeftWeaponCollision->SetupAttachment(GetMesh(), FName("WeaponBoneSocket_L"));
	RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWeaponBox"));
	RightWeaponCollision->SetupAttachment(GetMesh(), FName("WeaponBoneSocket_R"));
}

void AShooterEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateHitNumber();
}

void AShooterEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AShooterEnemy::BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* EventInstigator)
{
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location, FRotator(0.f), true);
	}
}

float AShooterEnemy::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float FinalDamageAmount = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	// Set the Target Blackboard key to agro the character
	if (EnemyController)
	{
		if (EnemyController->GetBlackboardComponent())
		{
			EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), DamageCauser);
		}
	}

	if (Health - FinalDamageAmount <= 0.f)
	{
		Health = 0.f;
		Die();
	}
	else
	{
		Health -= FinalDamageAmount;
	}

	if (bDying) return FinalDamageAmount;

	ShowHealthBar();

	// Determine whether bullet hit stuns
	const float Stunned = FMath::FRandRange(0.f, 1.f);
	if (Stunned <= StunChance)
	{
		// Stun the Enemy
		PlayHitMontage(FName("HitReactFront"));
		SetStunned(true);
	}

	return FinalDamageAmount;
}

void AShooterEnemy::BeginPlay()
{
	Super::BeginPlay();

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnAgroSphereOverlap);
	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnCombatRangeSphereOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnCombatRangeSphereEndOverlap);

	// Bind functions to overlap events for weapon boxes
	LeftWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnLeftWeaponOverlap);
	RightWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnRightWeaponOverlap);
	
	// Set collision presets for weapon boxes
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	LeftWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	LeftWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RightWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RightWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);


	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CombatRangeSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// Get the AI Controller
	EnemyController = Cast<AEnemyController>(GetController());

	if (EnemyController)
	{
		if (EnemyController->GetBlackboardComponent())
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("CanAttack"), bCanAttack);
		}
	}

	const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);
	const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);

	//DrawDebugSphere(GetWorld(), WorldPatrolPoint, 25.f, 12, FColor::Red, true);
	//DrawDebugSphere(GetWorld(), WorldPatrolPoint2, 25.f, 12, FColor::Red, true);

	if (EnemyController)
	{
		if (EnemyController->GetBlackboardComponent())
		{
			EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint"), WorldPatrolPoint);
			EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), WorldPatrolPoint2);
		}
		EnemyController->RunBehaviorTree(BehaviorTree);
	}
}

void AShooterEnemy::ShowHealthBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(HealthBarTimer);
	GetWorldTimerManager().SetTimer(HealthBarTimer, this, &ThisClass::HideHealthBar, HealthBarDisplayTime);
}

void AShooterEnemy::Die()
{
	if (bDying) return;

	bDying = true;

	HideHealthBar();

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}

	if (EnemyController)
	{
		if (EnemyController->GetBlackboardComponent())
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Dead"), true);
		}
		EnemyController->StopMovement();
	}
}

void AShooterEnemy::PlayHitMontage(FName Section, float PlayRate)
{
	if (bCanHitReact)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(HitMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(Section, HitMontage);
		}

		bCanHitReact = false;
		const float HitReactTime = FMath::FRandRange(HitReactTimeMin, HitReactTimeMax);
		GetWorldTimerManager().SetTimer(HitReactTimer, this, &ThisClass::ResetReactTimer, HitReactTime);
	}
}

void AShooterEnemy::ResetReactTimer()
{
	bCanHitReact = true;
}

void AShooterEnemy::StoreHitNumber(UUserWidget* HitNumberWidget, FVector Location)
{
	HitNumberMap.Add(HitNumberWidget, Location);

	FTimerHandle HitNumberTimer;
	FTimerDelegate HitNumberDelegate;
	HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumberWidget);
	GetWorldTimerManager().SetTimer(HitNumberTimer, HitNumberDelegate, HitNumberDestroyTime, false);
}

void AShooterEnemy::DestroyHitNumber(UUserWidget* HitNumberWidget)
{
	HitNumberMap.Remove(HitNumberWidget);
	HitNumberWidget->RemoveFromParent();
}

void AShooterEnemy::UpdateHitNumber()
{
	for (auto& HitPair : HitNumberMap)
	{
		UUserWidget* HitNumberWidget = HitPair.Key;
		const FVector Location = HitPair.Value;
		FVector2D ScreenPosition;

		UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), Location, ScreenPosition);
		HitNumberWidget->SetPositionInViewport(ScreenPosition);
	}
}

void AShooterEnemy::OnAgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;
	auto Character = Cast<AShooterPlayerCharacter>(OtherActor);
	if (Character)
	{
		if (EnemyController)
		{
			if (EnemyController->GetBlackboardComponent())
			{
				EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), Character);
			}
		}
	}
}

void AShooterEnemy::OnCombatRangeSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (nullptr == OtherActor) return;
		
	if (auto Character = Cast<AShooterPlayerCharacter>(OtherActor))
	{
		bInAttackRange = true;
		if (EnemyController)
		{
			if (EnemyController->GetBlackboardComponent())
			{
				EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), bInAttackRange);
			}
		}
	}	
}

void AShooterEnemy::OnCombatRangeSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (nullptr == OtherActor) return;

	if (auto Character = Cast<AShooterPlayerCharacter>(OtherActor))
	{
		bInAttackRange = false;
		if (EnemyController)
		{
			if (EnemyController->GetBlackboardComponent())
			{
				EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), bInAttackRange);
			}
		}
	}
}

void AShooterEnemy::SetStunned(bool Stunned)
{
	bStunned = Stunned;
	if (EnemyController)
	{
		if (EnemyController->GetBlackboardComponent())
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), bStunned);
		}
	}
}

void AShooterEnemy::PlayAttackMontage(FName SectionName, float PlayRate)
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (AttackMontage)
		{
			AnimInstance->Montage_Play(AttackMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
		}
	}

	bCanAttack = false;
	GetWorldTimerManager().SetTimer(AttackWaitTimer, this, &ThisClass::ResetCanAttack, AttackWaitTime);
	if (EnemyController)
	{
		if (EnemyController->GetBlackboardComponent())
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("CanAttack"), bCanAttack);
		}
	}
}

FName AShooterEnemy::GetAttackSectionName()
{
	FName SectionName;
	const int32 Section = FMath::RandRange(1, 4);
	switch (Section)
	{
	case 1:
		SectionName = AttackLFast;
		break;
	case 2:
		SectionName = AttackRFast;
		break;
	case 3:
		SectionName = AttackL;
		break;
	case 4:
		SectionName = AttackR;
		break;
	}

	return SectionName;
}

void AShooterEnemy::OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto Character = Cast<AShooterPlayerCharacter>(OtherActor);
	if (Character)
	{
		DoDamage(Character);
		SpawnBlood(Character, LeftWeaponFxSocket);
		StunCharacter(Character);
	}
}

void AShooterEnemy::OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto Character = Cast<AShooterPlayerCharacter>(OtherActor);
	if (Character)
	{
		DoDamage(Character);
		SpawnBlood(Character, LeftWeaponFxSocket);
		StunCharacter(Character);
	}
}

void AShooterEnemy::ActivateLeftWeapon()
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AShooterEnemy::DeactivateLeftWeapon()
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AShooterEnemy::ActivateRightWeapon()
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AShooterEnemy::DeactivateRightWeapon()
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AShooterEnemy::SpawnBlood(AShooterPlayerCharacter* InCharacter, FName const& SocketName)
{
	if (nullptr == InCharacter) return;

	const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName(SocketName);
	if (TipSocket)
	{
		const FTransform SocketTransform = TipSocket->GetSocketTransform(GetMesh());
		if (InCharacter->GetBloodParticles())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), InCharacter->GetBloodParticles(), SocketTransform);
		}
	}
}

void AShooterEnemy::StunCharacter(AShooterPlayerCharacter* InCharacter)
{
	if (InCharacter)
	{
		const float Stun = FMath::FRandRange(0.f, 1.f);
		if (Stun <= InCharacter->GetStunChance())
		{
			InCharacter->Stun();
		}
	}
}

void AShooterEnemy::FinishDeath()
{
	GetMesh()->bPauseAnims = true;

	GetWorldTimerManager().SetTimer(DeathTimer, this, &ThisClass::DestroyEnemy, DeathTime);
}

void AShooterEnemy::DestroyEnemy()
{
	Destroy();
}

void AShooterEnemy::DoDamage(AShooterPlayerCharacter* InPlayerCharacter)
{
	if (nullptr == InPlayerCharacter) return;

	UGameplayStatics::ApplyDamage(InPlayerCharacter, BaseDamage, EnemyController, this, UDamageType::StaticClass());

	if (InPlayerCharacter->GetMeleeImpactSound())
	{
		UGameplayStatics::PlaySoundAtLocation(this, InPlayerCharacter->GetMeleeImpactSound(), GetActorLocation());
	}
}

void AShooterEnemy::ResetCanAttack()
{
	bCanAttack = true;
	if (EnemyController)
	{
		if (EnemyController->GetBlackboardComponent())
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("CanAttack"), bCanAttack);
		}
	}
}



