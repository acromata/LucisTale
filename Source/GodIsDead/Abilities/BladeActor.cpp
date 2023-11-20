#include "GodIsDead/Abilities/BladeActor.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GodIsDead/Player/PlayerCharacter.h"
#include "GodIsDead/Components/HealthComponent.h"
#include "GodIsDead/Enemy/EnemyBase.h"
#include "NiagaraComponent.h"

// Sets default values
ABladeActor::ABladeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>("Collider");
	SphereComponent->SetupAttachment(RootComponent);

	BladeMesh = CreateDefaultSubobject<UStaticMeshComponent>("BladeMesh");
	BladeMesh->SetupAttachment(SphereComponent);

	Trail = CreateDefaultSubobject<UNiagaraComponent>("BladeTrail");
	Trail->SetupAttachment(BladeMesh);
	Trail->SetRelativeRotation(FRotator(180, 0, 0));
	Trail->bAutoActivate = false;

	BladeSpeed = 50.f;
	LifeTime = 5.f;
}

// Called when the game starts or when spawned
void ABladeActor::BeginPlay()
{
	Super::BeginPlay();
	
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ABladeActor::OnOverlapBegin);

	// Disable collision
	SetActorEnableCollision(false);
}

void ABladeActor::Die()
{
	Destroy();
}

void ABladeActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Collided with health actor
	UHealthComponent* HealthComponent = OtherActor->FindComponentByClass<UHealthComponent>();
	if (IsValid(HealthComponent))
	{
		// Collided with enemy head
		AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor);
		if (IsValid(Enemy) && OtherComp == Enemy->GetHeadHitbox())
		{
			HealthComponent->SubtractHealth(1000000);
			Die();
		}
		else
		{
			HealthComponent->SubtractHealth(Damage);
			Die();
		}
	}

	// Collided with something else
	if (OtherActor != UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) && !OtherActor->IsA<ABladeActor>())
	{
		Die();
	}
}

// Called every frame
void ABladeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsFree)
	{
		// Enable collison
		SetActorEnableCollision(true);

		// Detatch from player
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		// Set rotation to 0
		SetActorRotation(BladeRotation);

		// Move forward
		SetActorLocation(GetActorLocation() + (GetActorForwardVector() * BladeSpeed));

		// Destroy after timer
		FTimerHandle DieTimer;
		GetWorld()->GetTimerManager().SetTimer(DieTimer, this, &ABladeActor::Die, LifeTime, false);
	}
}

void ABladeActor::SetRotation(FRotator Rotation)
{
	BladeRotation = Rotation;
}

void ABladeActor::ThrowBlade()
{
	bIsFree = true;
	SetLifeSpan(LifeTime);
	Trail->ActivateSystem();
}

