// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/HitInterface.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UAnimMontage;
class UAttributeComponent;

UCLASS()
class SLASH_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

protected:
	virtual void BeginPlay() override;
	virtual void Attack();
	virtual void Die();
	virtual void PlayHitAttackMontage();
	virtual void PlayHitReactMontage(const FName& SectionName); // 피격 몽타주 재생
	virtual void HandleDamage(float DamageAmount);
	virtual bool CanAttack();
	
	void PlayAttackMontage();
	void DirectionalHitReact(const FVector& ImpactPoint);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	
	bool IsAlive();

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();
	
	
	UPROPERTY(VisibleInstanceOnly, Category = "무기")
	AWeapon* EquippedWeapon;

	/* ==== Animation 몽타주 ==== */
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* AttackMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* HitReactMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TArray<FName> AttackMontageSection;
	/* ==== Animation 몽타주 ==== */
	
	/**
	 * Components
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UAttributeComponent* Attribute;

private:
	UPROPERTY(EditDefaultsOnly, Category = Sounds)
	USoundBase* HitSound;

	UPROPERTY(EditDefaultsOnly, Category = VisualEffects)
	UParticleSystem* HitParticles;
};
