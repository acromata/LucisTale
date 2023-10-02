#include "GodIsDead/Components/HealthComponent.h"
#include "GameFramework/Character.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	Health = 1;
}

void UHealthComponent::SubtractHealth(int HealthToSubtract)
{

	Health -= HealthToSubtract;

	if (Health <= 0)
	{
		Die();
	}
}

void UHealthComponent::Die()
{
	class ACharacter* EnemyCharacter = Cast<ACharacter>(GetOwner());
	if (IsValid(DeathAnimation) && IsValid(EnemyCharacter))
	{
		// Play animation
		EnemyCharacter->GetMesh()->GetAnimInstance()->Montage_Play(DeathAnimation);
	}
	else
	{
		// Animation is null, destroy actor
		GetOwner()->Destroy();
	}
}

