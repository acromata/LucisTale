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
	void SubtractHealth(int HealthToSubtract);

protected:

	void Die();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Health;
};
