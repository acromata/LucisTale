#include "LucisTale/Inventory/PickupActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "LucisTale/Player/PlayerCharacter.h"

// Sets default values
APickupActor::APickupActor()
{
	// Collider
	Collider = CreateDefaultSubobject<USphereComponent>("Collider");
	Collider->SetupAttachment(RootComponent);

	// Mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("ItemMesh");
	Mesh->SetupAttachment(Collider);
}

// Called when the game starts or when spawned
void APickupActor::BeginPlay()
{
	Super::BeginPlay();

	// Overlaps
	Collider->OnComponentBeginOverlap.AddDynamic(this, &APickupActor::BeginOverlapPlayer);
	Collider->OnComponentEndOverlap.AddDynamic(this, &APickupActor::EndOverlapPlayer);
}

// Set the players pick up in range to this object
void APickupActor::BeginOverlapPlayer(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsValid(OtherActor))
	{
		APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
		if (IsValid(Player))
		{
			Player->PickupsInRange.Add(this);
		}
	}
}

// Remove the pickup in range if player is not nearby
void APickupActor::EndOverlapPlayer(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsValid(OtherActor))
	{
		APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
		if (IsValid(Player))
		{
			Player->PickupsInRange.Remove(this);
		}
	}
}

