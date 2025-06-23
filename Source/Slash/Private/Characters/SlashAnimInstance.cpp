// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/SlashAnimInstance.h"
#include "Characters/SlashCharacter.h"
#include "kismet/KismetMaterialLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void USlashAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	SlashCharacter = Cast<ASlashCharacter>(TryGetPawnOwner());

	if (SlashCharacter)
	{
		/* 캐릭터의 움직임을 포착 */
		SlashCharacterMovement = SlashCharacter->GetCharacterMovement();
	}
	/* 모든 프레임을 업데이트 */
}

void USlashAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (SlashCharacterMovement)
	{
		GroundSpeed = UKismetMathLibrary::VSizeXY(SlashCharacterMovement->Velocity);
		IsFalling = SlashCharacterMovement->IsFalling();
		CharacterState = SlashCharacter->GetCharacterState();
	}
}
