#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class GODISDEAD_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* PickupRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* SwordMesh;

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

	// Stanima
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


	// Interact
	void Interact();

	// Pickup
	void Pickup();

	// Inventory
	UFUNCTION(BlueprintImplementableEvent)
	void OnItemPickup(class UItemData* DataOfItemAdded);
	UPROPERTY(BlueprintReadOnly)
	TArray<class UItemData*> ItemsInInventory;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int InventoryItemLimit;

	// Attacking
	void Attack();
	UFUNCTION()
	void OnAttackCombo(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
	UFUNCTION(BlueprintCallable)
	void AttackTrace();
	UPROPERTY(BlueprintReadOnly)
	bool bIsAttacking;
	UPROPERTY(EditAnywhere)
	class UAnimMontage* AttackAnimation;
	bool bIsBufferingAttack;

public:

	// Pickup
	UPROPERTY(BlueprintReadWrite)
	class APickupActor* PickupInRange;
};
