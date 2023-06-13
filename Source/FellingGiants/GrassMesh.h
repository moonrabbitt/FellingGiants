// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrassMesh.generated.h"

UCLASS()
class FELLINGGIANTS_API AGrassMesh : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrassMesh();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	UStaticMesh* GrassMesh;

	UPROPERTY(EditAnywhere,Category = "Components")
	UStaticMeshComponent* GrassMeshComponent;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;



};