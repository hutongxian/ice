//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef FILTER_MANAGER_H
#define FILTER_MANAGER_H

//
// It'd be better if we didn't have to include this everywhere, but
// this is the most expeditious approach for now.
//
#include <Glacier2/Instance.h>
#include <Glacier2/FilterT.h>
#include <Ice/ObjectAdapter.h>

namespace Glacier2
{

class FilterManager
{
public:
    FilterManager(std::shared_ptr<Instance>,
                  std::shared_ptr<StringSetI>,
                  std::shared_ptr<StringSetI>,
                  std::shared_ptr<IdentitySetI>);
    virtual ~FilterManager();

    void destroy();

    std::shared_ptr<StringSetI>
    categories() const
    {
        return _categories;
    }

    std::shared_ptr<StringSetI>
    adapterIds() const
    {
        return _adapters;
    }

    std::shared_ptr<IdentitySetI>
    identities() const
    {
        return _identities;
    }

    StringSetPrxPtr
    categoriesPrx() const
    {
        return _categoriesPrx;
    }

    StringSetPrxPtr
    adapterIdsPrx() const
    {
        return _adapterIdsPrx;
    }

    IdentitySetPrxPtr
    identitiesPrx() const
    {
        return _identitiesPrx;
    }

    static std::shared_ptr<FilterManager>
    create(std::shared_ptr<Instance>, const std::string&, bool);

private:

    StringSetPrxPtr _categoriesPrx;
    StringSetPrxPtr _adapterIdsPrx;
    IdentitySetPrxPtr _identitiesPrx;

    const std::shared_ptr<StringSetI> _categories;
    const std::shared_ptr<StringSetI> _adapters;
    const std::shared_ptr<IdentitySetI> _identities;
    const std::shared_ptr<Instance> _instance;
};
}

#endif /* FILTER_MANAGER_H */
