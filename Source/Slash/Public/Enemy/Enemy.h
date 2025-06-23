// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/HitInterface.h"
#include "Characters/CharacterType.h"
#include "Enemy.generated.h"

class UAnimMontage;
class UAttributeComponent;
class UHealthBarComponent;
class UPawnSensingComponent;

UCLASS()
class SLASH_API AEnemy : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	AEnemy();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void DirectionalHitReact(const FVector& ImpactPoint);
	/* 타격 받을시 */
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;
	/* 이 액터가 피해를 받을 때 호출되는 함수*/
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;
	void PlayHitReactMontage(const FName& SectionName); // 피격 몽타주 재생
	void Die();
	void MoveToTarget(AActor* Target);
	bool InTargetRange(AActor* Target, double Radius); // 특정 반경에 타깃의 범위 내에있다면 true
	AActor* ChoosePatrolTarget();

	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);
	
	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose = EDeathPose::EDP_Alive;

private:
	/* Components */
	
	UPROPERTY(VisibleAnywhere)
	class UAttributeComponent* Attributes;
	
	UPROPERTY(VisibleAnywhere)
	class UHealthBarComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;
	
	/* Animation 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* HitReactMontage;

	/* Animation 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = Sounds)
	USoundBase* HitSound;

	UPROPERTY(EditDefaultsOnly, Category = VisualEffects)
	UParticleSystem* HitParticles;

	UPROPERTY()
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere)
	double CombatRadius = 500.f;

	UPROPERTY(EditAnywhere)
	double AttackRadius = 150.f;
	
	UPROPERTY(EditAnywhere)
	double PatrolRadius = 200.f;
	
	/* Navigation */
	// 현재 순찰 대상
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation", BlueprintReadOnly, meta =(AllowPrivateAccess = "true"))
	AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY()
	class AAIController* EnemyController;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMin = 5.f;
	
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMax = 10.f;

	EEnemyState EnemyState = EEnemyState::EES_Patrolling;
	
	FTimerHandle PatrolTimer;
	void PatrolTimerFinished();
	void CheckCombatTarget();
	void CheckPatrolTarget();

};
