#include<algorithm>
#include<memory>
#include<utility>
#include<limits>
#include "huff_def.h"
#include "huff_n_write.h"

#ifdef __HUFF_DEBUG
enum class DirChild { left, right, root };
#endif

//parameterization just for practice
template< typename T >
struct nod
{
   using nodptr_t = std::unique_ptr<nod>;
   using data_t = T;

   nod( nodptr_t&& lft, nodptr_t&& rght, T&& val ) : l_lnk{lft}, r_lnk{rght}, contents{std::move(val)}{}
   nod( nod&& lft, nod&& rght, T&& val ) : l_lnk{new nod{std::move(lft.contents)}}, r_lnk{new nod{std::move(rght.contents)}}, contents{std::move(val)}
   {
#ifdef __HUFF_DEBUG
      l_lnk->nod_type = DirChild::left;
      r_lnk->nod_type = DirChild::right;
#endif
      l_lnk->l_lnk = std::move(lft.l_lnk);
      l_lnk->r_lnk = std::move(lft.r_lnk);
      r_lnk->l_lnk = std::move(rght.l_lnk);
      r_lnk->r_lnk = std::move(rght.r_lnk);
   }
   nod( T&& val= {{},{}} ) : l_lnk{nullptr}, r_lnk{nullptr}, contents{std::move(val)}{}
   nod( const nodptr_t&, const nodptr_t&, const T& val ) = delete;
   nod( nod<T>&& src ) : l_lnk {std::move(src.l_lnk)}, r_lnk {std::move(src.r_lnk)}, contents{std::move(src.contents)} {}
   nod& operator=( nod&& src ) 
   {
      if( &src == this )
         return *this;
      l_lnk = std::move(src.l_lnk);
      r_lnk = std::move(src.r_lnk);
      contents = std::move( src.contents );
      return *this;
   }

#ifdef __HUFF_DEBUG
   DirChild nod_type {DirChild::root};
   const nod* parent {nullptr};
#endif
   std::unique_ptr<nod>   l_lnk {};
   std::unique_ptr<nod>   r_lnk {};
   T contents;
};

template< typename Symbl, typename Encoding, typename Tally>
template<typename C>
void huff_n_write<Symbl, Encoding, Tally>::tally_up( std::basic_istream<C>& scanner )
{
   //using optio_tuple_t = std::pair< std::optional<Symbl>, Tally>;

   Symbl n;
   while( scanner.get(n) )
      _count[n]++;


   std::vector< nod<optio_tuple_t> > nods {};
   for( auto x = _count.begin(); x != _count.end(); ++x )
      nods.emplace_back( std::move(*x) );

   huffman( nods );

   _treey = std::make_unique<nod<optio_tuple_t>>( std::move( nods[0] ) );
#ifdef __HUFF_DEBUG
   repair_root();
#endif
}

template< typename Symbl, typename Encoding, typename Tally >
void huff_n_write<Symbl, Encoding, Tally>::huffman( std::vector<nod<optio_tuple_t>>& syms )
{
   auto gt_lamb = [](const nod<optio_tuple_t>& a, const nod<optio_tuple_t>& b)->bool
   {
      return a.contents.second > b.contents.second;
   };

   auto gt2_lamb = [](const int val, const nod<optio_tuple_t>& curr) -> bool
   {
      return val > curr.contents.second;
   };

   int max_val = std::numeric_limits<unsigned char>::max();
   std::cout << "Size given is: " << max_val << '\n';
   syms.emplace_back( optio_tuple_t{max_val+1, 1} );

   std::sort( syms.begin(), syms.end(), gt_lamb );

   while( syms.size() > 1 )
   {
      nod<optio_tuple_t> t_2 = std::move(syms.back());
      syms.pop_back();
      nod<optio_tuple_t> t_1 = std::move(syms.back());
      syms.pop_back();

#ifdef __HUFF_DEBUG
      t_2.nod_type = DirChild::right;
      t_1.nod_type = DirChild::left;
#endif

      Tally sum = t_2.contents.second + t_1.contents.second;
      nod<optio_tuple_t> parent { {std::move(t_1)}, {std::move(t_2)}, {{}, sum} };

#ifdef __HUFF_DEBUG
      if( parent.l_lnk ) {
         if( parent.l_lnk->l_lnk )
            parent.l_lnk->l_lnk->parent = parent.l_lnk.get();
	 if( parent.l_lnk->r_lnk )
            parent.l_lnk->r_lnk->parent = parent.l_lnk.get();
      }
      if( parent.r_lnk ) {
         if( parent.r_lnk->l_lnk )
            parent.r_lnk->l_lnk->parent = parent.r_lnk.get();
	 if( parent.r_lnk->r_lnk )
            parent.r_lnk->r_lnk->parent = parent.r_lnk.get();
      }
#endif
      
      auto rightful_spot = std::upper_bound( syms.begin(), syms.end(), sum, gt2_lamb );
      syms.insert( rightful_spot, std::move(parent) );
   }

#ifdef __HUFF_DEBUG
   if( syms[0].l_lnk )
      syms[0].l_lnk->parent = &syms[0];
   if( syms[0].r_lnk )
      syms[0].r_lnk->parent = &syms[0];
#endif

   huff_imprint_tree( syms[0] );
   huff_encode( syms[0], 0, 0 );
   return;
}

