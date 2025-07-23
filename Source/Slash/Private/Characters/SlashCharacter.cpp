// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GroomComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Item/Item.h"
#include "Item/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"
#include "Components/AttributeComponent.h"
#include "Components/StaticMeshComponent.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"
#include "Item/HealPotion.h"
#include "Slash/DebugMacros.h"
#include "Item/Soul.h"
#include "Item/Treasure.h"

ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	
	Attribute = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	
	/* 스프링암 컴포넌트 생성 */
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent()); /* 스프링암 부착 */
	CameraBoom->TargetArmLength = 300.f; /* 스프링암 길이 */

	/* 카메라 컴포넌트 생성 */
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom); /* 카메라 부착 */

	/* 머리카락 부착 */
	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");

	KatanaSheath = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KatanaSheath"));
	KatanaSheath->SetupAttachment(GetMesh(), FName("Katana_sheath_01"));
}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();
	Tags.Add(FName("EngageableTarget"));
	
	// 컨트롤러가 플레이어 컨트롤러인지 확인
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		// 플레이어 컨트롤러에서 로컬 플레이어를 가져와 Enhanced Input Local Player Subsystem을 얻는다
		// 이 Subsystem을 통해 입력 매핑 컨텍스트를 관리
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// "SlashContext"라는 이름의 매핑 컨텍스트를 Subsystem에 추가
			// 이 컨텍스트는 특정 입력 액션(예: 마우스 클릭, 키 누름)을 게임 내 동작에 연결하는 데 사용
			// 두 번째 인자 0은 우선순위로, 숫자가 낮을수록 우선순위가 높음
			Subsystem->AddMappingContext(SlashContext, 0);
		}
	}
	InitializeSlashOverlay();
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Attribute && SlashOverlay)
	{
		Attribute->RegenStamina(DeltaTime);
		SlashOverlay->SetStaminaBarPercent(Attribute->GetStaminaPercent());
	}
}

/* move 함수 */
void ASlashCharacter::Move(const FInputActionValue& Value)
{
	if (ActionState != EActionState::EAS_Unoccupied) return;
	/* w,s를 누르면 y 요소가 채워짐 */
	const FVector2D MovementVector = Value.Get<FVector2D>();
	
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	/* w,s 컨트롤러가 가르키는 방향의 전방(x) 백터 */
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);
	/* 
	 * ForwardDirection: 이 변수는 캐릭터가 현재 바라보고 있는 방향(Yaw 회전 기준)에서 앞쪽을 가리키는 3D 벡터
	 * FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X)는 회전된 좌표계에서 X축 방향, 즉 앞쪽 방향 벡터를 가져오는 역할을 합니다.
	 *
	 * EAxis::X는 캐릭터의 회전 기준에서 '앞' 방향을 의미
	 * 
	 * MovementVector.Y는 사용자의 입력에서 '앞/뒤' 움직임의 강도를 의미하는 것입니다
	 * MovementVector 이 변수는 사용자로부터 받은 2D 입력 값 을 나타낸다
	 * MovementVector.Y: 일반적으로 앞/뒤 움직임에 대한 입력 값 (예: W/S 키 또는 조이스틱의 세로 방향) 값이 양수이면 앞으로, 음수이면 뒤로 움직이려는 의도를 나타낸다
	 * MovementVector.X: 일반적으로 좌/우 움직임에 대한 입력 값 (예: A/D 키 또는 조이스틱의 가로 방향) 값이 양수이면 오른쪽으로, 음수이면 왼쪽으로 움직이려는 의도를 나타낸다
	 */
	
	/*
	* RightDirection: 이 변수는 캐릭터가 현재 바라보고 있는 방향(Yaw 회전 기준)에서 오른쪽을 가리키는 3D 벡터
	* FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y)는 회전된 좌표계에서 Y축 방향, 즉 오른쪽 방향 벡터를 가져오는 역할
	*/
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
	
}

void ASlashCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void ASlashCharacter::StartWalking()
{
	// 
	bIsWalking = !bIsWalking;
	if (bIsWalking)
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	}
}

void ASlashCharacter::Jump()
{
	if (IsUnoccupied())
	{
		ACharacter::Jump();
	}
}

void ASlashCharacter::EKeyPressed()
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		OverlappingWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		/* 무기 획득 후에도 해당 무기와의 "중첩 상태"가 계속 유지되지 않도록 하기 위함 */
		OverlappingItem = nullptr;
		EquippedWeapon = OverlappingWeapon;
	}
	else
	{
		if (CanDisarm())
		{
			// 무장해제
			PlayEquipMontage(FName("Unequip"));
			CharacterState = ECharacterState::ECS_Unequipped;
			ActionState = EActionState::EAS_EquippingWeapon;
		}
		else if (CanArm())
		{
			// 무기장착
			PlayEquipMontage(FName("Equip"));
			CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
			ActionState = EActionState::EAS_EquippingWeapon;
		}
	}
}

void ASlashCharacter::Dodge()
{
	if (IsOccupied() || !HasDodgeEnoughStamina()) return;
	PlayDodgeMontage();
	ActionState = EActionState::EAS_Dodge;

	if (Attribute && SlashOverlay)
	{
		Attribute->UseStamina(Attribute->GetDodgeConst());
		SlashOverlay->SetStaminaBarPercent(Attribute->GetStaminaPercent());
	}
}

void ASlashCharacter::Attack()
{
	Super::Attack();
	/* 행동액션이 없고, 무기가 장착되어있을때만 공격 몽타주 재생 */
	if (CanAttack())
	{
		PlayAttackMontage();
		if (Attribute && SlashOverlay)
		{
			Attribute->UseStamina(Attribute->GetAttackStamina());
			SlashOverlay->SetStaminaBarPercent(Attribute->GetStaminaPercent());
			UE_LOG(LogTemp, Log, TEXT("AttackStamina: %f"), Attribute->GetAttackStamina());
		}
		
		ActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::DodgeEnd()
{
	Super::DodgeEnd();

	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::StrongAttack()
{
	Super::Attack();
	if (CanAttack())
	{
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}

	if (Attribute && SlashOverlay)
	{
		Attribute->UseStamina(Attribute->GetAttackStamina());
		SlashOverlay->SetStaminaBarPercent(Attribute->GetStaminaPercent());
	}
}

void ASlashCharacter::PlayEquipMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

void ASlashCharacter::Die()
{
	Super::Die();

	ActionState = EActionState::EAS_Dead;
	DisableMeshCollision();
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped && HasAttackEnoughStamina();
}

/*
 * 행동상태가 비어있고 && 무기가 손에 "있을때" = true (장비 해제 모션 실행 가능)
 */
bool ASlashCharacter::CanDisarm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

/*
 * 행동상태가 비어있고 && 무기가 손에 "없을때" = true (장비 장착 모션 실행 가능)
 */
bool ASlashCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped && EquippedWeapon;
}

void ASlashCharacter::Disarm()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void ASlashCharacter::Arm()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void ASlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	/* 부모 클래스(APawn)의 입력 바인딩 설정을 호출하여 기본 입력 설정을 유지 */
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	/* 전달받은 PlayerInputComponent를 UEnhancedInputComponent 타입으로 변환 시도 */
	// CastChecked는 실패 시 에디터에서 에러를 발생시켜 디버깅에 유리함
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		/* MoveAction이 Triggered 되었을 때 ASlashCharacter 클래스의 Move 함수를 호출하도록 입력 바인딩 */
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		/* LookAction이 Triggered 되었을 때 ASlashCharacter 클래스의 Look 함수를 호출하도록 입력 바인딩 */
		EnhancedInputComponent->BindAction(LookingAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		/* LookAction이 Triggered 되었을 때 ASlashCharacter 클래스의 Jump 함수를 호출하도록 입력 바인딩 */
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started , this, &ASlashCharacter::Jump);
		/* LookAction이 Triggered 되었을 때 ASlashCharacter 클래스의 StartWalking 함수를 호출하도록 입력 바인딩 */
		EnhancedInputComponent->BindAction(EKeyPressedAction, ETriggerEvent::Started, this, &ASlashCharacter::EKeyPressed);
		/* LookAction이 Triggered 되었을 때 ASlashCharacter 클래스의  함수를  호출하도록 AttackAction 입력 바인딩 */
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Triggered, this, &ASlashCharacter::StartWalking);
		/* LookAction이 Triggered 되었을 때 ASlashCharacter 클래스의  함수를 호출하도록 DodgeAction 입력 바인딩 */
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Dodge);
		/* LookAction이 Triggered 되었을 때 ASlashCharacter 클래스의  함수를 호출하도록 AttackAction 입력 바인딩 */
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Attack);
		/* LookAction이 Triggered 되었을 때 ASlashCharacter 클래스의  함수를 호출하도록 StrongAttackAction 입력 바인딩 */
		EnhancedInputComponent->BindAction(StrongAttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::StrongAttack);
	}
}

float ASlashCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	SetHUDHealth();
	return DamageAmount;
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);

	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Attribute && Attribute->GetHealthPercent() > 0.f)
	{
		ActionState = EActionState::EAS_HitReaction;
	}
}

void ASlashCharacter::InitializeSlashOverlay()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD());
		if (SlashHUD)
		{
			SlashOverlay = SlashHUD->GetSlashOverlay();
			if (SlashOverlay && Attribute)
			{
				SlashOverlay->SetHealthBarPercent(Attribute->GetHealthPercent());
				// SlashOverlay->SetStaminaBarPercent(Attribute->GetStaminaPercent());
				SlashOverlay->SetGold(0);
				SlashOverlay->SetSouls(0);
			}
		}
	}
}

void ASlashCharacter::SetHUDHealth()
{
	if (SlashOverlay && Attribute)
	{
		SlashOverlay->SetHealthBarPercent(Attribute->GetHealthPercent());
	}
}

// void ASlashCharacter::SetHUDStamina()
// {
// 	if (SlashOverlay && Attribute)
// 	{
// 		SlashOverlay->SetStaminaBarPercent(Attribute->GetStaminaPercent());
// 	}
// }

bool ASlashCharacter::IsUnoccupied()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

void ASlashCharacter::SetOverlappingItem(class AItem* Item)
{
	OverlappingItem = Item;
}

void ASlashCharacter::AddSoul(ASoul* Soul)
{
	if (Attribute && SlashOverlay)
	{
		Attribute->AddSouls(Soul->GetSouls());
		SlashOverlay->SetSouls(Attribute->GetSouls());
	}
	DEBUG_LOG(TEXT("ASlashCharacter::AddSoul"));
}

void ASlashCharacter::AddGold(ATreasure* ATreasure)
{
	if (Attribute && SlashOverlay)
	{
		Attribute->AddGold(ATreasure->GetGold());
		SlashOverlay->SetGold(Attribute->GetGold());
	}
}

void ASlashCharacter::AddHealth(AHealPotion* AHealPotion)
{
	if (Attribute && SlashOverlay)
	{
		Attribute->AddHealPotion(AHealPotion->GetHealAmount());
		SlashOverlay->SetHealthBarPercent(Attribute->GetHealthPercent());
	}
}


bool ASlashCharacter::IsOccupied()
{
	return ActionState != EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::HasDodgeEnoughStamina()
{
	return Attribute && Attribute->GetStamina() > Attribute->GetDodgeConst();
}

bool ASlashCharacter::HasAttackEnoughStamina()
{
	return Attribute && Attribute->GetStamina() > Attribute->GetAttackStamina();
}
