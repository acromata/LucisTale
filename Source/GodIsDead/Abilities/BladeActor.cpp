#include "GodIsDead/Abilities/BladeActor.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GodIsDead/Player/PlayerCharacter.h"

// Sets default values
ABladeActor::ABladeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>("Collider");
	SphereComponent->SetupAttachment(RootComponent);

	BladeMesh = CreateDefaultSubobject<UStaticMeshComponent>("BladeMesh");
	BladeMesh->SetupAttachment(SphereComponent);
}

// Called when the game starts or when spawned
void ABladeActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABladeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsFree)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "Threw blade");

		// Detatch from player
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		// Set rotation
		if (IsValid(TargettedActor))
		{
			
		}

		// Move forward
		SetActorLocation(GetActorLocation() + (GetActorForwardVector() * BladeSpeed));
	}
}

void ABladeActor::SetTarget(AActor* Target)
{
	Target = TargettedActor;
}

