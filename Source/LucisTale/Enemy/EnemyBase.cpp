#include "LucisTale/Enemy/EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../Player/PlayerCharacter.h"
#include "../Inventory/ItemData.h"
#include "AIController.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Mesh
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// Capsule Component
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	// Head
	HeadCollider = CreateDefaultSubobject<USphereComponent>("Head Hitbox");
	HeadCollider->SetupAttachment(GetMesh());

	// Sword mesh
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>("WeaponMesh");
	WeaponMesh->SetupAttachment(GetMesh(), "RightHand");
	WeaponMesh->SetCollisionProfileName("NoCollision");

	// Health comp
	HealthComponent = CreateDefaultSubobject<UHealthComponent>("HealthComponent");

	// Pawn sensing
	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>("PawnSensing");
	PawnSensing->SetPeripheralVisionAngle(70.f);

	// Stun
	StunTime = 5.f;

	// Root
	StunTime = 2.5f;

	// States
	LastStumbleIndex = 0;

	// Attacking
	AttackingDistance = 300.f;
	DamageMultiplier = 1.0;
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	// State default
	ActiveState = EEnemyState::EnemyIdle;

	// Set player as target
	Target = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	// Pawn Sensing Bindings
	PawnSensing->OnSeePawn.AddDynamic(this, &AEnemyBase::OnSeePawn);
	PawnSensing->OnHearNoise.AddDynamic(this, &AEnemyBase::OnHearNoise);

	// Set Item Mesh
	if (IsValid(EquippedItemData))
	{
		WeaponMesh->SetStaticMesh(EquippedItemData->ItemMesh);
	}
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckState();
}

#pragma region PawnSensing

void AEnemyBase::OnSeePawn(APawn* Pawn)
{
	if (Pawn == UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		bCanSeePlayer = true;
	}
}

void AEnemyBase::OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
	// Investigate noise
	LocationToInvestigate = Location;
	bCanHearNoise = true;
}

#pragma endregion

#pragma region States

void AEnemyBase::CheckState()
{
	switch (ActiveState)
	{
	case EEnemyState::EnemyStun:
		StateStun();
		break;
	case EEnemyState::EnemyIdle:
		StateIdle();
		break;
	case EEnemyState::EnemyChase:
		StateChase();
		break;
	case EEnemyState::EnemyAttack:
		StateAttack();
		break;
	case EEnemyState::EnemyInvestigate:
		StateInvestigate();
		break;
	}

	// Print state
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Active State: %s"), *UEnum::GetValueAsString<EEnemyState>(ActiveState)));
}

void AEnemyBase::SetState(EEnemyState NewState)
{
	ActiveState = NewState;
}

// Idle state
void AEnemyBase::StateIdle()
{
	if (bCanSeePlayer)
	{
		// Chase player
		SetState(EEnemyState::EnemyChase);
	}
	else if (bCanHearNoise)
	{
		bCanHearNoise = false;

		// Investiage noise
		SetState(EEnemyState::EnemyInvestigate);
	}
}

// Chase player
void AEnemyBase::StateChase()
{
	// Get distance between target and self
	float DistanceFromTarget = FVector::Distance(Target->GetActorLocation(), GetActorLocation());

	if (DistanceFromTarget <= AttackingDistance)
	{
		// Attack if close enough
		if (!bIsAttacking && !bIsStumbling)
		{
			SetState(EEnemyState::EnemyAttack);
		}
	}
	else
	{
		// Chase target
		AAIController* AIController = Cast<AAIController>(Controller);
		if (IsValid(AIController) && !AIController->IsFollowingAPath())
		{
			AIController->MoveToActor(Target);
		}
	}
}

void AEnemyBase::StateStun()
{
	// Can't move or attack
}

void AEnemyBase::StateInvestigate()
{
	// Go to noise
	AAIController* AIController = Cast<AAIController>(Controller);
	if (IsValid(AIController) && !AIController->IsFollowingAPath() && LocationToInvestigate != FVector(0))
	{
		AIController->MoveToLocation(LocationToInvestigate);
	}
}

#pragma endregion

#pragma region Attack

// Attacking
void AEnemyBase::StateAttack()
{
	if (AttackAnimations.IsValidIndex(AttackAnimations.Num() - 1) && !bIsAttacking)
	{
		int32 RandomIndex = FMath::RandRange(0, AttackAnimations.Num() - 1);
		UAnimMontage* AttackAnim = AttackAnimations[RandomIndex]; // Get a random number from array

		if (IsValid(AttackAnim))
		{
			GetMesh()->GetAnimInstance()->Montage_Play(AttackAnim);
			bIsAttacking = true;
		}
	}
}

// Trace that does damage, called in Attack Animation Montage
void AEnemyBase::Attack()
{
	FVector StartLocation = WeaponMesh->GetSocketLocation("Start");
	FVector EndLocation = WeaponMesh->GetSocketLocation("End");

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
			if (!bHasDamagedPlayer)
			{
				if (Player->bIsParrying)
				{
					StunTime = Player->StunTime;
					Stun();
					bHasDamagedPlayer = true;
				}
				else
				{
					if (IsValid(EquippedItemData))
					{
						int32 Damage = EquippedItemData->ItemValue * DamageMultiplier;
						Player->SubtractHealth(Damage);
						bHasDamagedPlayer = true;
					}
				}
			}
		}
	}
}

void AEnemyBase::StopAttack()
{
	bHasDamagedPlayer = false;
	bIsAttacking = false;

	if (!bIsStunned && !bIsRooted)
	{
		SetState(EEnemyState::EnemyChase);
	}
	else
	{
		SetState(EEnemyState::EnemyStun);
	}
}

#pragma endregion

#pragma region Stun
void AEnemyBase::Stun()
{
	if (!bIsStunned)
	{
		// Stun
		SetState(EEnemyState::EnemyStun);
		bIsStunned = true;

		// Play SFX
		UGameplayStatics::PlaySound2D(GetWorld(), StunSound);

		// Unstun
		FTimerHandle StunTimer;
		GetWorld()->GetTimerManager().SetTimer(StunTimer, this, &AEnemyBase::EndStun, StunTime, false);
	}
}

void AEnemyBase::EndStun()
{
	bIsStunned = false;
	SetState(EEnemyState::EnemyChase);

	bHasDamagedPlayer = false;
}

#pragma endregion

#pragma region Root
void AEnemyBase::Root()
{
	FTimerHandle RootTimer;

	if (!bIsRooted)
	{
		// Root
		bIsRooted = true;
		SetState(EEnemyState::EnemyStun);

		GetWorld()->GetTimerManager().SetTimer(RootTimer, this, &AEnemyBase::EndRoot, RootTime, false);
	}
}

USphereComponent* AEnemyBase::GetHeadHitbox()
{
	return HeadCollider;
}

void AEnemyBase::EndRoot()
{
	bIsRooted = false;
	if (bCanSeePlayer)
	{
		SetState(EEnemyState::EnemyChase);
	}
	else
	{
		SetState(EEnemyState::EnemyIdle);
	}
}
#pragma endregion
