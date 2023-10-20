#include "GodIsDead/Player/PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GodIsDead/Inventory/PickupActor.h"
#include "DrawDebugHelpers.h"
#include "GodIsDead/Components/HealthComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Camera spring arm
	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>("Spring Arm");
	CameraSpringArm->SetupAttachment(RootComponent);
	CameraSpringArm->bUsePawnControlRotation = true;
	CameraSpringArm->TargetArmLength = 420.f;

	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(CameraSpringArm);

	// Sword mesh
	SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>("SwordMesh");
	SwordMesh->SetupAttachment(GetMesh(), "RightHand");

	// Lock on target range
	TargetRange = CreateDefaultSubobject<USphereComponent>("LockOnTargetRange");
	TargetRange->SetupAttachment(RootComponent);

	// Ability spawn zone
	AbilitySpawnZone = CreateDefaultSubobject<USphereComponent>("AbilitySpawnZone");
	AbilitySpawnZone->SetupAttachment(RootComponent);

	// Movement
	WalkSpeed = 600.f;
	SprintSpeed = 800.f;
	MaxStanima = 500.f;
	StanimaDrainTime = 1.f;
	StanimaRefillTime = 2.f;
	RefillStanimaDelay = 1.f;
	bCanMove = true;

	// Health
	MaxHealth = 3;

	// Primary trigger
	PrimaryTrigger = EPrimaryTrigger::Sword;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Values
	CurrentStanima = MaxStanima;
	CurrentHealth = MaxHealth;
	CurrentRefillStanimaDelay = RefillStanimaDelay;

	// Delagate/ Bindings
	GetMesh()->GetAnimInstance()->OnPlayMontageNotifyBegin.AddDynamic(this, &APlayerCharacter::OnAttackCombo);

	TargetRange->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::BeginOverlapTarget);
	TargetRange->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::EndOverlapTarget);
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateStanima();

	OnTargettingActor();
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Input mapping context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem
			<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMapping, 0);
		}
	}

	// Input
	if (UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Jump);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Triggered, this, &APlayerCharacter::StartSprint);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopSprint);
		
		EnhancedInput->BindAction(InteractAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Interact);

		EnhancedInput->BindAction(AttackAction, ETriggerEvent::Triggered, this, &APlayerCharacter::PrimaryFire);

		EnhancedInput->BindAction(TargetAction, ETriggerEvent::Triggered, this, &APlayerCharacter::TargetActor);

		EnhancedInput->BindAction(BladeAction, ETriggerEvent::Triggered, this, &APlayerCharacter::SpawnBlades);
	}
}

#pragma region Movement
// Move the player
void APlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D InputVector = Value.Get<FVector2D>();

	if (IsValid(Controller) && bCanMove)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// Add movement
		AddMovementInput(ForwardDirection, InputVector.Y);
		AddMovementInput(RightDirection, InputVector.X);
	}
}

// Move 3rd person camera
void APlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D InputVector = Value.Get<FVector2D>();

	if (IsValid(Controller))
	{
		AddControllerYawInput(InputVector.X);
		AddControllerPitchInput(InputVector.Y);
	}
}

// Jump
void APlayerCharacter::Jump()
{

	StopSprint();
	StopAttack();
	bIsJumping = true;
	ACharacter::Jump();
}

// Start sprinting
void APlayerCharacter::StartSprint()
{
	if (bHasStanima && !bIsJumping)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;

		// Running is not true if stationary, include this so stanima is not drained
		if (GetVelocity().Size() >= 0.5f)
		{
			bIsRunning = true;
		}
		else
		{
			bIsRunning = false;
		}
	}
}

// Stop sprinting
void APlayerCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	bIsRunning = false;
}

