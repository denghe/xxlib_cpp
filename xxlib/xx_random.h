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

		 void Init(int32_t const& seed = 0) noexcept;

		/*====================================Sample====================================
		**Action: Return a new random number [0..1) and reSeed the Seed array.
		**Returns: A double [0..1)
		**Arguments: None
		**Exceptions: None
		==============================================================================*/
		 double Sample() noexcept;

		 int32_t InternalSample() noexcept;
		 double GetSampleForLargeRange() noexcept;

	public:
		explicit Random(int32_t const& seed = 0);
		Random(Random&& o) noexcept;
		Random(Random const&) = delete;
		Random& operator=(Random const&) = delete;
		/*=====================================Next=====================================
		**Returns: An int32_t [0..Int32.MaxValue)
		**Arguments: None
		**Exceptions: None.
		==============================================================================*/
		 int32_t Next() noexcept;

		/*=====================================Next=====================================
		**Returns: An int32_t [minvalue..maxvalue)
		**Arguments: minValue -- the least legal value for the Random number.
		**           maxValue -- One greater than the greatest legal return value.
		**Exceptions: None.
		==============================================================================*/
		int32_t Next(int32_t const& minValue, int32_t const& maxValue) noexcept;


		/*=====================================Next=====================================
		**Returns: An int32_t [0..maxValue)
		**Arguments: maxValue -- One more than the greatest legal return value.
		**Exceptions: None.
		==============================================================================*/
		 int32_t Next(int32_t const& maxValue) noexcept;


		/*=====================================Next=====================================
		**Returns: A double [0..1)
		**Arguments: None
		**Exceptions: None
		==============================================================================*/
		 double NextDouble() noexcept;


		///*==================================NextBytes===================================
		//**Action:  Fills the byte array with random bytes [0..0x7f].  The entire array is filled.
		//**Returns:Void
		//**Arugments:  buffer -- the array to be filled.
		//**Exceptions: None
		//==============================================================================*/
		//void NextBytes(BBuffer* buffer);

		double NextDouble(double const& minValue, double const& maxValue) noexcept;


		// object's interface

		virtual uint16_t GetTypeId() const noexcept override;
		 void ToBBuffer(BBuffer& bb) const noexcept override;
		 int FromBBuffer(BBuffer& bb) noexcept override;

		 void ToString(std::string& s) const noexcept override;
	};

	using Random_s = std::shared_ptr<Random>;
}
