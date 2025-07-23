// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterType.h"
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

	FORCEINLINE TEnumAsByte<EDeathPose> GetDeathPose() const { return DeathPose; }


protected:
	virtual void BeginPlay() override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	virtual void Attack();
	virtual void Die();
	virtual void HandleDamage(float DamageAmount);
	virtual bool CanAttack();
	
	virtual void PlayHitAttackMontage();
	virtual void PlayHitReactMontage(const FName& SectionName); // 피격 몽타주 재생
	virtual int32 PlayAttackMontage();
	virtual int32 PlayDeathMontage();
	virtual void PlayDodgeMontage();
	void StopAttackMontage();
	
	void DirectionalHitReact(const FVector& ImpactPoint);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
	void DisableCapsule();
	void DisableMeshCollision();
	
	bool IsAlive();

	UFUNCTION(BlueprintCallable)
	FVector GetTranslationWarpTarget();
	
	UFUNCTION(BlueprintCallable)
	FVector GetRotationWarpTarget();

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	UFUNCTION(BlueprintCallable)
	virtual void DodgeEnd();
	
	/**
	 * Components
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UAttributeComponent* Attribute;

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	AActor* CombatTarget;

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	double WarpTargetDistance = 75.f;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	USoundBase* HitSound;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UParticleSystem* HitParticles;
	
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UParticleSystem* AttackStamina;
	
	UPROPERTY(VisibleAnywhere, Category = Weapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EDeathPose> DeathPose;


private:
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);

	/* ==== Animation 몽타주 ==== */
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* AttackMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* HitReactMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TArray<FName> AttackMontageSection;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TArray<FName> DeathMontageSection;

	/* 걷기 부착 */
	UPROPERTY(EditAnywhere, Category = combat)
	UAnimMontage* DodgeMontage;
	/* ==== Animation 몽타주 ==== */
	
};
