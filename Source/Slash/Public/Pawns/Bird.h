// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "Bird.generated.h"

class USpringArmComponent;
class UCapsuleComponent;
class UStaticMeshComponent;
class UInputMappingContext;
class UInputAction;
class UPawnMovementComponent;
class UCameraComponent;

UCLASS()
class SLASH_API ABird : public APawn
{
	GENERATED_BODY()

public:
	ABird();
	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* BirdMappingContext;

	/* w, s 움직임 액션 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;

	/* 마우스로 둘러보기 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;
	
	
	UPROPERTY(VisibleAnywhere);
	UPawnMovementComponent* MovementComponent;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

private:
	/**
	 * 캡슐 컴포넌트
	 */
	UPROPERTY(VisibleAnywhere)
	UCapsuleComponent* Capsule;

	/**
	 * 스켈레탈메시 컴포넌트
	 */
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* BirdMesh;

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
};
