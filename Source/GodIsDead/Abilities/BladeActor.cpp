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

	BladeSpeed = 50.f;
	LifeTime = 5.f;
}

// Called when the game starts or when spawned
void ABladeActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABladeActor::Die()
{
	Destroy();
}

// Called every frame
void ABladeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsFree)
	{
		GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, "Threw blade");

		// Detatch from player
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		// Move forward
		SetActorLocation(GetActorLocation() + (GetActorForwardVector() * BladeSpeed));

		// Destroy after timer
		FTimerHandle DieTimer;
		GetWorld()->GetTimerManager().SetTimer(DieTimer, this, &ABladeActor::Die, LifeTime, false);
	}
}

void ABladeActor::SetTarget(AActor* Target)
{
	TargettedActor = Target;

	if(IsValid(TargettedActor))
	{
		FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargettedActor->GetActorLocation());
		SetActorRotation(TargetRotation);

		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "B");
	}
}

