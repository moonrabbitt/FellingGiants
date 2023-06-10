// Fill out your copyright notice in the Description page of Project Settings.
//https://github.com/Fetiska/Unreal-Procedural-Terrain-Generation-CPP/blob/main/%232%20Landscape%20generation/Source/DiamondSquareCPP/Public/DiamondSquare.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BasicProceduralLandscape.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;

UCLASS()
class FELLINGGIANTS_API ABasicProceduralLandscape : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABasicProceduralLandscape();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Meta = (MakeEditWidget = true))
	TArray<FVector> Vertices;

	UPROPERTY(EditAnywhere)
	TArray<int> Triangles;

	UPROPERTY(EditAnywhere)
	TArray<FVector2D> UV0;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UProceduralMeshComponent* ProceduralMesh;

};
