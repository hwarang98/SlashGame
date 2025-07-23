// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLASH_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttributeComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void RegenStamina(float DeltaTime);

protected:
	virtual void BeginPlay() override;

private:
	/* 현재 체력 */
	UPROPERTY(EditAnywhere, Category = "액터 속성")
	float Health;

	UPROPERTY(EditAnywhere, Category = "액터 속성")
	float MaxHealth;

	UPROPERTY(EditAnywhere, Category = "액터 속성")
	float Stamina;

	UPROPERTY(EditAnywhere, Category = "액터 속성")
	float MaxStamina;

	UPROPERTY(EditAnywhere, Category = "액터 속성")
	int32 Gold;

	UPROPERTY(EditAnywhere, Category = "액터 속성")
	int32 Souls;

	UPROPERTY(EditAnywhere, Category = "액터 속성")
	float DodgeConst = 14.f;

	UPROPERTY(EditAnywhere, Category = "액터 속성")
	float AttackConst = 20.f;

	UPROPERTY(EditAnywhere, Category = "액터 속성")
	float StaminaRegenRate = 8.f;


public:
	void ReceiveDamage(float Damage);
	void UseStamina(float StaminaConst);
	float GetHealthPercent();
	float GetStaminaPercent();
	bool IsAlive();
	void AddGold(int32 NumberOfGold);
	void AddSouls(int32 NumberOfSouls);
	void AddHealPotion(int32 NumberOfHealPotion);

	FORCEINLINE int32 GetGold() const { return Gold; }
	FORCEINLINE int32 GetSouls() const { return Souls; }
	FORCEINLINE int32 GetHealPotion() const { return Souls; }
	FORCEINLINE float GetDodgeConst() const { return DodgeConst; }
	FORCEINLINE float GetAttackStamina() const { return AttackConst; }
	FORCEINLINE float GetStamina() const { return Stamina; }
	
};
