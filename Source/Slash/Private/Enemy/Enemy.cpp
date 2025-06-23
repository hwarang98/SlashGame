// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "AIController.h"
#include "Characters/SlashCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/AttributeComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

#include "Slash/DebugMacros.h"


AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

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
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("속성"));
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
}

void AEnemy::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

void AEnemy::Die()
{
	// TODO: 죽는 몽타주 재생
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
		
		const int32 Selection = FMath::RandRange(0, 1);
		FName SelectionName = FName("Death1");
		DeathPose = EDeathPose::EDP_Death1;
		
		switch (Selection)
		{
		case 0:
			SelectionName = FName("Death1");
			DeathPose = EDeathPose::EDP_Death1;
			break;
		
		case 1:
			SelectionName = FName("Death2");
			DeathPose = EDeathPose::EDP_Death2;
			break;
		
		default:
			break;
		}
		AnimInstance->Montage_JumpToSection(SelectionName, DeathMontage);
	}

	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetLifeSpan(3.f);
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return;
	FAIMoveRequest MoveRequest; // AI 이동 요청 생성
	MoveRequest.SetGoalActor(Target); // 목표 액터 설정
	MoveRequest.SetAcceptanceRadius(15.f); // 목표에 얼마나 가까이 가면 도착으로 간주할지 설정
	EnemyController->MoveTo(MoveRequest); // 이동 요청 실행, 이동 경로를 NavPath에 저장
}

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	if (Target == nullptr) return false;
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	DRAW_SPHERE_SingleFrame(GetActorLocation());
	DRAW_SPHERE_SingleFrame(Target->GetActorLocation());
	return DistanceToTarget <= Radius;
}

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
	
	const int32 NumPatrolTargets = PatrolTargets.Num();
	if (NumPatrolTargets > 0)
	{
		const int32 TargetSelection = FMath::RandRange(0, NumPatrolTargets - 1);
		return PatrolTargets[TargetSelection];
	}
	return nullptr;
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	if (EnemyState == EEnemyState::EES_Chasing) return;
	if (SeenPawn->ActorHasTag("SlashCharacter"))
	{
		GetWorldTimerManager().ClearTimer(PatrolTimer);
		GetCharacterMovement()->MaxWalkSpeed = 300.f;
		CombatTarget = SeenPawn;
		
		if (EnemyState != EEnemyState::EES_Attacking)
		{
			EnemyState = EEnemyState::EES_Chasing;
			MoveToTarget(CombatTarget);
			
		}
	}
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget);
}

void AEnemy::CheckCombatTarget()
{
	if (!InTargetRange(CombatTarget, CombatRadius)) // 전투 대상이 범위 밖에 있으면
	{
		CombatTarget = nullptr; // 대상 초기화
		if (HealthBarWidget)
		{
			HealthBarWidget->SetVisibility(false); // 체력바 숨김
		}
		EnemyState = EEnemyState::EES_Patrolling;
		GetCharacterMovement()->MaxWalkSpeed = 125.f; // 순찰 속도로 변경/ 상태를 순찰로 변경
		MoveToTarget(PatrolTarget); // 순찰 지점으로 이동
	}
	// 공격 반경에서 벗어났는지
	else if (!InTargetRange(CombatTarget, AttackRadius) && EnemyState != EEnemyState::EES_Chasing)
	{
		// outside attack range, chase character, 공격 범위 밖, 추격 캐릭터
		EnemyState = EEnemyState::EES_Chasing;
		GetCharacterMovement()->MaxWalkSpeed = 300.f;
		MoveToTarget(CombatTarget);
	}
	else if (InTargetRange(CombatTarget, CombatRadius), EnemyState != EEnemyState::EES_Attacking)
	{
		EnemyState = EEnemyState::EES_Attacking;
		// TODO: play Enemy Attack Montage
	}
}

