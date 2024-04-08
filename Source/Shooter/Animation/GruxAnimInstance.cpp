#include "Animation/GruxAnimInstance.h"
#include "Character/ShooterEnemy.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GruxAnimInstance)

void UGruxAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (nullptr == Enemy)
	{
		Enemy = Cast<AShooterEnemy>(TryGetPawnOwner());
	}

	if (Enemy)
	{
		FVector Velocity = Enemy->GetVelocity();
		Velocity.Z = 0.f;
		Speed = Velocity.Size();
	}
	
}
