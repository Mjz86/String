# converting any 8 digit binary number to ascii with very few instructions (  max 4mul , min of none  ):

first , we multiply by ceil(2^56/10^p) ,(p being 2*floor(logi(x)/2))
this makes sure that if the number was in decimal , it would only be bwtwwn 0 and 99



so we have a decimal between 100
XX.YYYYYYYYYYYYYYYYYYYYYYYYYYY

and we know that XX es information is only stored in the most significant 7bits (56+7=64) and all the YYes are in the low 56 bits



we shift by 57 to get the 7bits in place ,
and get the first 2 digits in a lookup,
 we set the numbers 

 and for the next round we zero out the 7 highs and then  multiply by 100 to get to a similar situation as before 
 ( this can be generalized to more than 4 rounds, but the first multiply is hard to generalize because of big integers)
 
because I dont want to burden you with my lib , i made it only depend on std


 here is the code:

 https://github.com/Mjz86/String/blob/main/mjz_lib/mjz_uintconv.hpp



note: 

in the branchless (always 4 muls , 4 loads )version , we would encoundeter 00.YYYYYYYYYYYYYYYYYYYYYYYYYYY if the number was smaller than the largest chunck power , but its ok , because it generates leading zeroes and not garbage



in the loadless branchless version (https://github.com/Mjz86/String/blob/main/integer_conv_old.md) we have more work to do (5muls).




