// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/BulletHitInterface.h"
#include "ShooterExplosive.generated.h"

UCLASS()
class SHOOTER_API AShooterExplosive : public AActor, public IBulletHitInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShooterExplosive();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* EventInstigator) override;

private:

	/** Explosion when hit by a bullet */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UParticleSystem> ExplodeParicles;

	/** Sound to play when hit by bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USoundCue> ImpactSound;

	/** Mesh for the explosive */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> ExplosiveMesh;

	/** Used to determine what actors overlap during explosion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USphereComponent> OverlapSphere;

	/** Damage amount for explosive */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Damage;
};
