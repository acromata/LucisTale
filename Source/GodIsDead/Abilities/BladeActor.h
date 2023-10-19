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

	AActor* TargettedActor;

	UPROPERTY(VisibleAnywhere, Category = "BladeValues")
	float BladeSpeed;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Set target
	void SetTarget(AActor* Target);

	// Is free
	bool bIsFree;

};
