#include "GodIsDead/Enemy/EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "GodIsDead/Player/PlayerCharacter.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// Sword mesh
	SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>("SwordMesh");
	SwordMesh->SetupAttachment(GetMesh(), "RightHand");

	HealthComponent = CreateDefaultSubobject<UHealthComponent>("Health Component");

	// Stun
	StunTime = 5.f;
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemyBase::Root()
{
	FTimerHandle RootTimer;
	
	if (!bIsRooted)
	{
		bIsRooted = true;

		GetWorld()->GetTimerManager().SetTimer(RootTimer, this, &AEnemyBase::EndRoot, RootTime, false);
	}
}

void AEnemyBase::EndRoot()
{
	bIsRooted = false;
}


// Trace that does damage, called in Attack Animation Montage
void AEnemyBase::Attack()
{
	FVector StartLocation = SwordMesh->GetSocketLocation("Start");
	FVector EndLocation = SwordMesh->GetSocketLocation("End");

	FHitResult HitResult;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	// Trace
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, CollisionParams);

	// On hit
	if (bHit)
	{
		APlayerCharacter* Player = Cast<APlayerCharacter>(HitResult.GetActor());

		if (IsValid(Player))
		{
			if (Player->bIsParrying)
			{
				Stun();
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "stun call");
			}
			else if (!bHasDamagedPlayer)
			{
				Player->SubtractHealth(Damage);
				bHasDamagedPlayer = true;
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "no stun");
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "no player");
		}
	}

	// Debug
	 DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1, 0, 1); 
}

void AEnemyBase::StopAttack()
{
	bHasDamagedPlayer = false;
}

void AEnemyBase::Stun()
{
	FTimerHandle StunTimer;

	if (!bIsStunned)
	{
		bIsStunned = true;

		GetWorld()->GetTimerManager().SetTimer(StunTimer, this, &AEnemyBase::EndStun, StunTime, false);

		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "stun");
	}
}

void AEnemyBase::EndStun()
{
	bIsStunned = false;
}

