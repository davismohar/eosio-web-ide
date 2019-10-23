#include <eosio/eosio.hpp>
#include "token.hpp"

class [[epsio::eosio::contract]] escrow_contract : public::contract {
    using eosio::contract::contract;
    [[eosio::action]]
    void createcontract(name acnt1, name acnt2, asset amt) {
        is_account(acnt1);
        is_account(acnt2);
    }

}
void token::deposit(name acnt, asset amt) {
    print_f("Account (%) deposited %\n", acnt, amt);
    const auto& iter = recs.find(acnt.value);
    if (iter == recs.end()) {
        recs.emplace(get_self(), [&](auto& elem) {
            elem.account = acnt;
            elem.amount = amt;
            elem.number_of_record_changes = 1;
        });
    } else {
        recs.modify(iter, get_self(), [&](auto& elem) {
            elem.amount += amt;
            elem.number_of_record_changes++;
        });
    }
    require_recipient(acnt);
}
//token defs
void token::transfer(name from, name to, asset amt) {
    print_f("% transferring % to %\n", from, amt, to);
    const auto& to_iter = recs.find(to.value);
    check(to_iter != recs.end(), "to doesn't have an account initialized");
    const auto& from_iter = recs.find(from.value);
    if (from_iter != recs.end()) {
    check(amt <= from_iter->amount, "from doesn't have enough deposited");
        recs.modify(from_iter, get_self(), [&](auto& elem) {
            elem.amount -= amt;
            elem.number_of_record_changes++;
        });
        recs.modify(to_iter, get_self(), [&](auto& elem) {
            elem.amount += amt;
            elem.number_of_record_changes++;
        });
    }
    require_recipient(from);
    require_recipient(to);
}