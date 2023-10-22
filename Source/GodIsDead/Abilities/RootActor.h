#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RootActor.generated.h"

UCLASS()
class GODISDEAD_API ARootActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* RootMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* SphereComponent;
	
public:	
	// Sets default values for this actor's properties
	ARootActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Destroy
	void Die();

	// Collision
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditDefaultsOnly, Category = "BladeValues")
	float TravelSpeed;
	UPROPERTY(EditDefaultsOnly, Category = "BladeValues")
	float LifeTime;

	AActor* TargettedActor;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Set target
	void SetTarget(AActor* Target);

	// Is free
	bool bIsFree;

};
