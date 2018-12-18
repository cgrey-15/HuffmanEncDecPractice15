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

   _treey = std::move( nods[0] );
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

   int max_val = std::numeric_limits<plainword_t>::max();
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
      write_n_bits_w_val( 1, 1 );
      write_n_bits_w_val( EOF_ABLE_ALPH_BITSPAN, *node.contents.first );
   } else {
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
   _treey = std::move(nerd);
   return;
}

template< typename Symbl, typename Encoding, typename Tally >
void huff_n_write<Symbl, Encoding, Tally>::huff_get_tree( std::basic_istream<plainword_t>& scanner, nod<optio_tuple_t>& node, bool root_node_started )
{
   std::optional<plainword_t> bit_val = read_n_bits_from( 1, scanner );
   if( !bit_val ) {
      std::cerr << "Bad dra- input...\n";
      return;
   }
   if( *bit_val ) {
      std::optional<plainword_t> val = read_n_bits_from( EOF_ABLE_ALPH_BITSPAN, scanner );
      node.contents.first.emplace(val.value());
      return;
   } else {
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
#endif

      //std::wcout << "'"<<*nodey.contents.first << "' has the encoding: " << output;
      //std::wcout << " with bit-width of " << wid << "\n";
      _tabl[*nodey.contents.first] = {val, wid};
   }
   else {
      if (nodey.l_lnk) huff_encode( *nodey.l_lnk, (val << 1), wid+1 );
      if (nodey.r_lnk) huff_encode( *nodey.r_lnk, ((val << 1) | 1), wid+1 );
   }
}

//#ifdef __HUFF_DEBUG
std::string huff_str( unsigned int val, int wid )
{
   std::string output;
   if( wid < 33 )
      output = std::bitset<32>(val).to_string().substr( 32 - wid );
   else
      output = std::bitset<32>(val).to_string();
   return output;
}
//#endif


template< typename Symbl, typename Encoding, typename Tally >
void huff_n_write<Symbl, Encoding, Tally>::huff_decode( std::basic_istream<plainword_t>& scanner ) {
   int i;
   int j = 1;

   Encoding wordy;
   const nod<optio_tuple_t> *hook = &_treey;

#ifdef __HUFF_DEBUG
   _treey.l_lnk->parent = &_treey;
   _treey.r_lnk->parent = &_treey;
#endif


   std::optional<plainword_t> result;

#if 1
   while( result = read_n_bits_from( 1, scanner ) ) {
      if( !(*result) )
         hook = hook->l_lnk.get();
      else
         hook = hook->r_lnk.get();
      
      if( hook->contents.first ) {
         if( *hook->contents.first == 0b100000000 )
            break;
	 //std::wcout << *hook->contents.first;
	 _printer << *hook->contents.first;
	 hook = &_treey;
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
void huff_n_write<Symbl, Encoding, Tally>::print2strm( const Symbl& let )
{
   int i = 0;
   int j = 0;
   if( _tabl.find( let ) == _tabl.end() ){
      ++i;
      //if( !(j++ % 32 ) )
      //   std::cout << "... "<< i;
      return;
   }
   std::pair <char16_t, bitwidth_t> codesymbol = _tabl[let];
   //write_n_bits_w_val( codesymbol.second, codesymbol.first );
   write_oversized(codesymbol.second, codesymbol.first);

}

template<typename Symbl, typename Encoding, typename Tally>
std::optional<Encoding> huff_n_write<Symbl, Encoding, Tally>::read_n_bits_from( bitwidth_t n, std::basic_istream<plainword_t>& scanner )
{
   //using plainword_t = wchar_t;
   using bitwidth_t = int;

   plainword_t read_val {};
   bitwidth_t padwidth = WORD_LEN - n;
   bitwidth_t unseenwidth = WORD_LEN - _word_pos_rd;

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
}

template<typename Symbl, typename Encoding, typename Tally>
void huff_n_write<Symbl, Encoding, Tally>::write_n_bits_w_val( bitwidth_t n, plainword_t val )
{
   //using plainword_t = wchar_t;
   using bitwidth_t = int;

   int vacnt_width = WORD_LEN - _word_pos_wt;
   int leftover_width = vacnt_width - n;

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
}
template<typename Symbl, typename Encoding, typename Tally>
void huff_n_write<Symbl, Encoding, Tally>::write_oversized(bitwidth_t n, unsigned short val)
{
   char* ptr = reinterpret_cast<char*>(&val);

   if (n < WORD_LEN + 1)
      write_n_bits_w_val(n, ptr[1]);
   else {
      write_n_bits_w_val(n - WORD_LEN, ptr[0]);
      write_n_bits_w_val(WORD_LEN, ptr[1]);
   }
}

