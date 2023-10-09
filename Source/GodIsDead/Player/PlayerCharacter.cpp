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

	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(CameraSpringArm);

	// Sword mesh
	SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>("SwordMesh");
	SwordMesh->SetupAttachment(GetMesh(), "RightHand");

	// Movement
	WalkSpeed = 600.f;
	SprintSpeed = 800.f;
	MaxStanima = 500.f;
	StanimaDrainTime = 1.f;
	StanimaRefillTime = 2.f;
	RefillStanimaDelay = 1.f;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Input mapping context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMapping, 0);
		}
	}

	// Values
	CurrentStanima = MaxStanima;
	CurrentRefillStanimaDelay = RefillStanimaDelay;

	// Delagate/ Bindings
	GetMesh()->GetAnimInstance()->OnPlayMontageNotifyBegin.AddDynamic(this, &APlayerCharacter::OnAttackCombo);
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateStanima();

	RotateTowardsTargetedActor();
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Jump);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Triggered, this, &APlayerCharacter::StartSprint);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopSprint);
		
		EnhancedInput->BindAction(InteractAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Interact);

		EnhancedInput->BindAction(AttackAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Attack);

		EnhancedInput->BindAction(TargetAction, ETriggerEvent::Triggered, this, &APlayerCharacter::TargetActor);
	}
}

#pragma region Movement
// Move the player
void APlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D InputVector = Value.Get<FVector2D>();

	if (IsValid(Controller))
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

#pragma region Attack

// Start attack animation
void APlayerCharacter::Attack()
{
	if (IsValid(AttackAnimation) && IsValid(EquippedItemData) && EquippedItemData->ItemType == ItemType::Sword)
	{
		if (!bIsAttacking && !bIsJumping)
		{
			GetMesh()->GetAnimInstance()->Montage_Play(AttackAnimation);
			bIsAttacking = true;
			UE_LOG(LogTemp, Warning, TEXT("Valid."));
		}
		else
		{
			bIsBufferingAttack = true;
			UE_LOG(LogTemp, Warning, TEXT("Buffering"));
		}
	}
}

void APlayerCharacter::StopAttack()
{
	GetMesh()->GetAnimInstance()->Montage_Stop(0.2f, AttackAnimation);
	bIsAttacking = false;
	bIsBufferingAttack = false;
	ActorsHit.Empty();
}

// If not buffering, stop attack
void APlayerCharacter::OnAttackCombo(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == "NewAttackCombo")
	{
		if (!bIsBufferingAttack)
		{
			GetMesh()->GetAnimInstance()->Montage_Stop(0.2f, AttackAnimation);
			bIsAttacking = false;
		}

		bIsBufferingAttack = false;
		ActorsHit.Empty();
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
	// Check for actors the camera sees
	TArray<FHitResult> TargetActorsHit;

	// Trace location
	FVector TraceLocation = GetActorLocation();

	// Make shape for sweep
	FCollisionShape ColShape = FCollisionShape::MakeSphere(TargetMaxDistance);

	// Debug
	DrawDebugSphere(GetWorld(), GetActorLocation(), ColShape.GetSphereRadius(), 22, FColor::Green, false, 1, 0, 1);

	// Sweep
	bool Hits = GetWorld()->SweepMultiByChannel(TargetActorsHit, TraceLocation, TraceLocation, FQuat::Identity, ECC_Visibility, ColShape);

	// Get actors to target
	TArray<AActor*> ActorsToTarget;

	if (Hits)
	{
		// Get all actors hit
		for (const FHitResult& Hit : TargetActorsHit)
		{
			UHealthComponent* Damageable = Hit.GetActor()->FindComponentByClass<UHealthComponent>();

			if (IsValid(Damageable))
			{
				// Add to array
				ActorsToTarget.Add(Hit.GetActor());
			}
		}

		// Target enemies if not already targetting, move onto the next enemy if already locked on. When there are no more enemies, unlock
		if (!bIsTargetting)
		{
			if (ActorsToTarget.Num() > 0)
			{
				TargetNum = 0;
				TargetedActor = ActorsToTarget[0];

				bIsTargetting = true;
			}
		}
		else
		{
			TargetNum++;
			if (ActorsToTarget.IsValidIndex(TargetNum) && IsValid(ActorsToTarget[TargetNum]))
			{
				TargetedActor = ActorsToTarget[TargetNum];
			}
			else
			{
				bIsTargetting = false;
			}
		}
	}
}

// Face the target
void APlayerCharacter::RotateTowardsTargetedActor()
{
	if (bIsTargetting)
	{
		if (IsValid(TargetedActor))
		{
			// Rotate towards the target
			FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetedActor->GetActorLocation());
			NewRotation.Pitch = GetControlRotation().Pitch;
			NewRotation.Roll = GetControlRotation().Roll;

			SetActorRotation(FRotator(0, NewRotation.Yaw, 0));
			GetController()->SetControlRotation(NewRotation);

			// If target out of range, stop targetting
			if (GetDistanceTo(TargetedActor) >= TargetMaxDistance)
			{
				bIsTargetting = false;
			}
		}
		else
		{
			bIsTargetting = false;
		}
	}

#pragma endregion
}

