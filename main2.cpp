#include <iostream>
#include <fstream>
#include <map>
#include <bitset>

#include "huff_n_write3.h"
#include "huff_n_write.h"

int main( int argc, char** argv )
{
   using tally_t = int;
   using mysym_t = wchar_t;

   if( !(argc > 1) ) {
      std::cout << "dummy_dec: please specify a file to uncompress\n";
      return 0;
   }
   
   std::ofstream output_file {"arrrrrrrgh.txt"};

   std::basic_ifstream<char> inhuff_kun {argv[1], std::ifstream::binary};

   if( !inhuff_kun.is_open() ) {
      std::cout << "dummy_dec: given file doesn't exist\n";
      return 0;
   }

   huff_n_write<char, char> my_huff {output_file};

   my_huff.huff_spawna_tree( inhuff_kun );

   my_huff.huff_decode( inhuff_kun );
}
