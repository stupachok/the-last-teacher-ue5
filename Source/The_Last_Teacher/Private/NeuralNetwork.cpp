#include "NeuralNetwork.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

NeuralNetwork::NeuralNetwork()
{
    LoadDefaultWeights();
}

NeuralNetwork::~NeuralNetwork()
{
}

float NeuralNetwork::ReLU(float X)
{
    return X > 0.0f ? X : 0.0f;
}

void NeuralNetwork::Softmax(TArray<float>& Values)
{
    float MaxVal = Values[0];
    for (int32 i = 1; i < Values.Num(); ++i)
        if (Values[i] > MaxVal) MaxVal = Values[i];

    float Sum = 0.0f;
    for (int32 i = 0; i < Values.Num(); ++i)
    {
        Values[i] = FMath::Exp(Values[i] - MaxVal);
        Sum += Values[i];
    }

    if (Sum > 0.0f)
        for (int32 i = 0; i < Values.Num(); ++i)
            Values[i] /= Sum;
}

TArray<float> NeuralNetwork::Forward(const TArray<float>& Inputs)
{
    TArray<float> Output;
    Output.Init(0.0f, OutputSize);

    if (Inputs.Num() != InputSize)
    {
        for (int32 i = 0; i < OutputSize; ++i) Output[i] = 1.0f / OutputSize;
        return Output;
    }

    // Layer 1: Input -> Hidden
    TArray<float> HiddenLayer;
    HiddenLayer.Init(0.0f, HiddenSize);

    for (int32 h = 0; h < HiddenSize; ++h)
    {
        float Sum = BiasesHidden[h];
        for (int32 i = 0; i < InputSize; ++i)
            Sum += Inputs[i] * WeightsInputHidden[h][i];
        HiddenLayer[h] = ReLU(Sum);
    }

    // Layer 2: Hidden -> Output
    for (int32 o = 0; o < OutputSize; ++o)
    {
        float Sum = BiasesOutput[o];
        for (int32 h = 0; h < HiddenSize; ++h)
            Sum += HiddenLayer[h] * WeightsHiddenOutput[o][h];
        Output[o] = Sum;
    }

    Softmax(Output);
    return Output;
}

int32 NeuralNetwork::Predict(const TArray<float>& Inputs)
{
    TArray<float> Probabilities = Forward(Inputs);

    int32 BestAction = 0;
    float BestScore = Probabilities[0];
    for (int32 i = 1; i < Probabilities.Num(); ++i)
    {
        if (Probabilities[i] > BestScore)
        {
            BestScore = Probabilities[i];
            BestAction = i;
        }
    }
    return BestAction;
}

