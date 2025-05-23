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

so we can now write the code :

```c++


{ 
 const uint32_t number_less_than_pow10_8{/*...*/};
constexpr uint64_t zero_8parallel_ascii = 0x3030303030303030;
const uint32_t upper_half{number_less_than_pow10_8 / 10000};
const uint32_t lower_half{number_less_than_pow10_8 % 10000};
const uint64_t awnser_8parallel =
    [](const uint32_t lower_half,
       const uint32_t upper_half) noexcept -> uint64_t {
  constexpr uint64_t inv25_16b = 2622;

  constexpr uint64_t inv5_8b = 52;

  constexpr uint64_t mask_16b =
      uint64_t(uint16_t(-1)) | (uint64_t(uint16_t(-1)) << 32);
  constexpr uint64_t mask_8b =
      uint64_t(uint8_t(-1)) | (uint64_t(uint8_t(-1)) << 16) |
      (uint64_t(uint8_t(-1)) << 32) | (uint64_t(uint8_t(-1)) << 48);
  if constexpr (std::endian::big==std::endian::native) {
    const uint64_t div_2parellel_old =
        ((uint64_t(upper_half) << 32) | uint64_t(lower_half));

    const uint64_t div_2parallel =
        ((((div_2parellel_old >> 2) & mask_16b) * inv25_16b) >> 16) &
        mask_16b;

    const uint64_t modulo_2parallel = div_2parellel_old - 100 * div_2parallel;

    const uint64_t div_4parellel_old =
        modulo_2parallel | (div_2parallel << 16);

    const uint64_t div_4parellel =
        ((((div_4parellel_old >> 1) & mask_8b) * inv5_8b) >> 8) & mask_8b;

    const uint64_t modulo_4parallel = div_4parellel_old - 10 * div_4parellel;
    return modulo_4parallel | (div_4parellel << 8);
  } else {
    const uint64_t div_2parellel_old =
        (uint64_t(upper_half) | (uint64_t(lower_half) << 32));

    const uint64_t div_2parallel =
        ((((div_2parellel_old >> 2) & mask_16b) * inv25_16b) >> 16) &
        mask_16b;

    const uint64_t modulo_2parallel = div_2parellel_old - 100 * div_2parallel;

    const uint64_t div_4parellel_old =
        div_2parallel | (modulo_2parallel << 16);

    const uint64_t div_4parellel =
        ((((div_4parellel_old >> 1) & mask_8b) * inv5_8b) >> 8) & mask_8b;

    const uint64_t modulo_4parallel = div_4parellel_old - 10 * div_4parellel;
    return div_4parellel | (modulo_4parallel << 8);
  }
}(lower_half, upper_half);
const size_t num_0ch = size_t((std::endian::big == std::endian::native
                                        ? std::countl_zero(awnser_8parallel)
                   : std::countr_zero(awnser_8parallel)) >>
    3);
const uint64_t awnser_8parallel_ascii =
    awnser_8parallel | zero_8parallel_ascii;
/* do bitcasting to char[8]*/
}

```
