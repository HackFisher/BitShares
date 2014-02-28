#include <bts/address.hpp>
#include <bts/blockchain/transaction.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/io/raw.hpp>

#include <fc/log/logger.hpp>

namespace bts { namespace blockchain {

   fc::sha256 transaction::digest()const
   {
      fc::sha256::encoder enc;
      fc::raw::pack( enc, *this );
      return enc.result();
   }

   std::unordered_set<bts::address> signed_transaction::get_signed_addresses()const
   {
       auto dig = digest(); 
       std::unordered_set<address> r;
       for( auto itr = sigs.begin(); itr != sigs.end(); ++itr )
       {
            r.insert( address(fc::ecc::public_key( *itr, dig )) );
       }
       return r;
   }

   std::unordered_set<bts::pts_address> signed_transaction::get_signed_pts_addresses()const
   {
       auto dig = digest(); 
       std::unordered_set<pts_address> r;
       // add both compressed and uncompressed forms...
       for( auto itr = sigs.begin(); itr != sigs.end(); ++itr )
       {
            auto signed_key_data = fc::ecc::public_key( *itr, dig ).serialize();
            auto signed_key_point = fc::ecc::public_key( *itr, dig ).serialize_ecc_point();

            
            // note: 56 is the version bit of protoshares
            r.insert( pts_address(fc::ecc::public_key( signed_key_data),false,56) );
            r.insert( pts_address(fc::ecc::public_key( signed_key_data ),true,56) );
            // note: 5 comes from en.bitcoin.it/wiki/Vanitygen where version bit is 0
            r.insert( pts_address(fc::ecc::public_key( signed_key_data ),false,0) );
            r.insert( pts_address(fc::ecc::public_key( signed_key_data ),true,0) );
       }
       ilog( "${signed_addr}", ("signed_addr",r) );
       return r;
   }

   uint160 signed_transaction::id()const
   {
      fc::sha512::encoder enc;
      fc::raw::pack( enc, *this );
      return small_hash( enc.result() );
   }

   void    signed_transaction::sign( const fc::ecc::private_key& k )
   {
    try {
      sigs.insert( k.sign_compact( digest() ) );  
     } FC_RETHROW_EXCEPTIONS( warn, "error signing transaction", ("trx", *this ) );
   }

   size_t signed_transaction::size()const
   {
      fc::datastream<size_t> ds;
      fc::raw::pack(ds,*this);
      return ds.tellp();
   }

} }
namespace fc {
   void to_variant( const bts::blockchain::trx_output& var,  variant& vo )
   {
      fc::mutable_variant_object obj;
      obj["amount"] = var.amount; //std::string(bts::blockchain::asset( var.amount, var.unit ));
      obj["claim_func"] = var.claim_func;
      switch( var.claim_func )
      {
         case bts::blockchain::claim_by_signature:
            obj["claim_data"] = fc::raw::unpack<bts::blockchain::claim_by_signature_output>(var.claim_data);
            break;
         case bts::blockchain::claim_by_pts:
            obj["claim_data"] = fc::raw::unpack<bts::blockchain::claim_by_pts_output>(var.claim_data);
            break;
         case bts::blockchain::claim_by_bid:
            obj["claim_data"] = fc::raw::unpack<bts::blockchain::claim_by_bid_output>(var.claim_data);
            break;
         case bts::blockchain::claim_by_long:
            obj["claim_data"] = fc::raw::unpack<bts::blockchain::claim_by_long_output>(var.claim_data);
            break;
         case bts::blockchain::claim_by_cover:
            obj["claim_data"] = fc::raw::unpack<bts::blockchain::claim_by_cover_output>(var.claim_data);
            break;
      };
      vo = std::move(obj);
   }

   void from_variant( const variant& var,  bts::blockchain::trx_output& vo )
   {
      FC_ASSERT( !"TODO: implement from_variant(trx_output)" );
   }
};