void NeuralNetwork::LoadDefaultWeights()
{
    // Feature indices:
    //   [0] DistToMonster  [1] HasMonster   [2] BotHP        [3] DistToBlock
    //   [4] HasBlock       [5] DistToPlayer [6] IsFollowing  [7] RepairSkill
    //   [8] FightSkill
    //
    // Action indices:
    //   [0] Fight  [1] Repair  [2] Follow  [3] GoHome

    // Zero everything first
    for (int32 h = 0; h < HiddenSize; ++h)
    {
        BiasesHidden[h] = 0.0f;
        for (int32 i = 0; i < InputSize; ++i)
            WeightsInputHidden[h][i] = 0.0f;
    }
    for (int32 o = 0; o < OutputSize; ++o)
    {
        BiasesOutput[o] = 0.0f;
        for (int32 h = 0; h < HiddenSize; ++h)
            WeightsHiddenOutput[o][h] = 0.0f;
    }

    // ===== HIDDEN NEURONS =====

    // Neuron 0: Monster threat Ч потребуЇ FightSkill
    WeightsInputHidden[0][1] = 2.0f;   // HasMonster
    WeightsInputHidden[0][0] = -1.5f;  // DistToMonster
    WeightsInputHidden[0][8] = 3.0f;   // FightSkill Ч без нього не б'Їтьс€
    BiasesHidden[0] = -2.0f;

    // Neuron 1: Block repair Ч потребуЇ RepairSkill ≥ зламаний блок
    WeightsInputHidden[1][4] = 2.0f;   // HasBlock
    WeightsInputHidden[1][3] = -1.0f;  // DistToBlock
    WeightsInputHidden[1][7] = 3.0f;   // RepairSkill Ч нав≥ть мале значенн€ допомагаЇ
    BiasesHidden[1] = -1.5f;           // низький пор≥г Ч легко активуЇтьс€

    // Neuron 2: Follow signal
    WeightsInputHidden[2][6] = 4.0f;   // IsFollowing
    BiasesHidden[2] = -1.0f;

    // Neuron 3: Idle Ч нема задач
    WeightsInputHidden[3][1] = -1.0f;  // нема монстра
    WeightsInputHidden[3][4] = -1.0f;  // нема блока
    WeightsInputHidden[3][6] = -1.0f;  // не сл≥дуЇ
    BiasesHidden[3] = 2.0f;

    // Neuron 4: Low HP
    WeightsInputHidden[4][2] = -2.0f;
    BiasesHidden[4] = 1.0f;

    // Neurons 5-11: допом≥жн≥
    WeightsInputHidden[5][0] = 0.3f;   WeightsInputHidden[5][2] = 0.5f;
    WeightsInputHidden[6][7] = 1.0f;   WeightsInputHidden[6][4] = 0.5f;
    WeightsInputHidden[7][5] = -0.5f;  WeightsInputHidden[7][6] = 0.8f;
    WeightsInputHidden[8][1] = 0.5f;   WeightsInputHidden[8][2] = -0.3f;
    WeightsInputHidden[9][3] = -0.3f;  WeightsInputHidden[9][7] = 0.5f;
    WeightsInputHidden[10][0] = 0.2f;  WeightsInputHidden[10][6] = -0.4f;
    WeightsInputHidden[11][4] = 0.6f;  WeightsInputHidden[11][3] = -0.3f;

    // ===== OUTPUT WEIGHTS =====

    // Action 0 (Fight) Ч потр≥бен FightSkill
    WeightsHiddenOutput[0][0] = 3.0f;   // Monster detector
    WeightsHiddenOutput[0][4] = -1.5f;  // Low HP знижуЇ
    BiasesOutput[0] = -3.0f;            // жорсткий пор≥г

    // Action 1 (Repair) Ч нав≥ть малий RepairSkill + блок = йде лагодити
    WeightsHiddenOutput[1][1] = 3.0f;   // Block+RepairSkill detector
    WeightsHiddenOutput[1][6] = 0.5f;
    WeightsHiddenOutput[1][0] = -0.5f;  // ћонстр трохи знижуЇ
    BiasesOutput[1] = -1.0f;            // низький пор≥г Ч легко спрацьовуЇ

    // Action 2 (Follow) Ч найвищий пр≥оритет через IsFollowing
    // (але вже перехоплюЇтьс€ в BTService_DecideAction до нейромереж≥)
    WeightsHiddenOutput[2][2] = 5.0f;
    WeightsHiddenOutput[2][0] = -0.5f;
    BiasesOutput[2] = -1.0f;

    // Action 3 (GoHome) Ч дефолт коли нема ск≥л≥в ≥ задач
    WeightsHiddenOutput[3][3] = 2.0f;   // Idle detector
    BiasesOutput[3] = -1.0f;            // середн≥й пор≥г Ч не перемагаЇ Repair
}

bool NeuralNetwork::LoadWeightsFromFile(const FString& FilePath)
{
    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
        return false;

    TArray<FString> Lines;
    FileContent.ParseIntoArrayLines(Lines);

    int32 Expected = (HiddenSize * InputSize) + HiddenSize
        + (OutputSize * HiddenSize) + OutputSize;

    if (Lines.Num() < Expected)
        return false;

    int32 Idx = 0;
    for (int32 h = 0; h < HiddenSize; ++h)
        for (int32 i = 0; i < InputSize; ++i)
            WeightsInputHidden[h][i] = FCString::Atof(*Lines[Idx++]);

    for (int32 h = 0; h < HiddenSize; ++h)
        BiasesHidden[h] = FCString::Atof(*Lines[Idx++]);

    for (int32 o = 0; o < OutputSize; ++o)
        for (int32 h = 0; h < HiddenSize; ++h)
            WeightsHiddenOutput[o][h] = FCString::Atof(*Lines[Idx++]);

    for (int32 o = 0; o < OutputSize; ++o)
        BiasesOutput[o] = FCString::Atof(*Lines[Idx++]);

    return true;
}