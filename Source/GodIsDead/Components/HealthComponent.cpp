#include "GodIsDead/Components/HealthComponent.h"
#include "GameFramework/Character.h"
#include "GodIsDead/Enemy/EnemyBase.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	MaxHealth = 3;
}

void UHealthComponent::SubtractHealth(int HealthToSubtract)
{
	CurrentHealth -= HealthToSubtract;

	if (CurrentHealth <= 0)
	{
		Die();
	}
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
}

void UHealthComponent::Die()
{
	class AEnemyBase* EnemyCharacter = Cast<AEnemyBase>(GetOwner());
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