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

	// Collision
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	AActor* TargettedActor;

	UPROPERTY(EditDefaultsOnly, Category = "BladeValues")
	float BladeSpeed;
	UPROPERTY(EditDefaultsOnly, Category = "BladeValues")
	float LifeTime;
	UPROPERTY(EditDefaultsOnly, Category = "BladeValues")
	int Damage;

	// Is free?
	bool bIsFree;

	// Camera rotation
	FRotator BladeRotation;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Set target
	void FaceTarget(AActor* Target);
	void SetRotation(FRotator Rotation);

	void ThrowBlade();

};
