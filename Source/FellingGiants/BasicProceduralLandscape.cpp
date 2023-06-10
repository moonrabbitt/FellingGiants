// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicProceduralLandscape.h"
#include "ProceduralMeshComponent.h"

// Sets default values
ABasicProceduralLandscape::ABasicProceduralLandscape()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>("ProceduralMesh");
	ProceduralMesh->SetupAttachment(GetRootComponent());

}

// Called when the game starts or when spawned
void ABasicProceduralLandscape::BeginPlay()
{
	Super::BeginPlay();

	CreateVertices();
	CreateTriangles();

	ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, TArray<FVector>(), UV0, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
	ProceduralMesh->SetMaterial(0, Material);

	
}

// Called every frame
void ABasicProceduralLandscape::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABasicProceduralLandscape::CreateVertices()
{
	for (int X = 0; X <= XSize; ++X)
	{
		for (int Y = 0; Y <= YSize; ++Y)
		{	float Z = FMath::PerlinNoise2D(FVector2D(X + 0.1 , Y + 0.1)*NoiseScale) * ZMultiplier; // Perlin noise int always return same values
			GEngine->AddOnScreenDebugMessage(-1,999.0f,FColor::Yellow,FString::Printf(TEXT("Z %f"),Z));
			Vertices.Add(FVector(X * Scale, Y * Scale, Z ));
			UV0.Add(FVector2D(X * UVScale, Y * UVScale));
		}
	}
}

void ABasicProceduralLandscape::CreateTriangles()
{
	int Vertex = 0;

	for (int X = 0; X < XSize; ++X)
	{
		for (int Y = 0; Y < YSize; ++Y)
		{
			Triangles.Add(Vertex);//Bottom left corner
			Triangles.Add(Vertex + 1);//Bottom right corner
			Triangles.Add(Vertex + YSize + 1);//Top left corner
			Triangles.Add(Vertex + 1);//Bottom right corner
			Triangles.Add(Vertex + YSize + 2);//Top right corner
			Triangles.Add(Vertex + YSize + 1);//Top left corner

			++Vertex;
		}
		++Vertex;
	}
}