#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

/** forward declarations */
class UBlackboardComponent;
class UBehaviorTreeComponent;

/**
 * 
 */
UCLASS()
class SHOOTER_API AEnemyController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyController();
	virtual void OnPossess(APawn* InPawn) override;

private:
	/** Blackboard component for this enemy */
	UPROPERTY(BlueprintReadWrite, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBlackboardComponent> BlackboardComponent;

	/** Behavior tree component for this enemy */
	UPROPERTY(BlueprintReadWrite, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

public:
	FORCEINLINE UBlackboardComponent* GetBlackboardComponent() const { return BlackboardComponent; }
	FORCEINLINE UBehaviorTreeComponent* GetBehaviorTreeComponent() const { return BehaviorTreeComponent; }
};
