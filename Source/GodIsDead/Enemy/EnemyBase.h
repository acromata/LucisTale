#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GodIsDead/Components/HealthComponent.h"
#include "EnemyBase.generated.h"

UCLASS()
class GODISDEAD_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* SwordMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UHealthComponent* HealthComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Root enemy
	void Root();

protected:

	// Root
	void EndRoot();

	UPROPERTY(BlueprintReadOnly)
	bool bIsRooted;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Root")
	float RootTime;

	// Attacking
	UFUNCTION(BlueprintCallable)
	void Attack();
	UFUNCTION(BlueprintCallable)
	void StopAttack();

	UPROPERTY(BlueprintReadOnly)
	bool bIsAttacking;
	bool bHasDamagedPlayer;

	// Damage
	UPROPERTY(EditAnywhere, Category = "Values")
	float Damage;

	// Stun
	void Stun();
	void EndStun();

	UPROPERTY(BlueprintReadOnly)
	bool bIsStunned;
	UPROPERTY(EditAnywhere, Category = "Values")
	float StunTime;
	UPROPERTY(EditAnywhere, Category = "SFX")
	USoundBase* StunSound;

};
