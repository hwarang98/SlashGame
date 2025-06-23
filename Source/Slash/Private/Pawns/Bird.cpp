// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Bird.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
ABird::ABird()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 블루프린트에서 캡슐 컴포넌트 장착
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetCapsuleHalfHeight(20.f);
	Capsule->SetCapsuleRadius(15.f);

	// 캡슐을 루트 구성요소 만들기
	SetRootComponent(Capsule);

	/* bird 컴포넌트 생성 */
	BirdMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BirdMesh"));
	BirdMesh->SetupAttachment(GetRootComponent());

	/* 움직임 관련 컴포넌트 */
	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));

	/* 스프링암 컴포넌트 생성 */
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent()); /* 스프링암 부착 */
	CameraBoom->TargetArmLength = 300.f; /* 스프링암 길이 */

	/* 카메라 컴포넌트 생성 */
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);	/* 카메라 부착 */
	
	/* 플레이어0 으로 빙의 */
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// Called when the game starts or when spawned
void ABird::BeginPlay()
{
	Super::BeginPlay();

	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(BirdMappingContext, 0);
		}
	}
}

// Called every frame
void ABird::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/**
 * 새 움직이는 함수
 * 1 = 전진, 2 = 후진
 * @param Value 
 */
void ABird::Move(const FInputActionValue& Value)
{
	// 이 입력 작업의 현재 값을 가지고온다 즉, float값을 가지고 온다
	const float DirectionValue = Value.Get<float>();
	if (Controller && (DirectionValue != 0.f))
	{
		FVector Direction = GetActorForwardVector();
		AddMovementInput(Direction, DirectionValue);
		UE_LOG(LogTemp, Warning, TEXT("IA_Move trigger"));
	}
}

void ABird::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisValue = Value.Get<FVector2D>();
	if (GetController())
	{
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
		UE_LOG(LogTemp, Warning, TEXT("IA_Look trigger"));
	}
}

// Called to bind functionality to input
void ABird::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	/* 부모 클래스(APawn)의 입력 바인딩 설정을 호출하여 기본 입력 설정을 유지 */
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	/* 전달받은 PlayerInputComponent를 UEnhancedInputComponent 타입으로 변환 시도 */
	// CastChecked는 실패 시 에디터에서 에러를 발생시켜 디버깅에 유리함
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		/* MoveAction이 Triggered 되었을 때 ABird 클래스의 Move 함수를 호출하도록 입력 바인딩 */
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABird::Move);
		/* LookAction이 Triggered 되었을 때 ABird 클래스의 Look 함수를 호출하도록 입력 바인딩 */
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABird::Look);
	}
}



