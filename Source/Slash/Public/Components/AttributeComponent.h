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

protected:
	virtual void BeginPlay() override;

private:
	/* 현재 체력 */
	UPROPERTY(EditAnywhere, Category = "액터 속성")
	float Health;

	UPROPERTY(EditAnywhere, Category = "액터 속성")
	float MaxHealth;

public:
	void ReceiveDamage(float Damage);
	float GetHealthPercent();
	bool IsAlive();
};
