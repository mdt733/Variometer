#define MAX_Z_SAMPLES   10
#define SENSOR_SAMPLES_PER_SEC  25L

class REG
{
  public:
    int numSamples;
    long gnLRDenominator;
    int gnSampleIndex, gnNumSamples;
    long gZBuffer[MAX_Z_SAMPLES];
    long gZAverage;
    long gSlope;
    
    REG()
    {

    }

    void lr_Init(long zSample, int _numSamples) {
      numSamples = _numSamples;
      long sumT, sumT2;
      int inx;
      sumT = -(numSamples * (numSamples - 1L)) / 2L;
      sumT2 = (numSamples * (numSamples - 1L) * (2L * numSamples - 1L)) / 6L;
      gnLRDenominator = (numSamples * sumT2) - (sumT * sumT);
      gnSampleIndex = 0;
      gnNumSamples = numSamples;
      inx = gnNumSamples;
      while (inx--) gZBuffer[inx] = zSample;  // fill the ZBuffer with first sample value
    }

    void lr_Sample(long newSample)
    {
      gZBuffer[gnSampleIndex] = newSample;
      gnSampleIndex++;
      if (gnSampleIndex >= gnNumSamples) gnSampleIndex = 0;
    }

    void lr_CalculateAverage() {
      int inx;
      long accumulator, average;
      inx = numSamples;
      accumulator = 0;
      while (inx--)  {
        accumulator += gZBuffer[inx];
      }
      accumulator = (accumulator >= 0 ? accumulator + numSamples / 2 : accumulator - numSamples / 2);
      gZAverage = accumulator / numSamples; // rounded up average
    }

    /// Linear regression of samples in buffer to calculate slope.
    void lr_CalculateSlope()   {
      int inx, tRelative;
      long z, sumZT, slope;

      sumZT = 0;
      inx = numSamples;
      while (inx--)  {
        z = gZBuffer[inx] - gZAverage;   // subtract out the average value to simplify the arithmetic
        tRelative = inx - gnSampleIndex; // time origin is the current sample in window
        if (tRelative > 0) {
          tRelative -= numSamples;
        }
        sumZT += ((long)tRelative * z);
      }

      gSlope = (sumZT * (long)(SENSOR_SAMPLES_PER_SEC * numSamples)) / gnLRDenominator;
    }

};

REG reg;