void AEnemy::CheckPatrolTarget()
{
	if (InTargetRange(PatrolTarget, PatrolRadius))
	{
		PatrolTarget = ChoosePatrolTarget();
		const float WaitTime = FMath::RandRange(WaitMin, WaitMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
		// MoveToTarget(PatrolTarget);
	}
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// 순찰 상태가 아니면
	if (EnemyState > EEnemyState::EES_Patrolling)
	{
		CheckCombatTarget();
	}
	else
	{
		CheckPatrolTarget();
		
	}
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::DirectionalHitReact(const FVector& ImpactPoint)
{
	/**
	 * 맞은 방향을 계산하여 피격 리액션 방향을 판별하는 절차:
	 * 1. Enemy의 전방 벡터(Forward Vector)를 구한다
	 * 2. 피격된 지점(ImpactPoint)과 Enemy 위치를 기준으로 ToHit 방향 벡터를 구한다
	 * 3. ToHit과 전방 벡터 사이의 내적(Dot Product)을 계산해 앞/뒤 여부를 판단한다
	 * 4. 이후 필요 시 오른쪽 벡터(Right Vector)로 좌/우 여부도 판단한다
	 */

	// 1. Enemy의 전방 벡터 (월드 좌표 기준)
	const FVector Forward = GetActorForwardVector();

	// 2. 피격 지점의 Z값을 그대로 유지 (현재는 그대로 사용 중)
	const FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);

	// 3. ToHit 벡터 = Enemy → 피격 지점 방향 :: “적 → 공격자” 방향 벡터
	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	// 4. 전방 벡터와 ToHit 벡터의 내적 계산 → 방향 간 각도를 구하는 데 사용 :: 내적 (Dot Product) 구하기
	// CosTheta가 1이면 → 정면에서 맞음
	// CosTheta가 -1이면 → 뒤에서 맞음
	// CosTheta가 0이면 → 옆(왼쪽 또는 오른쪽)에서 맞음
	const double CosTheta = FVector::DotProduct(Forward, ToHit);

	// 5. 내적 값의 절댓값을 이용해 실제 각도 계산 (0도 = 완전 정면, 180도 = 정반대)
	double Theta = FMath::Abs(CosTheta);
	Theta = FMath::RadiansToDegrees(Theta);

	// Forward(적 전방 벡터)와 ToHit(공격 지점 방향 벡터)의 외적을 통해 회전 방향(좌/우)을 판단
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);

	// Z값이 음수이면 공격자가 적의 왼쪽에서 공격해온 것 → Theta를 음수로 바꿔 왼쪽임을 표현
	if (CrossProduct.Z < 0)
	{
		// 각도를 음수로 바꿔서 왼쪽에서 맞았음을 나타냄
		Theta *= -1.f;
	}

	FName SectionName("FromBack");
	
	if (Theta >= -45.f && Theta < 45.f)
	{
		SectionName = FName("FromFront");
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		SectionName = FName("FromLeft");
	}else if (Theta >= 45.f && Theta < 135.f)
	{
		SectionName = FName("FromRight");
	}

	PlayHitReactMontage(SectionName);

	/*
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + CrossProduct * 100.f, 5.f, FColor::Blue, 5.f);

	// 화면에 디버그 메시지로 각도 출력
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Green, FString::Printf(TEXT("Theta: %f"), Theta));
	}
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + Forward * 60.f, 5.f, FColor::Red, 5.f);
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + ToHit * 60.f, 5.f, FColor::Green, 5.f);
	*/
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	// 맞은 위치 시각화 (마젠타 색상 구)
	// DRAW_SPHERE_COLOR(ImpactPoint, FColor::Magenta);
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(true);
	}

	if (Attributes && Attributes->IsAlive())
	{
		DirectionalHitReact(ImpactPoint);
	}
	else
	{
		Die();
	}

	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
	}

	if (HitParticles && GetWorld())
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, ImpactPoint);
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator,
	AActor* DamageCauser)
{
	if (Attributes && HealthBarWidget)
	{
		Attributes->ReceiveDamage(DamageAmount);
		HealthBarWidget->SetHealthBarPercent(Attributes->GetHealthPercent());
		
	}

	// EventInstigator는 피해를 입히도록 유도한 컨트롤러(AController)*
	// 예를 들어, 플레이어가 공격 버튼을 눌러 AI를 공격했다면, 이 컨트롤러는 플레이어 컨트롤러가 된다
	// EventInstigator->GetPawn()은 공격자의 캐릭터(또는 Pawn) 
	CombatTarget = EventInstigator->GetPawn();

	// TODO: 공격을 받으면 공격한 대상을 쫓게
	EnemyState = EEnemyState::EES_Chasing;
	MoveToTarget(CombatTarget);
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	return DamageAmount;
}
