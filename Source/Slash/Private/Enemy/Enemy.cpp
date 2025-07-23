// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "AIController.h"
#include "Characters/SlashCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/AttributeComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Item/Soul.h"
#include "Navigation/PathFollowingComponent.h"
#include "Item/Weapons/Weapon.h"
#include "Item/Soul.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	Attribute = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));

	/* 메시(Mesh)의 충돌 타입을 '월드 다이내믹'으로 설정
	 * → 움직이는 물체로 취급되며, 다른 오브젝트와 충돌할 수 있음
	 */ 
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

	/* 메시가 '가시성(Visibility)' 채널과 충돌할 때는 Block(막기)으로 설정
	 * → 라인 트레이스 같은 것으로 이 메시를 감지할 수 있게 됨 (예: 시야 체크)
	 */
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	// 이 메시(Mesh)에서 오버랩 이벤트를 발생시킬 수 있게 허용해주는 설정
	GetMesh()->SetGenerateOverlapEvents(true);

	// 카메라 무시
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	
	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("체력 바"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// TODO: ⛔ 기존 감지 시스템 제거
	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("감지"));
	PawnSensing->SightRadius = 4000.f;
	PawnSensing->SetPeripheralVisionAngle(45.f);

	// TODO: ✅ AI Perception System 도입
	// AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	// SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
		HealthBarWidget->SetHealthBarPercent(1.f);
	}

	EnemyController = Cast<AAIController>(GetController());
	MoveToTarget(PatrolTarget);
	
	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}
	
	Tags.Add(FName("Enemy"));

	UWorld* World = GetWorld();
	if (World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
}

/**
 * 전투/순찰 대상을 주기적으로 확인하고 AI의 행동을 결정합니다.
 * @param DeltaTime 프레임 간 경과 시간
 */
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsDead()) return;
	if (EnemyState > EEnemyState::EES_Patrolling)
	{
		CheckCombatTarget();
	}
	else
	{
		CheckPatrolTarget();
	}
}

/**
 * 적 캐릭터를 사망 상태로 전환하고 관련 처리를 수행합니다.
 *
 * - 적 캐릭터의 상태를 '사망'(EES_Dead)으로 설정합니다.
 * - 사망 애니메이션 몽타주를 재생합니다.
 * - 공격 타이머를 초기화합니다.
 * - 체력바를 화면에서 숨깁니다.
 * - 캡슐 컴포넌트를 비활성화하여 충돌 처리를 중단합니다.
 * - 사망 후 지정된 시간(DeathLifeSpan)이 경과하면 캐릭터를 제거합니다.
 */
void AEnemy::Die()
{
	Super::Die();
	EnemyState = EEnemyState::EES_Dead;
	// PlayDeathMontage();
	ClearAttackTimer();
	HideHealthBar();
	DisableCapsule();
	SetLifeSpan(DeathLifeSpan);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	DisableMeshCollision();
	SpawnSoul();
}

/**
 * 지정된 대상으로 AI를 이동시킵니다.
 * @param Target 이동할 목표 액터
 * @note 도착 허용 반경은 60 유닛으로 설정됩니다.
 */
void AEnemy::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return;
	FAIMoveRequest MoveRequest; // AI 이동 요청 생성
	MoveRequest.SetGoalActor(Target); // 목표 액터 설정
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius); // 목표에 얼마나 가까이 가면 도착으로 간주할지 설정
	EnemyController->MoveTo(MoveRequest); // 이동 요청 실행, 이동 경로를 NavPath에 저장
}

/**
 * 대상이 지정된 반경 내에 있는지 확인합니다.
 * @param Target 확인할 대상 액터
 * @param Radius 검사할 반경
 * @return 대상이 반경 내에 있으면 true, 그렇지 않으면 false
 */

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	if (Target == nullptr) return false;
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= Radius;
}

/**
 * 현재 순찰 대상을 제외한 다른 순찰 지점 중 하나를 무작위로 선택합니다.
 * @return 선택된 순찰 대상, 유효한 대상이 없으면 nullptr
 */
AActor* AEnemy::ChoosePatrolTarget()
{
	// 목표 배열 생성
	TArray<AActor*> ValidTargets;
	for (AActor* Target : PatrolTargets)
	{
		if (Target != PatrolTarget)
		{
			ValidTargets.AddUnique(Target);
		}
	}
	
	const int32 NumPatrolTargets = ValidTargets.Num();
	if (NumPatrolTargets > 0)
	{
		const int32 TargetSelection = FMath::RandRange(0, NumPatrolTargets - 1);
		return ValidTargets[TargetSelection];
	}
	return nullptr;
}
/**
 * AI 캐릭터가 공격을 수행합니다.
 * AttackMontage를 재생하여 공격 애니메이션을 실행합니다.
 */
void AEnemy::Attack()
{
	Super::Attack();
	if (CombatTarget == nullptr) return;
	EnemyState = EEnemyState::EES_Engaged;
	PlayAttackMontage();
}