// Refill and drain stanima, disable sprint if stanima empty
void APlayerCharacter::UpdateStanima()
{
	// Drain stanima
	if (bIsRunning && !bIsJumping)
	{
		CurrentStanima -= StanimaDrainTime;
		CurrentRefillStanimaDelay = RefillStanimaDelay;
	}

	// Refill stanima
	if (!bIsRunning && CurrentStanima < MaxStanima)
	{
		CurrentRefillStanimaDelay--;
		if (CurrentRefillStanimaDelay <= 0)
		{
			CurrentStanima += StanimaRefillTime;
		}
	}

	// Check if you have stanima, if the player uses all their stanima, they shouldn't sprint until it is refilled
	if (CurrentStanima <= 0)
	{
		bHasStanima = false;
		StopSprint();
	}
	else if (CurrentStanima == MaxStanima)
	{
		bHasStanima = true;
	}

	// Don't let the stanima go over it's max limit
	if (CurrentStanima > MaxStanima)
	{
		CurrentStanima = MaxStanima;
	}
}

#pragma endregion

#pragma region Interact

// Interact with objects
void APlayerCharacter::Interact()
{
	// If interactable is a  pickup object
	if (PickupsInRange.Num() > 0)
	{
		Pickup();
	}
}

// Pickup items
void APlayerCharacter::Pickup()
{
	// Get first pickup in array
	class APickupActor* PickupInRange = PickupsInRange[0];

	// Get item data
	if (IsValid(PickupInRange))
	{
		class UItemData* ItemData = PickupInRange->GetItemData();

		if (IsValid(ItemData))
		{
			// Add item to inventory
			if (ItemsInInventory.Num() < InventoryItemLimit)
			{
				ItemsInInventory.Add(ItemData);
				OnItemPickup(ItemData);

				// Remove from array and destroy
				PickupsInRange.Remove(PickupInRange);
				PickupInRange->Destroy();
			}
			else
			{
				// Inventory is full function here
			}

		}
	}
}

#pragma endregion

// Check what should happen when you press the attack button
void APlayerCharacter::PrimaryFire()
{
	switch (PrimaryTrigger)
	{
		case EPrimaryTrigger::Sword:
			Attack();
			break;
		case EPrimaryTrigger::Blade:
			ThrowBlades();
			break;
	}
}

#pragma region Attack

// Start attack animation
void APlayerCharacter::Attack()
{
	if (IsValid(AttackAnimation) && IsValid(EquippedItemData) && EquippedItemData->ItemType == ItemType::SwordType && PrimaryTrigger == EPrimaryTrigger::Sword)
	{
		if (!bIsAttacking && !bIsJumping)
		{
			GetMesh()->GetAnimInstance()->Montage_Play(AttackAnimation);
			bIsAttacking = true;
			bCanMove = false;
		}
		else
		{
			bIsBufferingAttack = true;
		}
	}
}

void APlayerCharacter::StopAttack()
{
	GetMesh()->GetAnimInstance()->Montage_Stop(0.2f, AttackAnimation);

	bIsAttacking = false;
	bIsBufferingAttack = false;
	bCanMove = true;

	ActorsHit.Empty();
}

// If not buffering, stop attack
void APlayerCharacter::OnAttackCombo(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == "NewAttackCombo")
	{
		if (!bIsBufferingAttack)
		{
			StopAttack();
		}
		else
		{
			bIsBufferingAttack = false;
			ActorsHit.Empty();
		}
	}
	else if (NotifyName == "AttackEnd") // Reset values on attack end
	{
		StopAttack();
	}
}

// Trace that does damage, called in Attack Animation Montage
void APlayerCharacter::AttackTrace()
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
		UHealthComponent* HealthComponent = HitResult.GetActor()->FindComponentByClass<UHealthComponent>();
		if (IsValid(HealthComponent) && !ActorsHit.Contains(HealthComponent))
		{
			HealthComponent->SubtractHealth(EquippedItemData->ItemValue);
			ActorsHit.Add(HealthComponent);
		}
	}

	// Debug
	/* DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1, 0, 1); */
}

#pragma endregion

