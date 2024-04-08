// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterExplosive.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ShooterExplosive)

// Sets default values
AShooterExplosive::AShooterExplosive() :
	Damage(100.f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ExplosiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExplosiveMesh"));
	SetRootComponent(ExplosiveMesh);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AShooterExplosive::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AShooterExplosive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AShooterExplosive::BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* EventInstigator)
{
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (ExplodeParicles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeParicles, HitResult.Location, FRotator(0.f), true);
	}

	// Apply explosive damage
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for (auto Actor : OverlappingActors)
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor damaged bt explosive: %s"), *Actor->GetName());
		UGameplayStatics::ApplyDamage(Actor, Damage, EventInstigator, Shooter, UDamageType::StaticClass());
	}

	Destroy();
}