template< typename Symbl, typename Encoding, typename Tally >
void huff_n_write<Symbl, Encoding, Tally>::huff_imprint_tree( const nod<optio_tuple_t>& node )
{
   if( node.contents.first ) {
#ifdef __HUFF_DEBUG
      std::cout << "huff_imprint_tree(): ";
#endif
      write_n_bits_w_val( 1, 1 );
#ifdef __HUFF_DEBUG
      std::cout << "huff_imprint_tree(): ";
#endif
      write_n_bits_w_val( EOF_ABLE_ALPH_BITSPAN, *node.contents.first );
   } else {
#ifdef __HUFF_DEBUG
      std::cout << "huff_imprint_tree(): ";
#endif
      write_n_bits_w_val( 1, 0 );
      huff_imprint_tree( *node.l_lnk );
      huff_imprint_tree( *node.r_lnk );
   }
}
template< typename Symbl, typename Encoding, typename Tally >
void huff_n_write< Symbl, Encoding, Tally>::huff_spawna_tree( std::basic_istream<plainword_t>& scanner )
{
   nod<optio_tuple_t> nerd {};
   huff_get_tree( scanner, nerd, true );
   _treey = std::make_unique<nod<optio_tuple_t>>( std::move( nerd ) );
#ifdef __HUFF_DEBUG
   repair_root();
#endif
   return;
}

template< typename Symbl, typename Encoding, typename Tally >
void huff_n_write<Symbl, Encoding, Tally>::huff_get_tree( std::basic_istream<plainword_t>& scanner, nod<optio_tuple_t>& node, bool root_node_started )
{
//#ifdef __HUFF_DEBUG
//   std::cout << "huff_get_tree(): ";
//#endif
   std::optional<char32_t> bit_val = read_n_bits_from( 1, scanner );
   if( !bit_val ) {
      std::cerr << "Bad dra- input...\n";
      return;
   }
   if( *bit_val ) {

#ifdef __HUFF_DEBUG
      //std::cout << "huff_get_tree() [bit_val=1]: ";
      std::cout << static_cast<int>(*bit_val) << '\n';
#endif
      std::optional<char32_t> val = read_n_bits_from( EOF_ABLE_ALPH_BITSPAN, scanner );
      node.contents.first.emplace(val.value());
      return;
   } else {
#ifdef __HUFF_DEBUG
      std::cout << static_cast<int>(*bit_val);//"huff_get_tree()[bit_val=0]: ";
#endif
      node.l_lnk.reset( new nod<optio_tuple_t>{} );
      node.r_lnk.reset( new nod<optio_tuple_t>{} );

      huff_get_tree( scanner, *node.l_lnk, false );
      huff_get_tree( scanner, *node.r_lnk, false );
   }
}

