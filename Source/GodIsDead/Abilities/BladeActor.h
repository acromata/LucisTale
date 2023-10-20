#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BladeActor.generated.h"

UCLASS()
class GODISDEAD_API ABladeActor : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BladeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* SphereComponent;

public:	
	// Sets default values for this actor's properties
	ABladeActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Destroy
	void Die();

	AActor* TargettedActor;

	UPROPERTY(EditAnywhere, Category = "BladeValues")
	float BladeSpeed;
	UPROPERTY(EditAnywhere, Category = "BladeValues")
	float LifeTime;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Set target
	void SetTarget(AActor* Target);

	// Is free
	bool bIsFree;

};
