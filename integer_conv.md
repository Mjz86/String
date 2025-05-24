# converting a base2 integer to a base10 string


# introduction:
 i made a way to convert 8digit ingeters with only 6  64bit-integer multiplications , without a lookup.


# details:
 if we look at most compiler outputs (in ghidra) for devitions , we see a patter of :
```
const idk_t factor =ceil((1<<N)/k);

 X/k  as (X*factor) >>N
```

this gave me an idea , 

what if we use the smallest posible N to make it faster?

 i got to desmos and found some pretty good values :


 such that 
`floor((fllor(X)*ceil((2^N)/k)) /2^N) == floor(X/k)`

i wanted  the devition for powers of 10 ( or at leat 5) , 
and surprisingly i found very good ones:


`52(x/2)/256` for numbers less than 100 to divide by 10.


`2622*(x/4)/65536` for numbers less than 10000 to divide by 100.



and i got an idea , what if we devide all of them in a single multiply instruction,

this may sound crazy , but its possible ,

if we want to do a multiply of 8 bit integers , 16 bits is enough  for getting the overflow,

so we can put 4 16bit integers side by side in a 64bit one , and multiply that , 
we get it as if we did each of them separately  ,

this is valid for 2 32 muls as well


in ascii '0'|n is 'n' .

so we can now write the code 
:
(by making them signed (  it wont overflow because upper bit of ascii is 0, so its always positove ) , the compiler can pull more triks)

(Note that 

10x10<128
and
100x100<2^16

Ascii char is positive 

So
No overflow and safe)

```c++


{ 
 const int32_t number_less_than_pow10_8{/*...*/};
constexpr int64_t zero_8parallel_ascii = 0x3030303030303030;
const int32_t upper_half{number_less_than_pow10_8 / 10000};
const int32_t lower_half{number_less_than_pow10_8 % 10000};
const int64_t awnser_8parallel =
    [](const int32_t lower_half,
       const int32_t upper_half) noexcept -> int64_t {
  constexpr int64_t inv25_16b = 2622;

  constexpr int64_t inv5_8b = 52;

  constexpr int64_t mask_16b =
      int64_t(uint16_t(-1)) | (int64_t(uint16_t(-1)) << 32);
  constexpr int64_t mask_8b =
      int64_t(uint8_t(-1)) | (int64_t(uint8_t(-1)) << 16) |
      (int64_t(uint8_t(-1)) << 32) | (int64_t(uint8_t(-1)) << 48);
  if constexpr (std::endian::big==std::endian::native) {
    const int64_t div_2parellel_old =
        ((int64_t(upper_half) << 32) | int64_t(lower_half));

    const int64_t div_2parallel =
        ((((div_2parellel_old >> 2) & mask_16b) * inv25_16b) >> 16) &
        mask_16b;

    const int64_t modulo_2parallel = div_2parellel_old - 100 * div_2parallel;

    const int64_t div_4parellel_old =
        modulo_2parallel | (div_2parallel << 16);

    const int64_t div_4parellel =
        ((((div_4parellel_old >> 1) & mask_8b) * inv5_8b) >> 8) & mask_8b;

    const int64_t modulo_4parallel = div_4parellel_old - 10 * div_4parellel;
    return modulo_4parallel | (div_4parellel << 8);
  } else {
    const int64_t div_2parellel_old =
        (int64_t(upper_half) | (int64_t(lower_half) << 32));

    const int64_t div_2parallel =
        ((((div_2parellel_old >> 2) & mask_16b) * inv25_16b) >> 16) &
        mask_16b;

    const int64_t modulo_2parallel = div_2parellel_old - 100 * div_2parallel;

    const int64_t div_4parellel_old =
        div_2parallel | (modulo_2parallel << 16);

    const int64_t div_4parellel =
        ((((div_4parellel_old >> 1) & mask_8b) * inv5_8b) >> 8) & mask_8b;

    const int64_t modulo_4parallel = div_4parellel_old - 10 * div_4parellel;
    return div_4parellel | (modulo_4parallel << 8);
  }
}(lower_half, upper_half);
const int num_0ch = ((std::endian::big == std::endian::native
                                        ? std::countl_zero(uint64_t(awnser_8parallel))
                   : std::countr_zero(uint64_t(awnser_8parallel))) >>
    3);
const int64_t awnser_8parallel_ascii =
    awnser_8parallel | zero_8parallel_ascii;
/* do bitcasting to char[8]*/
}

```





