#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LucisTale/Components/HealthComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "EnemyBase.generated.h"


UENUM(BlueprintType)
enum class EEnemyState
{
	EnemyIdle,
	EnemyChase,
	EnemyAttack,
	EnemyStun,
	EnemyInvestigate
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
	void CheckState();
	void SetState(EEnemyState NewState);
	EEnemyState ActiveState;

	void StateIdle();
	void StateChase();
	void StateAttack();
	void StateStun();
	void StateInvestigate();

	int LastStumbleIndex;

	bool bIsStumbling;

	// Attacking
	UFUNCTION(BlueprintCallable)
	void Attack();
	UFUNCTION(BlueprintCallable)
	void StopAttack();

	UPROPERTY(EditAnywhere, Category = "Attack")
	float DamageMultiplier;
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackingDistance;
	UPROPERTY(EditAnywhere, Category = "Attack")
	TArray<UAnimMontage*> AttackAnimations;

	// Weapon
	UPROPERTY(EditAnywhere, Category = "Attack")
	class UItemData* EquippedItemData;

	AActor* Target;

	// Stun
	void Stun();
	void EndStun();

	UPROPERTY(BlueprintReadOnly)
	bool bIsStunned;
	UPROPERTY(EditAnywhere, Category = "SFX")
	USoundBase* StunSound;

	float StunTime;

	// Attacking
	UPROPERTY(BlueprintReadOnly)
	bool bIsAttacking;
	bool bHasDamagedPlayer;

	// Investigate
	FVector LocationToInvestigate;

	// Targeting
	bool bIsBeingTargeted;

public:

	void SetTargeted(bool IsTargetting);
};
