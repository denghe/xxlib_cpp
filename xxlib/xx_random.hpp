#pragma once
#include "xx_random.h"
#include "xx_bbuffer.h"
namespace xx {
	inline void Random::Init(int32_t const& seed) noexcept {
		int32_t ii;
		int32_t mj, mk;

		//Initialize our Seed array.
		//This algorithm comes from Numerical Recipes in C (2nd Ed.)
		int32_t subtraction = (seed == std::numeric_limits<int32_t>::min()) ? std::numeric_limits<int32_t>::max() : std::abs(seed);
		mj = MSEED - subtraction;
		seedArray[55] = mj;
		mk = 1;
		for (int32_t i = 1; i < 55; i++)
		{  //Apparently the range [1..55] is special (Knuth) and so we're wasting the 0'th position.
			ii = (21 * i) % 55;
			seedArray[ii] = mk;
			mk = mj - mk;
			if (mk < 0) mk += MBIG;
			mj = seedArray[ii];
		}
		for (int32_t k = 1; k < 5; k++)
		{
			for (int32_t i = 1; i < 56; i++)
			{
				seedArray[i] -= seedArray[1 + (i + 30) % 55];
				if (seedArray[i] < 0) seedArray[i] += MBIG;
			}
		}
		inext = 0;
		inextp = 21;
	}

	/*====================================Sample====================================
	**Action: Return a new random number [0..1) and reSeed the Seed array.
	**Returns: A double [0..1)
	**Arguments: None
	**Exceptions: None
	==============================================================================*/
	inline double Random::Sample() noexcept {
		//Including this division at the end gives us significantly improved
		//random number distribution.
		return (InternalSample() * (1.0 / MBIG));
	}

	inline int32_t Random::InternalSample() noexcept {
		int32_t retVal;
		int32_t locINext = inext;
		int32_t locINextp = inextp;

		if (++locINext >= 56) locINext = 1;
		if (++locINextp >= 56) locINextp = 1;

		retVal = seedArray[locINext] - seedArray[locINextp];

		if (retVal == MBIG) retVal--;
		if (retVal < 0) retVal += MBIG;

		seedArray[locINext] = retVal;

		inext = locINext;
		inextp = locINextp;

		return retVal;
	}
	inline double Random::GetSampleForLargeRange() noexcept {
		// The distribution of double value returned by Sample 
		// is not distributed well enough for a large range.
		// If we use Sample for a range [Int32.MinValue..Int32.MaxValue)
		// We will end up getting even numbers only.

		int32_t result = InternalSample();
		// Note we can't use addition here. The distribution will be bad if we do that.
		bool negative = (InternalSample() % 2 == 0) ? true : false;  // decide the sign based on second sample
		if (negative)
		{
			result = -result;
		}
		double d = result;
		d += (std::numeric_limits<int32_t>::max() - 1); // get a number in range [0 .. 2 * Int32MaxValue - 1)
		d /= 2 * (uint32_t)std::numeric_limits<int32_t>::max() - 1;
		return d;
	}

	inline Random::Random(int32_t const& seed) {
		memset(seedArray, 0, sizeof(seedArray));
		Init(seed);
	}
	inline Random::Random(Random && o) noexcept {
		inext = o.inext;
		inextp = o.inextp;
		memcpy(seedArray, o.seedArray, sizeof(seedArray));
	}
	/*=====================================Next=====================================
	**Returns: An int32_t [0..Int32.MaxValue)
	**Arguments: None
	**Exceptions: None.
	==============================================================================*/
	inline int32_t Random::Next() noexcept {
		return InternalSample();
	}

	/*=====================================Next=====================================
	**Returns: An int32_t [minvalue..maxvalue)
	**Arguments: minValue -- the least legal value for the Random number.
	**           maxValue -- One greater than the greatest legal return value.
	**Exceptions: None.
	==============================================================================*/
	inline int32_t Random::Next(int32_t const& minValue, int32_t const& maxValue) noexcept {
		assert(minValue <= maxValue);

		int64_t range = (int64_t)maxValue - minValue;
		if (range <= (int64_t)std::numeric_limits<int32_t>::max())
		{
			return ((int32_t)(Sample() * (double)range) + minValue);
		}
		else
		{
			return (int32_t)((int64_t)(GetSampleForLargeRange() * (double)range) + minValue);
		}
	}


	/*=====================================Next=====================================
	**Returns: An int32_t [0..maxValue)
	**Arguments: maxValue -- One more than the greatest legal return value.
	**Exceptions: None.
	==============================================================================*/
	inline int32_t Random::Next(int32_t const& maxValue) noexcept {
		assert(maxValue >= 0);
		return (int32_t)(Sample() * maxValue);
	}


	/*=====================================Next=====================================
	**Returns: A double [0..1)
	**Arguments: None
	**Exceptions: None
	==============================================================================*/
	inline double Random::NextDouble() noexcept {
		return Sample();
	}


	///*==================================NextBytes===================================
	//**Action:  Fills the byte array with random bytes [0..0x7f].  The entire array is filled.
	//**Returns:Void
	//**Arugments:  buffer -- the array to be filled.
	//**Exceptions: None
	//==============================================================================*/
	//void NextBytes(BBuffer* buffer);

	inline double Random::NextDouble(double const& minValue, double const& maxValue) noexcept {
		if (minValue == maxValue || maxValue - minValue <= 0) return minValue;
		return minValue + (maxValue - minValue) * NextDouble();
	}


	// object's interface

	inline uint16_t Random::GetTypeId() const noexcept {
		return TypeId_v<Random>;
	}

	inline void Random::ToBBuffer(BBuffer & bb) const noexcept {
		// data len = 2 int + int[56] = 4 * 58 = 232
		bb.Reserve(bb.len + 232);
		memcpy(bb.buf + bb.len, &inext, 232);
		bb.len += 232;
	}

	inline int Random::FromBBuffer(BBuffer & bb) noexcept {
		if (bb.offset + 232 > bb.len) return -1;
		memcpy(&inext, bb.buf + bb.offset, 232);
		bb.offset += 232;
		return 0;
	}

	inline void Random::ToString(std::string & s) const noexcept {
		s.append("{ \"type\":\"Random\" }");
	}
}
