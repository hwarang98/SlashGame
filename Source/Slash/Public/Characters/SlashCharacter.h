// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CharacterType.h"
#include "SlashCharacter.generated.h"

class AItem;
class AWeapon;
class UInputAction;
class UAnimMontage;
class UGroomComponent;
class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;

UCLASS()
class SLASH_API ASlashCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASlashCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	/*  { OverlappingItem = Item; } 항목이 중첩되게 적용가능 setter 함수임
	 *  FORCEINLINE inline은 C++에서 함수의 인라인(inline) 처리를 강력하게 요청하는 언리얼 스타일의 방식이에요.
	 */
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; };

protected:
	virtual void BeginPlay() override;
	
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


	/* 걷기 부착 */
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* DodgeAction;

	/* 공격 액션 부착 */
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* StrongAttackAction;



	/**
	 * call back Input
	 */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartWalking();
	void Jump();
	void EKeyPressed();
	// virtual void Attack() override;
	void Dodge();
	void Attack();
	void StrongAttack();

	/**
	 * play montage functions
	 */
	void PlayAttackMontage();
	void PlayEquipMontage(const FName& SectionName);

	UFUNCTION(BlueprintCallable)
	void AttackEnd();
	bool CanAttack();
	/* 무장해제 가능 여부 */
	bool CanDisarm();
	/* 무장장착 가능 여부 */
	bool CanArm();

	UFUNCTION(BlueprintCallable)
	void Disarm();

	UFUNCTION(BlueprintCallable)
	void Arm();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();
	
private:
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;
	
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
	UAnimMontage* AttackMontage;

	/* Animation 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* StrongAttackMontage;
	
	/* Animation 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* EquipMontage;

	/* VisibleInstanceOnly = 디테일 패널에서만 볼수있음 */
	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;
	
	UPROPERTY(VisibleInstanceOnly, Category = "무기")
	AWeapon* EquippedWeapon;
	
	int32 Selection = 0;
	int32 StrongAttackSection = 0;
};
