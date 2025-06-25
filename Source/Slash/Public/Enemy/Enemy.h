// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterType.h"
#include "Characters/BaseCharacter.h"
#include "Enemy.generated.h"

class UHealthBarComponent;
class UPawnSensingComponent;

UCLASS()
class SLASH_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();
	virtual void Tick(float DeltaTime) override;
	void CheckPatrolTarget();
	void CheckCombatTarget();
	/* 타격 받을시 */
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;
	/* 이 액터가 피해를 받을 때 호출되는 함수*/
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;
	virtual void Die() override;
	bool InTargetRange(AActor* Target, double Radius); // 특정 반경에 타깃의 범위 내에있다면 true
	void MoveToTarget(AActor* Target);
	AActor* ChoosePatrolTarget();
	virtual void Attack() override;
	virtual void HandleDamage(float DamageAmount) override;

	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);
	
	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose;

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	virtual bool CanAttack() override;

private:
	/* Components */
	
	UPROPERTY(VisibleAnywhere)
	class UHealthBarComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponClass;

	UPROPERTY(EditAnywhere)
	double CombatRadius = 500.f;

	UPROPERTY(EditAnywhere)
	double AttackRadius = 150.f;

	UPROPERTY()
	class AAIController* EnemyController;

	UPROPERTY()
	AActor* CombatTarget;
	
	/* Navigation */
	// 현재 순찰 대상
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere)
	double PatrolRadius = 200.f;

	FTimerHandle PatrolTimer;
	void PatrolTimerFinished();

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMin = 5.f;
	
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMax = 10.f;

	/* AI behavior */
    
	/**
	 * 체력바를 화면에서 숨깁니다.
	 */
	void HideHealthBar();

	/**
	 * 체력바를 화면에 표시합니다.
	 */
	void ShowHealthBar();

	/**
	 * 현재 전투 대상을 초기화하고 체력바를 숨깁니다.
	 */
	void LoseInterest();

	/**
	 * AI를 순찰 상태로 전환하고 순찰 지점으로 이동을 시작합니다.
	 * 이동 속도를 PatrollingSpeed로 설정합니다.
	 */
	void StartPatrolling();

	/**
	 * AI를 추적 상태로 전환하고 전투 대상을 추적하기 시작합니다.
	 * 이동 속도를 ChasingSpeed로 설정합니다.
	 */
	void ChaseTarget();

	/**
	 * 순찰 타이머를 초기화합니다.
	 */
	void ClearPatrolTimer();

	/**
	 * 전투 대상이 CombatRadius(500.f) 밖에 있는지 여부를 판단합니다.
	 * @return 전투 대상이 CombatRadius(500.f) 밖에 있는지 여부
	 */
	bool IsOutsideCombatRadius();

	/**
	 * 전투 대상이 AttackRadius(150.f) 밖에 있는지 판단합니다.
	 * @return 전투 대상이 AttackRadius(150.f) 밖에 있는지 여부
	 */
	bool IsOutsideAttackRadius();

	/**
	 * 전투 대상이 AttackRadius(150.f) 안에 있는지 판단합니다.
	 * @return 전투 대상이 AttackRadius(150.f) 안에 있는지 여부
	 */
	bool IsInsideAttackRadius();

	/**
	 * 현재 추적 상태(EES_Chasing)인지 여부를 판단합니다.
	 * @return 현재 추적 상태(EES_Chasing)인지 여부
	 */
	bool IsChasing();

	/**
	 * 현재 공격 상태(EES_Attacking)인지 여부를 판단합니다.
	 * @return 현재 공격 상태(EES_Attacking)인지 여부
	 */
	bool IsAttacking();

	/**
	 * AI가 대상을 추적할 수 있는 상태인지 여부를 판단합니다.
	 * @return AI가 대상을 추적할 수 있는 상태인지 여부
	 * @note 죽지 않았고, 추적 중이 아니며, 공격 중이 아닌 상태여야 함
	 */
	bool CanChaseTarget();

	/**
	 * 주어진 대상이 플레이어인지 여부를 판단합니다.
	 * @param Target 확인할 Pawn
	 * @return 주어진 대상이 플레이어인지 여부
	 */
	bool IsTargetPlayer(const APawn* Target);

	/**
	 * AI가 사망 상태(EES_Dead)인지 여부를 판단합니다.
	 * @return AI가 사망 상태(EES_Dead)인지 여부
	 */
	bool IsDead();

	/**
	 * AI가 전투 참여 상태(EES_Engaged)인지 여부를 판단합니다.
	 * @return AI가 전투 참여 상태(EES_Engaged)인지 여부
	 * @note 죽지 않았고, 공격 중이 아닌 상태여야 함
	 */
	bool IsEngaged();

	/* 전투 */
    
	/**
	 * 공격 상태로 전환하고 AttackMin~Max 사이의 랜덤한 시간 후 공격을 실행하는 타이머를 설정합니다.
	 */
	void StartAttackTimer();

	/**
	 * 현재 실행 중인 공격 타이머를 취소합니다.
	 */
	void ClearAttackTimer();

	
	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = Combat)
	float AttackMin = 0.5f;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	float AttackMax = 1.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float PatrollingSpeed = 125.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ChasingSpeed = 300.f;
};
