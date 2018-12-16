#ifndef __HUFF_N_WRITE_H__
#define __HUFF_N_WRITE_H__

#include <iostream>
#include <map>
#include <vector>

template< typename T >
struct nod;

template< typename Symbl, typename Encoding=unsigned char, typename Tally=int >
class huff_n_write
{
using plainbyte = char32_t;
using bitwidth = int;

public :
   huff_n_write( std::basic_ostream<Encoding>& printer ) : _printer{printer}, _tabl{ /*practice_table*/ }, _treey{}, _word_pos_wt{}, _word_pos_rd{}, wordbuf{}, wordbuf_in{} {}
   template<typename C>
   void tally_up( std::basic_istream<C>& scanner );
   void print2strm( const Symbl& letter );
   void flush() { _printer << wordbuf; }
   void huff_decode( std::basic_istream<plainbyte>& scanner );
   void huff_spawna_tree( std::basic_istream<plainbyte>& scanner );
private:
   using optio_tuple_t = std::pair< std::optional<Symbl>, Tally>;


   void huffman( std::vector< nod<optio_tuple_t> >& symbls );
   void huff_encode( const nod<optio_tuple_t>&, unsigned int val, int wid );
   void huff_imprint_tree( const nod<optio_tuple_t>& node );
   void write_n_bits_w_val( bitwidth n, plainbyte val );
   std::optional<plainbyte> read_n_bits_from( bitwidth n, std::basic_istream<plainbyte>& scanner );
   void huff_get_tree( std::basic_istream<plainbyte>& scanner, nod<optio_tuple_t>& node, bool root_node_set );

   std::basic_ostream<Encoding>&                     _printer;
   std::map<Symbl, Tally>                            _count;
   std::map<Symbl, std::pair< plainbyte, bitwidth >> _tabl;

   nod<optio_tuple_t> _treey;

   int _word_pos_wt;
   int _word_pos_rd;
   plainbyte wordbuf;
   plainbyte wordbuf_in;
};

#include "huff_n_write.hpp"

#endif // __HUFF_N_WRITE_H__
