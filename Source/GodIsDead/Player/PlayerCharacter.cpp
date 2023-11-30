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
#include "GodIsDead/Enemy/EnemyBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

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

	// Rotation
	bUseControllerRotationYaw = false;

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
	StanimaRefillAmount = 2.f;
	RefillStanimaDelay = 2.f;
	bCanMove = true;

	// Health
	MaxHealth = 6; // 3 hearts

	// Primary trigger
	PrimaryTrigger = EPrimaryTrigger::Sword;

	//Abilities
	bCanUseAbility = true;

	// Spirit
	MaxSpirit = 100.f;
	SpiritRefillAmount = 5.f;
	SpiritRefillDelay = 2.f;

	// Healing
	SpiritNeededToHeal = 30.f;
	AmountToHeal = 2;
	HealSpiritToSubtract = 1.f;

	// Blade
	AimingFOV = 70.f;

	// Shockwave
	ShockwaveSpiritNeeded = 40.f;
	ShockwaveSpiritToSubtract = 1.f;
	ShockwaveDamage = 10;
	ShockwaveRange = 200.f;

	// Parry
	bCanParry = true;
	ParryCooldown = 2.f;
	StunTime = 3.f;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Values
	CurrentStanima = MaxStanima;
	CurrentHealth = MaxHealth;
	CurrentRefillStanimaDelay = RefillStanimaDelay;

	CurrentSpirit = MaxSpirit;
	CurrentSpiritRefillDelay = SpiritRefillDelay;

	// Overlaps
	TargetRange->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::BeginOverlapTarget);
	TargetRange->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::EndOverlapTarget);
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateStanima();
	UpdateSpirit();
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

		EnhancedInput->BindAction(PrimaryAction, ETriggerEvent::Triggered, this, &APlayerCharacter::PrimaryButton);

		EnhancedInput->BindAction(TargetAction, ETriggerEvent::Triggered, this, &APlayerCharacter::TargetActor);

		EnhancedInput->BindAction(BladeAction, ETriggerEvent::Triggered, this, &APlayerCharacter::SpawnBlades);

		EnhancedInput->BindAction(HealAction, ETriggerEvent::Started, this, &APlayerCharacter::CheckSpiritNeededForHeal);
		EnhancedInput->BindAction(HealAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Heal);
		
		EnhancedInput->BindAction(ShockwaveAction, ETriggerEvent::Started, this, &APlayerCharacter::CheckSpiritNeededForShockwave);
		EnhancedInput->BindAction(ShockwaveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::ChargeShockwave);

		EnhancedInput->BindAction(ParryAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Parry);
	}
}

// Check what should happen when you press the attack button
void APlayerCharacter::PrimaryButton()
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