the tested code:
https://quick-bench.com/q/lNS-MaCq9DrIUaS-OL5y3mjuKy8


```c++
#include <bit>
#include<array>
#include <cstring>
#include<string>
  static int64_t  dec_from_uint_backwards_parallel_less_than_pow10_8_pair_impl_(
      const int32_t lower_half, const int32_t upper_half) noexcept { 
    constexpr int64_t inv25_16b = 2622;

    constexpr int64_t inv5_8b = 52;

    constexpr int64_t mask_16b =
        int64_t(uint16_t(-1)) | (int64_t(uint16_t(-1)) << 32);
    constexpr int64_t mask_8b =
        int64_t(uint8_t(-1)) | (int64_t(uint8_t(-1)) << 16) |
        (int64_t(uint8_t(-1)) << 32) | (int64_t(uint8_t(-1)) << 48);
    if constexpr (std::endian::big == std::endian::native) {
      const int64_t div_2parellel_old =
          ((int64_t(upper_half) << 32) | int64_t(lower_half));

      const int64_t div_2parallel =
          ((((div_2parellel_old >> 2) & mask_16b) * inv25_16b) >> 16) &
          mask_16b;

      const int64_t modulo_2parallel = div_2parellel_old - 100 * div_2parallel;

      const int64_t div_4parellel_old =
          modulo_2parallel | (div_2parallel << 16);

      const int64_t div_4parellel =
          ((((div_4parellel_old >> 1) & mask_8b) * inv5_8b) >> 8) & mask_8b;

      const int64_t modulo_4parallel = div_4parellel_old - 10 * div_4parellel;
      return modulo_4parallel | (div_4parellel << 8);
    } else {
      const int64_t div_2parellel_old =
          (int64_t(upper_half) | (int64_t(lower_half) << 32));

      const int64_t div_2parallel =
          ((((div_2parellel_old >> 2) & mask_16b) * inv25_16b) >> 16) &
          mask_16b;

      const int64_t modulo_2parallel = div_2parellel_old - 100 * div_2parallel;

      const int64_t div_4parellel_old =
          div_2parallel | (modulo_2parallel << 16);

      const int64_t div_4parellel =
          ((((div_4parellel_old >> 1) & mask_8b) * inv5_8b) >> 8) & mask_8b;

      const int64_t modulo_4parallel = div_4parellel_old - 10 * div_4parellel;
      return div_4parellel | (modulo_4parallel << 8);
    }
  }   
  
   static std::tuple<std::array<int64_t,3>,
      size_t, size_t, size_t>
  dec_from_uint_impl_semi_parallel_impl_(const int64_t number_) noexcept {
    constexpr int64_t zero_8parallel_ascii = 0x3030303030303030;
    constexpr int64_t lookup_full = 1000;
    constexpr int64_t parallel_half = 10000;
    constexpr int64_t parallel_full = parallel_half * parallel_half;
    constexpr int64_t count_max =3;
    std::array<int64_t,3> str_int_buf;
    int64_t number_0_ = number_;
    for(size_t i{},iteration_count_backwards{count_max};i<count_max;i++){
    
      iteration_count_backwards--;
      int32_t upper_half{};
      int32_t lower_half{};

      int64_t u64ch_{};
   
    
    
    if(number_0_ < parallel_half){
        lower_half = int32_t(number_0_);
        upper_half = 0;
       number_0_ = 0;
    }else{       
 
        int32_t number_less_than_pow10_8;
        if (number_0_ < parallel_full) {
          number_less_than_pow10_8 = int32_t(number_0_);
          number_0_ = 0;
        } else {
          number_less_than_pow10_8 = int32_t(number_0_ % parallel_full);
          number_0_ = number_0_ / parallel_full;
        }
        
        lower_half = int32_t(number_less_than_pow10_8 % parallel_half);
        upper_half = int32_t(number_less_than_pow10_8 / parallel_half);
        }
        u64ch_ = dec_from_uint_backwards_parallel_less_than_pow10_8_pair_impl_(lower_half, upper_half);
      
      const int64_t u64ch = u64ch_;
      int64_t u64ch_ascii = u64ch | zero_8parallel_ascii;
      str_int_buf[count_max - 1 - iteration_count_backwards] =
          u64ch_ascii;
      if (number_0_) continue;
      const int64_t num_high_0ch =
          size_t((  std::endian::big == std::endian::native? std::countl_zero(u64ch)
                                       : std::countr_zero(u64ch)) >>
                    3);
      const size_t num_0ch{num_high_0ch +
                              size_t(iteration_count_backwards << 3)};
      const size_t num_ch =
          std::max<size_t>(sizeof(str_int_buf) - num_0ch, 1);
      return {str_int_buf, num_ch, iteration_count_backwards,
              std::min<size_t>(7, num_high_0ch)};
    }

    return {};
  }
  
  static std::string  dec_from_uint_impl_semi_parallel(int64_t number_0_) noexcept {
    const auto [str_int_buf, num_ch, iteration_count_backwards, offset] =
        dec_from_uint_impl_semi_parallel_impl_(number_0_);
    std::string s{};
        s.resize_and_overwrite(num_ch, [&str_int_buf,offset](char* buf, std::size_t num_ch_) noexcept
    {
  std::memcpy(buf, reinterpret_cast<const char *>(&str_int_buf) + offset,
           num_ch_); 
        return num_ch_;
    });
    return s;
  }

static void rand_base(benchmark::State& state) { 
  
  for (auto _ : state) { 

    benchmark::DoNotOptimize(int64_t(std::rand()));//|(int64_t(std::rand())<<32) );
  }
}
// Register the function as a benchmark
BENCHMARK(rand_base);

static void mjz_to_string(benchmark::State& state) { 
  
  for (auto _ : state) { 
const  int64_t val= int64_t(std::rand());//|(int64_t(std::rand())<<32) ;
  std::string str=dec_from_uint_impl_semi_parallel(val);
    benchmark::DoNotOptimize(str);
  }
}
// Register the function as a benchmark
BENCHMARK(mjz_to_string);

static void std_to_string(benchmark::State& state) {  

  for (auto _ : state) {
const  int64_t val= int64_t(std::rand());//|(int64_t(std::rand())<<32) ;
  std::string str=std::to_string(val);
    benchmark::DoNotOptimize(str);
  }
}
BENCHMARK(std_to_string);
```