template< typename Symbl, typename Encoding, typename Tally >
void huff_n_write<Symbl, Encoding, Tally>::huff_encode( const nod<optio_tuple_t>& nodey, unsigned int val, int wid) {
   if(nodey.contents.first){
#ifdef __HUFF_DEBUG
      std::wstring output;
      if( wid < 33 )
         output = std::bitset<32>(val).to_string<wchar_t>().substr( 32 - wid );
      else
         output = std::bitset<32>(val).to_string<wchar_t>();

      std::wcout << "'"<< static_cast<wchar_t>(*nodey.contents.first) << "'(" << *nodey.contents.first << ") has the encoding: " << output;
      std::wcout << " with bit-width of " << wid << "\n";
#endif
      _tabl[*nodey.contents.first] = {val, wid};
   }
   else {
      if (nodey.l_lnk) huff_encode( *nodey.l_lnk, (val << 1), wid+1 );
      if (nodey.r_lnk) huff_encode( *nodey.r_lnk, ((val << 1) | 1), wid+1 );
   }
}

std::string huff_str( unsigned int val, int wid )
{
   std::string output;
   if( wid < 33 )
      output = std::bitset<32>(val).to_string().substr( 32 - wid );
   else
      output = std::bitset<32>(val).to_string();
   return output;
}
std::string huff_strLL( unsigned long long val, int wid )
{
   std::string output;
   if( wid < 65 )
      output = std::bitset<64>(val).to_string().substr( 64 - wid );
   else
      output = std::bitset<64>(val).to_string();
   return output;
}


template< typename Symbl, typename Encoding, typename Tally >
void huff_n_write<Symbl, Encoding, Tally>::huff_decode( std::basic_istream<plainword_t>& scanner ) {
   int i;
   int j = 1;

   Encoding wordy;
   const nod<optio_tuple_t> *hook = _treey.get();

#if 0
#ifdef __HUFF_DEBUG
   _treey.l_lnk->parent = &_treey;
   _treey.r_lnk->parent = &_treey;
#endif
#endif


   //std::optional<plainword_t> result;
   std::optional<char32_t> result;

#if 1
   while( /*(std::cout << "huff_decode(): ") &&*/ (result = read_n_bits_from( 1, scanner )) ) {
      if( !(*result) )
         hook = hook->l_lnk.get();
      else
         hook = hook->r_lnk.get();
      
      if( hook->contents.first ) {
         if( *hook->contents.first == plainword_max+1 )
            break;
	 std::cout << static_cast<char>(*hook->contents.first) << '[' << *hook->contents.first << ']';
	 _printer << static_cast<char>(*hook->contents.first);
	 hook = _treey.get();
	 if( !(j++%72) )
            std::wcout << '\n';
      }
   }
#else
   bool last_eof = false;

   while( wordy = scanner.get() ) {
      i = 0;
      while( i < 32 ) {
#ifdef __HUFF_DEBUG
         Encoding mask_bit = (wordy >> (31-i));
#endif
         Encoding is_one = (wordy >> (31-i)) & 1;
	 if( !is_one )
            hook = hook->l_lnk.get();
	 else
            hook = hook->r_lnk.get();

	 if( hook->contents.first ) {
            if( *hook->contents.first == 0b100000000 ) {
               last_eof = true;
               break;
	    }
            std::wcout << *hook->contents.first;
	    hook = &_treey;
	    if( !(j++ % 72) )
               std::wcout << '\n';
	 }
	 ++i;
      }
      if( last_eof )
         break;
   }
#endif
}
   

template< typename Symbl, typename Encoding, typename Tally >
void huff_n_write<Symbl, Encoding, Tally>::print2strm( const wchar_t& let )
{
   int i = 0;
   int j = 0;
   if( _tabl.find( let ) == _tabl.end() ){
      ++i;
      //if( !(j++ % 32 ) )
      //   std::cout << "... "<< i;
      return;
   }
#ifdef __HUFF_DEBUG
   std::cout << "print2strm(): ";
#endif
   std::pair <char16_t, bitwidth_t> codesymbol = _tabl[let];
   write_n_bits_w_val( codesymbol.second, codesymbol.first );
   //write_oversized(codesymbol.second, codesymbol.first);

}

