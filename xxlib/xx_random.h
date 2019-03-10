#pragma once
#include "xx_object.h"
namespace xx {
	// 从 .NET System.Random 翻写, 理论上讲相同种子能输出相同结果. 支持序列化.
	// 必须传入种子

	struct BBuffer;
	struct Random : Object {
	protected:
		static constexpr int32_t MBIG = std::numeric_limits<int32_t>::max();
		static constexpr int32_t MSEED = 161803398;
		static constexpr int32_t MZ = 0;

	public:
		int32_t inext;
		int32_t inextp;
		int32_t seedArray[56];
	protected:

		inline void Init(int32_t const& seed = 0) noexcept {
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
		inline double Sample() noexcept {
			//Including this division at the end gives us significantly improved
			//random number distribution.
			return (InternalSample() * (1.0 / MBIG));
		}

		inline int32_t InternalSample() noexcept {
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
		inline double GetSampleForLargeRange() noexcept {
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

	public:
		explicit Random(int32_t const& seed = 0) {
			memset(seedArray, 0, sizeof(seedArray));
			Init(seed);
		}
		Random(Random&& o) {
			inext = o.inext;
			inextp = o.inextp;
			memcpy(seedArray, o.seedArray, sizeof(seedArray));
		}
		Random(Random const&) = delete;
		Random& operator=(Random const&) = delete;
		/*=====================================Next=====================================
		**Returns: An int32_t [0..Int32.MaxValue)
		**Arguments: None
		**Exceptions: None.
		==============================================================================*/
		inline int32_t Next() noexcept {
			return InternalSample();
		}

		/*=====================================Next=====================================
		**Returns: An int32_t [minvalue..maxvalue)
		**Arguments: minValue -- the least legal value for the Random number.
		**           maxValue -- One greater than the greatest legal return value.
		**Exceptions: None.
		==============================================================================*/
		inline int32_t Next(int32_t const& minValue, int32_t const& maxValue) noexcept {
			assert(minValue <= maxValue);

			int64_t range = (int64_t)maxValue - minValue;
			if (range <= (int64_t)std::numeric_limits<int32_t>::max())
			{
				return ((int32_t)(Sample() * range) + minValue);
			}
			else
			{
				return (int32_t)((int64_t)(GetSampleForLargeRange() * range) + minValue);
			}
		}


		/*=====================================Next=====================================
		**Returns: An int32_t [0..maxValue)
		**Arguments: maxValue -- One more than the greatest legal return value.
		**Exceptions: None.
		==============================================================================*/
		inline int32_t Next(int32_t const& maxValue) noexcept {
			assert(maxValue >= 0);
			return (int32_t)(Sample() * maxValue);
		}


		/*=====================================Next=====================================
		**Returns: A double [0..1)
		**Arguments: None
		**Exceptions: None
		==============================================================================*/
		inline double NextDouble() noexcept {
			return Sample();
		}


		///*==================================NextBytes===================================
		//**Action:  Fills the byte array with random bytes [0..0x7f].  The entire array is filled.
		//**Returns:Void
		//**Arugments:  buffer -- the array to be filled.
		//**Exceptions: None
		//==============================================================================*/
		//void NextBytes(BBuffer* buffer);

		double NextDouble(double const& minValue, double const& maxValue) noexcept {
			if (minValue == maxValue || maxValue - minValue <= 0) return minValue;
			return minValue + (maxValue - minValue) * NextDouble();
		}


		// object's interface

		inline void ToBBuffer(BBuffer& bb) const noexcept;
		inline int FromBBuffer(BBuffer& bb) noexcept;

		inline void ToString(std::string& s) const noexcept {
			s.append("{ \"type\":\"Random\" }");
		}
	};
}
