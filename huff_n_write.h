#ifndef __HUFF_N_WRITE_H__
#define __HUFF_N_WRITE_H__

#include <iostream>
#include <map>
#include <vector>
#include <optional>
#include <limits>
#include <memory>
#include <cstdint>

template< typename T >
struct nod;

template< typename Symbl, typename Encoding=unsigned char, typename Tally=int >
class huff_n_write
{
using plainword_t = Encoding;
using bitwidth_t = int;
using _charx_t = uint32_t;
using _char2x_t = unsigned long long int;

public :
   huff_n_write( std::basic_ostream<Encoding>& printer ) : _printer{printer}, _tabl{ /*practice_table*/ }, _treey{}, _word_pos_wt{}, _word_pos_rd{}, wordbuf{}, wordbuf_in{}, mutated{} {}
   template<typename C>
   void tally_up( std::basic_istream<C>& scanner );
   void print2strm( const wchar_t& letter );
   void flush() { _printer << wordbuf; }
   void huff_decode( std::basic_istream<plainword_t>& scanner );
   void huff_spawna_tree( std::basic_istream<plainword_t>& scanner );
private:
   using optio_tuple_t = std::pair< std::optional<char16_t>, Tally>;


   void huffman( std::vector< nod<optio_tuple_t> >& symbls );
   void huff_encode( const nod<optio_tuple_t>&, unsigned int val, int wid );
   void huff_imprint_tree( const nod<optio_tuple_t>& node );
   void write_n_bits_w_val( bitwidth_t n, char32_t val );
   void write_oversized(bitwidth_t n, unsigned short val);
   std::optional<char32_t> read_n_bits_from( bitwidth_t n, std::basic_istream<plainword_t>& scanner );
   void huff_get_tree( std::basic_istream<plainword_t>& scanner, nod<optio_tuple_t>& node, bool root_node_set );
#ifdef __HUFF_DEBUG
   void repair_root();
#endif

   std::basic_ostream<Encoding>&                     _printer;
   std::map<Symbl, Tally>                            _count;
   std::map<char16_t, std::pair< /*plainword_t*/char16_t, bitwidth_t >> _tabl;

   std::unique_ptr< nod<optio_tuple_t> > _treey;

   bool mutated;
   int _word_pos_wt;
   int _word_pos_rd;
   plainword_t wordbuf;
   plainword_t wordbuf_in;
   static constexpr plainword_t plainword_max = std::numeric_limits<unsigned char>::max();
   static constexpr unsigned char _char_mask = std::numeric_limits<unsigned char>::max();
   static constexpr int _char_bitlen = sizeof(char)*8;
   static constexpr int _charx_t_bitlen = sizeof(_charx_t)*8;
   static constexpr int _char2x_t_bitlen = sizeof(_char2x_t)*8;
};

#include "huff_n_write.hpp"

#endif // __HUFF_N_WRITE_H__