void AEnemy::AttackEnd()
{
	EnemyState = EEnemyState::EES_NoState;
	CheckCombatTarget();
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

bool AEnemy::CanChaseTarget()
{
	return
	EnemyState != EEnemyState::EES_Dead &&
	EnemyState != EEnemyState::EES_Chasing &&
	EnemyState < EEnemyState::EES_Attacking;
}

bool AEnemy::IsTargetPlayer(const APawn* Target)
{
	return Target && Target->ActorHasTag(FName("EngageableTarget"));
}

/**
 * AI가 Pawn을 시야에 포착했을 때 호출되는 함수
 * @param SeenPawn 시야에 포착된 Pawn
 */
void AEnemy::PawnSeen(APawn* SeenPawn)
{
	const bool bShouldChaseTarget = CanChaseTarget() && IsTargetPlayer(SeenPawn);

	if (bShouldChaseTarget)
	{
		CombatTarget = SeenPawn;
		ClearPatrolTimer();
		ChaseTarget();
	}
}

/**
 * 현재 AI가 공격할 수 있는지 여부를 확인합니다.
 * 공격 범위(AttackRadius) 내에 있으며, 공격 중이지 않고, 사망 상태가 아닐 때 true를 반환합니다.
 * @return 공격 가능한 상태일 경우 true, 그렇지 않을 경우 false
 */
bool AEnemy::CanAttack()
{
	return IsInsideAttackRadius() && !IsAttacking() && !IsDead() && !IsEngaged();
}

/**
 * AI 캐릭터가 파괴될 때 호출됩니다.
 * 기본 클래스를 통해 파괴 프로세스를 처리한 후, 장착된 무기(EquippedWeapon)가 있을 경우 해당 무기를 파괴합니다.
 */
void AEnemy::Destroyed()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

/**
 * 순찰 타이머가 완료된 후 호출됩니다.
 * AI를 다음 순찰 지점으로 이동시킵니다.
 */
void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget);
}

void AEnemy::HideHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false); // 체력바 숨김
	}
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(true); // 체력바 숨김
	}
}

void AEnemy::LoseInterest()
{
	CombatTarget = nullptr; // 대상 초기화
	HideHealthBar(); // 체력바 숨김
}

void AEnemy::StartPatrolling()
{
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed; // 순찰 속도로 변경/ 상태를 순찰로 변경
	MoveToTarget(PatrolTarget); // 순찰 지점으로 이동
}

bool AEnemy::IsOutsideCombatRadius()
{
	return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsOutsideAttackRadius()
{
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsInsideAttackRadius()
{
	return InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsChasing()
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsAttacking()
{
	return EnemyState == EEnemyState::EES_Attacking;
}

/**
 * 공격 상태로 전환하고 AttackMin~AttackMax 사이의 랜덤한 시간 후 공격을 실행하는 타이머를 설정합니다.
 */
void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

/**
 * 현재 실행 중인 공격 타이머를 초기화하여 공격 주기를 중단합니다.
 */
void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

void AEnemy::ChaseTarget()
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

bool AEnemy::IsEngaged()
{
	return EnemyState == EEnemyState::EES_Engaged;
}

/**
 * 전투 상황을 체크하고 적절한 AI 행동을 결정합니다.
 * - 전투 반경 밖: 관심 상실 및 순찰 시작
 * - 공격 반경 밖: 대상 추적
 * - 공격 반경 내: 공격 시작
 */
void AEnemy::CheckCombatTarget()
{
	if (IsOutsideCombatRadius())
	{
		ClearAttackTimer();
		LoseInterest();
		if (!IsEngaged()) StartPatrolling();
	}
	else if (IsOutsideAttackRadius() && !IsChasing())
	{
		ClearAttackTimer();
		if (!IsEngaged()) ChaseTarget();
	}
	else if (CanAttack())
	{
		StartAttackTimer();
	}
}


bool AEnemy::ActorsSameType(AActor* OtherActor)
{
	return GetOwner()->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy"));
}

void AEnemy::ActivateArmCollision(bool bActivate)
{
	// 팔 콜리전 활성화/비활성화
	ECollisionEnabled::Type CollisionType = bActivate ? 
		ECollisionEnabled::QueryOnly : 
		ECollisionEnabled::NoCollision;
}

/**
 * 순찰 대상과의 거리를 체크하고 새로운 순찰 지점을 설정합니다.
 * 현재 순찰 지점에 도달하면 WaitMin~WaitMax 시간 후 다음 지점으로 이동합니다.
 */
void AEnemy::CheckPatrolTarget()
{
	if (InTargetRange(PatrolTarget, PatrolRadius))
	{
		PatrolTarget = ChoosePatrolTarget();
		const float WaitTime = FMath::RandRange(PatrolWaitMin, PatrolWaitMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
}

/**
 * AI가 사망 상태(EES_Dead)인지 여부를 반환합니다.
 * @return AI가 사망 상태(EES_Dead)인지 여부
 */
bool AEnemy::IsDead()
{
	return EnemyState == EEnemyState::EES_Dead;
}

/**
 * 피해를 받았을 때 호출되는 함수
 * @param ImpactPoint 피해를 입은 위치
 */
void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	if (!IsDead()) ShowHealthBar();
	ClearPatrolTimer();
	ClearAttackTimer();
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);

	StopAttackMontage();
	if (IsInsideAttackRadius())
	{
		if (!IsDead()) StartAttackTimer();
	}
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);

	if (Attribute && HealthBarWidget)
	{
		HealthBarWidget->SetHealthBarPercent(Attribute->GetHealthPercent());
		
	}
}

/**
 * 데미지 처리 함수
 * @param DamageAmount 받은 데미지량
 * @param DamageEvent 데미지 이벤트 정보
 * @param EventInstigator 데미지를 가한 컨트롤러
 * @param DamageCauser 데미지를 가한 액터
 * @return 처리된 데미지량
 */
float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn();
	
	if (IsInsideAttackRadius())
	{
		EnemyState = EEnemyState::EES_Attacking;
	}
	else if (IsOutsideAttackRadius())
	{
		ChaseTarget();
	}

	return DamageAmount;
}

void AEnemy::SpawnSoul()
{
	UWorld* World = GetWorld();
	if (World && SoulClass && Attribute)
	{
		ASoul* SpawnSoul = World->SpawnActor<ASoul>(SoulClass, GetActorLocation(), GetActorRotation());
		if (SpawnSoul)
		{
			SpawnSoul->SetSouls(Attribute->GetSouls());
		}
	}
}