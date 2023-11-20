#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GODISDEAD_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* DeathAnimation;

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	UFUNCTION(BlueprintCallable)
	void SubtractHealth(int32 HealthToSubtract);

protected:

	// Begin play
	virtual void BeginPlay() override;

	// Death
	void Die();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxHealth;
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentHealth;
};
