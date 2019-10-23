#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
using namespace eosio;
class [[eosio::contract]] token : public contract {
    public:
        token(name s, name fr, datastream<const char*> ds) :
            recs(s, s.value),
            contract(s, fr, ds) {}
        [[eosio::action]]
        void deposit(name acnt, asset amt);
        [[eosio::action]]
        void transfer(name from, name to, asset amt);
        struct [[eosio::table]] records {
            name account;
            asset amount;
            uint128_t number_of_record_changes;
            uint64_t primary_key()const { return account.value; }
        };
        using records_t = multi_index<"records"_n, records>;
        using deposit_action = action_wrapper<"deposit"_n, &token::deposit>;
        using transfer_action = action_wrapper<"transfer"_n, &token::transfer>;
    private:
        records_t recs;
};