template<typename Symbl, typename Encoding, typename Tally>
std::optional<char32_t> huff_n_write<Symbl, Encoding, Tally>::read_n_bits_from( bitwidth_t n, std::basic_istream<plainword_t>& scanner )
{
   using plainword_t = wchar_t;
   using bitwidth_t = int;

#ifdef __HUFF_DEBUG
   //std::cout << "wordbuf
   //std::cout << "args{n=" << n << "} ";
   std::cout << "_word_pos_rd (old)=" << _word_pos_rd << " ";
#endif

   plainword_t read_val {};
   bitwidth_t padwidth = WORD_LEN - n; // This assumes a fixed specified length
   bitwidth_t unseenwidth = WORD_LEN - _word_pos_rd;

   int end = _word_pos_rd + n;
   int prepped = _char_bitlen - _word_pos_rd;

   if( end < (sizeof(char)*8) ) {

      if( _word_pos_rd == 0 ) {
         wordbuf_in = scanner.get();
         std::cout << "[NEW] [" << huff_str(wordbuf_in, 8) << "] ";
      }
      int right_dlt = prepped - n;
      uint32_t output = ((_char_mask >> _word_pos_rd) & wordbuf_in) >> right_dlt;

      _word_pos_rd = end;

#ifdef __HUFF_DEBUG
      std::cout << "ret=" << output << "  ";
      std::cout << "_word_pos_rd=" << _word_pos_rd << "\n";
#endif

      return {output};
   }
   else {

      int empty_tray = n - prepped;

      //unsigned char val_buf = wordbuf_in << _word_pos_rd;
      unsigned char val_buf = (_char_mask >> _word_pos_rd) & wordbuf_in;
      uint32_t output = (static_cast<uint32_t>(val_buf) << empty_tray);

//      std::cout << '[' << huff_str(wordbuf_in, _char_bitlen) << "] ";

      while( (wordbuf_in = scanner.get()) && empty_tray > _char_bitlen ) {
         std::cout << "[NEW] [" << huff_str(wordbuf_in, 8) << "] ";
//         std::cout << '[' << huff_str(wordbuf_in, _char_bitlen) << "] ";
         output |= (static_cast<uint32_t>(wordbuf_in) << empty_tray);
	 empty_tray -= _char_bitlen;
      }
      std::cout << "[NEW] [" << huff_str(wordbuf_in, 8) << "] ";
//    std::cout << '[' << huff_str(wordbuf_in, _char_bitlen) << "]\n";

      if( empty_tray < 0 )
         std::cerr << "Negative left-shift operand detected. Incorrect procedure.\n";

      unsigned char uwordbuf_in = wordbuf_in;
      output |= static_cast<uint32_t>(uwordbuf_in) >> (_char_bitlen - empty_tray);
      _word_pos_rd = empty_tray;

      if( n == 9 )
         std::cout << "ret=" << output << "[symbol='" << static_cast<char>(output) << "'," << huff_strLL(output, 16) << "]  ";
      else
         std::cout << "ret=" << output << "  ";


#ifdef __HUFF_DEBUG
      std::cout << "_word_pos_rd=" << _word_pos_rd << "\n";
#endif
      return {output};
   }

#if 0
#endif
}

template<typename Symbl, typename Encoding, typename Tally>
void huff_n_write<Symbl, Encoding, Tally>::write_n_bits_w_val( bitwidth_t n, char32_t val )
{
   //using plainword_t = wchar_t;
   using bitwidth_t = int;

#ifdef __HUFF_DEBUG
   //std::cout << "wordbuf
   std::cout << "args{n=" << n << ", val=" << val << "[" << huff_str(val, n) << "]} ";
   std::cout << "_word_pos_wt (old)=" << _word_pos_wt << " ";
#endif

   int vacnt_width = WORD_LEN - _word_pos_wt;
   int leftover_width = vacnt_width - n;

   int peak = _word_pos_wt + n;

   int i_end = (_word_pos_wt+n) % _char_bitlen;
   int padwidth = _char2x_t_bitlen - n;

   unsigned char uwordbuf = wordbuf;
   unsigned char32_t uval = val;

   _char2x_t buf_contents = static_cast<_char2x_t>(uwordbuf) << (_char2x_t_bitlen - _char_bitlen);
#if 0
   _char2x_t output = (val << (_char_bitlen - i_end) % _char_bitlen) | buf_contents;
#else
   _char2x_t output = (static_cast<_char2x_t>(uval) << (_char2x_t_bitlen - peak) ) | buf_contents;
#endif

   int bytes_needed = (peak + (_char_bitlen-1)) / _char_bitlen;

   int i = sizeof(_char2x_t) - 1;//bytes_needed - 1;

   char *ptr = reinterpret_cast<char*>(&output);

   while( i > (sizeof(_char2x_t)-bytes_needed) ) {
      _printer.put(static_cast<char>(ptr[i--]));
      peak -= _char_bitlen;
   }

   wordbuf = ptr[i];

   _word_pos_wt = peak;//i_end;

#ifdef __HUFF_DEBUG
   std::cout << "_word_pos_wt=" << _word_pos_wt << "\n";
#endif

}

