#include <eosio/eosio.hpp>
#include "token.hpp"

class [[epsio::contract]] escrow_contract : public::contract {
    using eosio::contract::contract;

    //holdings table, keeps track of who has how many coins
    struct [[eosio::table]] holdingsTable {
      uint64_t pkey;
      bool flag;
      name accountHolder;
      asset amount;
      uint64_t primary_key(){ const {return pkey};}
    };

    //contracts table, keeps track of current contracts
    struct [[eosio::table]] contractsTable {
      uint64_t pkey;
      bool flag;
      name party1;
      bool party1HasCompletedContract;
      name party2;
      bool party2HasCompletedContract;
      asset amount;
      uint64_t primary_key(){ const {return pkey};}
    };
    //metrics table, keeps track of account metrics
    struct [[eosio::table]] metricsTable {
      uint64_t pkey;
      bool flag;
      name accountHolder;
      int totalContractCount;
      int successfulContractCount;
      int releaseCount;
      uint64_t primary_key(){ const {return pkey};}
    };

    [[eosio::action]]
    void createcontract(name acnt1, name acnt2, asset amt) {
        //check if both are valid accounts
        if (is_account(acnt1) || is_account(acnt2)) {
            require_auth(acnt1);
            require_auth(acnt2);
            //add new contract to table
            table contractsTable(get_self(), get_self().value);
            contractsTable.emplace(get_self(), [&](auto& elem) {
                elem.party1 = acnt1;
                elem.party2 = acnt2;
                elem.party1HasCompletedContract = false;
                elem.party2HasCompletedContract = false;
                elem.amount = amt;
            });
            //add 1 to metricstable for each account
            table metricsTable(get_self, get_self().value);
            const auto& iter = metricsTable.find(acnt1);
            metricsTable.modify(iter, get_self(), [&](auto& elem) {
                elem.totalContractCount++;
            });
            iter = metricsTable.find(acnt2);
            metricsTable.modify(iter, get_self(), [&](auto& elem) {
                elem.totalContractCount++;
            });

        }
    }
    [[eosio::action]]
    void release(name acnt) {
        if (is_account(acnt)) {
            require_auth(acnt);
            //check if any contracts have this account in them by looping through the table
            table contractsTable(get_self(), get_self().value);
            bool foundContract = false;
            for (const auto& elem: contractsTable) {
                //if we find one, notify them and erase from the table
                if (elem.party1 == acnt || elem.party2 == acnt) {
                    foundContract = true;
                    require_recipient(elem.party1);
                    require_recipient(elem.party2);
                    contractsTable.erase(elem);
                }
            }
            //if we did not find an existing contract
            if (!foundContract) {
                table holdingsTable(get_self(), get_self().value);
                const auto& elem = holdingsTable.find(acnt);
                //set value to zero
                elem.amount = 0;
                //remove account from table
                holdingsTable.erase(elem);
            }

        }
    }

    [[eosio::on_notify("student::resolve")]]
    public void resolve() {
        table contractsTable(get_self(), get_self().value);
        const auto& iter = contractsTable.find(student);
        //check if the notifier is party 1 in the contract
        if (iter.party1 == student) {
            iter.party1HasCompletedContract = true;
        }
        //check if the notifier is party 2 in the contract
        else if(iter.party2 == student) {
            iter.party2HasCompletedContract = true;
        }
        //if both parties have completed the contract
        if (iter.party1HasCompletedContract && iter.party2HasCompletedContract) {
            token::transfer(this, iter.party1, iter.amount);
            token::transfer(this, iter.party2, iter.amount);
            name student1 = iter.party1;
            name student2 = iter.party2;
            //update metrics tables to add to successount
            iter = metricsTable.find(student1);
            iter.successfulContractCount ++;
            iter = metricsTable.find(student2);
            iter.successfulContractCount ++;
            require_recipient(elem.party1);
            require_recipient(elem.party2);
        }
    }


};
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