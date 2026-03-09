#include "DattorroReverb.h" #include

// ============================================================ // CosineOsc // ============================================================

float DattorroReverb::CosineOsc::value() const { return std::cos(2.0f \* static\_cast(M\_PI) \* phase); }

float DattorroReverb::CosineOsc::next() { phase += freq; if (phase >= 1.0f) phase -= 1.0f; return value(); }

void DattorroReverb::CosineOsc::setFrequency(float f) { freq = f; }

// ============================================================ // DattorroReverb // ============================================================

DattorroReverb::DattorroReverb() { init(); }

void DattorroReverb::init() { clear(); lfo\_\[0\].setFrequency(0.5f / 32000.0f); lfo\_\[1\].setFrequency(0.3f / 32000.0f); lp\_ = 0.7f; diffusion\_ = 0.625f; lpDecay1\_ = 0.0f; lpDecay2\_ = 0.0f; lastWetL\_ = 0.0f; lastWetR\_ = 0.0f; }

void DattorroReverb::clear() { std::memset(buffer\_, 0, sizeof(buffer\_)); writePtr\_ = 0; lpDecay1\_ = 0.0f; lpDecay2\_ = 0.0f; lastWetL\_ = 0.0f; lastWetR\_ = 0.0f; }

void DattorroReverb::setModSpeed(float speed) { float scale = 0.2f + speed \* 1.6f; lfo\_\[0\].setFrequency(0.5f / 32000.0f \* scale); lfo\_\[1\].setFrequency(0.3f / 32000.0f \* scale); }

// ============================================================ // Delay helpers // ============================================================

float DattorroReverb::readDelay(int base, int offset) const { return buffer\_\[(writePtr\_ + base + offset) & kBufferMask\]; }

float DattorroReverb::readDelayInterp(int base, float offset) const { int intPart = static\_cast(offset); float fracPart = offset - static\_cast(intPart); float a = buffer\_\[(writePtr\_ + base + intPart) & kBufferMask\]; float b = buffer\_\[(writePtr\_ + base + intPart + 1) & kBufferMask\]; return a + (b - a) \* fracPart; }

void DattorroReverb::writeDelay(int base, int offset, float value) { buffer\_\[(writePtr\_ + base + offset) & kBufferMask\] = value; }

float DattorroReverb::readTail(int base, int length) const { return buffer\_\[(writePtr\_ + base + length - 1) & kBufferMask\]; }

// ============================================================ // Context-style helpers // ============================================================

void DattorroReverb::ctxLoad(float value) { accumulator\_ = value; }

void DattorroReverb::ctxRead(float value, float scale) { accumulator\_ += value \* scale; }

void DattorroReverb::ctxReadDelay(int base, int offset, float scale) { float r = readDelay(base, offset); previousRead\_ = r; accumulator\_ += r \* scale; }

void DattorroReverb::ctxReadTail(int base, int length, float scale) { float r = readTail(base, length); previousRead\_ = r; accumulator\_ += r \* scale; }

void DattorroReverb::ctxInterpolate(int base, float offset, float scale) { float x = readDelayInterp(base, offset); previousRead\_ = x; accumulator\_ += x \* scale; }

void DattorroReverb::ctxInterpolateLfo(int base, float offset, int lfoIdx, float amplitude, float scale) { float modOffset = offset + amplitude \* lfoValue\_\[lfoIdx\]; float x = readDelayInterp(base, modOffset); previousRead\_ = x; accumulator\_ += x \* scale; }

void DattorroReverb::ctxWrite(float& value, float scale) { value = accumulator\_; accumulator\_ \*= scale; }

void DattorroReverb::ctxWriteDelay(int base, int offset, float scale) { buffer\_\[(writePtr\_ + base + offset) & kBufferMask\] = accumulator\_; accumulator\_ \*= scale; }

void DattorroReverb::ctxWriteAllPass(int base, int offset, float scale) { buffer\_\[(writePtr\_ + base + offset) & kBufferMask\] = accumulator\_; accumulator\_ \*= scale; accumulator\_ += previousRead\_; }

void DattorroReverb::ctxLp(float& state, float coefficient) { state += coefficient \* (accumulator\_ - state); accumulator\_ = state; }

void DattorroReverb::ctxAdvanceWritePtr() { --writePtr\_; if (writePtr\_ < 0) writePtr\_ += kBufferSize; }

// ============================================================ // Process Dattorro plate reverb (Clouds topology) // ============================================================

void DattorroReverb::process(float\* inOutL, float\* inOutR, int numSamples) { const float kap = diffusion\_; const float klp = lp\_; const float krt = reverbTime\_; const float amt = amount\_; const float gain = inputGain\_;

```
for (int i = 0; i < numSamples; ++i)
{
    ctxAdvanceWritePtr();

    if ((writePtr_ & 31) == 0)
    {
        lfoValue_[0] = lfo_[0].next();
        lfoValue_[1] = lfo_[1].next();
    }

    accumulator_ = 0.0f;
    previousRead_ = 0.0f;

    float wet;
    float apout = 0.0f;

    // Smear AP1
    ctxInterpolateLfo(kAp1Base, 10.0f, 0, 60.0f, 1.0f);
    ctxWriteDelay(kAp1Base, 100, 0.0f);

    // Input (mono sum)
    ctxLoad(0.0f);
    ctxRead(inOutL[i] + inOutR[i], gain);

    // Diffuse through 4 allpasses
    ctxReadTail(kAp1Base, kAp1Len, kap);
    ctxWriteAllPass(kAp1Base, 0, -kap);

    ctxReadTail(kAp2Base, kAp2Len, kap);
    ctxWriteAllPass(kAp2Base, 0, -kap);

    ctxReadTail(kAp3Base, kAp3Len, kap);
    ctxWriteAllPass(kAp3Base, 0, -kap);

    ctxReadTail(kAp4Base, kAp4Len, kap);
    ctxWriteAllPass(kAp4Base, 0, -kap);

    ctxWrite(apout, 0.0f);

    // Loop side A
    ctxLoad(apout);
    ctxInterpolateLfo(kDel2Base, 4680.0f, 1, 100.0f, krt);
    ctxLp(lpDecay1_, klp);

    ctxReadTail(kDap1aBase, kDap1aLen, -kap);
    ctxWriteAllPass(kDap1aBase, 0, kap);

    ctxReadTail(kDap1bBase, kDap1bLen, kap);
    ctxWriteAllPass(kDap1bBase, 0, -kap);

    ctxWriteDelay(kDel1Base, 0, 2.0f);
    ctxWrite(wet, 0.0f);

    lastWetL_ = wet;
    inOutL[i] += (wet - inOutL[i]) * amt;

    // Loop side B
    ctxLoad(apout);
    ctxReadTail(kDel1Base, kDel1Len, krt);
    ctxLp(lpDecay2_, klp);

    ctxReadTail(kDap2aBase, kDap2aLen, kap);
    ctxWriteAllPass(kDap2aBase, 0, -kap);

    ctxReadTail(kDap2bBase, kDap2bLen, -kap);
    ctxWriteAllPass(kDap2bBase, 0, kap);

    ctxWriteDelay(kDel2Base, 0, 2.0f);
    ctxWrite(wet, 0.0f);

    lastWetR_ = wet;
    inOutR[i] += (wet - inOutR[i]) * amt;
}
```

}