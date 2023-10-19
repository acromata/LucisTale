#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "GodIsDead/Abilities/BladeActor.h"
#include "PlayerCharacter.generated.h"

UENUM(BlueprintType)
enum EPrimaryTrigger
{
	None,
	Sword,
	Blade,
	Root,
	Heal
};

UCLASS()
class GODISDEAD_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* TargetRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* SwordMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AbilitySpawnZone;

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Input mapping context
	UPROPERTY(EditAnywhere, Category = "EnhancedInput|InputMappingContext")
	class UInputMappingContext* InputMapping;

	// Input Actions
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* OpenInventoryAction;

	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* TargetAction;

	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* HealAction;

	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* BladeAction;

protected:

	//Movement functions
	void Move(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();
	void Look(const FInputActionValue& Value);
	void Jump();

	UPROPERTY(EditAnywhere, Category = "Movement")
	float WalkSpeed;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float SprintSpeed;
	UPROPERTY(BlueprintReadWrite)
	bool bIsJumping;

	bool bIsRunning;
	bool bCanMove;

	// Primary Fire
	void PrimaryFire();
	UPROPERTY(BlueprintReadWrite)
	TEnumAsByte<EPrimaryTrigger> PrimaryTriggerEnum;
	EPrimaryTrigger PrimaryTrigger;
	EPrimaryTrigger LastPrimaryValue;

	// Stanima
	UFUNCTION()
	void UpdateStanima();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MaxStanima;
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float CurrentStanima;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float StanimaDrainTime;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float StanimaRefillTime;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float RefillStanimaDelay;

	float CurrentRefillStanimaDelay;
	bool bHasStanima;

	// Health
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	int32 MaxHealth;
	UPROPERTY(BlueprintReadWrite, Category = "Health")
	int32 CurrentHealth;

	// Interact
	UFUNCTION()
	void Interact();

	// Pickup
	UFUNCTION()
	void Pickup();

	// Inventory
	UFUNCTION(BlueprintImplementableEvent)
	void OnItemPickup(class UItemData* DataOfItemAdded);
	UPROPERTY(BlueprintReadOnly)
	TArray<class UItemData*> ItemsInInventory;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int InventoryItemLimit;
	UPROPERTY(BlueprintReadWrite)
	class UItemData* EquippedItemData;

	// Attacking
	void Attack();

	UFUNCTION(BlueprintCallable)
	void StopAttack();

	UFUNCTION()
	void OnAttackCombo(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
	UFUNCTION(BlueprintCallable)
	void AttackTrace();

	UPROPERTY(BlueprintReadOnly)
	bool bIsAttacking;
	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* AttackAnimation;
	UPROPERTY()
	TArray<class UHealthComponent*> ActorsHit;

	bool bIsBufferingAttack;

	// Lock on actor
	void TargetActor();
	void OnTargettingActor();
	void StopTarget();

	UFUNCTION()
	void BeginOverlapTarget(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void EndOverlapTarget(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY()
	TArray<AActor*> TargetsInRange;
	UPROPERTY(EditAnywhere, Category = "Targetting")
	float TargetMaxDistance;

	int TargetNum;
	UPROPERTY(BlueprintReadOnly)
	bool bIsTargetting;

	// Blade ability
	void SpawnBlades();
	void ThrowBlades();

	UPROPERTY(EditAnywhere, Category = "Abilities|Blade")
	int32 BladesToSpawn;
	UPROPERTY(EditAnywhere, Category = "Abilities|Blade")
	TSubclassOf<ABladeActor> BladeActor;
	UPROPERTY(BlueprintReadOnly, Category = "Abilities|Blade")
	TArray<ABladeActor*> BladesSpawned;

public:

	// Pickup
	UPROPERTY(BlueprintReadWrite)
	TArray<class APickupActor*> PickupsInRange;

	// Target
	UPROPERTY(BlueprintReadOnly)
	AActor* TargettedActor;
};