void APlayerCharacter::SubtractHealth(float Amount)
{
	CurrentHealth -= Amount;
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
			CurrentStanima += StanimaRefillAmount;
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
		class UItemData* ItemData = PickupInRange->ItemData;

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
	if (IsValid(AttackAnimation) && IsValid(EquippedItemData) && 
		EquippedItemData->ItemType == EItemType::SwordType && PrimaryTrigger == EPrimaryTrigger::Sword)
	{
		if (!bIsAttacking && !bIsJumping)
		{
			GetMesh()->GetAnimInstance()->Montage_Play(AttackAnimation);
			bIsAttacking = true;
			bCanMove = false;
			bCanUseAbility = false;
		}
		else
		{
			bIsBufferingAttack = true;
		}
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

void APlayerCharacter::StopAttack()
{
	GetMesh()->GetAnimInstance()->Montage_Stop(0.2f, AttackAnimation);

	bIsAttacking = false;
	bIsBufferingAttack = false;
	bCanMove = true;
	bCanUseAbility = true;

	ActorsHit.Empty();
}

void APlayerCharacter::OnAttackCombo()
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
	AEnemyBase* TargetEnemy = Cast<AEnemyBase>(OtherActor);
	if (IsValid(TargetEnemy))
	{
		TargetsInRange.Add(OtherActor);
	}
}

void APlayerCharacter::EndOverlapTarget(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AEnemyBase* TargetEnemy = Cast<AEnemyBase>(OtherActor);
	if (IsValid(TargetEnemy))
	{
		TargetsInRange.Remove(OtherActor);
	}
}

#pragma endregion

#pragma region Abilities

// Increase spirit
void APlayerCharacter::UpdateSpirit()
{
	if (CurrentSpirit < MaxSpirit)
	{
		CurrentSpiritRefillDelay--;
		if (CurrentSpiritRefillDelay <= 0)
		{
			CurrentSpirit += SpiritRefillAmount;
		}
	}
}

// Subtract spirit
void APlayerCharacter::DrainSpirit(float SpiritToDrain)
{
	CurrentSpirit -= SpiritToDrain;
	CurrentSpiritRefillDelay = SpiritRefillDelay;
}

void APlayerCharacter::SpawnBlades()
{
	if (PrimaryTrigger != EPrimaryTrigger::Blade && CurrentSpirit >= BladeSpiritNeeded && bCanUseAbility)
	{	
		// Subtract spirit
		DrainSpirit(BladeSpiritNeeded);

		// Set primary trigger
		LastPrimaryValue = PrimaryTrigger;
		PrimaryTrigger = EPrimaryTrigger::Blade;

		// Blade SFX
		UGameplayStatics::PlaySound2D(GetWorld(), BladeSpawnSound);

		// Blade spawn location
		FVector BladeLocation = FVector(0, 0, 0);

		// Aim
		StartAim();

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

				// Set Location
				//SpawnedBlade->SetActorLocation(SpawnedBlade->GetActorLocation() + FMath::FRandRange(20.f, -20.f));
				SpawnedBlade->SetActorRelativeLocation(BladeLocation);
				BladeLocation.X += 10;

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
	ABladeActor* Blade = BladesSpawned.Last();
	int BladeNum = BladesSpawned.Num();

	if (IsValid(Blade))
	{
		if (bIsTargetting)
		{
			Blade->SetRotation(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargettedActor->GetActorLocation()));
		}
		else
		{
			Blade->SetRotation(Camera->GetComponentRotation());
		}

		// Throw the blade
		Blade->ThrowBlade();

		// Play SFX
		UGameplayStatics::PlaySound2D(GetWorld(), BladeThrowSound[BladeNum]);

		// Remove blade from array
		BladesSpawned.Remove(Blade);

		// No more blades to throw
		if (BladesSpawned.Num() <= 0)
		{
			PrimaryTrigger = LastPrimaryValue;
			StopAim();
		}
	}
}

void APlayerCharacter::StartAim()
{
	Camera->FieldOfView = AimingFOV;
	Camera->SetRelativeLocation(AimCameraOffset);
	bUseControllerRotationYaw = true;
}

void APlayerCharacter::StopAim()
{
	Camera->FieldOfView = 90.f;
	Camera->SetRelativeLocation(FVector(0));
	bUseControllerRotationYaw = false;
}

void APlayerCharacter::CheckSpiritNeededForHeal()
{
	if (CurrentSpirit >= SpiritNeededToHeal && bCanUseAbility)
	{
		// Check how much spirit needed to subtract
		SpiritAfterHeal = CurrentSpirit - SpiritNeededToHeal;
		bCanHeal = true;
	}
}

void APlayerCharacter::Heal()
{
	if (CurrentHealth != MaxHealth && bCanHeal)
	{
		if (CurrentSpirit <= SpiritAfterHeal)
		{
			// Heal if enough spirit has been subtracted
			CurrentHealth += AmountToHeal;
			bCanHeal = false;
			bCanMove = true;
			CheckSpiritNeededForHeal();
		}
		else
		{
			DrainSpirit(HealSpiritToSubtract);

			if (bCanMove)
			{
				bCanMove = false;
			}
		}
	}
}

void APlayerCharacter::CheckSpiritNeededForShockwave()
{
	if (CurrentSpirit >= ShockwaveSpiritNeeded && bCanUseAbility)
	{
		SpiritAfterShockwave = CurrentSpirit - ShockwaveSpiritNeeded;
		bCanShockwave = true;
	}
}

void APlayerCharacter::ChargeShockwave()
{
	if (bCanShockwave)
	{
		if (CurrentSpirit <= SpiritAfterShockwave)
		{
			// Shockwave if enough spirit has been subtracted
			Shockwave();
			bCanShockwave = false;
			bCanMove = true;
		}
		else
		{
			DrainSpirit(ShockwaveSpiritToSubtract);

			if (bCanMove)
			{
				bCanMove = false;
			}
		}
	}
}

void APlayerCharacter::Shockwave()
{
	// Sweep for enemies
	TArray<FHitResult> HitResults;
	FVector SweepLocation = GetActorLocation();
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(ShockwaveRange);

	bool bIsHit = GetWorld()->SweepMultiByChannel(HitResults, SweepLocation,
		SweepLocation, FQuat::Identity, ECC_Visibility, SphereShape);

	for (FHitResult& HitResult : HitResults)
	{
		if (IsValid(HitResult.GetActor()))
		{
			UHealthComponent* HealthComponent = HitResult.GetActor()->FindComponentByClass<UHealthComponent>();
			if (IsValid(HealthComponent))
			{
				HealthComponent->SubtractHealth(ShockwaveDamage);
			}
		}
	}

	// Debug
	DrawDebugSphere(GetWorld(), GetActorLocation(), SphereShape.GetSphereRadius(), 50, FColor::Purple, false, 1.f);
}

void APlayerCharacter::Parry()
{
	if (bCanParry && !bIsAttacking && !bIsJumping)
	{
		bIsParrying = true;
		bCanMove = false;
		bCanParry = false;

		// Play anim
		GetMesh()->GetAnimInstance()->Montage_Play(ParryAnimation);

		// End parry
		FTimerHandle ParryTimer;
		GetWorld()->GetTimerManager().SetTimer(ParryTimer, this, &APlayerCharacter::EndParry, ParryTime);
	}
}

void APlayerCharacter::EndParry()
{
	bIsParrying = false;
	bCanMove = true;

	// Cooldown
	FTimerHandle ParryTimer;
	GetWorld()->GetTimerManager().SetTimer(ParryTimer, this, &APlayerCharacter::AllowParry, ParryCooldown);
}

void APlayerCharacter::AllowParry()
{
	bCanParry = true;
}

#pragma endregion
