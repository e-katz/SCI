#pragma once
#ifndef _FixCodePass_HPP_
#define _FixCodePass_HPP_

#include "../Types.hpp"
#include <llvm/IR/Module.h>

BEGIN_NAMESPACE_SCI

class FixCodePass
{
public:
    FixCodePass();
    ~FixCodePass();

    void run();

private:
    llvm::Function* createIsKindOfFunctionPrototype(llvm::Module *module) const;
    llvm::Function* createIsKindOfMethodFunction(llvm::Module *module) const;

    llvm::IntegerType *m_sizeTy;
};

END_NAMESPACE_SCI

#endif // !_FixCodePass_HPP_