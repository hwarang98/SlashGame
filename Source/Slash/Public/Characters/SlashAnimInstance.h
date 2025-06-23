// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SlashCharacter.h"
#include "Animation/AnimInstance.h"
#include "CharacterType.h"
#include "SlashAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SLASH_API USlashAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	/* 애니메이션 초기화 */
	virtual void NativeInitializeAnimation() override;

	/* 애니메이션 업데이트(Tick 같은 개념) */
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	/* 슬래시 캐릭터 선언 BlueprintReadOnly = 블루프린트에서만 접근 가능 */
	UPROPERTY(BlueprintReadOnly)
	class ASlashCharacter* SlashCharacter;

	/* 캐릭터 동작 구성 요소 선언 */
	UPROPERTY(BlueprintReadOnly, Category = Movement)
	class UCharacterMovementComponent* SlashCharacterMovement;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float GroundSpeed;

	/* 추락 값 할당 */
	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool IsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "Movement | 캐릭터 상태")
	ECharacterState CharacterState;
};
