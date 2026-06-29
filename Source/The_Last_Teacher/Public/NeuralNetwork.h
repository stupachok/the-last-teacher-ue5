#pragma once

#include "CoreMinimal.h"

/**
 * Feedforward neural network for NPC decision making.
 * Architecture: 9 inputs -> 12 hidden neurons (ReLU) -> 4 outputs (Softmax)
 *
 * Inputs (features):
 *   [0] Distance to nearest monster (normalized 0-1)
 *   [1] Has monster nearby (0 or 1)
 *   [2] Bot HP (normalized 0-1)
 *   [3] Distance to nearest broken block (normalized 0-1)
 *   [4] Has broken block (0 or 1)
 *   [5] Distance to player (normalized 0-1)
 *   [6] Is player calling (IsFollowing flag)
 *   [7] Repair skill (normalized 0-1)
 *   [8] Fight skill (normalized 0-1)
 *
 * Outputs (actions):
 *   [0] Fight monster
 *   [1] Repair block
 *   [2] Follow player
 *   [3] Go home
 */
class THE_LAST_TEACHER_API NeuralNetwork
{
public:
    NeuralNetwork();
    ~NeuralNetwork();

    static constexpr int32 InputSize = 9;
    static constexpr int32 HiddenSize = 12;
    static constexpr int32 OutputSize = 4;

    int32 Predict(const TArray<float>& Inputs);
    TArray<float> Forward(const TArray<float>& Inputs);
    void LoadDefaultWeights();
    bool LoadWeightsFromFile(const FString& FilePath);

private:
    float WeightsInputHidden[HiddenSize][InputSize];
    float BiasesHidden[HiddenSize];
    float WeightsHiddenOutput[OutputSize][HiddenSize];
    float BiasesOutput[OutputSize];

    static float ReLU(float X);
    static void Softmax(TArray<float>& Values);
};