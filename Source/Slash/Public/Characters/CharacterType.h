﻿#pragma once

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	/* 장비가 없는 상태 */
	ECS_Unequipped UMETA(DisplayName = "장비가 없는 상태"),

	/* 장비가 손에 하나만 있는 상태 */
	ECS_EquippedOneHandedWeapon UMETA(DisplayName = "장비가 손에 하나만 있는 상태"),

	/* 장비가 손에 두개 있는 상태 */
	ECS_EquippedTwoHandedWeapon UMETA(DisplayName = "장비가 손에 두개 있는 상태")
};   

UENUM(BlueprintType)
enum class EActionState : uint8
{
	/* 캐릭터가 다른 공격 행동을 하지못하게 */
	EAS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	EAS_Attacking UMETA(DisplayName = "공격"),
	EAS_EquippingWeapon UMETA(DisplayName = "무기 장착"),
};

UENUM(BlueprintType)
enum class EDeathPose : uint8
{
	/* 캐릭터가 다른 공격 행동을 하지못하게 */
	EDP_Alive UMETA(DisplayName = "살아있는 상태"),
	EDP_Death1 UMETA(DisplayName = "죽어있는 상태 1"),
	EDP_Death2 UMETA(DisplayName = "죽어있는 상태 2"),
	EDP_Death3 UMETA(DisplayName = "죽어있는 상태 3"),
	EDP_Death4 UMETA(DisplayName = "죽어있는 상태 4"),
	EDP_Death5 UMETA(DisplayName = "죽어있는 상태 5"),
	EDP_Death6 UMETA(DisplayName = "죽어있는 상태 6")
};

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	/* 특정 지역 순찰 */
	EES_Patrolling UMETA(DisplayName = "순찰"),
	/* 대상을 추격 */
	EES_Chasing UMETA(DisplayName = "추격"),
	/* 대상을 공격 */
	EES_Attacking UMETA(DisplayName = "공격")
};