#pragma region Target
// Target enemies
void APlayerCharacter::TargetActor()
{
	// Target enemies if not already targetting, move onto the next enemy if already locked on. When there are no more enemies, unlock
	if (!bIsTargetting)
	{
		if (TargetsInRange.Num() > 0)
		{
			TargetNum = 0;
			TargettedActor = TargetsInRange[0];

			bIsTargetting = true;
		}
	}
	else
	{
		TargetNum++;
		if (TargetsInRange.IsValidIndex(TargetNum) && IsValid(TargetsInRange[TargetNum]))
		{
			TargettedActor = TargetsInRange[TargetNum];
		}
		else
		{
			StopTarget();
		}
	}
}

// Lock on the target
void APlayerCharacter::OnTargettingActor()
{
	if (bIsTargetting)
	{
		if (IsValid(TargettedActor))
		{
			// Rotate player towards the target
			FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargettedActor->GetActorLocation());
			SetActorRotation(FRotator(0, NewRotation.Yaw, 0));

			// Move camera spring arm to medium point of target and player
			FVector PlayerTargetMidpoint = FMath::Lerp(GetActorLocation(), TargettedActor->GetActorLocation(), .5f);
			CameraSpringArm->SetWorldLocation(PlayerTargetMidpoint);

			// Adjust spring arm distance accordingly
			float NewSpringArmLength = FVector::Distance(PlayerTargetMidpoint, GetActorLocation());
			CameraSpringArm->TargetArmLength = FMath::Max(NewSpringArmLength * 2.5f, 420.f);

			// If target out of range, stop targetting
			if (GetDistanceTo(TargettedActor) >= TargetMaxDistance)
			{
				StopTarget();
			}
		}
		else
		{
			// If target is invalid, stop targetting
			StopTarget();
		}
	}
}

void APlayerCharacter::StopTarget()
{
	bIsTargetting = false;
	CameraSpringArm->SetRelativeLocation(FVector(0, 0, 0));
	CameraSpringArm->TargetArmLength = 420.f;
}

void APlayerCharacter::BeginOverlapTarget(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UHealthComponent* TargetEnemy = OtherActor->FindComponentByClass<UHealthComponent>();
	if (IsValid(TargetEnemy))
	{
		TargetsInRange.Add(OtherActor);
	}
}

void APlayerCharacter::EndOverlapTarget(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UHealthComponent* TargetEnemy = OtherActor->FindComponentByClass<UHealthComponent>();
	if (IsValid(TargetEnemy))
	{
		TargetsInRange.Remove(OtherActor);
	}
}




#pragma endregion

#pragma region Abilities

// Increase spirit

void APlayerCharacter::SpawnBlades()
{
	if (PrimaryTrigger != EPrimaryTrigger::Blade)
	{	
		LastPrimaryValue = PrimaryTrigger;
		PrimaryTrigger = EPrimaryTrigger::Blade;

		// Spawn blades
		for (int32 i = 0; i < BladesToSpawn; i++)
		{
			ABladeActor* SpawnedBlade = GetWorld()->SpawnActor<ABladeActor>(BladeActor, AbilitySpawnZone->GetRelativeTransform());
			
			if (IsValid(SpawnedBlade))
			{
				// Attach to player
				SpawnedBlade->AttachToComponent(GetMesh(), FAttachmentTransformRules(
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::KeepRelative,
					EAttachmentRule::SnapToTarget, false
				), "AbilitySocket");

				// Set random location
				SpawnedBlade->SetActorLocation(SpawnedBlade->GetActorLocation() + FVector(FMath::FRandRange(20.f, -20.f)));

				// Set rotation
				SpawnedBlade->SetActorRotation(FRotator(0, GetActorRotation().Yaw, GetActorRotation().Pitch));

				// Add to array
				BladesSpawned.Add(SpawnedBlade);
			}
		}
	}
}

void APlayerCharacter::ThrowBlades()
{
	PrimaryTrigger = LastPrimaryValue;
	
	for (ABladeActor* Blade : BladesSpawned)
	{
		if (bIsTargetting)
		{
			Blade->SetTarget(TargettedActor);

			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "A");
		}

		Blade->bIsFree = true;
	}
}

// Summon Root
// Throw Root

// Charge heal
// Heal

#pragma endregion
