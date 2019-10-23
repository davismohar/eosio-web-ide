
#include "token.hpp"
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
