#include "LucisTale/Enemy/EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../Player/PlayerCharacter.h"
#include "../Inventory/ItemData.h"

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
	StunTime = 1.f;

	// Strafe
	CloseStrafeDistance = 300.f;
	FarStrafeDistance = 800.f;

	// Attacking
	DamageMultiplier = 1.0;
	MinTimeBeforeAttack = 5.f;
	MaxTimeBeforeAttack = 15.f;
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	// Set AI Controller
	AIController = Cast<AAIController>(Controller);

	// State default
	ActiveState = EEnemyState::EIdleState;

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

	UpdateState();

	// Update distance between target and self
	DistanceFromTarget = FVector::Distance(Target->GetActorLocation(), GetActorLocation());
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

void AEnemyBase::UpdateState()
{
	switch (ActiveState)
	{
	case EEnemyState::EIdleState:
		Idle();
		break;
	case EEnemyState::EInvestigateState:
		Investigate();
		break;
	case EEnemyState::EApproachState:
		Approach();
		break;
	case EEnemyState::ECloseStrafeState:
		CloseStrafe();
		break;
	case EEnemyState::EFarStrafeState:
		FarStrafe();
		break;
	case EEnemyState::EAttackingState:
		AttackingState();
		break;
	default:
		SetState(EEnemyState::EIdleState);
		break;
	}

	// Print state
	GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Green, FString::Printf(TEXT("Active State: %s"), *UEnum::GetValueAsString<EEnemyState>(ActiveState)));
}

void AEnemyBase::SetState(EEnemyState NewState)
{
	ActiveState = NewState;
}

// Idle state
void AEnemyBase::Idle()
{
	if (bCanSeePlayer)
	{
		// Chase player
		SetState(EEnemyState::EApproachState);
	}
	else if (bCanHearNoise)
	{
		bCanHearNoise = false;

		// Investiage noise
		SetState(EEnemyState::EInvestigateState);
	}
}

void AEnemyBase::Investigate()
{
	// Go to noise
	if (IsValid(AIController) && !AIController->IsFollowingAPath() && LocationToInvestigate != FVector(0))
	{
		AIController->MoveToLocation(LocationToInvestigate);
	}
}

// Approach player
void AEnemyBase::Approach()
{
	if (DistanceFromTarget <= CloseStrafeDistance)
	{
		SetState(EEnemyState::ECloseStrafeState);
	}
	else
	{
		// Approach target
		if (IsValid(AIController) && !AIController->IsFollowingAPath())
		{
			AIController->MoveToActor(Target);
		}
	}
}

void AEnemyBase::FarStrafe()
{
	if (DistanceFromTarget <= FarStrafeDistance)
	{
		if (!bIsAttackWaitTimeSet)
		{
			// Stop movement
			AIController->StopMovement();

			// Random attack times
			float WaitTime = FMath::FRandRange(MinTimeBeforeAttack, MaxTimeBeforeAttack);

			FTimerHandle FarAttackHandle;
			GetWorld()->GetTimerManager().SetTimer(FarAttackHandle, this, &AEnemyBase::CallAttack, WaitTime);
			bIsAttackWaitTimeSet = true;
		}

		if (bAwaitingAttack)
		{
			SetState(EEnemyState::EAttackingState);
			FarAttack();
		}
	}
	else
	{
		SetState(EEnemyState::EApproachState);
	}
	
}

void AEnemyBase::CloseStrafe()
{
	if (DistanceFromTarget <= CloseStrafeDistance)
	{
		if (!bIsAttackWaitTimeSet)
		{
			// Stop movement
			AIController->StopMovement();

			// Random attack times
			float WaitTime = FMath::FRandRange(MinTimeBeforeAttack, MaxTimeBeforeAttack);

			FTimerHandle CloseAttackHandle;
			GetWorld()->GetTimerManager().SetTimer(CloseAttackHandle, this, &AEnemyBase::CallAttack, WaitTime);
			bIsAttackWaitTimeSet = true;
		}

		if (bAwaitingAttack)
		{
			SetState(EEnemyState::EAttackingState);
			CloseAttack();
		}
	}
	else
	{
		SetState(EEnemyState::EApproachState);
	}
}

void AEnemyBase::AttackingState()
{
}

void AEnemyBase::FarAttack()
{
	bAwaitingAttack, bIsAttackWaitTimeSet = false;

	// Random Far Attack
	if (FarAttackAnimations.IsValidIndex(FarAttackAnimations.Num() - 1))
	{
		int32 RandomIndex = FMath::RandRange(0, FarAttackAnimations.Num() - 1);
		UAnimMontage* AttackAnim = FarAttackAnimations[RandomIndex]; // Get a random number from array

		if (IsValid(AttackAnim))
		{
			GetMesh()->GetAnimInstance()->Montage_Play(AttackAnim);
			bIsAttacking = true;
		}
	}
}

void AEnemyBase::CloseAttack()
{
	bAwaitingAttack, bIsAttackWaitTimeSet = false;

	// Random Close Attack
	if (CloseAttackAnimations.IsValidIndex(CloseAttackAnimations.Num() - 1))
	{
		int32 RandomIndex = FMath::RandRange(0, CloseAttackAnimations.Num() - 1);
		UAnimMontage* AttackAnim = CloseAttackAnimations[RandomIndex]; // Get a random number from array

		if (IsValid(AttackAnim))
		{
			GetMesh()->GetAnimInstance()->Montage_Play(AttackAnim);
			bIsAttacking = true;
		}
	}
}

#pragma endregion

#pragma region Attack

// Sets bAwaitingAttack to true
void AEnemyBase::CallAttack()
{
	if (!bIsAttacking)
	{
		bAwaitingAttack = true;
	}
}

// Trace that does damage, called in Attack Animation Montage
void AEnemyBase::AttackTrace()
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

void AEnemyBase::StopAttackTrace()
{
	bHasDamagedPlayer = false;
	bIsAttacking = false;

	if (!bIsStunned)
	{
		SetState(EEnemyState::EApproachState);
	}
}

#pragma endregion

#pragma region Stun
void AEnemyBase::Stun()
{
	if (!bIsStunned)
	{
		// Stun
		bIsStunned = true;
		StopAttackTrace();

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
	bHasDamagedPlayer = false;

	SetState(EEnemyState::EApproachState);
}

#pragma endregion

void AEnemyBase::SetTargeted(bool bIsTargetting)
{
	bIsBeingTargeted = bIsTargetting;
}