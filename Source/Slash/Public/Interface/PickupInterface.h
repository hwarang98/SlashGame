// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PickupInterface.generated.h"

UINTERFACE(MinimalAPI)
class UPickupInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SLASH_API IPickupInterface
{
	GENERATED_BODY()
public:
	virtual void SetOverlappingItem(class AItem* Item);
	virtual void AddSoul(class ASoul* Soul);
	virtual void AddHealth(class AHealPotion* Health);
	virtual void AddGold(class ATreasure* ATreasure);
};