also for peridictable inputs:
https://quick-bench.com/q/jZoAaiGPgY_dxYe4rWXRPbnxaaU


```c++


#include <bit>
#include<array>
#include <cstring>
#include<string>
  static int64_t  dec_from_uint_backwards_parallel_less_than_pow10_8_pair_impl_(
      const int32_t lower_half, const int32_t upper_half) noexcept { 
    constexpr int64_t inv25_16b = 2622;

    constexpr int64_t inv5_8b = 52;

    constexpr int64_t mask_16b =
        int64_t(uint16_t(-1)) | (int64_t(uint16_t(-1)) << 32);
    constexpr int64_t mask_8b =
        int64_t(uint8_t(-1)) | (int64_t(uint8_t(-1)) << 16) |
        (int64_t(uint8_t(-1)) << 32) | (int64_t(uint8_t(-1)) << 48);
    if constexpr (std::endian::big == std::endian::native) {
      const int64_t div_2parellel_old =
          ((int64_t(upper_half) << 32) | int64_t(lower_half));

      const int64_t div_2parallel =
          ((((div_2parellel_old >> 2) & mask_16b) * inv25_16b) >> 16) &
          mask_16b;

      const int64_t modulo_2parallel = div_2parellel_old - 100 * div_2parallel;

      const int64_t div_4parellel_old =
          modulo_2parallel | (div_2parallel << 16);

      const int64_t div_4parellel =
          ((((div_4parellel_old >> 1) & mask_8b) * inv5_8b) >> 8) & mask_8b;

      const int64_t modulo_4parallel = div_4parellel_old - 10 * div_4parellel;
      return modulo_4parallel | (div_4parellel << 8);
    } else {
      const int64_t div_2parellel_old =
          (int64_t(upper_half) | (int64_t(lower_half) << 32));

      const int64_t div_2parallel =
          ((((div_2parellel_old >> 2) & mask_16b) * inv25_16b) >> 16) &
          mask_16b;

      const int64_t modulo_2parallel = div_2parellel_old - 100 * div_2parallel;

      const int64_t div_4parellel_old =
          div_2parallel | (modulo_2parallel << 16);

      const int64_t div_4parellel =
          ((((div_4parellel_old >> 1) & mask_8b) * inv5_8b) >> 8) & mask_8b;

      const int64_t modulo_4parallel = div_4parellel_old - 10 * div_4parellel;
      return div_4parellel | (modulo_4parallel << 8);
    }
  }   
  
   static std::tuple<std::array<int64_t,3>,
      size_t, size_t, size_t>
  dec_from_uint_impl_semi_parallel_impl_(const int64_t number_) noexcept {
    constexpr int64_t zero_8parallel_ascii = 0x3030303030303030;
    constexpr int64_t lookup_full = 1000;
    constexpr int64_t parallel_half = 10000;
    constexpr int64_t parallel_full = parallel_half * parallel_half;
    constexpr int64_t count_max =3;
  std::array<int64_t,3> str_int_buf;
    int64_t number_0_ = number_;
    for(size_t i{},iteration_count_backwards{count_max};i<count_max;i++){
    
      iteration_count_backwards--;
      int32_t upper_half{};
      int32_t lower_half{};

      int64_t u64ch_{};
   
    
    
    if(number_0_ < parallel_half){
        lower_half = int32_t(number_0_);
        upper_half = 0;
       number_0_ = 0;
    }else{       
 
        int32_t number_less_than_pow10_8;
        if (number_0_ < parallel_full) {
          number_less_than_pow10_8 = int32_t(number_0_);
          number_0_ = 0;
        } else {
          number_less_than_pow10_8 = int32_t(number_0_ % parallel_full);
          number_0_ = number_0_ / parallel_full;
        }
        
        lower_half = int32_t(number_less_than_pow10_8 % parallel_half);
        upper_half = int32_t(number_less_than_pow10_8 / parallel_half);
        }
        u64ch_ = dec_from_uint_backwards_parallel_less_than_pow10_8_pair_impl_(lower_half, upper_half);
      
      const int64_t u64ch = u64ch_;
      int64_t u64ch_ascii = u64ch | zero_8parallel_ascii;
      str_int_buf[count_max - 1 - iteration_count_backwards] =
          u64ch_ascii;
      if (number_0_) continue;
      const int64_t num_high_0ch =
          size_t((  std::endian::big == std::endian::native? std::countl_zero(int64_t(u64ch))
                                       : std::countr_zero(int64_t(u64ch))) >>
                    3);
      const size_t num_0ch{num_high_0ch +
                              size_t(iteration_count_backwards << 3)};
      const size_t num_ch =
          std::max<size_t>(sizeof(str_int_buf) - num_0ch, 1);
      return {str_int_buf, num_ch, iteration_count_backwards,
              std::min<size_t>(7, num_high_0ch)};
    }

    return {};
  }
  
  static std::string  dec_from_uint_impl_semi_parallel(int64_t number_0_) noexcept {
    const auto [str_int_buf, num_ch, iteration_count_backwards, offset] =
        dec_from_uint_impl_semi_parallel_impl_(number_0_);
    std::string s{};
        s.resize_and_overwrite(num_ch, [&str_int_buf,offset](char* buf, std::size_t num_ch_) noexcept
    {
  std::memcpy(buf, reinterpret_cast<const char *>(&str_int_buf) + offset,
           num_ch_); 
        return num_ch_;
    });
    return s;
  }

static void rand_base(benchmark::State& state) { 
  int64_t i{};
  for (auto _ : state) { 

    benchmark::DoNotOptimize(i++);//|(int64_t(std::rand())<<32) );
  }
}
// Register the function as a benchmark
BENCHMARK(rand_base);

static void mjz_to_string(benchmark::State& state) { 
   
  int64_t i{};
  for (auto _ : state) { 
const  int64_t val=i++;
  std::string str=dec_from_uint_impl_semi_parallel(val);
    benchmark::DoNotOptimize(str);
  }
}
// Register the function as a benchmark
BENCHMARK(mjz_to_string);

static void std_to_string(benchmark::State& state) {  

  int64_t i{};
  for (auto _ : state) {
const  int64_t val=i++;
  std::string str=std::to_string(val);
    benchmark::DoNotOptimize(str);
  }
}
BENCHMARK(std_to_string);




```



