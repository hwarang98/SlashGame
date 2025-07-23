// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/AttributeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Item/Weapons/Weapon.h"
#include "Components/CapsuleComponent.h"
#include "HUD/SlashOverlay.h"

#include "Slash/DebugMacros.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	// Attribute = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (IsAlive() && Hitter)
	{ 
		DirectionalHitReact(Hitter->GetActorLocation());
	}
	else Die();
	
	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);
}

void ABaseCharacter::Attack()
{
	if (CombatTarget && CombatTarget->ActorHasTag(FName("Dead")))
	{
		CombatTarget = nullptr;
	}
}

void ABaseCharacter::Die()
{
	Tags.Add(FName("Dead"));
	PlayDeathMontage();
}

void ABaseCharacter::PlayHitAttackMontage()
{
}

void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
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
	double Theta = FMath::Acos(CosTheta);
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
}

/**
 * 지정된 위치에서 Hit 사운드를 재생합니다.
 *
 * @param ImpactPoint 사운드가 재생될 충돌 지점의 위치를 나타내는 벡터
 */
void ABaseCharacter::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
	}
}

/**
 * 지정된 위치에서 충돌 입자 효과를 생성합니다.
 *
 * @param ImpactPoint 입자 효과가 나타날 충돌 지점의 위치를 나타내는 벡터
 */
void ABaseCharacter::SpawnHitParticles(const FVector& ImpactPoint)
{
	if (HitParticles && GetWorld())
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, ImpactPoint);
	}
}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if (Attribute)
	{
		Attribute->ReceiveDamage(DamageAmount);
	}
}

/**
 * 지정된 섹션에서 주어진 몽타주를 재생합니다.
 *
 * @param Montage 재생할 애니메이션 몽타주
 * @param SectionName 재생할 섹션의 이름
 */
void ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::DisableMeshCollision()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0) return -1;
	const int32 MaxSectionIndex = SectionNames.Num() - 1;
	const int32 Selection = FMath::RandRange(0, MaxSectionIndex);
	const FName& SectionName = SectionNames[Selection];
	
	PlayMontageSection(Montage, SectionNames[Selection]);
	return Selection;
}

int32 ABaseCharacter::PlayAttackMontage()
{
	return PlayRandomMontageSection(AttackMontage, AttackMontageSection);
}

int32 ABaseCharacter::PlayDeathMontage()
{
	const int32 Selection = PlayRandomMontageSection(DeathMontage, DeathMontageSection);
	TEnumAsByte<EDeathPose> Pose(Selection);
	if (Pose < EDeathPose::EDP_MAX)
	{
		DeathPose = Pose;
	}
	return Selection;
}

void ABaseCharacter::PlayDodgeMontage()
{
	PlayMontageSection(DodgeMontage, FName("Default"));
}

void ABaseCharacter::StopAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Stop(0.25, AttackMontage);
	}
}

bool ABaseCharacter::CanAttack()
{
	return false;
}

bool ABaseCharacter::IsAlive()
{
	return Attribute && Attribute->IsAlive();
}

FVector ABaseCharacter::GetTranslationWarpTarget()
{
	if (CombatTarget == nullptr) return FVector();
	const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
	const FVector Location = GetActorLocation();

	FVector TargetToMe = (Location - CombatTargetLocation).GetSafeNormal();

	TargetToMe *= WarpTargetDistance;
	return CombatTargetLocation + TargetToMe;
}

FVector ABaseCharacter::GetRotationWarpTarget()
{
	if (CombatTarget)
	{
		return CombatTarget->GetActorLocation();
	}
	return FVector();
}

void ABaseCharacter::AttackEnd()
{
}

void ABaseCharacter::DodgeEnd()
{
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	/* 장착된 무기가 있는 경우 */
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoreActors.Empty();
	}
}

