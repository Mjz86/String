# converting any 8 digit binary number to ascii with very few instructions ( min 1mul , max 4mul ):

first , we multiply by 2^56/10^p ,(p being 2*floor(p/2))
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
