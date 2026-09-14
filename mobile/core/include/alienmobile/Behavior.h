#pragma once

#include <array>
#include <cstddef>
#include "alienmobile/Math.h"

namespace alienmobile {
inline constexpr std::size_t SignalChannelCount = 8;
using Signals = std::array<float, SignalChannelCount>;
enum SignalChannel : std::size_t { Oscillator, EnergyX, EnergyY, EnergyIntensity,
    CreatureX, CreatureY, CreatureIntensity, Activation };
enum class CellRole { Structural, Constructor, Generator, Motor, EnergySensor, CreatureSensor, Attacker, Digestor, Depot, Defender, Memory, ObstacleSensor, Sender, Receiver, Count };
enum class MemoryMode { Delay, Integrate };
enum class Waveform { Sine, Square };
enum class MotorMode { Thrust, Contractile, Bending };

struct BehaviorGene {
    CellRole role = CellRole::Structural;
    Waveform waveform = Waveform::Square;
    MotorMode motorMode = MotorMode::Thrust;
    float period = 2.0f;
    float phase = 0.0f; // cycles
    float amplitude = 1.0f;
    float bendingAngle = 0.65f;
    float contraction = 0.28f; // Maximum fractional change of the parent spring.
    float motorStrength = 1.0f;
    float axisAngle = 0.0f; // relative to the physical parent-to-cell edge
    float signalWeight = 1.0f; // incoming weight for the genomic parent edge
    std::size_t motorChannel = Oscillator;
    float sensorRange = 4.0f;
    float sensitivity = 1.0f;
    float extractionRate = 1.0f;
    float digestionRate = 1.0f;
    float storageCapacity = 3.0f; // Extra usable reservoir, in energy units.
    float defenseStrength = 1.0f;
    MemoryMode memoryMode = MemoryMode::Delay;
    float memoryTime = 0.5f;
    bool neural = false; // false: residual relay; true: replacement tanh network.
    float selfWeight = 0.0f;
    std::array<Signals, SignalChannelCount> weights{};
    Signals biases{};
};
inline bool operator==(BehaviorGene const& a, BehaviorGene const& b) {
    return a.role==b.role && a.waveform==b.waveform && a.motorMode==b.motorMode
        && a.period==b.period && a.phase==b.phase && a.amplitude==b.amplitude
        && a.bendingAngle==b.bendingAngle && a.contraction==b.contraction && a.motorStrength==b.motorStrength && a.axisAngle==b.axisAngle
        && a.signalWeight==b.signalWeight && a.motorChannel==b.motorChannel
        && a.sensorRange==b.sensorRange && a.sensitivity==b.sensitivity
        && a.extractionRate==b.extractionRate && a.digestionRate==b.digestionRate
        && a.storageCapacity==b.storageCapacity && a.defenseStrength==b.defenseStrength
        && a.memoryMode==b.memoryMode && a.memoryTime==b.memoryTime && a.neural==b.neural && a.selfWeight==b.selfWeight && a.weights==b.weights && a.biases==b.biases;
}
class World;
Vec2 cellAxis(World const& world, std::size_t index);
Signals energySensor(World const& world, std::size_t index);
Signals creatureSensor(World const& world, std::size_t index);
float generatorOutput(BehaviorGene const& gene, double age);
// Separate read/evaluate and commit passes. Newly constructed cells are dormant.
void updateSignals(World& world, float dt);
// Produces per-cell forces; physics alone integrates them.
double updateActuation(World& world, float dt, float energyPerForceSecond);
}
