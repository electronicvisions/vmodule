#ifndef CTRLMOD_H
#define CTRLMOD_H

#include <vector>
#include "vmodule.h"
#include <map>

template <typename CONFTYPE, typename... Args >
class ctrlmod {
    public:
        ctrlmod() {};

        virtual void connect(Args... /*args*/) {};

        virtual void reset() {};

        virtual ~ctrlmod() {
            reset();
        }

        virtual void configure(CONFTYPE const&) {};

        int get_constant (std::string const& key) const {
            return *(hw_consts.find(key));
        }

    protected:
        std::map<std::string,int> hw_consts;
};

#endif
