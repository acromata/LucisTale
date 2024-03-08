#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "LucisTale/Abilities/BladeActor.h"
#include "PlayerCharacter.generated.h"

// keep going n make a shit ton of money from this game BITCH!!!!!!!!!!!!!!!!!!!
UCLASS()
class LUCISTALE_API APlayerCharacter : public ACharacter
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
	class UInputAction* AttackAction;

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
	class UInputAction* TargetAction;

	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* ParryAction;

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

	// Stanima
	void UpdateStanima();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MaxStanima;
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float CurrentStanima;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float StanimaDrainTime;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float StanimaRefillAmount;
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
	int32 InventoryItemLimit;
	UPROPERTY(BlueprintReadWrite)
	class UItemData* EquippedItemData;

	// Attacking
	void Attack();

	UFUNCTION(BlueprintCallable)
	void StopAttack();
	UFUNCTION(BlueprintCallable)
	void OnAttackCombo();
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
	TArray<class AEnemyBase*> TargetsInRange;
	UPROPERTY(EditAnywhere, Category = "Targetting")
	float TargetMaxDistance;
	UPROPERTY(BlueprintReadOnly)
	class AEnemyBase* TargettedActor; 
	UPROPERTY(BlueprintReadOnly)
	bool bIsTargetting;

	int32 TargetNum;

	bool bCanUseAbility;

	// Parry
	void Parry();
	void EndParry();
	void AllowParry();

	UPROPERTY(EditAnywhere, Category = "Abilities|Parry")
	float ParryTime;
	UPROPERTY(EditAnywhere, Category = "Abilities|Parry")
	float ParryCooldown;
	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* ParryAnimation;

	bool bCanParry;

public:

	// Pickup
	UPROPERTY(BlueprintReadWrite)
	TArray<class APickupActor*> PickupsInRange;

	// Heal
	void SubtractHealth(int32 Health);

	// Parry
	bool bIsParrying;
	UPROPERTY(EditAnywhere, Category = "Abilites|Parry")
	float StunTime;
};
