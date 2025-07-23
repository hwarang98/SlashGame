// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Item.h"
#include "HealPotion.generated.h"

/**
 * 
 */
UCLASS()
class SLASH_API AHealPotion : public AItem
{
	GENERATED_BODY()

public:
	FORCEINLINE int32 GetHealAmount() const { return HealAmount; }
	FORCEINLINE void SetHealAmount(int32 NumberOfHeal) { HealAmount = NumberOfHeal; }
	
protected:
	virtual void OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
) override;

private:
	UPROPERTY(EditAnywhere, Category = "Heal Properties")
	int32 HealAmount = 20;


};