more comparasions:
https://quick-bench.com/q/esuJAHxU3f35_fcDBY0dq5ILDD0



```

#include <bit>
#include<array> 
#include <cstring>
#include<string> 

static constexpr char radix_100_table[] = {
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4',
    '0', '5', '0', '6', '0', '7', '0', '8', '0', '9',
    '1', '0', '1', '1', '1', '2', '1', '3', '1', '4',
    '1', '5', '1', '6', '1', '7', '1', '8', '1', '9',
    '2', '0', '2', '1', '2', '2', '2', '3', '2', '4',
    '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
    '3', '0', '3', '1', '3', '2', '3', '3', '3', '4',
    '3', '5', '3', '6', '3', '7', '3', '8', '3', '9',
    '4', '0', '4', '1', '4', '2', '4', '3', '4', '4',
    '4', '5', '4', '6', '4', '7', '4', '8', '4', '9',
    '5', '0', '5', '1', '5', '2', '5', '3', '5', '4',
    '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
    '6', '0', '6', '1', '6', '2', '6', '3', '6', '4',
    '6', '5', '6', '6', '6', '7', '6', '8', '6', '9',
    '7', '0', '7', '1', '7', '2', '7', '3', '7', '4',
    '7', '5', '7', '6', '7', '7', '7', '8', '7', '9',
    '8', '0', '8', '1', '8', '2', '8', '3', '8', '4',
    '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
    '9', '0', '9', '1', '9', '2', '9', '3', '9', '4',
    '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'
};

static char* jeaiii(std::uint32_t n, char* buffer) {
  std::uint64_t prod;

  auto get_next_two_digits = [&]() {
    prod = std::uint32_t(prod) * std::uint64_t(100);
    return int(prod >> 32);
  };
  auto print_1 = [&](int digit) {
    buffer[0] = char(digit + '0');
    buffer += 1;
  };
  auto print_2 = [&] (int two_digits) {
    std::memcpy(buffer, radix_100_table + two_digits * 2, 2);
    buffer += 2;
  };
  auto print = [&](std::uint64_t magic_number, int extra_shift, auto remaining_count) {
    prod = n * magic_number;
    prod >>= extra_shift;
    auto two_digits = int(prod >> 32);

    if (two_digits < 10) {
      print_1(two_digits);
      for (int i = 0; i < remaining_count; ++i) {
        print_2(get_next_two_digits());
      }
    }
    else {
      print_2(two_digits);
      for (int i = 0; i < remaining_count; ++i) {
        print_2(get_next_two_digits());
      }
    }
  };

  if (n < 100) {
    if (n < 10) {
      // 1 digit.
      print_1(n);
    }
    else {
      // 2 digit.
      print_2(n);
    }
  }
  else {
    if (n < 100'0000) {
      if (n < 1'0000) {
        // 3 or 4 digits.
        // 42949673 = ceil(2^32 / 10^2)
        print(42949673, 0, std::integral_constant<int, 1>{});
      }
      else {
        // 5 or 6 digits.
        // 429497 = ceil(2^32 / 10^4)
        print(429497, 0, std::integral_constant<int, 2>{});
      }
    }
    else {
      //if (n < 1'0000'0000) {
        // 7 or 8 digits.
        // 281474978 = ceil(2^48 / 10^6) + 1
        print(281474978, 16, std::integral_constant<int, 3>{});
      //}
      /*else {
        if (n < 10'0000'0000) {
          // 9 digits.
          // 1441151882 = ceil(2^57 / 10^8) + 1
          prod = n * std::uint64_t(1441151882);
          prod >>= 25;
          print_1(int(prod >> 32));
          print_2(get_next_two_digits());
          print_2(get_next_two_digits());
          print_2(get_next_two_digits());
          print_2(get_next_two_digits());
        }
        else {
          // 10 digits.
          // 1441151881 = ceil(2^57 / 10^8)
          prod = n * std::uint64_t(1441151881);
          prod >>= 25;
          print_2(int(prod >> 32));
          print_2(get_next_two_digits());
          print_2(get_next_two_digits());
          print_2(get_next_two_digits());
          print_2(get_next_two_digits());
        }
      }*/
    }
  }
  return buffer;
}

static std::string jeaiii_string(std::uint64_t n) {
  char buffer[8];
  return std::string(buffer, jeaiii(static_cast<std::uint32_t>(n), buffer) - buffer);
}

  static int64_t  dec_from_uint_backwards_parallel_less_than_pow10_8_pair_impl_(
      const int32_t lower_half, const int32_t upper_half) noexcept { 
    constexpr int64_t inv25_16b = 2622;

    constexpr int64_t inv5_8b = 52;

    constexpr int64_t mask_16b =
        int64_t(uint16_t(-1)) | (int64_t(uint16_t(-1)) << 32);
    constexpr int64_t mask_8b =
        int64_t(uint8_t(-1)) | (int64_t(uint8_t(-1)) << 16) |
        (int64_t(uint8_t(-1)) << 32) | (int64_t(uint8_t(-1)) << 48);
    if constexpr (std::endian::big == std::endian::native) {
      const int64_t div_2parellel_old =
          ((int64_t(upper_half) << 32) | int64_t(lower_half));

      const int64_t div_2parallel =
          ((((div_2parellel_old >> 2) & mask_16b) * inv25_16b) >> 16) &
          mask_16b;

      const int64_t modulo_2parallel = div_2parellel_old  - 100 * div_2parallel;

      const int64_t div_4parellel_old =
          modulo_2parallel | (div_2parallel << 16);

      const int64_t div_4parellel =
          ((((div_4parellel_old >> 1) & mask_8b) * inv5_8b) >> 8) & mask_8b;

      const int64_t modulo_4parallel = div_4parellel_old - 10 * div_4parellel;
      return modulo_4parallel | (div_4parellel << 8);
    } else {
      const int64_t div_2parellel_old =
          (int64_t(upper_half) | (int64_t(lower_half) << 32));

      const int64_t div_2parallel =
          ((((div_2parellel_old >> 2) & mask_16b) * inv25_16b) >> 16) &
          mask_16b;

      const int64_t modulo_2parallel = div_2parellel_old -   100 * div_2parallel;

      const int64_t div_4parellel_old =
          div_2parallel | (modulo_2parallel << 16);

      const int64_t div_4parellel =
          ((((div_4parellel_old >> 1) & mask_8b) * inv5_8b) >> 8) & mask_8b;

      const int64_t modulo_4parallel = div_4parellel_old - 10 * div_4parellel;
      return div_4parellel | (modulo_4parallel << 8);
    }
  }   
  
   static std::tuple<std::array<int64_t,3>,
      size_t, size_t, size_t>
  dec_from_uint_impl_semi_parallel_impl_(const uint64_t number_) noexcept {
    constexpr int64_t zero_8parallel_ascii = 0x3030303030303030;
    constexpr uint64_t parallel_half = 10000;
    constexpr int64_t parallel_full = parallel_half * parallel_half;
    constexpr int64_t count_max =3;
  std::array<int64_t,3> str_int_buf;
    uint64_t number_0_ = number_;
    for(size_t i{},iteration_count_backwards{count_max};i<count_max;i++){
    
      iteration_count_backwards--;
      int32_t upper_half{};
      int32_t lower_half{};

      int64_t u64ch_{};
   
    
     
 
        int32_t number_less_than_pow10_8;
        if (number_0_ < parallel_full) {
          number_less_than_pow10_8 = int32_t(number_0_);
          number_0_ = 0;
        } else {
          number_less_than_pow10_8 = int32_t(number_0_ % parallel_full);
          number_0_ = number_0_ / parallel_full;
        } 
        lower_half = number_less_than_pow10_8/10000;
        upper_half =number_less_than_pow10_8%10000;


        u64ch_ = dec_from_uint_backwards_parallel_less_than_pow10_8_pair_impl_(lower_half, upper_half);
      
      const int64_t u64ch = u64ch_;
      int64_t u64ch_ascii = u64ch | zero_8parallel_ascii;
      str_int_buf[count_max - 1 - iteration_count_backwards] =
          u64ch_ascii;
      if (number_0_) continue;
      const int64_t num_high_0ch =
          size_t((  std::endian::big == std::endian::native? std::countl_zero(uint64_t(u64ch))
                                       : std::countr_zero(uint64_t(u64ch))) >>
                    3);
      const size_t num_0ch{num_high_0ch +
                              size_t(iteration_count_backwards << 3)};
      const size_t num_ch =
          std::max<size_t>(sizeof(str_int_buf) - num_0ch, 1);
      return {str_int_buf, num_ch, iteration_count_backwards,
              std::min<size_t>(7, num_high_0ch)};
    }

    return {};
  }
  
  static std::string  dec_from_uint_impl_semi_parallel(int64_t number_0_) noexcept {
    const auto [str_int_buf, num_ch, iteration_count_backwards, offset] =
        dec_from_uint_impl_semi_parallel_impl_(number_0_);
  return std::string(reinterpret_cast<const char *>(&str_int_buf) + offset,num_ch); 
  }static void rand_base(benchmark::State& state) { 
  uint64_t val= 0 ;
  
  for (auto _ : state) { 

    benchmark::DoNotOptimize(val++);//|(uint64_t(std::rand())<<32) );
  }
}
// Register the function as a benchmark
BENCHMARK(rand_base);

static void mjz_to_string(benchmark::State& state) { 
  uint64_t val= 0 ;
  
  for (auto _ : state) {  
  std::string str=dec_from_uint_impl_semi_parallel(val++);
    benchmark::DoNotOptimize(str);
  }
}
// Register the function as a benchmark
BENCHMARK(mjz_to_string);

static void std_to_string(benchmark::State& state) {  

  uint64_t val= 0 ;
  for (auto _ : state) { 
  std::string str=std::to_string(val++);
    benchmark::DoNotOptimize(str);
  }
}
BENCHMARK(std_to_string);




static void jeaiii_to_string(benchmark::State& state) {  
  uint64_t val= 0 ;
  for (auto _ : state) {
  std::string str=jeaiii_string(val++);
    benchmark::DoNotOptimize(str);
  }
}
BENCHMARK(jeaiii_to_string);


```
