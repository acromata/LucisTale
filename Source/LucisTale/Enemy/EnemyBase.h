#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LucisTale/Components/HealthComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "EnemyBase.generated.h"


UENUM(BlueprintType)
enum class EEnemyState
{
	EIdleState,
	EInvestigateState,
	EApproachState,
	ECloseStrafeState,
	EFarStrafeState
};

UCLASS()
class LUCISTALE_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UPawnSensingComponent* PawnSensing;

public:
	// Sets default values for this character's properties
	AEnemyBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:

	// Pawn Sensing
	UFUNCTION()
	void OnSeePawn(APawn* Pawn);
	UFUNCTION()
	void OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume);

	bool bCanSeePlayer;
	bool bCanHearNoise;

	// States
	void UpdateState();
	void SetState(EEnemyState NewState);
	EEnemyState ActiveState;

	void Idle();
	void Investigate();
	void Approach();
	void CloseStrafe();
	void FarStrafe();

	// Investigate
	FVector LocationToInvestigate;

	// Strafe
	UPROPERTY(EditAnywhere, Category = "Values|Strafe")
	float FarStrafeDistance;
	UPROPERTY(EditAnywhere, Category = "Values|Strafe")
	float CloseStrafeDistance;

	// Attacking
	void CloseAttack();
	void FarAttack();
	void CallAttack();

	UFUNCTION(BlueprintCallable)
	void AttackTrace();
	UFUNCTION(BlueprintCallable)
	void StopAttackTrace();

	UPROPERTY(EditAnywhere, Category = "Values|Attack")
	float DamageMultiplier;
	UPROPERTY(EditAnywhere, Category = "Values|Attack")
	TArray<UAnimMontage*> AttackAnimations;
	UPROPERTY(EditAnywhere, Category = "Values|Attack")
	float MinTimeBeforeAttack;
	UPROPERTY(EditAnywhere, Category = "Values|Attack")
	float MaxTimeBeforeAttack;
	UPROPERTY(BlueprintReadOnly)
	bool bIsAttacking;

	bool bHasDamagedPlayer;
	bool bAwaitingAttack;
	bool bIsAttackWaitTimeSet;

	// Weapon
	UPROPERTY(EditAnywhere, Category = "Values|Attack")
	class UItemData* EquippedItemData;

	// Target
	AActor* Target;

	// Stun
	void Stun();
	void EndStun();

	UPROPERTY(BlueprintReadOnly)
	bool bIsStunned;
	UPROPERTY(EditAnywhere, Category = "SFX")
	USoundBase* StunSound;
	UPROPERTY(EditDefaultsonly, Category = "Value|Stun")
	float StunTime;

	// Targeting
	bool bIsBeingTargeted;

public:

	void SetTargeted(bool IsTargetting);
};
