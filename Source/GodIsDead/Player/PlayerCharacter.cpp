#include "GodIsDead/Player/PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GodIsDead/Inventory/PickupActor.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Camera spring arm
	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	CameraSpringArm->SetupAttachment(RootComponent);
	CameraSpringArm->bUsePawnControlRotation = true;

	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraSpringArm);

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
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateStanima();
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

// Start sprinting
void APlayerCharacter::StartSprint()
{
	if (bHasStanima && !bIsRunning && !bIsJumping)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		bIsRunning = true;
	}
}

// Stop sprinting
void APlayerCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	bIsRunning = false;
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
	bIsJumping = true;
	ACharacter::Jump();
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

// Interact with objects
void APlayerCharacter::Interact()
{
	// If interactable is a  pickup object
	if (IsValid(PickupInRange))
	{
		Pickup();
	}
}

// Pickup items
void APlayerCharacter::Pickup()
{
	// Get item data
	class UItemData* ItemData = PickupInRange->GetItemData();
	if (IsValid(ItemData))
	{
		// Add item to inventory
		if (ItemsInInventory.Num() < InventoryItemLimit)
		{
			ItemsInInventory.Add(ItemData);
			OnItemPickup(ItemData);
			PickupInRange->Destroy();
		}
		else
		{
			// Inventory is full function here
		}

	}
}