#ifdef __HUFF_DEBUG
std::string test_indi( int currpos, int n )
{
   int peak = currpos + n;

   int i_end = (currpos + n) % (sizeof(char)*8);
   int padwidth = (sizeof(long long int)*8) - n;

   unsigned long long int val_mask = std::numeric_limits<unsigned long long>::max() >> padwidth;
   unsigned long long int adj_mask = val_mask << (((sizeof(char)*8) - i_end) % (sizeof(char)*8));

   int pos = currpos;

   int bytes_needed = (peak + ((sizeof(char)*8)-1)) / (sizeof(char)*8);; //pigeon-hole-principle; check!!!
   int i = bytes_needed - 1;

   std::string result {};
   unsigned char *ptr = reinterpret_cast<unsigned char*>(&adj_mask);
   while( i > -1 ) {
      //if( pos < peak )
      //   bytes_needed++;
      result += std::bitset<sizeof(char)*8>(ptr[i--]).to_string() + " ";
      pos += (sizeof(char)*8);
   }
   //result += std::bitset<sizeof(char)*8>(ptr[i++/*--*/]).to_string() + " ";

   result+= "\nBytes needed: " + std::to_string(bytes_needed) + "\n";
   return result;
}
#endif

template<typename Symbl, typename Encoding, typename Tally>
void huff_n_write<Symbl, Encoding, Tally>::write_oversized(bitwidth_t n, unsigned short val)
{
   char* ptr = reinterpret_cast<char*>(&val);

   if (n < WORD_LEN + 1)
      write_n_bits_w_val(n, ptr[0]);
   else {
      write_n_bits_w_val(n - WORD_LEN, ptr[1]);
      write_n_bits_w_val(WORD_LEN, ptr[0]);
   }
}

#ifdef __HUFF_DEBUG
template<typename Symbl, typename Encoding, typename Tally>
void huff_n_write<Symbl, Encoding, Tally>::repair_root()
{
   _treey->l_lnk->parent = _treey.get();
   _treey->r_lnk->parent = _treey.get();
}
#endif

// junk code
#if 0
// from read_n_bits_from()
   if( n > unseenwidth ) {
      bitwidth_t uncaptured_width = n - unseenwidth;
      plainword_t part_val = (wordbuf_in << uncaptured_width) & (ONES_MASK >> padwidth);
      if( !(wordbuf_in = scanner.get()) )
         return std::optional<plainword_t> {};
      plainword_t final_val = part_val | (wordbuf_in >> (WORD_LEN-uncaptured_width));
      _word_pos_rd = uncaptured_width;
      return std::optional<plainword_t> {final_val};
   } else {
      if( _word_pos_rd == 0 ) {
         if( !(wordbuf_in = scanner.get()) )
            return std::optional<plainword_t> {};
      }
      bitwidth_t leftover_width = WORD_LEN-(_word_pos_rd + n);
      plainword_t final_val = (wordbuf_in >> leftover_width) & (ONES_MASK >> padwidth );
      _word_pos_rd += n;
      return std::optional<plainword_t> {final_val};
   }

// from write_n_bits_w_val()
   if( leftover_width > -1 )
   {
      plainword_t adju_symbol = val << static_cast<unsigned int>(leftover_width); 
      wordbuf |= adju_symbol;
      _word_pos_wt += n;//leftover_width;
   }
   else
   {
      unsigned int oppo_dlt = static_cast<unsigned int>( leftover_width*-1);
      plainword_t adju_symbol = val >> oppo_dlt;
      wordbuf |= adju_symbol;

      //if( _printer.bad() )
      //   std::cout << "oh no!\n";
      //_printer << wordbuf;
	  _printer.put(wordbuf);

      unsigned int overflow_dlt = static_cast<unsigned int>( WORD_LEN + leftover_width );

      wordbuf = val << overflow_dlt;
      _word_pos_wt = oppo_dlt;
   }


#endif
