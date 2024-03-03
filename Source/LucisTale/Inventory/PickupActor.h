#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemData.h"
#include "PickupActor.generated.h"
UCLASS()
class LUCISTALE_API APickupActor : public AActor
{
	GENERATED_BODY()

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"));
	class UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* Collider;
	
public:	
	// Sets default values for this actor's properties
	APickupActor();

	// Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (ExposeOnSpawn = "true"))
	UItemData* ItemData;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Check for player collision
	UFUNCTION()
	void BeginOverlapPlayer(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void EndOverlapPlayer(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
