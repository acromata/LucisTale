#include "GodIsDead/Abilities/RootActor.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GodIsDead/Player/PlayerCharacter.h"
#include "GodIsDead/Components/HealthComponent.h"

// Sets default values
ARootActor::ARootActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>("Collider");
	SphereComponent->SetupAttachment(RootComponent);

	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>("BladeMesh");
	RootMesh->SetupAttachment(SphereComponent);

	TravelSpeed = 50.f;
	LifeTime = 5.f;
}

// Called when the game starts or when spawned
void ARootActor::BeginPlay()
{
	Super::BeginPlay();
	
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ARootActor::OnOverlapBegin);

	// Disable collison
	SetActorEnableCollision(false);
}

// Called every frame
void ARootActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsFree)
	{
		// Enable collision
		SetActorEnableCollision(true);

		// Detatch from player
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		// Move forward
		SetActorLocation(GetActorLocation() + (GetActorForwardVector() * TravelSpeed));

		// Destroy after timer
		FTimerHandle DieTimer;
		GetWorld()->GetTimerManager().SetTimer(DieTimer, this, &ARootActor::Die, LifeTime, false);
	}
}

void ARootActor::Die()
{
	Destroy();
}

void ARootActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UHealthComponent* HealthComponent = OtherActor->FindComponentByClass<UHealthComponent>();
	if (IsValid(HealthComponent))
	{
		Die();
	}
}

void ARootActor::SetTarget(AActor* Target)
{
	TargettedActor = Target;

	if (IsValid(TargettedActor))
	{
		FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargettedActor->GetActorLocation());
		SetActorRotation(TargetRotation);
	}
}

