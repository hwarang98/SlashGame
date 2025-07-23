// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "CharacterType.h"
#include "Interface/PickupInterface.h"
#include "SlashCharacter.generated.h"

class AItem;
class ASoul;
class ATreasure;
class AHealPotion;
class UInputAction;
class UAnimMontage;
class USlashOverlay;
class UGroomComponent;
class UCameraComponent;
class USpringArmComponent;
class UStaticMeshComponent;
class UInputMappingContext;

UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	ASlashCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	virtual void SetOverlappingItem(AItem* Item) override;
	virtual void AddSoul(ASoul* Soul) override;
	virtual void AddGold(ATreasure* ATreasure) override;
	virtual void AddHealth(AHealPotion* AHealPotion) override;

	/*  { OverlappingItem = Item; } 항목이 중첩되게 적용가능 setter 함수임
	 *  FORCEINLINE inline은 C++에서 함수의 인라인(inline) 처리를 강력하게 요청하는 언리얼 스타일의 방식
	 */
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }

protected:
	virtual void BeginPlay() override;

	/* call back Input */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	virtual void Jump() override;
	void Dodge();
	void StartWalking();
	void EKeyPressed();
	void StrongAttack();
	virtual void Attack() override;
	virtual void DodgeEnd() override;

	/* 전투관련 함수들 */
	virtual void AttackEnd() override;
	virtual bool CanAttack() override;
	virtual void Die() override;
	void PlayEquipMontage(const FName& SectionName);
	bool CanDisarm();
	bool CanArm();
	bool IsOccupied();
	bool HasDodgeEnoughStamina();
	bool HasAttackEnoughStamina();
	
	/* 입력 컨텍스트 부착 */
	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* SlashContext;
	
	/* 입력액션 부착 */
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* MovementAction;

	/* 입력액션 부착 */
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* LookingAction;

	/* 점프액션 부착 */
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* JumpAction;

	/* 걷기 부착 */
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* WalkAction;

	/* 아이탬 줍기 */
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* EKeyPressedAction;


	/* 공격 액션 부착 */
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* AttackAction;


	/* 구르기 부착 */
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* DodgeAction;

	/* 공격 액션 부착 */
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* StrongAttackAction;
	

	UFUNCTION(BlueprintCallable)
	void Disarm();

	UFUNCTION(BlueprintCallable)
	void Arm();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();
	
	UFUNCTION(BlueprintCallable)
	void HitReactEnd();
	
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY()
	USlashOverlay* SlashOverlay;

	
private:
	void SetHUDHealth();
	void SetHUDStamina();
	bool IsUnoccupied();
	void InitializeSlashOverlay();

	/* 캐릭터 컴포넌트 */
	/**
	 * 스프링암 컴포넌트
	 */
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	/**
	 * 카메라 컴포넌트
	 */
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	/**
	 * 구름컴포넌트 부착
	 */
	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* Hair;

	/**
	 * 눈썹 컴포넌트 부착
	 */
	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* Eyebrows;
	
	UPROPERTY(EditAnywhere, Category = Movement)
	float WalkSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category = Movement)
	float RunSpeed = 600.f;
	
	/* 걷기상태 true = 뛰기, false = 걷기 */
	bool bIsWalking = false;
	
	/* Animation 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* StrongAttackMontage;
	
	/* Animation 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* EquipMontage;

	/* VisibleInstanceOnly = 디테일 패널에서만 볼수있음 */
	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	/**
	 * 구름컴포넌트 부착
	 */
	UPROPERTY(VisibleAnywhere, Category = Weapon)
	UStaticMeshComponent* KatanaSheath;
	
	int32 Selection = 0;
	int32 StrongAttackSection = 0;
};
