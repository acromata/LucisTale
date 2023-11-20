#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GodIsDead/Components/HealthComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "EnemyBase.generated.h"


UENUM(BlueprintType)
enum class EEnemyState
{
	EnemyIdle,
	EnemyChase,
	EnemyAttack,
	EnemyStun
};

UCLASS()
class GODISDEAD_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* HeadCollider;

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

	// States
	void CheckState();
	void SetState(EEnemyState NewState);
	EEnemyState ActiveState;

	void StateIdle();
	void StateChase();
	void StateAttack();
	void StateStun();

	int LastStumbleIndex;

	bool bIsStumbling;

	// Attacking
	UFUNCTION(BlueprintCallable)
	void Attack();
	UFUNCTION(BlueprintCallable)
	void StopAttack();

	UPROPERTY(EditAnywhere, Category = "Attack")
	float Damage;
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackingRange;
	UPROPERTY(EditAnywhere, Category = "Attack")
	UAnimMontage* AttackAnimation;

	AActor* Target;

	// Stun
	void Stun();
	void EndStun();

	UPROPERTY(BlueprintReadOnly)
	bool bIsStunned;
	UPROPERTY(EditAnywhere, Category = "SFX")
	USoundBase* StunSound;

	float StunTime;

	// Root
	void EndRoot();

	UPROPERTY(BlueprintReadOnly)
	bool bIsRooted;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Root")
	float RootTime;

	UPROPERTY(BlueprintReadOnly)
	bool bIsAttacking;
	bool bHasDamagedPlayer;

public:

	// Root enemy
	void Root();

	// Get head hitbox
	USphereComponent* GetHeadHitbox();
};
