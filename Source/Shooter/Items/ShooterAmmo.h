#pragma once

#include "ShooterItem.h"
#include "ShooterAmmoType.h"
#include "ShooterAmmo.generated.h"

UCLASS()
class AShooterAmmo : public AShooterItem
{
	GENERATED_BODY()

public:
	AShooterAmmo();

	virtual void Tick(float DeltaSeconds) override;

	virtual void EnableCustomDepth() override;
	virtual void DisableCustomDepth() override;

protected:
	virtual void BeginPlay() override;

	/** Override of SetItemProperties so we can set AmmoMesh properties */
	virtual void SetItemProperties(EShooterItemState State) override;

	/** Called when overlapping AmmoCollisionSphere */
	UFUNCTION()
	void OnAmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	

private:
	/** Mesh for the ammo Pickup */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> AmmoMesh;

	/** Ammo type for the ammo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	EShooterAmmoType AmmoType;

	/** The texture for the ammo icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> AmmoIconTexture;

	/** Overlap Sphere for picking up the ammo */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> AmmoCollisionSphere;

public:
	FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const { return AmmoMesh; }
	FORCEINLINE EShooterAmmoType GetAmmoType() const { return AmmoType; }
};