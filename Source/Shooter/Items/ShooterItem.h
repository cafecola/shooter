#pragma once


#include "ShooterItemType.h"
#include "GameFramework/Actor.h"
#include "ShooterItem.generated.h"

/** forward declarations */
class UBoxComponent;
class USkeletalMeshComponent;
class UWidgetComponent;
class USphereComponent;
class UCurveFloat;
class UCurveVector;
class AShooterPlayerCharacter;
class USoundCue;
class UPrimitiveComponent;
class UDataTable;

UENUM(BlueprintType)
enum class EShooterItemState : uint8
{
	EIS_Pickup UMETA(DisplayName = "Pickup"),
	EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
	EIS_PickedUp UMETA(DisplayName = "PickedUp"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),
	EIS_Falling UMETA(DisplayName = "Falling"),

	EIS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class AShooterItem : public AActor
{
	GENERATED_BODY()

public:
	AShooterItem();

protected:
	virtual void BeginPlay() override;

	/** Called when overlapping AreaSphere */
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	/** Called when end overlapping AreaSphere */
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Set the ActiveStars array of bools based on rarity */
	void SetActiveStars();

	/** Sets properties of the Item's component based on State */
	virtual void SetItemProperties(EShooterItemState State);

	/** Called whe ItemInterpTimer is finished */
	void FinishInterping();

	/** Haldles item interpolation when in the EquipInterping state */
	void ItemInterp(float DeltaTime);

	/** Get interp location based on the item type */
	FVector GetInterpLocation();

	void PlayPickupSound(bool bForcePlaySound = false);

	void InitializeCustomDepth();

	virtual void OnConstruction(const FTransform& Transform) override;

	void UpdatePulse();
	
	void ResetPulseTimer();
	void StartPulseTimer();


public:
	virtual void Tick(float DeltaSeconds) override;

	// Called in AShooterPlayerCharacter::GetPickupItem
	void PlayEquipSound(bool bForcePlaySound = false);

	virtual void EnableCustomDepth();
	virtual void DisableCustomDepth();

	void EnableGlowMaterial();
	void DisableGlowMaterial();

private:
	/** Skeletal Mesh for the item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> ItemMesh;

	/** Line trace collides with box tho show HUD widgets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> CollisionBox;

	/** Popup widget for when the player looks at the item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> PickupWidget;

	/** Enable item tracing when overlapped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> AreaSphere;

	/** The name which appears on the Pickup widget */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FString ItemName;

	/** Item Count (ammo, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 ItemCount;

	/** Item rarity - determines number of stars in pickup widget */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	EShooterItemRarity ItemRarity;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TArray<bool> ActiveStars;

	/** State of the Item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EShooterItemState ItemState;

	/** the curve asset to use for the item's Z location when interping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> ItemZCurve;

	/** Starting location when interping begins */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FVector ItemInterpStartLocation;

	/** Target interp location in front of the camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FVector CameraTargetLocation;

	/** ture when interping */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	bool bInterping;

	/** plays when we start interping */
	FTimerHandle ItemInterpTimer;
	/** Duration of the curve and timer */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float ZCurveTime;

	/** Pointer to the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AShooterPlayerCharacter> Character;

	/** X and Y for the Item while interping in the EquipIntering state */
	float ItemInterpX;
	float ItemInterpY;

	/** Initial Yaw offset between the camera and the interping item */
	float InterpInitialYawOffset;

	/** Curve used to scale the item when interping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> ItemScaleCurve;

	/** Sound played when Item is picked up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundCue> PickupSound;

	/** Sound played when Item is equipped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundCue> EquipSound;

	/** Enum for the type of item this Items is */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EShooterItemType ItemType;

	/** Index of the interp localtion this item is interping to */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 InterpLocationIndex;

	/** Index for the material we'd like to change at runtime */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 MaterialIndex;

	/** Dynamic instance that we can change at runtime */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterialInstance;

	/** Material Instance used with the Dynamic Material Instance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInstance> MaterialInstance;

	/** Curve to drive the dynamic material parameters */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveVector> PulseCurve;

	/** Curve to drive the dynamic material parameters */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveVector> InterpPulseCurve;

	FTimerHandle PulseTimer;

	/** Time for the PulseTimer */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float PulseCurveTime;

	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float GlowAmount;

	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FresnelExponent;

	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FresnelReflectFraction;;

	/** Icon for this item in the inventory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> IconItem;

	/** Ammo Icon for this item in the inventory array*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> IconAmmo;

	/** Slot in the Inventory */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex;

	/** True whe the Character's inventory is full */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	bool bCharacterInventoryFull;

	/** Item Rarity data table */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataTable> ItemRarityDataTable;

	/** Color in the glow material */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor GlowColor;

	/** Ligit Color int the pickup Widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor LightColor;

	/** Dark Color int the pickup Widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor DarkColor;

	/** Number of stars in the pickup widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	int32 NumberOfStars;

	/** Background icon for the inventory */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> IconBackground;

protected:
	bool bCanChangeCustomDepth;


public:
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }
	FORCEINLINE EShooterItemState GetItemState() const { return ItemState; }
	FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound; }
	FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound; }
	FORCEINLINE void SetPickupSound(USoundCue* InPickupSound) { PickupSound = InPickupSound; }
	FORCEINLINE void SetEquipSound(USoundCue* InEquipSound) { EquipSound = InEquipSound; }
	FORCEINLINE int32 GetItemCount() const { return ItemCount; }
	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }
	FORCEINLINE void SetSlotIndex(int32 Index) { SlotIndex = Index; }
	FORCEINLINE void SetCharacter(AShooterPlayerCharacter* InCharacter) { Character = InCharacter; }
	FORCEINLINE void SetCharacterInventoryFull(bool bFull) { bCharacterInventoryFull = bFull; }
	FORCEINLINE void SetItemName(FString Name) { ItemName = Name; }
	FORCEINLINE void SetIconItem(UTexture2D* Icon) { IconItem = Icon; }
	FORCEINLINE void SetIconAmmo(UTexture2D* Icon) { IconAmmo = Icon; }
	FORCEINLINE UMaterialInstance* GetMaterialInstance() const { return MaterialInstance; }
	FORCEINLINE void SetMaterialInstance(UMaterialInstance* Instance) { MaterialInstance = Instance; }
	FORCEINLINE int32 GetMaterialIndex() const { return MaterialIndex; }
	FORCEINLINE void SetMaterialIndex(int32 Index) { MaterialIndex = Index; }
	FORCEINLINE UMaterialInstanceDynamic* GetDynamicMaterialInstance() const { return DynamicMaterialInstance; }
	FORCEINLINE void SetDynamicMaterialInstance(UMaterialInstanceDynamic* Instance) { DynamicMaterialInstance = Instance; }
	FORCEINLINE FLinearColor GetGlowColor() const { return GlowColor; }

	void SetItemState(EShooterItemState State);
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }

	/** Called from AShooterPlayerCharacter class */
	void StartItemCurve(AShooterPlayerCharacter* InCharacter, bool bForcePlaySound = false);
};